#include <stdint.h>  // for size_t

// malloc/free are defined in malloc.c (C linkage).
extern "C" void* malloc(size_t size);
extern "C" void  free(void* ptr);

// Overload global operator new so C++ allocations use our kernel malloc.
void* operator new(size_t size)              { return malloc(size); }
void* operator new[](size_t size)            { return malloc(size); }
void  operator delete(void* ptr)   noexcept  { free(ptr); }
void  operator delete[](void* ptr) noexcept  { free(ptr); }
// Sized-delete overloads (suppress -Wsized-deallocation warning)
void  operator delete(void* ptr,   size_t)   noexcept { free(ptr); }
void  operator delete[](void* ptr, size_t)   noexcept { free(ptr); }

extern "C" int kernel_main(void);
int kernel_main() { return 0; }
