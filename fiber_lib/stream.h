#ifndef __DAG_STREAM_H__
#define __DAG_STREAM_H__

#include <memory>
#include "bytearray.h"

namespace dag {

class Stream {
public:
    using ptr = std::shared_ptr<Stream>;

    virtual ~Stream() {}

    virtual int read(void* buffer, size_t length) = 0;

    virtual int read(ByteArray::ptr ba, size_t length) = 0;

    virtual int readFixSize(void* buffer, size_t length);

    virtual int readFixSize(ByteArray::ptr ba, size_t length);

    virtual int write(const void* buffer, size_t length) = 0;

    virtual int write(ByteArray::ptr ba, size_t length) = 0;

    virtual int writeFixSize(const void* buffer, size_t length);

    virtual int writeFixSize(ByteArray::ptr ba, size_t length);

    virtual void close() = 0;
};

}
#endif
