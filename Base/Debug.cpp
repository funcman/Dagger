#include "Debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static char const* FileName = "DaggerDebug.log";

void DDebugLog(char const* fmt, ...) {
    va_list va;
    FILE* fp = fopen(FileName, "a");
    if (fp) {
        time_t t = time(NULL);
        struct tm time;
#ifdef _WIN32
        localtime_s(&time, &t);
#else
        localtime_r(&t, &time);
#endif
        fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] ",
                time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
                time.tm_hour, time.tm_min, time.tm_sec);
        va_start(va, fmt);
        vfprintf(fp, fmt, va);
        va_end(va);
        fprintf(fp, "\n");
        fclose(fp);
    }
}
