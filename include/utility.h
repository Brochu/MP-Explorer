#pragma once
#include <assert.h>

#define ThrowIfFailed(hr)   \
do {                        \
    if (FAILED(hr)) {       \
        assert(false);      \
    }                       \
} while(0);                 \
