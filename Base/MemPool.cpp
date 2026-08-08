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
        blockSizes_[i] = (1 << MIN_BLOCK);
    }
    memset(blocks_, 0, sizeof(blocks_));
}

DMemPool::~DMemPool() {
    FreeChunkList_();
    GpMemPool = 0;
}

void* DMemPool::AllocChunk_(long blockSize, int blockNum) {
    blockSize = blockSize + sizeof(struct BlockHeader_);
    long chunkSize = sizeof(struct ChunkHeader_) + (blockSize * blockNum);
    unsigned char* c = (unsigned char*)calloc(chunkSize, sizeof(unsigned char));
    if (!c) {
        DDebugLog("DMemPool::AllocChunk_() Fail, size=%ld, num=%d", blockSize, blockNum);
        return 0;
    }

    // init chunk header
    struct ChunkHeader_* ch = (struct ChunkHeader_*)c;
    ch->blockNum = blockNum;
    ch->blockSize = blockSize;
    chunks_.push_front(ch);
    ch->itr = chunks_.begin();

    // init block header
    unsigned char* f = c + sizeof(struct ChunkHeader_);
    unsigned char* p = c + chunkSize;
    unsigned char* n = 0;
    while (p > f) {
        p -= blockSize;
        struct BlockHeader_* bh = (struct BlockHeader_*)p;
        bh->data = 0;
        bh->next = (void*)n;
        bh->size = 0;
        bh->file = 0;
        bh->line = 0;
        n = p;
    }
    return f;
}

void DMemPool::FreeChunk_(struct ChunkHeader_* ch) {
    chunks_.erase(ch->itr);
    ::free(ch);
}

void DMemPool::FreeChunkList_() {
    for (
        std::list<struct ChunkHeader_*>::iterator itr = chunks_.begin();
        itr != chunks_.end();
        ++itr) {
        struct ChunkHeader_* ch = *itr;
        unsigned char* bp = ((unsigned char*)ch) + sizeof(struct ChunkHeader_);
        for (int i = 0; i < ch->blockNum; ++i) {
            struct BlockHeader_* bh = (struct BlockHeader_*)bp;
            if (bh->size != 0) {
                DDebugLog("DMemPool Find Leak, File=\"%s\", Line=%d, Size=%d", bh->file, bh->line, bh->size);
            }
            bp += ch->blockSize;
        }
        FreeChunk_(ch);
    }
}

void* DMemPool::Alloc(long size, char const* file, int line) {
    unsigned char* p = 0;
    if (size > (1 << MAX_BLOCK)) {  // >1K
        p = (unsigned char*)AllocChunk_(size, 1);
        if (p) {
            struct BlockHeader_* bh = (struct BlockHeader_*)p;
            bh->data = p + sizeof(struct BlockHeader_);
            bh->next = (void*)-1L;  // single block
            bh->size = size;
            bh->file = file;
            bh->line = line;
            p += sizeof(struct BlockHeader_);
        }
    } else {
        // find right size chunk
        long i, mask;
        for (i = 0; i < NUM_BLOCK - 1; ++i) {
            mask = ~(blockSizes_[i] - 1);
            if ((size & mask) == 0) {
                break;
            }
        }
        if (0 == blocks_[i]) {
            blocks_[i] = AllocChunk_(blockSizes_[i], CHUNK_SIZE / blockSizes_[i]);
        }
        p = (unsigned char*)blocks_[i];
        if (p) {
            struct BlockHeader_* bh = (struct BlockHeader_*)p;
            blocks_[i] = bh->next;
            bh->data = p + sizeof(struct BlockHeader_);
            bh->next = (void*)(intptr_t)i;
            bh->size = size;
            bh->file = file;
            bh->line = line;
            p += sizeof(struct BlockHeader_);
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
    struct BlockHeader_* bh = (struct BlockHeader_*)(pc - sizeof(struct BlockHeader_));
    if (bh->data != mem) {
        DDebugLog("DMemPool::Free() Fail, File=%s, Line=%s", file, line);
        return;
    }
    // zero clearing
    memset(pc, 0, bh->size);
    // single block
    if (bh->next == (void*)-1LL) {
        struct ChunkHeader_* ch = (struct ChunkHeader_*)(((unsigned char*)bh) - sizeof(struct ChunkHeader_));
        bh->size = 0;
        FreeChunk_(ch);
    } else {
        long i = (long)(intptr_t)bh->next;
        struct BlockHeader_* next = (struct BlockHeader_*)blocks_[i];
        blocks_[i] = bh;
        bh->next = next;
        bh->size = 0;
    }
}

long DMemPool::size() {
    struct ChunkHeader_* ch;
    struct BlockHeader_* bh;
    unsigned char* bp;
    long size = 0;
    for (
        std::list<struct ChunkHeader_*>::iterator itr = chunks_.begin();
        itr != chunks_.end();
        ++itr) {
        ch = *itr;
        bp = ((unsigned char*)ch) + sizeof(struct ChunkHeader_);
        for (int i = 0; i < ch->blockNum; ++i) {
            bh = (struct BlockHeader_*)bp;
            size += bh->size;
            bp += ch->blockSize;
        }
    }
    return size;
}

void DMemPool::FreeAll() {
    for (
        std::list<struct ChunkHeader_*>::iterator itr = chunks_.begin();
        itr != chunks_.end();
        ++itr) {
        FreeChunk_(*itr);
    }
    memset(blocks_, 0, sizeof(blocks_));
}
