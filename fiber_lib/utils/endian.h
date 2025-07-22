#ifndef __DAG_ENDIAN_H__
#define __DAG_ENDIAN_H__

#ifdef __cplusplus
extern "C++"{
#endif
#define DAG_LITTLE_ENDIAN 1
#define DAG_BIG_ENDIAN 2

#include <byteswap.h>
#include <type_traits>
#include <stdint.h>
#include <endian.h>

namespace dag {

/**
 * @brief 8字节类型的字节序转化
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
byteswap(T value) {
    return (T)bswap_64((uint64_t)value);
}

/**
 * @brief 4字节类型的字节序转化
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
byteswap(T value) {
    return (T)bswap_32((uint32_t)value);
}

/**
 * @brief 2字节类型的字节序转化
 */
template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
byteswap(T value) {
    return (T)bswap_16((uint16_t)value);
}

#if __BYTE_ORDER__ == BIG_ENDIAN
#define DAG_BYTE_ORDER DAG_BIG_ENDIAN
#else
#define DAG_BYTE_ORDER DAG_LITTLE_ENDIAN
#endif

#if DAG_BYTE_ORDER == DAG_BIG_ENDIAN

/**
 * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
 */
template<class T>
T byteswapOnLittleEndian(T t) {
    return t;
}

/**
 * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
 */
template<class T>
T byteswapOnBigEndian(T t) {
    return byteswap(t);
}
#else

/**
 * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
 */
template<class T>
T byteswapOnLittleEndian(T t) {
    return byteswap(t);
}

/**
 * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
 */ template<class T>
T byteswapOnBigEndian(T t) {
    return t;
}
#endif

}
#ifdef __cplusplus
}
#endif

#endif
