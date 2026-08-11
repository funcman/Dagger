#include "PakPath.h"

#include <ctype.h>
#include <filesystem>
#include <string.h>

#include "System.h"

namespace fs = std::filesystem;

namespace {

bool ExtMatch(char const* fileName, char const* ext) {
    const char* dot = strrchr(fileName, '.');
    if (!dot) return false;
    while (*dot && *ext) {
        if (tolower((unsigned char)*dot) != tolower((unsigned char)*ext)) return false;
        ++dot;
        ++ext;
    }
    return *dot == 0 && *ext == 0;
}

void ScanDir(FILE* fileFp, FILE* pathFp, char const* prefix, fs::path const& dir, char const* ext) {
    for (auto const& entry : fs::directory_iterator(dir)) {
        std::string name = entry.path().filename().string();
        if (entry.is_directory()) {
            std::string newPrefix = std::string(prefix) + "\\" + name;
            fprintf(pathFp, "%s\n", newPrefix.c_str());
            ScanDir(fileFp, pathFp, newPrefix.c_str(), entry.path(), ext);
        } else {
            if (ext && !ExtMatch(name.c_str(), ext)) continue;
            fprintf(fileFp, "%s\\%s\n", prefix, name.c_str());
        }
    }
}

}

DPakPath::DPakPath() {
    fileExt_[0] = '*';
    fileExt_[1] = 0;
}

bool DPakPath::Scan(char const* scanPath, char const* fileName) {
    fs::path root = fs::path(scanPath).lexically_normal();
    if (!fs::exists(root) || !fs::is_directory(root)) return false;

    char pathName[DMAX_PATH];
    DStrLCopy(pathName, fileName, DMAX_PATH - 1);
    DPathChangeExt(pathName, ".dir");

    FILE* fileFp = fopen(fileName, "w+");
    if (!fileFp) return false;
    FILE* pathFp = fopen(pathName, "w+");
    if (!pathFp) {
        fclose(fileFp);
        return false;
    }

    ScanDir(fileFp, pathFp, "", root, fileExt_[0] == '*' ? nullptr : fileExt_);

    fclose(fileFp);
    fclose(pathFp);
    return true;
}

void DPakPath::SetFileExt(char const* fileExt) {
    DStrLCopy(fileExt_, fileExt, sizeof(fileExt_) - 1);
    fileExt_[sizeof(fileExt_) - 1] = 0;
}