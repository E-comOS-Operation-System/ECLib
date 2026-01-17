#include "../../include/eclib/start.h"
#include "../../include/eclib/utils.h" 
#include "../../include/eclib/error.h"

// Define user main
extern int main(int argc, char**argv);

// Exit
extern void sys_exit(int exit_code);

// Symbols defined by the linker script (describing the program's memory layout)
extern char __bss_start;    // Start address of the BSS segment (uninitialized global variables)
extern char __bss_end;      // End address of the BSS segment
extern char __stack_start;  // Start address of the stack (specified by the linker script)
extern char __stack_end;    // End address of the stack

// --------------------------
// Program entry point (_start)
// Process: Initialize environment → Call main → Exit
// --------------------------
void _start(void) {
    // 1. Zero out the BSS segment (uninitialized global variables must be set to 0)
    size_t bss_len = &__bss_end - &__bss_start;
    eclib_memset(&__bss_start, 0, bss_len);

    // 2. Initialize the stack (if the microkernel has not set it, configure manually)
    // ARM architecture: Stack pointer (sp) points to the top of the stack (__stack_end)
    // Note: The stack grows downward, with the top being the high address
    register void* stack_top asm("sp") = &__stack_end;
    (void)stack_top; // Prevent compiler optimization

    // 3. Call the user's main function (pass command-line arguments)
    int main_ret = main(eclib_argc, eclib_argv);

    // 4. After main returns, notify the microkernel to exit the program
    sys_exit(main_ret);
}