#ifndef DAGGER_INIFILE_H
#define DAGGER_INIFILE_H

#include "HashList.h"
#include "Memory.h"
#include "TypeDef.h"

class DIniKeyNode : public DNameNode {
public:
    char* value_;
};

class DIniSecNode : public DNameNode {
public:
    DHashList keyList_;
};

class DIniFile : public DHashList {
public:
    ~DIniFile();
    bool Load(char const* fileName);
    bool Save(char const* fileName);
    bool LoadMemory(DMemory* memory);
    DWORD SaveMemory(DMemory* memory);
    void Free();
    DIniSecNode* NewSection(char const* section);
    DIniSecNode* FindSection(char const* section);
    void FreeSection(DIniSecNode* sec);
    void EraseSection(char const* section);
    DIniSecNode* FirstSection();
    DIniKeyNode* NewKey(char const* section, char const* key);
    DIniKeyNode* FindKey(char const* section, char const* key);
    void EraseKey(char const* section, char const* key);
    DIniKeyNode* GetKeyValue(char const* section, char const* key, char* value);
    DIniKeyNode* SetKeyValue(char const* section, char const* key, char const* value);
    DIniKeyNode* AddKeyValue(char const* section, char const* key, char const* value);
    bool GetString(char const* section, char const* key, char const* def, char* str);
    bool GetInteger(char const* section, char const* key, long def, long* val);
    void SetString(char const* section, char const* key, char const* str);
    void SetInteger(char const* section, char const* key, long val);
    char const* GetStr(char const* section, char const* key, char const* def);
    long GetInt(char const* section, char const* key, long def);
    void SetStr(char const* section, char const* key, char const* str);
    void SetInt(char const* section, char const* key, long val);

private:
    DWORD GetSaveSize();
    void InitList(char* buffer, long len);
    bool IsKeyChar(BYTE c);
    char* SplitKeyValue(char* str);
};

#endif//DAGGER_INIFILE_H
