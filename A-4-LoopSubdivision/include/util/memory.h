#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>

struct mem_trace {
    size_t nbytes;
    char content[0];
};

#if defined(MEMORY_IMPL) && !defined(MEMORY_IMPL_ONCE)
#define MEMORY_IMPL_ONCE
size_t allocated_bytes = 0;
#else
extern size_t allocated_bytes = 0;
#endif

void * operator new(size_t nbytes)
{
    mem_trace *obj = (mem_trace *)malloc(sizeof(mem_trace) + nbytes);
    if (obj != NULL) {
        obj->nbytes = nbytes;
    }
    // std::cout<<"Allocating:"<<nbytes<<std::endl;
    allocated_bytes += nbytes;
    return &(obj->content);
}

void operator delete(void *ptr) _NOEXCEPT
{
    // std::cout<<"Deleting:"<<ptr<<std::endl;
    mem_trace *obj = (mem_trace *)((char *)ptr - (int)offsetof(mem_trace, content));
    allocated_bytes -= obj->nbytes;
    free(obj);
}

static inline std::string readable_size(size_t bytes, size_t precision = 2)
{
    int i = 0;
    const char* units[] = {"Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    double size = bytes;
    while (size > 1024) {
        size /= 1024;
        i++;
    }
    char buf[1024];
    sprintf(buf, "%.*f %s", (int)precision, size, units[i]); /*%.*f*/
    return std::string(buf);
}
