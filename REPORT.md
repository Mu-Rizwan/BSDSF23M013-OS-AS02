Feature 2;
Difference between stat() and lstat()?

stat() follows symbolic links to their targets.

lstat() gets info about the link itself.
→ For ls, use lstat() to correctly identify links.

How to extract file type and permissions from st_mode?

Use bitwise AND (&) with macros like S_IFDIR, S_IRUSR, etc.
Example:
if (S_ISDIR(st.st_mode)) printf("Directory");
if (st.st_mode & S_IRUSR) printf("Owner can read");

Feature 3;
🧩 1. Logic for “Down then Across” Columnar Printing
Explain the general logic for printing items in a "down then across" columnar format. Why is
a simple single loop through the list of filenames insufficient for this task?

- n filenames total

- each column width = max_filename_length + spacing

- terminal width = termw

- number of columns = cols = termw / colw

- number of rows = rows = ceil(n / cols
- Each column contains rows items (i.e., fills downward first).

- The index = c * rows + r formula jumps ahead by rows items to find the next element in that column.

- Once all columns in a row are printed, a newline moves to the next visual row.
- 
🚫 Why a Single Loop is Insufficient
If we use simple loops then we can print the desired output but the output will not be dispalyed with respect to the size of terminal window.
Therefore we use the window size to know the rows and columns of the window in order to print it according to the size of terminal window.

🧩 2. Purpose of the ioctl() System Call;
What is the purpose of the ioctl system call in this context? What would be the limitations of
your program if you only used a fixed-width fallback (e.g., 80 columns) instead of detecting
the terminal size?

ioctl() (Input/Output Control) is a low-level system call that performs device-specific operations.
When used with TIOCGWINSZ, it queries the current size of the terminal window.
🎯 Why It’s Needed Here
Without knowing the terminal width, your program has no way to decide how many filenames can fit on one line, and
where to wrap to the next line. By using ioctl(), you can adapt the output neatly to any window size, avoid text overflowing or cutting off,make your ls behave like the real Linux ls command
