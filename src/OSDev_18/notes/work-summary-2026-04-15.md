# Assignment 4 – Part 2: Programmable Interval Timer

## Overview
In this part of the assignment, work was done on adding a basic driver for the Programmable Interval Timer (PIT) to the `OSDev_18` kernel. The goal was to initialize the PIT, count timer ticks using IRQ0, and use those ticks to support both busy-wait sleeping and interrupt-based sleeping.

The PIT was configured to use channel 0 and generate regular timer interrupts at `1000 Hz`. This gives a simple timing model where one timer tick is approximately equal to one millisecond.

---

## Implemented Features

### PIT Header and Constants
- Added:
  ```c
  include/kernel/pit.h
  ```
- Defined constants for:
  - PIT command port
  - PIT channel 0 port
  - PIT base frequency
  - target frequency
  - calculated divider
  - ticks-per-millisecond conversion

This provides a single place for PIT-related configuration and function declarations.

---

### PIT Initialization
- Implemented:
  ```c
  void PitInitialize(void);
  ```
- Configures the PIT using:
  ```c
  OutPortByte(PIT_CMD_PORT, 0x36);
  ```
- Loads the PIT divisor into channel 0:
  - low byte first
  - high byte second
- Registers the PIT interrupt handler on:
  ```c
  IRQ0
  ```

This allows the kernel to start receiving regular timer interrupts from the PIT.

---

### Tick Counter
- Added a global tick counter:
  ```c
  static volatile uint32_t pit_ticks = 0;
  ```
- Implemented a helper function:
  ```c
  uint32_t GetCurrentTick(void);
  ```

The tick counter is incremented every time IRQ0 is triggered. Since the PIT is configured to `1000 Hz`, this gives a simple way to measure elapsed time in milliseconds.

---

### PIT IRQ Handler
- Implemented:
  ```c
  static void PitIrqHandler(struct Registers* regs);
  ```
- The handler:
  - ignores the register argument
  - increments `pit_ticks`

This keeps the timer interrupt path minimal and lets the rest of the kernel measure time by reading the current tick count.

---

### Busy-Wait Sleep
- Implemented:
  ```c
  void SleepBusy(uint32_t milliseconds);
  ```
- Uses the current tick count to wait until enough time has passed
- Continuously checks the tick counter in a loop

This version works by actively spinning on the CPU, so it is simple but inefficient.

---

### Interrupt-Based Sleep
- Implemented:
  ```c
  void SleepInterrupt(uint32_t ticks);
  ```
- Uses:
  ```c
  __asm__ volatile ("sti; hlt");
  ```
  inside a loop
- Re-checks the current tick count after each wake-up

This version is more efficient than busy waiting because the CPU is halted between interrupts instead of constantly polling in a tight loop.

---

### Kernel Integration
- Added:
  ```c
  #include <kernel/pit.h>
  ```
  to `kernel.c`
- Added:
  ```c
  PitInitialize();
  ```
  during kernel startup
- Added:
  ```c
  SleepTest();
  ```
  during the PIT verification stage. In the final application-oriented kernel flow, this standalone test loop is no longer called from `main()`.

This connects the PIT driver to the boot flow so the timer logic is available after interrupt setup is complete.

---

### Build System Update
- Added:
  ```c
  src/pit.c
  ```
  to `CMakeLists.txt`

This ensures the PIT implementation is compiled and linked into the kernel binary together with the rest of the source files.

---

### Terminal Output Fix
- Added the missing declaration for:
  ```c
  void TerminalWriteHex(uint32_t num);
  ```
  to `terminal.h`

This was necessary because `TerminalWriteHex()` was already implemented in `terminal.c` and used in several source files, but it was not exposed through the header.

---

## Testing & Verification

### Build Issues Resolved
During integration, several issues were found and corrected:

- incorrect inline assembly syntax:
  ```c
  __asm__volatile
  ```
  was corrected to:
  ```c
  __asm__ volatile
  ```
- `printf()` calls were removed from PIT test code and replaced with terminal output functions
- the missing `TerminalWriteHex()` declaration in `terminal.h` was added

These changes allowed the kernel to build again with the PIT source included.

---

### Boot Image Verification
- Verified that rebuilding only:
  ```c
  uiaos-kernel
  ```
  updates `kernel.bin`, but does not update the bootable ISO image
- Confirmed that:
  - the new `kernel.bin` contained the new PIT- and memory-related strings
  - the older `kernel.iso` still contained `Hello, World!`
- Rebuilt the image using:
  ```c
  uiaos-create-image
  ```
  so QEMU would boot the updated kernel

This explained why old output was still appearing even after the source code had changed.

---

### Runtime Result
After rebuilding the bootable image, the kernel successfully booted into the updated code and displayed:
- paging setup output
- heap and allocation debug output
- the `memory1`, `memory2`, `memory3`, and `memory4` address prints

This confirms that the updated kernel binary is now being booted correctly.

The PIT sleep test loop was also verified at runtime. The kernel repeatedly prints the expected sleep messages for both:
- busy-wait sleeping
- interrupt-based sleeping

This shows that the PIT tick counter is advancing and that both sleep functions return as expected during execution.

During this test, one remaining issue was observed in the terminal output: once the VGA text buffer wraps back to the top of the screen, new text overwrites old text directly. The later menu and game flow reduces this by clearing the screen before redrawing, but the terminal still does not implement proper scrolling.

---

## Conclusion
- Added a PIT driver skeleton to the kernel
- Configured PIT channel 0 for periodic timer interrupts
- Registered an IRQ0 handler and added a global tick counter
- Implemented both busy-wait and interrupt-based sleep functions
- Integrated PIT support into `kernel.c`
- Added `pit.c` to the build system
- Fixed the missing `TerminalWriteHex()` declaration
- Verified that the updated kernel is now booting from the rebuilt ISO image
- Verified that the PIT sleep test loop runs at runtime

This means the code structure for Part 2 is present in the kernel and has been tested at runtime. The remaining terminal limitation is that continuous logs still wrap instead of scrolling.

---

## Later Integration Note
The final application framework clears the screen before menu and game redraws, so the wraparound problem is less visible during normal use. Proper terminal scrolling would still be a useful future improvement for debug output.
