# Copilot Instructions for ECLib

## Overview
ECLib is the foundational C library for E-comOS, designed to provide essential functionality for the operating system. Since E-comOS is not Unix-like, many components have been custom-built. This library is primarily used internally by E-comOS and is not intended for direct user applications.

## Key Components
- **IPC (Inter-Process Communication):**
  - Files: `src/ipc/ipc_message.c`, `include/eclib/ipc_message.h`
  - Handles message passing between processes, including state management and error handling.
  - Example usage: `ipc_do_not_kill_sub()` prevents child processes from being killed when the parent exits.
- **Error Handling:**
  - Files: `include/eclib/error.h`
  - Defines error codes and utilities for consistent error reporting.
- **RUI (Resource User Interface):**
  - Files: `src/ipc/rui/`, `include/eclib/rui_message.h`
  - Manages UI-related tasks, such as removing app icons from the dock.

## Developer Workflows
### Building the Project
- Use the provided `Makefile` to build the library:
  ```bash
  make
  ```
- Ensure all dependencies are installed on E-comOS. If missing, install via:
  ```bash
  epm install eclib
  ```

### Testing
- Currently, no explicit test framework is integrated. Testing is done manually by invoking library functions in a controlled environment.

### Debugging
- Use `gdb` or similar tools to debug issues. Ensure debug symbols are enabled during compilation by modifying the `Makefile` if necessary.

## Project-Specific Conventions
- **Error Codes:**
  - Defined in `error.h`.
  - Always return meaningful error codes for failure scenarios.
- **Memory Management:**
  - Use `malloc` and `free` judiciously. Ensure no memory leaks by freeing allocated resources.
- **Thread Safety:**
  - Mutexes (e.g., `pthread_mutex_t`) are used for synchronizing shared resources.

## Integration Points
- **External Dependencies:**
  - Relies on standard C libraries (e.g., `stdint.h`, `stddef.h`, `pthread.h`).
  - Ensure compatibility with E-comOS-specific tools like `epm`.
- **Cross-Component Communication:**
  - IPC mechanisms facilitate communication between different processes and system components.

## Examples
### Preventing Child Process Termination
```c
#include <ipc_message.h>

int main() {
    ipc_do_not_kill_sub();
    return 0;
}
```

### Removing App Icon from Dock
```c
#include <rui_message.h>

int main() {
    rui_deleted_remove_icon();
    return 0;
}
```

## Notes
- This library is under the GNU LGPL license. Refer to the `LICENSE` file for details.
- Contributions should adhere to the existing coding style and conventions.