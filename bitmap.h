#pragma once
#include <string.h>

namespace BM {
    bool check(char* bitmap, int slot) {
        return bitmap[slot / 8] & (1 << (slot % 8));
    }

    void set(char* bitmap, int slot) {
        bitmap[slot / 8] |= 1 << (slot % 8);
    }

    void reset(char* bitmap, int slot) {
        bitmap[slot / 8] &= ~(1 << (slot % 8));
    }

    void clear(char* bitmap, int length) {
        memset(bitmap, 0, length);
    }
}