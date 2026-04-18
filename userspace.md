# Userspace Execution Plan

## Goal

Enable the shell to run in userspace, with ability to execute other userspace binaries on demand. Single userspace process at a time — no scheduling, no multiprocess.

## Current State

- Paging works with identity mapping and demand paging
- Syscalls work via `int 0x80`
- Page fault handler implemented
- Test_syscalls command passes all tests
- Kernel runs at 0x100000 (1MB), identity mapped
- Page directory at 0x104000, page tables at 0x108000
- **User address space infrastructure implemented** ✅
- **IRET to user mode implemented** ✅
- **Syscalls from user mode work** ✅
- **`run_userspace` command works** ✅

## Requirements

1. **Shell in userspace** — the shell (command interpreter) runs as a user process
2. **User address space** at 0x08000000 (128MB region)
3. **User GDT selectors** with DPL=3
4. **ELF loader** to load shell and other binaries
5. **IRET to user mode** to start shell
6. **Execute other binaries from shell** — load and run, no scheduling

## Implementation Steps

### Step 1: User Address Space Setup ✅ DONE

- Added `vmm_map_user_page()` in paging.c to create user page tables with PAGE_USER flag
- Updated page_fault.c to use `vmm_map_user_page` for demand paging
- User pages allocated on demand when accessed

### Step 2: User GDT Entries ✅ DONE

- User code selector (entry 3): base=0, limit=0xFFFFF, access=0xFA, DPL=3
- User data selector (entry 4): base=0, limit=0xFFFFF, access=0xF2, DPL=3
- TSS updated with user SS:ESP (0x23, 0x08040000)
- GDT reload after TSS init

### Step 3: ELF Loader ✅ DONE (basic)

- Created `user_mode.c` with `elf_get_info()` and `elf_load()`
- Parses ELF header, validates magic (0x7F 'E' 'L' 'F'), loads PT_LOAD segments

### Step 4: User Mode Switch ✅ DONE

- Created `user_mode_asm.asm` with `switch_to_user_mode()`
- Added `__attribute__((naked))` to prevent compiler from adding prologue/epilogue
- EFLAGS uses 0x202 (reserved bit 1 set)
- IRET to user mode works

### Step 5: Syscalls from User Mode ✅ DONE

- Tested with SYS_getpid (20), SYS_write (4)
- User program calls `int 0x80` to invoke syscalls
- write() syscall successfully outputs to VGA from user mode

### Step 6: Shell as Userspace Process 🔄 IN PROGRESS

- Need to build shell as userspace ELF and load it on boot
- Or add command to switch from kernel shell to userspace shell

### Step 7: Load External ELF

- Currently using embedded binary
- Next: build proper ELF and load from disk/memory

## Bugs Fixed

1. **GDT user selectors**: base must be 0 (flat model), rely on paging for address translation
2. **EFLAGS**: must set reserved bit 1 (use 0x202, not 0x200)
3. **Naked attribute**: `switch_to_user_mode` must have `__attribute__((naked))` 
4. **User program offsets**: calculate correct offset for embedded data
5. **Little-endian encoding**: immediate values encoded correctly (e.g., 0x08000018 = 0x18, 0x00, 0x00, 0x08)

## Files Created/Modified

- `src/group_42/src/kernel/paging.c` — Added `vmm_map_user_page()`
- `src/group_42/include/kernel/paging.h` — Added `vmm_map_user_page()` declaration
- `src/group_42/src/kernel/page_fault.c` — Updated to use user page mapping
- `src/group_42/src/kernel/user_mode.c` — ELF loader
- `src/group_42/include/kernel/user_mode.h` — Header with naked attribute
- `src/group_42/src/arch/i386/cpu/user_mode_asm.asm` — `switch_to_user_mode()` 
- `src/group_42/src/arch/i386/cpu/gdt.c` — User selectors, GDT reload
- `src/group_42/src/arch/i386/cpu/tss.c` — User SS:ESP in TSS
- `src/group_42/src/kernel/user_program.c` — Test user program
- `src/group_42/src/shell/commands/run_userspace.c` — `run_userspace` command
- `src/group_42/src/shell/shell.c` — Added run_userspace command

## Testing

1. Build kernel: `cd build && ninja`
2. Run QEMU with kernel
3. Run `run_userspace` command
4. User mode switches, prints "Hello from userspace!" via write syscall
5. Hangs in infinite loop (expected)

## Next Steps

1. **Load external ELF** - compile proper ELF and load from file
2. Build shell as ELF linked at 0x08000000
3. Make kernel automatically switch to userspace shell on boot
4. Add `execve` functionality to shell for running other binaries