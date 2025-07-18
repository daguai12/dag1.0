// #ifndef _DAG_ENDIAN_H_
// #define _DAG_ENDIAN_H_
//
// #ifdef __cplusplus
// extern "C++" {
// #endif
//
// #include <cstdint>
// #include <type_traits>
//
// #include <byteswap.h>  // 这是 C 风格头文件，必须包在 extern "C" 内部
//
//
//
// namespace dag {
//
// // 1. 字节交换函数模板
// template <typename T>
// typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
// byteSwap(T value) {
//     return static_cast<T>(bswap_64(static_cast<uint64_t>(value)));
// }
//
// template <typename T>
// typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
// byteSwap(T value) {
//     return static_cast<T>(bswap_32(static_cast<uint32_t>(value)));
// }
//
// template <typename T>
// typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
// byteSwap(T value) {
//     return static_cast<T>(bswap_16(static_cast<uint16_t>(value)));
// }
//
// template <typename T>
// typename std::enable_if<sizeof(T) == sizeof(uint8_t), T>::type
// byteSwap(T value) {
//     return value;
// }
//
// // 2. 判断当前字节序（注意：使用 __BYTE_ORDER__ 宏）
// #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
//     template <typename T>
//     T onBigEndian(T t) {
//         return t;
//     }
//
//     template <typename T>
//     T onLittleEndian(T t) {
//         return byteSwap(t);
//     }
// #else
//     template <typename T>
//     T onBigEndian(T t) {
//         return byteSwap(t);
//     }
//
//     template <typename T>
//     T onLittleEndian(T t) {
//         return t;
//     }
// #endif
//
// #ifdef __cplusplus
// } // extern "C++"
// } // namespace dag
// #endif
// #endif // _DAG_ENDIAN_H_
//
//


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

#if BYTE_ORDER == BIG_ENDIAN
#define DAG_BYTE_ORDER DAG_BIG_ENDIAN
#else
#define DAG_BYTE_ORDER DAG_LITTLE_ENDIAN
#endif

#if DAG_BYTE_ORDER != DAG_BIG_ENDIAN

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
 */
template<class T>
T byteswapOnBigEndian(T t) {
    return t;
}
#endif

}
#ifdef __cplusplus
}
#endif

#endif
