# Assignment 4 – Part 1: Memory Management

## Overview
In this part of the assignment, a basic memory management system was implemented for a simple OS kernel. The goal was to initialize kernel memory, enable paging, and implement dynamic memory allocation using `malloc()` and `free()`.

---

## Implemented Features

### Kernel Memory Initialization
- Used:
  ```c
  extern uint32_t end;
  ```
  to get the end address of the kernel from the linker.
- Implemented:
  ```c
  InitKernelMemory(&end);
  ```
- Set up:
  - `heap_begin`
  - `heap_end`
  - `pheap_begin` (page-aligned heap)
  - `pheap_end`

---

### Paging
- Implemented:
  ```c
  InitPaging();
  ```
- Set up:
  - Page directory at `0x400000`
  - Page tables starting at `0x404000`
- Identity-mapped:
  - `0x00000000 → 0x00000000`
  - `0x400000 → 0x400000`
- Enabled paging by modifying `cr0` and `cr3`.

---

### Dynamic Memory Allocation

#### malloc()
- Implemented a simple heap allocator.
- Stores metadata using:
  ```c
  typedef struct {
      uint8_t status;
      uint32_t size;
  } alloc_t;
  ```
- Supports:
  - Sequential allocation
  - Reuse of freed blocks
- Zeroes allocated memory using `memset`.

---

#### free()
- Frees memory by marking the block as unused.
- Prevents errors by:
  ```c
  if (!mem) return;
  ```
- Correctly updates `memory_used`.

---

#### Block Reuse
- When a block is freed, it can be reused by future `malloc()` calls.
- Verified by:
  - Freeing a block
  - Allocating a smaller block
  - Observing same address reused

---

### Page-Aligned Allocation (pmalloc)
- Implemented `pmalloc()` and `pfree()`:
  - Allocates memory in 4KB pages
  - Uses a descriptor array (`pheap_desc`)
- Located in upper memory region near `0x400000`

---

### Memory Utility Functions
Implemented:
- `memcpy()`
- `memset()`
- `memset16()`

These replace standard library functions in kernel space.

---

### Debug Output

#### Memory Layout
- Implemented:
  ```c
  PrintMemoryLayout();
  ```
- Displays:
  - Memory used
  - Memory free
  - Heap size
  - Heap start/end
  - Page heap start/end

#### Allocation Logs
- Outputs allocation details:
  ```
  Allocated X bytes from 0x... to 0x...
  Re-allocated X bytes from 0x... to 0x...
  ```

---

### Hex Output for Addresses
- Implemented:
  ```c
  TerminalWriteHex(uint32_t num);
  ```
- Ensures correct formatting of memory addresses (base 16)
- Avoids confusion from decimal output

---

## Testing & Verification

### Allocation Test
```c
void* memory1 = malloc(48261);
void* memory2 = malloc(27261);
void* memory3 = malloc(12617);
```

### Free and Reuse Test
```c
free(memory2);
void* memory4 = malloc(1000);
```

### Result
- `memory4` reused the same address as `memory2`
- Confirms:
  - `free()` works
  - allocator reuses memory correctly

---

## Conclusion
- Successfully implemented a basic kernel memory manager
- Paging is enabled and functioning
- `malloc()` and `free()` work correctly
- Memory reuse is verified
- Debug output is clear and correctly formatted

---

## Next Step
Proceed to **Part 2: PIT and Sleep Functions**
