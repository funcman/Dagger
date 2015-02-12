#include "dBuffer.h"
#include <stdlib.h>

dBuffer* GpBuffer = 0;

void dBuffer::Free(void* mem, const char* file, int line) {
    free(mem);
}
