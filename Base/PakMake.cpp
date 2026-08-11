#include "PakMake.h"

#include <ctype.h>
#include <filesystem>
#include <stdio.h>
#include <string.h>

#include "BinFile.h"
#include "Debug.h"
#include "MemPool.h"
#include "PakCode.h"
#include "PakFile.h"
#include "System.h"
#include "TxtFile.h"

namespace fs = std::filesystem;

namespace {

const DWORD BLOCK_SIZE = 0x10000;
const DWORD ZIP_MAGIC = 0x55AA55AA;

void LowerCase(char* str) {
    for (; *str; ++str) {
        *str = (char)tolower((unsigned char)*str);
    }
}

}

bool DPakMake::Pack(char const* listFile, char const* packFile) {
    DPakFileList* pFileList = nullptr;
    DPakOffsList* pOffsList = nullptr;
    DWORD offset = 0;
    DWORD length = 0;
    DPakCodeInfo info;
    DPakHead header;
    char* pLine = nullptr;
    char* pRootPath = nullptr;
    WORD blockSize[1024];
    char szListFile[DMAX_PATH];
    int nFileNum = 0;
    int nBlock = 0;

    if (!GpPakCode)
        return false;

    DTxtFile textFile;
    DPathSetCurrent((char*)"/");
    if (!textFile.Open(listFile))
        return false;

    nFileNum = textFile.GetLineCount() - 1;
    if (nFileNum == 0)
        return false;

    pFileList = (DPakFileList*)DCAlloc(sizeof(DPakFileList) * nFileNum);
    pOffsList = (DPakOffsList*)DCAlloc(sizeof(DPakOffsList) * nFileNum);

    pRootPath = textFile.FirstLine();

    for (int i = 0; i < nFileNum; i++) {
        pLine = textFile.NextLine();
        LowerCase(pLine);
        pFileList[i].fileName = pLine;
        pFileList[i].fileId = DHashString(pLine);
    }

    // Sort file list by id (bubble sort). Duplicate ids abort to avoid corrupting the archive.
    for (int i = 0; i < nFileNum - 1; i++) {
        for (int j = i + 1; j < nFileNum; j++) {
            if (pFileList[i].fileId > pFileList[j].fileId) {
                DPakFileList tmp = pFileList[i];
                pFileList[i] = pFileList[j];
                pFileList[j] = tmp;
            } else if (pFileList[i].fileId == pFileList[j].fileId) {
                DDebugLog("%s and %s have the same id",
                    pFileList[i].fileName, pFileList[j].fileName);
                DFree(pFileList);
                DFree(pOffsList);
                return false;
            }
        }
    }

    DBinFile pack;
    DPathSetCurrent((char*)"/");
    if (!pack.Create(packFile)) {
        DFree(pFileList);
        DFree(pOffsList);
        return false;
    }

    memset(&header, 0, sizeof(header));
    header.magicCode = 0x12345678;
    header.fileNumber = nFileNum;
    header.fileOffset = 0;
    header.listOffset = 0;
    pack.Write(&header, sizeof(header));

    offset = pack.Tell();

    info.dataBuf_ = (BYTE*)DCAlloc(BLOCK_SIZE);
    info.packBuf_ = (BYTE*)DCAlloc(GpPakCode->GetPackLen(BLOCK_SIZE));

    DPathSetCurrent(pRootPath);
    for (int i = 0; i < nFileNum; i++) {
        DBinFile listFile;
        if (!listFile.Open(pFileList[i].fileName)) {
            DDebugLog("Can't open file...%s", pFileList[i].fileName);
            DFree(pFileList);
            DFree(pOffsList);
            DFree(info.dataBuf_);
            DFree(info.packBuf_);
            return false;
        }
        DDebugLog("packing file %s...", pFileList[i].fileName);

        length = listFile.Size();
        pOffsList[i].fileId = pFileList[i].fileId;
        pOffsList[i].fileOffset = offset;
        pOffsList[i].fileLength = length;
        pOffsList[i].fileBlocks = (length + 0xffff) >> 16;

        pack.Seek(offset, DFILE_BEGIN);

        nBlock = pOffsList[i].fileBlocks;
        if (nBlock >= 1024) {
            DDebugLog("%s...Is too big file to pack", pFileList[i].fileName);
            break;
        }
        pack.Write(blockSize, sizeof(WORD) * nBlock);

        int j = 0;
        for (; length >= BLOCK_SIZE; length -= BLOCK_SIZE, j++) {
            listFile.Read(info.dataBuf_, BLOCK_SIZE);
            info.dataLen_ = BLOCK_SIZE;
            info.packLen_ = GpPakCode->GetPackLen(BLOCK_SIZE);
            GpPakCode->Encode(&info);
            if (info.packLen_ >= BLOCK_SIZE) {
                blockSize[j] = 0;
                pack.Write(info.dataBuf_, BLOCK_SIZE);
            } else {
                blockSize[j] = (WORD)info.packLen_;
                pack.Write(info.packBuf_, info.packLen_);
            }
        }

        // Last block: if compression produced output >= 64K, store uncompressed.
        if (length > 0) {
            listFile.Read(info.dataBuf_, length);
            info.dataLen_ = length;
            info.packLen_ = GpPakCode->GetPackLen(length);
            GpPakCode->Encode(&info);
            if (info.packLen_ >= BLOCK_SIZE) {
                blockSize[j] = 0;
                pack.Write(info.dataBuf_, BLOCK_SIZE);
            } else {
                blockSize[j] = (WORD)info.packLen_;
                pack.Write(info.packBuf_, info.packLen_);
            }
        }

        offset = pack.Tell();
        pack.Seek(pOffsList[i].fileOffset, DFILE_BEGIN);
        pack.Write(blockSize, sizeof(WORD) * nBlock);
        listFile.Close();
    }

    DFree(info.dataBuf_);
    DFree(info.packBuf_);

    info.dataLen_ = nFileNum * sizeof(DPakOffsList);
    info.dataBuf_ = (BYTE*)pOffsList;
    info.packLen_ = GpPakCode->GetPackLen(info.dataLen_);
    info.packBuf_ = (BYTE*)DCAlloc(info.packLen_);
    GpPakCode->Encode(&info);

    pack.Seek(0, DFILE_END);
    header.fileOffset = pack.Tell();
    DWORD magic = ZIP_MAGIC;
    pack.Write(&magic, sizeof(DWORD));
    pack.Write(&info.packLen_, sizeof(DWORD));
    pack.Write(&info.dataLen_, sizeof(DWORD));
    pack.Write(info.packBuf_, info.packLen_);
    header.listOffset = pack.Tell();

    DFree(pFileList);
    DFree(pOffsList);
    DFree(info.packBuf_);

    DStrLCopy(szListFile, listFile, DMAX_PATH - 1);
    DPathChangeExt(szListFile, ".dir");
    DPathSetCurrent((char*)"/");
    DBinFile dirFile;
    if (!dirFile.Open(szListFile))
        return false;
    info.dataLen_ = dirFile.Size();
    info.dataBuf_ = (BYTE*)DCAlloc(info.dataLen_);
    info.packLen_ = GpPakCode->GetPackLen(info.dataLen_);
    info.packBuf_ = (BYTE*)DCAlloc(info.packLen_);
    dirFile.Read(info.dataBuf_, info.dataLen_);
    dirFile.Close();
    GpPakCode->Encode(&info);
    pack.Seek(0, DFILE_END);
    pack.Write(&magic, sizeof(DWORD));
    pack.Write(&info.packLen_, sizeof(DWORD));
    pack.Write(&info.dataLen_, sizeof(DWORD));
    pack.Write(info.packBuf_, info.packLen_);
    DFree(info.dataBuf_);
    DFree(info.packBuf_);

    DPathChangeExt(szListFile, ".txt");
    DPathSetCurrent((char*)"/");
    DBinFile txtFile;
    if (!txtFile.Open(szListFile))
        return false;
    info.dataLen_ = txtFile.Size();
    info.dataBuf_ = (BYTE*)DCAlloc(info.dataLen_);
    info.packLen_ = GpPakCode->GetPackLen(info.dataLen_);
    info.packBuf_ = (BYTE*)DCAlloc(info.packLen_);
    txtFile.Read(info.dataBuf_, info.dataLen_);
    txtFile.Close();
    GpPakCode->Encode(&info);
    pack.Seek(0, DFILE_END);
    pack.Write(&magic, sizeof(DWORD));
    pack.Write(&info.packLen_, sizeof(DWORD));
    pack.Write(&info.dataLen_, sizeof(DWORD));
    pack.Write(info.packBuf_, info.packLen_);
    DFree(info.dataBuf_);
    DFree(info.packBuf_);

    pack.Seek(0, DFILE_BEGIN);
    pack.Write(&header, sizeof(header));
    pack.Close();
    return true;
}

