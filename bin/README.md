# IDT TOOL

idt_tool uses the elevate mechanism to provide a user with priviliged information about the current address and contents of an IDT, and allows the user to carry out privileged actions on a given IDT. This tool is built on symbiote library functionality that enables a user to read information from the current IDT, make copies of a given IDT, modify entries, and install an IDT from the command line. It also provides functionality for allocating kernel pages and installing custom interposing or replacement handlers for a given entry in the IDT. 

## Known Issues
The interposing_mitigator tool in the recipes dir produces a metadata log of per core handler virtual address locations. It is only updated through use of the interposint_mitigator and should not be considered robust to hand invocations of idt_tool.

## USAGE

idt_tool [OPTIONS]

-a <addr>: specifies address of the IDT to take an action on, if not provided the currently installed IDT is assumed.

-c [-a <addr>]: copies an IDT to a kernel page and returns a pointer to the newly copied IDT. If address of IDT to copy is unspecified the currently installed IDT is assumed.

-g: gets the current value in the IDTR

-h: help

-i [-a <addr>]: given the address of an IDT, writes <addr> to the IDTR to install the specified IDT

-m addr: [-a <addr> -v <vector>]: modifies the IDT entry specified by <vector>. Replaces the handler address for that entry with the address that follows 'addr:'. the -a <addr> option can be massed to modify to specify which IDT to modify, otherwise the currently isntalled IDT is assumed.

-p [-a <addr> -v <vector>]: prints the IDT entry specified by <vector>. Can optionally use -a <addr> to specify which IDT to print from, otherwise current installed IDT is assumed.

-z <mitigation>: copies the specified mitigation handler to a newly allocated kernel page and returns a pointer to the page.
