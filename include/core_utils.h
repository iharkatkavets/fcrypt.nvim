/* utils.h */

#ifndef CORE_UTILS_H
#define CORE_UTILS_H

#define MIN(a, b) ({          \
    typeof(a) _a = (a);       \
    typeof(b) _b = (b);       \
    _a < _b ? _a : _b;        \
})

#define MAX(a, b) ({          \
    typeof(a) _a = (a);       \
    typeof(b) _b = (b);       \
    _a < _b ? _b : _a;        \
})

#endif
