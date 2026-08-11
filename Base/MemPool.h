#ifndef DAGGER_MEMPOOL_H
#define DAGGER_MEMPOOL_H

#include <list>

#include "TypeDef.h"

#define DCAlloc(s) GpMemPool->Alloc((s), __FILE__, __LINE__)
#define DFree(m) GpMemPool->Free((m), __FILE__, __LINE__)

const int MIN_BLOCK = 4;   // min block size is 2^4
const int MAX_BLOCK = 10;  // max block size is 2^10
const int NUM_BLOCK = ((MAX_BLOCK - MIN_BLOCK) + 1);
const int CHUNK_SIZE = 16 * 1024;  // chunk size is 16kb

class DMemPool {
public:
    DMemPool();
    ~DMemPool();

    void* Alloc(long size, char const* file, int line);
    void Free(void* mem, char const* file, int line);
    long size();
    void FreeAll();

public:
    struct ChunkHeader {
        long blockNum;
        long blockSize;
        std::list<struct ChunkHeader*>::iterator itr;
    };

    struct BlockHeader {
        void* data;
        void* next;
        long size;
        char const* file;
        int line;
    };

    std::list<struct ChunkHeader*> chunks;
    void* blocks[NUM_BLOCK];
    long blockSizes[NUM_BLOCK];

    void* AllocChunk(long blockSize, int blockNum);
    void FreeChunk(struct ChunkHeader* ch);
    void FreeChunkList();
};

extern DAGGER_API DMemPool* GpMemPool;

#endif//DAGGER_MEMPOOL_H
