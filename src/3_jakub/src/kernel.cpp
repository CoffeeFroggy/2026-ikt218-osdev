#include "kernel/memory.h"
#include "libc/libs.h"

struct allocation_probe_t {
    int left;
    int right;
};

void *operator new(size_t size)
{
    return malloc(size);
}

void *operator new[](size_t size)
{
    return malloc(size);
}

void operator delete(void *ptr) noexcept
{
    free(ptr);
}

void operator delete[](void *ptr) noexcept
{
    free(ptr);
}

void operator delete(void *ptr, size_t) noexcept
{
    free(ptr);
}

void operator delete[](void *ptr, size_t) noexcept
{
    free(ptr);
}

extern "C" int kernel_main(void);

int kernel_main()
{
    allocation_probe_t *probe = new allocation_probe_t();

    if (probe == 0) {
        printf("C++ new failed\n");
        return 1;
    }

    probe->left = 20;
    probe->right = 22;
    printf("C++ new test: %d\n", probe->left + probe->right);
    delete probe;

    return 0;
}
