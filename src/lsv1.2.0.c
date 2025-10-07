/*
* Programming Assignment 02: lsv1.0.0
* This is the source file of version 1.0.0
* Read the write-up of the assignment to add the features to this base version
* Usage:
*       $ lsv1.0.0 
*       % lsv1.0.0  /home
*       $ lsv1.0.0  /home/kali/   /etc/
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

void do_ls(const char *dir);
void do_long_listing(const char *path);
int get_terminal_width(void);
char **read_dir_to_array(const char *dirpath, int *out_count, int *out_maxlen);
void print_columns_down_then_across(char **names, int n, int maxlen);

int main(int argc, char *argv[]){
    int long_listing = 0;
    int opt;
    while ((opt = getopt(argc, argv, "l")) != -1) {
    	if (opt == 'l') long_listing = 1;
    }

    if (optind == argc) {
    	if (long_listing) do_long_listing(".");
    	else do_ls(".");
    } 
    else {
    	for (int i = optind; i < argc; i++) {
        	printf("Directory listing of %s:\n", argv[i]);
        	if (long_listing) do_long_listing(argv[i]);
        	else do_ls(argv[i]);
        	puts("");
    	}
    }

    return 0;
}

void do_ls(const char *dir) {
    int n, maxlen;
    char **names = read_dir_to_array(dir, &n, &maxlen);
    if (!names) {
        fprintf(stderr, "Cannot open directory : %s\n", dir);
        return;
    }
    print_columns_down_then_across(names, n, maxlen);
    for (int i = 0; i < n; i++) free(names[i]);
    free(names);
}

void do_long_listing(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {

        // 🔹 Skip hidden files (those starting with '.')
        if (entry->d_name[0] == '.')
            continue;

        struct stat st;
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        if (lstat(fullpath, &st) == -1)
            continue;

        // Permissions
        printf((S_ISDIR(st.st_mode)) ? "d" : "-");
        printf((st.st_mode & S_IRUSR) ? "r" : "-");
        printf((st.st_mode & S_IWUSR) ? "w" : "-");
        printf((st.st_mode & S_IXUSR) ? "x" : "-");
        printf((st.st_mode & S_IRGRP) ? "r" : "-");
        printf((st.st_mode & S_IWGRP) ? "w" : "-");
        printf((st.st_mode & S_IXGRP) ? "x" : "-");
        printf((st.st_mode & S_IROTH) ? "r" : "-");
        printf((st.st_mode & S_IWOTH) ? "w" : "-");
        printf((st.st_mode & S_IXOTH) ? "x" : "-");

        // Links, owner, group, size, date, name
        struct passwd *pw = getpwuid(st.st_uid);
        struct group  *gr = getgrgid(st.st_gid);
        char timebuf[32];
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&st.st_mtime));

        printf(" %ld %s %s %5ld %s %s\n",
               (long)st.st_nlink,
               pw ? pw->pw_name : "?",
               gr ? gr->gr_name : "?",
               (long)st.st_size,
               timebuf,
               entry->d_name);
    }
    closedir(dir);
}

// ---------- Feature 3 helpers ----------
int get_terminal_width(void) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
        return ws.ws_col;
    return 80;
}

char **read_dir_to_array(const char *dirpath, int *out_count, int *out_maxlen) {
    DIR *dp = opendir(dirpath);
    if (!dp) return NULL;
    struct dirent *ent;
    int capacity = 64;
    char **names = malloc(capacity * sizeof(char*));
    int count = 0;
    int maxlen = 0;
    while ((ent = readdir(dp)) != NULL) {
        if (ent->d_name[0] == '.') continue;
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

void print_columns_down_then_across(char **names, int n, int maxlen) {
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
            if (idx < n) printf("%-*s", colw, names[idx]);
        }
        printf("\n");
    }
}

