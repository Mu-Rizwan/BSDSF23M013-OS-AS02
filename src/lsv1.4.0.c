/*
* Programming Assignment 02: lsv1.0.0
* Feature 5 — Alphabetical Sorting for All Display Modes
* Student: BSDSF23M039
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/ioctl.h>
#include <limits.h>

extern int errno;

enum display_mode { MODE_DEFAULT, MODE_LONG, MODE_ACROSS };

void do_ls_mode(const char *dir, enum display_mode mode);
void do_long_listing(const char *path);
int get_terminal_width(void);
char **read_dir_to_array(const char *dirpath, int *out_count, int *out_maxlen);
void print_columns_down_then_across(char **names, int n, int maxlen);
void print_columns_across(char **names, int n, int maxlen);
int name_cmp(const void *a, const void *b);

/* ---------------- MAIN FUNCTION ---------------- */
int main(int argc, char *argv[])
{
    enum display_mode mode = MODE_DEFAULT;
    int opt;
    while ((opt = getopt(argc, argv, "lx")) != -1) {
        if (opt == 'l') mode = MODE_LONG;
        else if (opt == 'x') mode = MODE_ACROSS;
    }

    if (optind == argc) {
        if (mode == MODE_LONG) do_long_listing(".");
        else do_ls_mode(".", mode);
    } else {
        for (int i = optind; i < argc; i++) {
            printf("%s:\n", argv[i]);
            if (mode == MODE_LONG) do_long_listing(argv[i]);
            else do_ls_mode(argv[i], mode);
            if (i < argc - 1) puts("");
        }
    }
    return 0;
}

/* ---------------- STRING COMPARATOR ---------------- */
int name_cmp(const void *a, const void *b) {
    const char * const *pa = a;
    const char * const *pb = b;
    return strcasecmp(*pa, *pb);  // ✅ Case-insensitive alphabetical order
}

/* ---------------- GENERIC LISTING (for default and -x) ---------------- */
void do_ls_mode(const char *dir, enum display_mode mode)
{
    int n, maxlen;
    char **names = read_dir_to_array(dir, &n, &maxlen);

    if (!names) {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    // ✅ Sort filenames alphabetically
    qsort(names, n, sizeof(char*), name_cmp);

    if (mode == MODE_ACROSS)
        print_columns_across(names, n, maxlen);
    else
        print_columns_down_then_across(names, n, maxlen);

    for (int i = 0; i < n; i++)
        free(names[i]);
    free(names);
}

/* ---------------- LONG LISTING MODE ---------------- */
void do_long_listing(const char *path)
{
    int n, maxlen;
    char **names = read_dir_to_array(path, &n, &maxlen);

    if (!names) {
        fprintf(stderr, "Cannot open directory: %s\n", path);
        return;
    }

    // ✅ Sort names alphabetically
    qsort(names, n, sizeof(char*), name_cmp);

    for (int i = 0; i < n; i++) {
        char fullpath[PATH_MAX];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, names[i]);

        struct stat st;
        if (lstat(fullpath, &st) == -1)
            continue;

        // File type and permissions
        printf((S_ISDIR(st.st_mode)) ? "d" :
               (S_ISLNK(st.st_mode)) ? "l" : "-");
        printf((st.st_mode & S_IRUSR) ? "r" : "-");
        printf((st.st_mode & S_IWUSR) ? "w" : "-");
        printf((st.st_mode & S_IXUSR) ? "x" : "-");
        printf((st.st_mode & S_IRGRP) ? "r" : "-");
        printf((st.st_mode & S_IWGRP) ? "w" : "-");
        printf((st.st_mode & S_IXGRP) ? "x" : "-");
        printf((st.st_mode & S_IROTH) ? "r" : "-");
        printf((st.st_mode & S_IWOTH) ? "w" : "-");
        printf((st.st_mode & S_IXOTH) ? "x" : "-");

        // Links, owner, group, size, time, name
        struct passwd *pw = getpwuid(st.st_uid);
        struct group  *gr = getgrgid(st.st_gid);
        char timebuf[32];
        strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", localtime(&st.st_mtime));

        printf(" %2ld %-8s %-8s %8ld %s %s\n",
               (long)st.st_nlink,
               pw ? pw->pw_name : "?",
               gr ? gr->gr_name : "?",
               (long)st.st_size,
               timebuf,
               names[i]);
    }

    for (int i = 0; i < n; i++)
        free(names[i]);
    free(names);
}

/* ---------------- SUPPORT FUNCTIONS ---------------- */
int get_terminal_width(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
        return ws.ws_col;
    return 80;
}

char **read_dir_to_array(const char *dirpath, int *out_count, int *out_maxlen)
{
    DIR *dp = opendir(dirpath);
    if (!dp) return NULL;

    struct dirent *ent;
    int capacity = 64;
    char **names = malloc(capacity * sizeof(char*));
    int count = 0;
    int maxlen = 0;

    while ((ent = readdir(dp)) != NULL) {
        // Skip hidden files (like real 'ls' without -a)
        if (ent->d_name[0] == '.')
            continue;

        if (count >= capacity) {
            capacity *= 2;
            names = realloc(names, capacity * sizeof(char*));
        }
        names[count] = strdup(ent->d_name);
        int len = strlen(ent->d_name);
        if (len > maxlen) maxlen = len;
        count++;
    }

    closedir(dp);
    *out_count = count;
    *out_maxlen = maxlen;
    return names;
}

void print_columns_down_then_across(char **names, int n, int maxlen)
{
    if (n == 0) return;
    int termw = get_terminal_width();
    int spacing = 2;
    int colw = maxlen + spacing;
    int cols = termw / colw;
    if (cols < 1) cols = 1;
    int rows = (n + cols - 1) / cols;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int idx = c * rows + r;
            if (idx < n)
                printf("%-*s", colw, names[idx]);
        }
        printf("\n");
    }
}

void print_columns_across(char **names, int n, int maxlen)
{
    if (n == 0) return;
    int termw = get_terminal_width();
    int spacing = 2;
    int colw = maxlen + spacing;
    int cur = 0;
    for (int i = 0; i < n; ++i) {
        if (cur + colw > termw) {
            printf("\n");
            cur = 0;
        }
        printf("%-*s", colw, names[i]);
        cur += colw;
    }
    if (cur != 0)
        printf("\n");
}
