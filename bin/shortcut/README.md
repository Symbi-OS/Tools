# shortcut tool

### <code>sc_lib.c</code>

Contains the shortcutted versions of glibc syscall functions to be interposed on the passed executable's syscalls. Currently the constructor elevates the entire program and the destructor lowers it. The resultant shared object <code> sc_lib.so </code> is compiled with <code> sc_lib.c </code>

Shortcutted Syscalls:
<code>
- write --> ksys_write
- read --> ksys_read (TODO)
</code>
## TO BE IMPLEMENTED

### <code>shortcut</code>

Executable that is passed a compiled program that the user wants to interpose shortcutted syscalls on to, as well as a series of flags that define which syscalls are to be replaced with their shortcutted versions.
