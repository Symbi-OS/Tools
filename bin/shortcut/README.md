# shortcut tool

### <code>sc_lib.c</code>

Contains the shortcutted versions of glibc syscall functions to be interposed on the passed executable's syscalls. Currently the constructor elevates the entire program and the destructor lowers it. The resultant shared object <code> sc_lib.so </code> is compiled with <code> sc_lib.c </code>

Shortcutted Syscalls:
<code>
- write -> __x64_sys_write
- read -> __x64_sys_read
- getppid -> __x64_sys_getppid
- getpid -> __x64_sys_getpid
- mmap -> __x64_sys_mmap
- munmap -> __x64_sys_munmap
</code>

### <code>deep_sc.c</code>
<code> deep_sc.c </code> Contains the necessary supporting functions to enable deep shortcutting on read and write system calls. This is compiled to create <code> libDeep.so </code> . Deep shortcutting is disabled by default. This can be toggled by modifying the <code> DEEP_SC_DEFINE </code> variable in the Tools/bin/Makefile and rebuilding Tools.  

  Deep Shortcutting Disabled:
  DEEP_SC_DEFINE = "-UDEEP_SHORTCUT"
  
  Deep Shortcutting Enabled:
  DEEP_SC_DEFINE = "-DDEEP_SHORTCUT"

### <code>shortcut.sh </code>

  shortcut.sh [args] --- \<app\> [app args]

  Script for launching apps with options for elevation and shortcutting.
  
### Example Usage
  Note: Be sure to mitigate all cores and source the prep_envt.sh script prior to running the shortcut tool
  
  Shallow Shortcuts:
  shortcut.sh -be -s "write->__x64_sys_write" -s "read->__x64_sys_read" --- \<app\>
  
  Deep Shortcuts:
  shortcut.sh -be -s "write->tcp_sendmsg" -s "read->tcp_recvmsg" --- \<app\> 
