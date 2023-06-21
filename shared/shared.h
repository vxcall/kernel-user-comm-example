#ifndef EWDK_QUICKSTART_SHARED_H
#define EWDK_QUICKSTART_SHARED_H

#include <Windows.h>

struct ThreadData {
    ULONG ThreadId;
    int Priority;
};

#endif //EWDK_QUICKSTART_SHARED_H