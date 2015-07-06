#include "MemPool.h"
#include <stdlib.h>
#include <string.h>

MemPool* GpMemPool = 0;

MemPool::MemPool() {
    GpMemPool = this;
    // init blocks
    for (int i=0; i<NUM_BLOCK; ++i) {
        blockSizes_[i] = (1 << MIN_BLOCK);
    }
    memset(blocks_, 0, sizeof(blocks_));
}

MemPool::~MemPool() {
    freeChunkList_();
    GpMemPool = 0;
}

void* MemPool::allocChunk_(long blockSize, int blockNum) {
    blockSize = blockSize + sizeof(struct BlockHeader_);
    long chunkSize = sizeof(struct ChunkHeader_) + (blockSize * blockNum);
    unsigned char* c = (unsigned char*)calloc(chunkSize, sizeof(unsigned char));
    if (!c) {
        // TODO: add log ...
        return 0;
    }

    // init chunk header
    struct ChunkHeader_* ch = (struct ChunkHeader_*)c;
    ch->blockNum   = blockNum;
    ch->blockSize  = blockSize;
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

void MemPool::freeChunk_(struct ChunkHeader_* ch) {
    chunks_.erase(ch->itr);
    ::free(ch);
}

void MemPool::freeChunkList_() {
    for (
        std::list<struct ChunkHeader_*>::iterator itr = chunks_.begin();
        itr != chunks_.end();
        ++itr
    ) {
        struct ChunkHeader_* ch = *itr;
        unsigned char* bp = ((unsigned char*)ch) + sizeof(struct ChunkHeader_);
        for (int i=0; i<ch->blockNum; ++i) {
            struct BlockHeader_* bh = (struct BlockHeader_*)bp;
            if (bh->size != 0) {
                // TODO: add log about memory leak
                if (bh->size <= 100) {
                    // TODO: print data of leak
                }
            }
            bp += ch->blockSize;
        }
        freeChunk_(ch);
    }
}

void* MemPool::alloc(long size, char const* file, int line) {
    unsigned char* p = 0;
    if (size > (1<<MAX_BLOCK)) {        // >1K
        p = (unsigned char*)allocChunk_(size, 1);
        if (p) {
            struct BlockHeader_* bh = (struct BlockHeader_*)p;
            bh->data = p + sizeof(struct BlockHeader_);
            bh->next = (void*)-1L;      // single block
            bh->size = size;
            bh->file = file;
            bh->line = line;
            p += sizeof(struct BlockHeader_);
        }
    }else {
        // find right size chunk
        long i, mask;
        for (i=0; i<NUM_BLOCK-1; ++i) {
            mask = ~ (blockSizes_[i] - 1);
            if ((size & mask) == 0) {
                break;
            }
        }
        if (0 == blocks_[i]) {
            blocks_[i] = allocChunk_(blockSizes_[i], CHUNK_SIZE / blockSizes_[i]);
        }
        p = (unsigned char*)blocks_[i];
        if (p) {
            struct BlockHeader_* bh = (struct BlockHeader_*)p;
            blocks_[i] = bh->next;
            bh->data = p + sizeof(struct BlockHeader_);
            bh->next = (void*)i;
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

void MemPool::free(void* mem, char const* file, int line) {
    if (!mem) {
        return;
    }
    // check block
    unsigned char* pc = (unsigned char*)mem;
    struct BlockHeader_* bh = (struct BlockHeader_*)(pc - sizeof(struct BlockHeader_));
    if (bh->data != mem) {
        // TODO: add log ...
        return;
    }
    // zero clearing
    memset(pc, 0, bh->size);
    // single block
    if (((long)bh->next) == -1L) {
        struct ChunkHeader_* ch = (struct ChunkHeader_*)(((unsigned char*)bh) - sizeof(struct ChunkHeader_));
        bh->size = 0;
        freeChunk_(ch);
    }else {
        long i = (long)bh->next;
        struct BlockHeader_* next = (struct BlockHeader_*)blocks_[i];
        blocks_[i] = bh;
        bh->next = next;
        bh->size = 0;
    }
}

long MemPool::size() {
    struct ChunkHeader_* ch;
    struct BlockHeader_* bh;
    unsigned char* bp;
    long size = 0;
    for (
        std::list<struct ChunkHeader_*>::iterator itr = chunks_.begin();
        itr != chunks_.end();
        ++itr
    ) {
        ch = *itr;
        bp = ((unsigned char*)ch) + sizeof(struct ChunkHeader_);
        for (int i=0; i<ch->blockNum; ++i) {
            bh = (struct BlockHeader_*)bp;
            size += bh->size;
            bp += ch->blockSize;
        }
    }
    return size;
}

void MemPool::freeAll() {
    for (
        std::list<struct ChunkHeader_*>::iterator itr = chunks_.begin();
        itr != chunks_.end();
        ++itr
    ) {
        freeChunk_(*itr);
    }
    memset(blocks_, 0, sizeof(blocks_));
}
