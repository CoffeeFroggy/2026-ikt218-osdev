# Assignment 4: Memory and PIT

## 1. Introduction

This assignment focused on extending the `3_jakub` operating system with two core kernel features: basic memory management and a programmable interval timer (PIT) driver. The goal was to make dynamic allocation possible inside the kernel, enable paging, and add timer-based sleep functions that could be used both as busy waiting and as interrupt-driven waits.

The implementation was carried out using a structured workflow. The provided assignment templates in `include/assignment_files` were reviewed first, then adapted to fit the existing codebase instead of being copied directly. This was important because the templates assumed headers and support code that did not exist in the current kernel project.

## 2. Starting Point

At the start of the task, the live kernel already supported:

- GDT setup
- IDT setup
- ISR and IRQ registration
- keyboard interrupt handling
- a minimal C and C++ kernel entry flow

However, Assignment 4 functionality was not yet integrated into the active build. The relevant template files existed only under `include/assignment_files`, and none of them were compiled by the active kernel target in `CMakeLists.txt`.

The starting codebase also had a very small local libc implementation. Because of that, the assignment templates needed adjustment in several places:

- template includes such as `<libc/system.h>` did not exist in this project
- the current `printf` implementation was minimal
- the live build only included the current `src/3_jakub/src` tree, not the template files

## 3. Planning and Design

The design goal was to keep the solution assignment-sized and stable, while still making a few careful improvements where they added value without much complexity.

The implementation followed these design choices:

- keep the public API in `include/kernel/` to match the assignment structure
- group memory-related logic under `src/memory/`
- implement PIT as a small IRQ0-driven module in `src/pit.c`
- preserve the existing interrupt framework instead of replacing it
- use a simple first-fit heap allocator
- keep paging deliberately simple with identity mapping of the first 8 MB

This kept the solution easy to reason about while still satisfying the assignment requirements.

## 4. Implemented Files

The following new files were added:

- `include/kernel/memory.h`
- `include/kernel/pit.h`
- `src/memory/memutils.c`
- `src/memory/malloc.c`
- `src/memory/memory.c`
- `src/pit.c`

The following existing files were updated:

- `src/kernel.c`
- `src/kernel.cpp`
- `CMakeLists.txt`
- `include/libc/stdio.h`
- `include/libc/string.h`

## 5. Memory Management Implementation

### 5.1 Kernel Memory Initialization

The linker already exposed the symbol `end` in `src/arch/i386/linker.ld`. This symbol marks the end of the kernel image in memory, so it was used as the anchor for the heap start.

In `kernel.c`, the required line

```c
extern uint32_t end;
```

was added below the includes. During boot, the kernel now calls:

```c
init_kernel_memory(&end);
init_paging();
print_memory_layout();
```

This matches the assignment requirement and ensures that the allocator and paging are initialized before the timer is used.

### 5.2 Heap Allocator

The allocator in `src/memory/malloc.c` is a simple first-fit allocator using a small allocation header:

- `status` marks whether a block is in use
- `size` stores the payload size

The allocator supports:

- `malloc(size_t size)`
- `free(void *mem)`
- reuse of freed blocks
- splitting of larger free blocks
- zero-initialization of allocated memory

Some quality improvements were added compared to the raw template:

- the template pointer arithmetic bug on `kernel_end + 0x1000` was corrected so the heap start is computed in bytes, not in `uint32_t` units
- `free()` now safely ignores `NULL`
- adjacent free blocks are merged to reduce fragmentation

### 5.3 Page-Aligned Allocation

The page-aligned allocator is implemented through:

- `pmalloc(size_t size)`
- `pfree(void *mem)`

This allocator uses a small descriptor array for fixed page-sized allocations near the 4 MB boundary. Unlike the template, the implementation supports multi-page requests by searching for a contiguous free run of pages.

### 5.4 Memory Utility Functions

The file `src/memory/memutils.c` provides:

- `memcpy`
- `memset`
- `memset16`

These functions are used by the allocator and paging setup and keep the memory module self-contained.

## 6. Paging Implementation

Paging is implemented in `src/memory/memory.c`.

The design is intentionally simple:

- the page directory is placed at `0x400000`
- page tables begin at `0x401000`
- the first 8 MB are identity-mapped

This means:

- virtual `0x00000000` to `0x003FFFFF` maps to physical `0x00000000` to `0x003FFFFF`
- virtual `0x00400000` to `0x007FFFFF` maps to physical `0x00400000` to `0x007FFFFF`

This layout is enough for the assignment because:

- the kernel is loaded at 1 MB
- the general heap lives below 4 MB
- paging structures live at and above 4 MB

The function `paging_enable()` loads the page directory into `cr3` and sets the paging bit in `cr0`.

## 7. PIT Implementation

The PIT driver is implemented in `src/pit.c`.

The main features are:

