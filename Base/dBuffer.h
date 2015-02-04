#ifndef __Base__dBuffer__
#define __Base__dBuffer__

#define dFree(m)    GpBuffer->Free((m), __FILE__, __LINE__)

class dBuffer {
public:
    void    Free(void* mem, char const* file, int line);
};

extern dBuffer* GpBuffer;

#endif//__Base__dBuffer__
