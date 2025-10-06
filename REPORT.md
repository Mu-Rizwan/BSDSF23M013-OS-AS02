Difference between stat() and lstat()

stat() follows symbolic links to their targets.

lstat() gets info about the link itself.
→ For ls, use lstat() to correctly identify links.

How to extract file type and permissions from st_mode

Use bitwise AND (&) with macros like S_IFDIR, S_IRUSR, etc.
Example:
if (S_ISDIR(st.st_mode)) printf("Directory");
if (st.st_mode & S_IRUSR) printf("Owner can read");
