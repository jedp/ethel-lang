#include "../inc/math.h"

int min(int a, int b) {
    if (a < b) return a;
    return b;
}

int max(int a, int b) {
    if (a > b) return a;
    return b;
}

int abs(int n) {
    if (n < 0) {
        return 0 - n;
    }

    return n;
}
