Feature 2:
Difference between stat() and lstat()?

stat() follows symbolic links to their targets.

lstat() gets info about the link itself.
→ For ls, use lstat() to correctly identify links.

How to extract file type and permissions from st_mode?

Use bitwise AND (&) with macros like S_IFDIR, S_IRUSR, etc.
Example:
if (S_ISDIR(st.st_mode)) printf("Directory");
if (st.st_mode & S_IRUSR) printf("Owner can read");

Feature 3:
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

Feature 4:

🧩 1. Comparing Implementation Complexity: “Down-Then-Across” vs “Across”
The “down then across” approach is more complex because it requires computing both the number of rows and columns and translating each linear list index into a 2D row–column position. The horizontal “across” layout can be achieved with a simple single loop and minimal pre-calculation

🧩 2. Strategy for Managing Display Modes (-l, -x, and default)

Implemented an enum display_mode and used getopt() to set it. The main() function then dispatches to do_long_listing() or do_ls_mode() based on the flag.

Feature 5:

🧩 1. Why is it necessary to read all directory entries into memory before sorting them?

Functions like readdir() return directory entries sequentially from the filesystem — one entry at a time, in the order stored on disk (which is not alphabetical and can vary depending on filesystem structure).
To sort those entries, the program needs access to the entire list of filenames at once.

⚠️ Potential Drawbacks for Very Large Directories
Drawback	Explanation
High Memory Usage:	Each filename (string) and the pointer array (char**) consume heap memory. For millions of files, this can easily reach hundreds of MBs or even GBs.
Performance Overhead:	Copying every name (via strdup()) and sorting them with qsort() (O(n log n)) can become slow for huge directories.
Possible Memory Fragmentation:	Frequent malloc()/realloc() calls while building the array can fragment heap memory.
Scalability Limits:	On resource-limited systems, reading millions of entries at once can cause the process to run out of memory or swap heavily.

🧩 2. Purpose and Signature of the qsort() Comparison Function
qsort() passes pointers to the elements of your array (char **names), so a and b are pointers to pointers (char **).

You cast them back to their actual type:
const char * const * → pointer to a constant string pointer.You then compare the actual strings (*pa, *pb) using strcmp() or strcasecmp().

🔒 Why const void *?

- The parameters must be void * because qsort() is a generic function that can sort any kind of data (integers, structs, strings, etc.).
- const ensures that the comparison function doesn’t modify the elements being compared — it’s only supposed to look at them.

Feature 6:
🎨 1. How ANSI Escape Codes Work in Linux Terminals
🔹 Concept

ANSI escape codes are special sequences of characters that begin with the escape character \033 (or \x1b), followed by a series of parameters and a command letter that tells the terminal to change color, move the cursor, or apply text effects

⚙️ 2. Checking Executable Permission Bits in st_mode

- Each file in Linux has a mode value (from struct stat) containing permission bits and file type bits.

- These permission bits determine whether a file is readable, writable, or executable by:

- The owner (user)

- The group

- Others (everyone else)

Feature 7:
🧩 1. What is a Base Case in a Recursive Function?
🔹 Definition

- A base case is the condition that stops the recursion — it prevents the function from calling itself infinitely.

- In any recursive algorithm:

- The recursive step divides the problem into smaller subproblems.

- The base case defines when the recursion should stop because the problem is already small enough to handle directly.

- Without a base case, the recursion would never end, eventually causing a stack overflow (your program would crash)

🔹 In the Recursive ls Context

In a recursive version of your ls function (for example, do_ls_recursive()), the function calls itself for each subdirectory.

The base case is reached when:

- The current path is not a directory (e.g., it’s a file, symlink, socket, etc.), or

- The directory is empty or cannot be opened (permissions, errors, etc.)

So the base case that stops recursion forever is:

“If the current path is not a directory or cannot be opened, do not recurse further.

📂 2. Why Construct a Full Path Before Recursing?
🔹 Concept

When you list files inside a directory, readdir() gives you just the entry name
However, these are relative to the directory you’re currently listing — not absolute paths
from inside do_ls("parent_dir"), then the function will try to open a directory named "subdir" relative to your current working directory, not relative to "parent_dir".

If your working directory isn’t "parent_dir", this call will fail or open the wrong directory.
