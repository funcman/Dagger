#include "IniFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BinFile.h"
#include "Debug.h"
#include "MemPool.h"

DIniFile::~DIniFile() {
    Free();
}

bool DIniFile::Load(char const* fileName) {
    if (!fileName || !fileName[0])
        return false;
    Free();
    DBinFile file;
    if (!file.Open(fileName))
        return false;
    DWORD size = file.Size();
    if (size == DFILE_SEEK_ERROR)
        return false;
    char* buffer = (char*)DCAlloc(size + 4);
    if (!buffer)
        return false;
    file.Read(buffer, size);
    buffer[size] = 0;
    InitList_(buffer, (long)size);
    DFree(buffer);
    return true;
}

bool DIniFile::Save(char const* fileName) {
    if (!fileName || !fileName[0])
        return false;
    DWORD size = GetSaveSize_();
    char* buffer = (char*)DCAlloc(size);
    if (!buffer)
        return false;
    char* p = buffer;
    DWORD remaining = size;
    DIniSecNode* sec = (DIniSecNode*)GetHead();
    while (sec) {
        int n = snprintf(p, remaining, "[%s]\r\n", sec->name);
        p += n;
        remaining -= n;
        DIniKeyNode* key = (DIniKeyNode*)sec->keyList.GetHead();
        while (key) {
            n = snprintf(p, remaining, "%s=%s\r\n", key->name, key->value);
            p += n;
            remaining -= n;
            key = (DIniKeyNode*)key->GetNext();
        }
        n = snprintf(p, remaining, "\r\n");
        p += n;
        remaining -= n;
        sec = (DIniSecNode*)sec->GetNext();
    }
    DWORD len = (DWORD)(p - buffer);
    DBinFile file;
    bool ok = file.Create(fileName);
    if (ok)
        file.Write(buffer, len);
    DFree(buffer);
    return ok;
}

bool DIniFile::LoadMemory(DMemory* memory) {
    Free();
    InitList_((char*)memory->GetMemPtr(), (long)memory->GetMemLen());
    return true;
}

DWORD DIniFile::SaveMemory(DMemory* memory) {
    DWORD size = GetSaveSize_();
    memory->Alloc(size);
    memory->Zero();
    char* p = (char*)memory->GetMemPtr();
    DWORD remaining = size;
    DIniSecNode* sec = (DIniSecNode*)GetHead();
    while (sec) {
        int n = snprintf(p, remaining, "[%s]\r\n", sec->name);
        p += n;
        remaining -= n;
        DIniKeyNode* key = (DIniKeyNode*)sec->keyList.GetHead();
        while (key) {
            n = snprintf(p, remaining, "%s=%s\r\n", key->name, key->value);
            p += n;
            remaining -= n;
            key = (DIniKeyNode*)key->GetNext();
        }
        n = snprintf(p, remaining, "\r\n");
        p += n;
        remaining -= n;
        sec = (DIniSecNode*)sec->GetNext();
    }
    *p++ = 0;
    return (DWORD)(p - (char*)memory->GetMemPtr());
}

void DIniFile::Free() {
    DIniSecNode* sec = (DIniSecNode*)GetHead();
    while (sec) {
        DIniSecNode* next = (DIniSecNode*)sec->GetNext();
        FreeSection(sec);
        sec = next;
    }
}

DWORD DIniFile::GetSaveSize_() {
    DWORD size = 0;
    DIniSecNode* sec = (DIniSecNode*)GetHead();
    while (sec) {
        size += strlen(sec->name) + 10;
        DIniKeyNode* key = (DIniKeyNode*)sec->keyList.GetHead();
        while (key) {
            size += strlen(key->name) + strlen(key->value) + 10;
            key = (DIniKeyNode*)key->GetNext();
        }
        size += 4;
        sec = (DIniSecNode*)sec->GetNext();
    }
    return size;
}

void DIniFile::InitList_(char* buffer, long len) {
    char line[1024] = "";
    char section[256] = "";
    int idx = 0;
    int lineLen = 0;
    while (idx < len && buffer[idx]) {
        lineLen = 0;
        while (idx < len && buffer[idx]) {
            if (buffer[idx] == '\r') {
                idx++;
                if (idx < len && buffer[idx] == '\n')
                    idx++;
                break;
            }
            if (buffer[idx] == '\n') {
                idx++;
                if (idx < len && buffer[idx] == '\r')
                    idx++;
                break;
            }
            if (lineLen < 1023) {
                line[lineLen] = buffer[idx];
                lineLen++;
            }
            idx++;
        }
        line[lineLen] = 0;
        if (line[0] == '[') {
            int i = 1;
            while (line[i] && line[i] != ']' && i < 255) {
                section[i - 1] = line[i];
                i++;
            }
            section[i - 1] = 0;
            if (i > 1)
                NewSection(section);
        } else if (IsKeyChar_((BYTE)line[0])) {
            char* value = SplitKeyValue_(line);
            if (value && value[0] && section[0]) {
                AddKeyValue(section, line, value);
            }
        }
    }
}

