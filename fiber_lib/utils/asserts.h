#ifndef _DAG_ASSERTS_H_
#define _DAG_ASSERTS_H_

#include <cassert>
#include "../logger.h"
#include "util.h"

#define DAG_ASSERT(x) \
    if (!(x)) {        \
        DAG_LOG_ERROR(DAG_LOG_ROOT()) << \
                "DAG_ASSERT:" << #x << "\nbacktrace:\n" << dag::backtraceToString(64, 2, "    "); \
        assert(x);                   \
    }

#define DAG_ASSERT2(x, w) \
    if (!(x)) {        \
        DAG_LOG_ERROR(DAG_LOG_ROOT()) << \
                "DAG_ASSERT:" << #x << "\n" << w << "\nbacktrace:\n" << dag::backtraceToString(64, 2, "    "); \
        assert(x);                   \
    }

#endif //DAG_ASSERTS_H
