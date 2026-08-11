#include "MemPool.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "Debug.h"

DMemPool* GpMemPool = nullptr;

DMemPool::DMemPool() {
    GpMemPool = this;
    DDebugLog("GpMemPool ready.");
    // init blocks
    for (int i = 0; i < NUM_BLOCK; ++i) {
        blockSizes[i] = (1 << (MIN_BLOCK + i));
    }
    memset(blocks, 0, sizeof(blocks));
}

DMemPool::~DMemPool() {
    FreeChunkList();
    GpMemPool = 0;
}

void* DMemPool::AllocChunk(long blockSize, int blockNum) {
    blockSize = blockSize + sizeof(struct BlockHeader);
    long chunkSize = sizeof(struct ChunkHeader) + (blockSize * blockNum);
    unsigned char* c = (unsigned char*)calloc(chunkSize, sizeof(unsigned char));
    if (!c) {
        DDebugLog("DMemPool::AllocChunk() Fail, size=%ld, num=%d", blockSize, blockNum);
        return 0;
    }

    // init chunk header
    struct ChunkHeader* ch = (struct ChunkHeader*)c;
    ch->blockNum = blockNum;
    ch->blockSize = blockSize;
    chunks.push_front(ch);
    ch->itr = chunks.begin();

    // init block header
    unsigned char* f = c + sizeof(struct ChunkHeader);
    unsigned char* p = c + chunkSize;
    unsigned char* n = 0;
    while (p > f) {
        p -= blockSize;
        struct BlockHeader* bh = (struct BlockHeader*)p;
        bh->data = 0;
        bh->next = (void*)n;
        bh->size = 0;
        bh->file = 0;
        bh->line = 0;
        n = p;
    }
    return f;
}

void DMemPool::FreeChunk(struct ChunkHeader* ch) {
    chunks.erase(ch->itr);
    ::free(ch);
}

void DMemPool::FreeChunkList() {
    while (!chunks.empty()) {
        struct ChunkHeader* ch = chunks.front();
        unsigned char* bp = ((unsigned char*)ch) + sizeof(struct ChunkHeader);
        for (int i = 0; i < ch->blockNum; ++i) {
            struct BlockHeader* bh = (struct BlockHeader*)bp;
            if (bh->size != 0) {
                DDebugLog("DMemPool Find Leak, File=\"%s\", Line=%d, Size=%d", bh->file, bh->line, bh->size);
            }
            bp += ch->blockSize;
        }
        FreeChunk(ch);
    }
}

void* DMemPool::Alloc(long size, char const* file, int line) {
    unsigned char* p = 0;
    if (size > (1 << MAX_BLOCK)) {  // >1K
        p = (unsigned char*)AllocChunk(size, 1);
        if (p) {
            struct BlockHeader* bh = (struct BlockHeader*)p;
            bh->data = p + sizeof(struct BlockHeader);
            bh->next = (void*)-1L;  // single block
            bh->size = size;
            bh->file = file;
            bh->line = line;
            p += sizeof(struct BlockHeader);
        }
    } else {
        // find right size chunk
        long i, mask;
        for (i = 0; i < NUM_BLOCK - 1; ++i) {
            mask = ~(blockSizes[i] - 1);
            if ((size & mask) == 0) {
                break;
            }
        }
        if (0 == blocks[i]) {
            blocks[i] = AllocChunk(blockSizes[i], CHUNK_SIZE / blockSizes[i]);
        }
        p = (unsigned char*)blocks[i];
        if (p) {
            struct BlockHeader* bh = (struct BlockHeader*)p;
            blocks[i] = bh->next;
            bh->data = p + sizeof(struct BlockHeader);
            bh->next = (void*)(intptr_t)i;
            bh->size = size;
            bh->file = file;
            bh->line = line;
            p += sizeof(struct BlockHeader);
        }
    }
    // zero clearing
    memset(p, 0, size);
    return p;
}

void DMemPool::Free(void* mem, char const* file, int line) {
    if (!mem) {
        return;
    }
    // check block
    unsigned char* pc = (unsigned char*)mem;
    struct BlockHeader* bh = (struct BlockHeader*)(pc - sizeof(struct BlockHeader));
    if (bh->data != mem) {
        DDebugLog("DMemPool::Free() Fail, File=%s, Line=%d", file, line);
        return;
    }
    // zero clearing
    memset(pc, 0, bh->size);
    // single block
    if (bh->next == (void*)-1LL) {
        struct ChunkHeader* ch = (struct ChunkHeader*)(((unsigned char*)bh) - sizeof(struct ChunkHeader));
        bh->size = 0;
        FreeChunk(ch);
    } else {
        long i = (long)(intptr_t)bh->next;
        struct BlockHeader* next = (struct BlockHeader*)blocks[i];
        blocks[i] = bh;
        bh->next = next;
        bh->size = 0;
    }
}

long DMemPool::size() {
    struct ChunkHeader* ch;
    struct BlockHeader* bh;
    unsigned char* bp;
    long size = 0;
    for (
        std::list<struct ChunkHeader*>::iterator itr = chunks.begin();
        itr != chunks.end();
        ++itr) {
        ch = *itr;
        bp = ((unsigned char*)ch) + sizeof(struct ChunkHeader);
        for (int i = 0; i < ch->blockNum; ++i) {
            bh = (struct BlockHeader*)bp;
            size += bh->size;
            bp += ch->blockSize;
        }
    }
    return size;
}

void DMemPool::FreeAll() {
    for (
        std::list<struct ChunkHeader*>::iterator itr = chunks.begin();
        itr != chunks.end();
        ++itr) {
        FreeChunk(*itr);
    }
    memset(blocks, 0, sizeof(blocks));
}
