#include "PakScan.h"

#include <filesystem>
#include <string.h>

#include "System.h"

namespace fs = std::filesystem;

namespace {

void WriteEntry(FILE* fp, char const* prefix, char const* name) {
    fprintf(fp, "%s\\%s\n", prefix, name);
}

void ScanDir(FILE* fileFp, FILE* pathFp, fs::path const& root, fs::path const& dir) {
    for (auto const& entry : fs::directory_iterator(dir)) {
        std::string name = entry.path().filename().string();
        if (entry.is_directory()) {
            fprintf(pathFp, "%s\\%s\n",
                dir.lexically_relative(root).string().c_str(),
                name.c_str());
            ScanDir(fileFp, pathFp, root, entry.path());
        } else {
            char prefix[DMAX_PATH] = "";
            std::string rel = dir.lexically_relative(root).string();
            if (!rel.empty()) {
                DStrLCopy(prefix, rel.c_str(), DMAX_PATH - 1);
            }
            WriteEntry(fileFp, prefix, name.c_str());
        }
    }
}

}

bool DPakScan::Scan(char const* scanPath, char const* fileName) {
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

    fprintf(fileFp, "%s\n", root.string().c_str());

    ScanDir(fileFp, pathFp, root, root);

    fclose(fileFp);
    fclose(pathFp);
    return true;
}