bool DPakMake::UnPack(char const* datFile, char const* outPath) {
    if (!GpPakCode)
        return false;

    DPakFile packFile;
    char szPathName[DMAX_PATH];
    DWORD magic = 0;

    DSetPakFileMode(DPAK_DISK_FIRST);
    if (!packFile.Open(datFile))
        return false;

    DPakHead header;
    packFile.Read(&header, sizeof(header));
    packFile.Seek(header.listOffset, DFILE_BEGIN);

    DPakCodeInfo info;
    packFile.Read(&magic, sizeof(DWORD));
    if (magic != ZIP_MAGIC)
        return false;
    packFile.Read(&info.packLen_, sizeof(DWORD));
    packFile.Read(&info.dataLen_, sizeof(DWORD));

    info.dataBuf_ = (BYTE*)DCAlloc(info.dataLen_);
    info.packBuf_ = (BYTE*)DCAlloc(info.packLen_);
    packFile.Read(info.packBuf_, info.packLen_);
    GpPakCode->Decode(&info);

    DTxtFile textFile;
    textFile.OpenMem(info.dataBuf_, info.dataLen_);
    DFree(info.dataBuf_);
    DFree(info.packBuf_);

    DPathSetCurrent((char*)outPath);
    char* pLine = textFile.FirstLine();
    while (pLine) {
        DDebugLog("Create Dir %s...", pLine);
        DPathGetFull(szPathName, pLine);
        std::error_code ec;
        fs::create_directories(szPathName, ec);
        pLine = textFile.NextLine();
    }

    packFile.Read(&magic, sizeof(DWORD));
    if (magic != ZIP_MAGIC)
        return false;
    packFile.Read(&info.packLen_, sizeof(DWORD));
    packFile.Read(&info.dataLen_, sizeof(DWORD));

    info.dataBuf_ = (BYTE*)DCAlloc(info.dataLen_);
    info.packBuf_ = (BYTE*)DCAlloc(info.packLen_);
    packFile.Read(info.packBuf_, info.packLen_);
    packFile.Close();
    GpPakCode->Decode(&info);

    textFile.OpenMem(info.dataBuf_, info.dataLen_);
    DFree(info.dataBuf_);
    DFree(info.packBuf_);

    DSetPakFileMode(DPAK_PACK_FIRST);
    DPathSetCurrent((char*)outPath);
    pLine = textFile.FirstLine();
    while (pLine) {
        pLine = textFile.NextLine();
        if (pLine) {
            DDebugLog("Unpack File %s...", pLine);
            DPathGetFull(szPathName, pLine);
            if (packFile.Open(pLine)) {
                packFile.Save(szPathName);
            } else {
                DDebugLog("Unpack File %s...Failed", pLine);
            }
        }
    }
    return true;
}