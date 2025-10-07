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

#define COLOR_RESET "\033[0m"
#define COLOR_BLUE  "\033[0;34m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED   "\033[0;31m"
#define COLOR_PINK  "\033[0;35m"
#define ATTR_REVERSE "\033[7m"

extern int errno;

enum display_mode { MODE_DEFAULT, MODE_LONG, MODE_ACROSS };

void do_ls_mode(const char *dir, enum display_mode mode);
void do_long_listing(const char *path);
int get_terminal_width(void);
char **read_dir_to_array(const char *dirpath, int *out_count, int *out_maxlen);
void print_columns_down_then_across(char **names, int n, int maxlen);
void print_columns_across(char **names, int n, int maxlen);
int name_cmp(const void *a, const void *b);
void print_colored_padded(const char *dir, const char *name, int colw);
void do_ls_recursive(const char *path, enum display_mode mode);

/* ---------------- MAIN FUNCTION ---------------- */
int main(int argc, char *argv[])
{
    enum display_mode mode = MODE_DEFAULT;
    int opt;
    int recursive_flag = 0;
    
    while ((opt = getopt(argc, argv, "lxR")) != -1) {
         if (opt == 'l') mode = MODE_LONG;
         else if (opt == 'x') mode = MODE_ACROSS;
         else if (opt == 'R') recursive_flag = 1;
    }
     
    if (optind == argc) {
        if (recursive_flag) do_ls_recursive(".", mode);
        else if (mode == MODE_LONG) do_long_listing(".");
        else do_ls_mode(".", mode);
    } else {
         for (int i = optind; i < argc; i++) {
            if (recursive_flag) do_ls_recursive(argv[i], mode);
            else if (mode == MODE_LONG) do_long_listing(argv[i]);
            else do_ls_mode(argv[i], mode);
            puts("");
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

        // Permissions and file type
        printf((S_ISDIR(st.st_mode)) ? "d" :
               (S_ISLNK(st.st_mode)) ? "l" :
               (S_ISCHR(st.st_mode)) ? "c" :
               (S_ISBLK(st.st_mode)) ? "b" :
               (S_ISFIFO(st.st_mode)) ? "p" :
               (S_ISSOCK(st.st_mode)) ? "s" : "-");

        printf((st.st_mode & S_IRUSR) ? "r" : "-");
        printf((st.st_mode & S_IWUSR) ? "w" : "-");
        printf((st.st_mode & S_IXUSR) ? "x" : "-");
        printf((st.st_mode & S_IRGRP) ? "r" : "-");
        printf((st.st_mode & S_IWGRP) ? "w" : "-");
        printf((st.st_mode & S_IXGRP) ? "x" : "-");
        printf((st.st_mode & S_IROTH) ? "r" : "-");
        printf((st.st_mode & S_IWOTH) ? "w" : "-");
        printf((st.st_mode & S_IXOTH) ? "x" : "-");

        // Owner/group/time
        struct passwd *pw = getpwuid(st.st_uid);
        struct group  *gr = getgrgid(st.st_gid);
        char timebuf[32];
        strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", localtime(&st.st_mtime));

        // File color selection
        const char *color = COLOR_RESET;
        if (S_ISDIR(st.st_mode))
            color = COLOR_BLUE;
        else if (S_ISLNK(st.st_mode))
            color = COLOR_PINK;
        else if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
            color = COLOR_GREEN;
        else if (strstr(names[i], ".tar") || strstr(names[i], ".gz") || strstr(names[i], ".zip"))
            color = COLOR_RED;
        else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode))
            color = ATTR_REVERSE;

        // Print with alignment similar to real `ls -l`
        printf(" %2ld %-8s %-8s %8ld %s %s%s%s\n",
               (long)st.st_nlink,
               pw ? pw->pw_name : "?",
               gr ? gr->gr_name : "?",
               (long)st.st_size,
               timebuf,
               color, names[i], COLOR_RESET);
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

void print_colored_padded(const char *dir, const char *name, int colw) {
    char fullpath[PATH_MAX];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, name);
    struct stat st;
    lstat(fullpath, &st);
    const char *color = NULL;

    if (S_ISLNK(st.st_mode)) color = COLOR_PINK;
    else if (S_ISDIR(st.st_mode)) color = COLOR_BLUE;
    else if (st.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)) color = COLOR_GREEN;
    else if (strstr(name, ".tar") || strstr(name, ".gz") || strstr(name, ".zip")) color = COLOR_RED;
    else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) ||
             S_ISSOCK(st.st_mode) || S_ISFIFO(st.st_mode)) color = ATTR_REVERSE;

    int namelen = strlen(name);
    printf("%s%s%s", color ? color : "", name, COLOR_RESET);
    int pad = colw - namelen;
    if (pad > 0) printf("%*s", pad, "");
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
                print_colored_padded(".", names[idx], colw);   // for down/across display
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
        print_colored_padded(".", names[i], colw);   // for down/across display
        cur += colw;
    }
    if (cur != 0)
        printf("\n");
}

void do_ls_recursive(const char *path, enum display_mode mode) {
    printf("%s:\n", path);
    int n, maxlen;
    char **names = read_dir_to_array(path, &n, &maxlen);
    if (!names) return;
    qsort(names, n, sizeof(char*), name_cmp);
    if (mode == MODE_ACROSS)
        print_columns_across(names, n, maxlen);
    else
        print_columns_down_then_across(names, n, maxlen);

    for (int i = 0; i < n; i++) {
        char fullpath[PATH_MAX];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, names[i]);
        struct stat st;
        if (lstat(fullpath, &st) == 0 && S_ISDIR(st.st_mode)) {
            if (strcmp(names[i], ".") && strcmp(names[i], "..")) {
                printf("\n");
                do_ls_recursive(fullpath, mode);
            }
        }
    }
    for (int i = 0; i < n; i++) free(names[i]);
    free(names);
}

