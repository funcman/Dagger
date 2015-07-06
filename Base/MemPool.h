#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <list>

#define DCalloc(s)  GpMemPool->alloc((s), _FILE_, _LINE_)
#define DFree(m)    GpMemPool->free((m), _FILE_, _LINE_)

const int MIN_BLOCK     = 4;                        // min block size is 2^4
const int MAX_BLOCK     = 10;                       // max block size is 2^10
const int NUM_BLOCK     = ((MAX_BLOCK-MIN_BLOCK)+1);
const int CHUNK_SIZE    = 16 * 1024;                // chunk size is 16kb

class MemPool {
public:
    MemPool();
    ~MemPool();

    void*   alloc(long size, char const* file, int line);
    void    free(void* mem, char const* file, int line);
    long    size();
    void    freeAll();

private:
    struct ChunkHeader_ {
        long        blockNum;
        long        blockSize;
        std::list<struct ChunkHeader_*>::iterator itr;
    };

    struct BlockHeader_ {
        void*       data;
        void*       next;
        long        size;
        char const* file;
        int         line;
    };

    std::list<struct ChunkHeader_*>  chunks_;
    void*                       blocks_[NUM_BLOCK];
    long                        blockSizes_[NUM_BLOCK];

    void*   allocChunk_(long blockSize, int blockNum);
    void    freeChunk_(struct ChunkHeader_* ch);
    void    freeChunkList_();
};

extern MemPool* GpMemPool;

#endif//MEMPOOL_H