- programming PIT channel 0 using the control port `0x43`
- registering an IRQ0 callback through the existing interrupt handler system
- maintaining a global `volatile uint32_t pit_ticks`
- exposing `pit_get_ticks()`
- implementing `sleep_busy(uint32_t milliseconds)`
- implementing `sleep_interrupt(uint32_t milliseconds)`

The timer is configured to `1000 Hz`, which makes one tick correspond to roughly one millisecond. This made the sleep functions straightforward:

- `sleep_busy()` spins in a loop until enough ticks have passed
- `sleep_interrupt()` waits using `sti` and `hlt`, allowing the CPU to sleep until the next interrupt

## 8. Kernel Integration

The kernel startup sequence in `src/kernel.c` was updated so the new subsystems are initialized in a safe order:

1. validate Multiboot2 magic
2. initialize GDT
3. initialize IDT
4. initialize ISRs
5. initialize IRQs
6. initialize kernel memory
7. initialize paging
8. print memory layout
9. register keyboard handler
10. initialize PIT
11. enable interrupts with `sti`

After initialization, the kernel now performs visible Assignment 4 checks:

- three `malloc()` allocations
- one `free()` followed by another `malloc()`
- a busy-wait sleep test
- an interrupt-driven sleep test

The C++ side in `src/kernel.cpp` now provides:

- `operator new`
- `operator new[]`
- `operator delete`
- `operator delete[]`
- sized delete overloads

This satisfies the assignment requirement to allocate memory with the overloaded `new` operator. A small C++ object allocation test was added and prints a computed value to confirm that the object was allocated, written to, and read back correctly.

## 9. Build and Validation

Validation was done in several stages.

### 9.1 Build Validation

The kernel was rebuilt successfully with:

```bash
cmake --build build/3_jakub -j4
```

After that, a bootable image was created successfully with:

```bash
cmake --build build/3_jakub --target uiaos-create-image -j2
```

This confirmed that:

- the new files were correctly added to `CMakeLists.txt`
- the kernel still linked correctly
- the ISO generation flow still worked

### 9.2 Symbol Validation

The final kernel binary was inspected with `i686-elf-nm`. The following required symbols were present in `build/3_jakub/kernel.bin`:

- `init_kernel_memory`
- `init_paging`
- `print_memory_layout`
- `malloc`
- `free`
- `init_pit`
- `sleep_busy`
- `sleep_interrupt`
- `pit_get_ticks`
- `operator new`
- `operator delete`

This confirmed that the assignment functionality was part of the active binary and not just present in unused source files.

### 9.3 QEMU and GDB Validation

QEMU and `gdb-multiarch` were used to verify the runtime path.

The first debugger pass confirmed that execution reached:

- `init_paging()`
- `init_pit()`
- `kernel_main()`

The CPU registers also confirmed that paging was enabled:

- `cr0 = 0x80000011`
- `cr3 = 0x400000`

This showed that:

- the page directory was loaded into `cr3`
- the paging bit in `cr0` was enabled

The second debugger pass confirmed that execution reached:

- the line immediately after `sleep_busy(50)`
- the line immediately after `sleep_interrupt(50)`
- the C++ allocation test line in `kernel_main()`

At that point, `pit_get_ticks()` returned `102`, showing that timer ticks were advancing while the kernel was running.

Together, these checks provided strong evidence that:

- paging initializes correctly
- the PIT is active
- both sleep functions return
- the C++ allocation path is reached successfully

## 10. Results

The Assignment 4 implementation is now integrated into the active `3_jakub` kernel build.

The final result includes:

- working kernel memory initialization
- working `malloc()` and `free()`
- working paging initialization
- working memory layout reporting
- working PIT initialization on IRQ0
- working busy-wait and interrupt-driven sleep functions
- working overloaded C++ `new` and `delete`

The code compiles, links, packages into an ISO, and passes debugger-based runtime validation for the main control flow.

## 11. Known Limitations

The implementation is intentionally simple and assignment-focused. The main limitations are:

- the heap allocator is still a basic first-fit allocator
- there is no advanced protection against invalid frees or heap corruption
- the page-aligned allocator uses a fixed number of slots
- paging is fixed to a simple identity-mapped first 8 MB
- the solution does not yet use a dynamic physical memory map from the bootloader
- the sleep timing is only as accurate as the PIT tick rate and scheduler-free kernel environment allow

These limitations are acceptable for the assignment because the main goal is to demonstrate working memory management and PIT-based sleeping rather than building a production-grade kernel memory subsystem.

## 12. Conclusion

Assignment 4 was completed by integrating a simple kernel memory manager and a PIT-based timer subsystem into the active `3_jakub` codebase. The final solution stayed close to the assignment requirements, while also adapting the provided templates to fit the real project structure and interrupt model already present in the kernel.

The result is a working foundation for future assignments. Memory allocation is now available in both C and C++, paging is enabled during boot, and the kernel can delay execution using either busy waiting or interrupt-driven sleeping.