bool DIniFile::IsKeyChar_(BYTE c) {
    return (c >= 33 && c <= 126) || (c >= 0xa1 && c <= 0xfe);
}

char* DIniFile::SplitKeyValue_(char* str) {
    while (*str) {
        if (*str == '=') {
            *str = 0;
            return str + 1;
        }
        str++;
    }
    return nullptr;
}

DIniSecNode* DIniFile::NewSection(char const* section) {
    DIniSecNode* sec = FindSection(section);
    if (sec)
        return sec;
    sec = (DIniSecNode*)DCAlloc(sizeof(DIniSecNode));
    sec->SetName(section);
    AddTail(sec);
    return sec;
}

void DIniFile::FreeSection(DIniSecNode* sec) {
    if (!sec)
        return;
    DIniKeyNode* key = (DIniKeyNode*)sec->keyList.GetHead();
    while (key) {
        DIniKeyNode* next = (DIniKeyNode*)key->GetNext();
        DFree(key->value);
        key->Delete();
        key = next;
    }
    sec->Delete();
}

DIniSecNode* DIniFile::FindSection(char const* section) {
    return (DIniSecNode*)FindName(section);
}

void DIniFile::EraseSection(char const* section) {
    FreeSection(FindSection(section));
}

DIniSecNode* DIniFile::FirstSection() {
    return (DIniSecNode*)GetHead();
}

DIniKeyNode* DIniFile::NewKey(char const* section, char const* key) {
    DIniSecNode* sec = NewSection(section);
    DIniKeyNode* k = FindKey(section, key);
    if (k)
        return k;
    k = (DIniKeyNode*)DCAlloc(sizeof(DIniKeyNode));
    k->SetName(key);
    sec->keyList.AddTail(k);
    return k;
}

DIniKeyNode* DIniFile::FindKey(char const* section, char const* key) {
    DIniSecNode* sec = FindSection(section);
    if (!sec)
        return nullptr;
    return (DIniKeyNode*)sec->keyList.FindName(key);
}

void DIniFile::EraseKey(char const* section, char const* key) {
    DIniKeyNode* k = FindKey(section, key);
    if (!k)
        return;
    DFree(k->value);
    k->Delete();
}

DIniKeyNode* DIniFile::GetKeyValue(char const* section, char const* key, char* value) {
    DIniKeyNode* k = FindKey(section, key);
    if (k)
        strcpy(value, k->value);
    return k;
}

DIniKeyNode* DIniFile::SetKeyValue(char const* section, char const* key, char const* value) {
    DIniKeyNode* k = NewKey(section, key);
    DFree(k->value);
    k->value = (char*)DCAlloc(strlen(value) + 2);
    strcpy(k->value, value);
    return k;
}

DIniKeyNode* DIniFile::AddKeyValue(char const* section, char const* key, char const* value) {
    DIniSecNode* sec = NewSection(section);
    DIniKeyNode* k = (DIniKeyNode*)DCAlloc(sizeof(DIniKeyNode));
    k->SetName(key);
    sec->keyList.AddTail(k);
    k->value = (char*)DCAlloc(strlen(value) + 2);
    strcpy(k->value, value);
    return k;
}

bool DIniFile::GetString(char const* section, char const* key, char const* def, char* str) {
    if (GetKeyValue(section, key, str))
        return true;
    if (str && def)
        strcpy(str, def);
    return false;
}

bool DIniFile::GetInteger(char const* section, char const* key, long def, long* val) {
    char buf[200];
    if (GetKeyValue(section, key, buf)) {
        *val = atol(buf);
        return true;
    }
    *val = def;
    return false;
}

void DIniFile::SetString(char const* section, char const* key, char const* str) {
    SetKeyValue(section, key, str);
}

void DIniFile::SetInteger(char const* section, char const* key, long val) {
    char buf[200];
    snprintf(buf, sizeof(buf), "%ld", val);
    SetKeyValue(section, key, buf);
}

char const* DIniFile::GetStr(char const* section, char const* key, char const* def) {
    DIniKeyNode* k = FindKey(section, key);
    return k ? k->value : def;
}

long DIniFile::GetInt(char const* section, char const* key, long def) {
    DIniKeyNode* k = FindKey(section, key);
    return k ? atol(k->value) : def;
}

void DIniFile::SetStr(char const* section, char const* key, char const* str) {
    SetKeyValue(section, key, str);
}

void DIniFile::SetInt(char const* section, char const* key, long val) {
    char buf[200];
    snprintf(buf, sizeof(buf), "%ld", val);
    SetKeyValue(section, key, buf);
}
