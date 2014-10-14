#ifndef RAPIDJSONXML_RAPIDJSONXML_H_
#define RAPIDJSONXML_RAPIDJSONXML_H_

// Copyright (c) 2011 Milo Yip (miloyip@gmail.com)
// Version 0.1

/*!\file rapidjsonxml.h
    \brief common definitions and configuration

    \todo Complete Doxygen documentation for configure macros.
*/

#include <cstdlib> // malloc(), realloc(), free()
#include <cstring> // memcpy()

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSONXML_NO_INT64DEFINE

// Here defines int64_t and uint64_t types in global namespace as well as the
// (U)INT64_C constant macros.
// If user have their own definition, can define RAPIDJSONXML_NO_INT64DEFINE to disable this.
#ifndef RAPIDJSONXML_NO_INT64DEFINE
//!@cond RAPIDJSONXML_HIDDEN_FROM_DOXYGEN
#ifndef __STDC_CONSTANT_MACROS
#  define __STDC_CONSTANT_MACROS 1 // required by C++ standard
#endif
#ifdef _MSC_VER
#include "msinttypes/stdint.h"
#include "msinttypes/inttypes.h"
#else
// Other compilers should have this.
#include <stdint.h>
#include <inttypes.h>
#endif
//!@endcond
#endif // RAPIDJSONXML_NO_INT64TYPEDEF

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSONXML_FORCEINLINE

#ifndef RAPIDJSONXML_FORCEINLINE
#ifdef _MSC_VER
#define RAPIDJSONXML_FORCEINLINE __forceinline
#elif defined(__GNUC__) && __GNUC__ >= 4
#define RAPIDJSONXML_FORCEINLINE __attribute__((always_inline))
#else
#define RAPIDJSONXML_FORCEINLINE
#endif
#endif // RAPIDJSONXML_FORCEINLINE

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSONXML_ENDIAN
#define RAPIDJSONXML_LITTLEENDIAN   0   //!< Little endian machine
#define RAPIDJSONXML_BIGENDIAN      1   //!< Big endian machine

//! Endianness of the machine.
/*! GCC 4.6 provided macro for detecting endianness of the target machine. But other
    compilers may not have this. User can define RAPIDJSONXML_ENDIAN to either
    \ref RAPIDJSONXML_LITTLEENDIAN or \ref RAPIDJSONXML_BIGENDIAN.

    Implemented with reference to
    https://gcc.gnu.org/onlinedocs/gcc-4.6.0/cpp/Common-Predefined-Macros.html
    http://www.boost.org/doc/libs/1_42_0/boost/detail/endian.hpp
*/
#ifndef RAPIDJSONXML_ENDIAN
// Detect with GCC 4.6's macro
#  ifdef __BYTE_ORDER__
#    if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#      define RAPIDJSONXML_ENDIAN RAPIDJSONXML_LITTLEENDIAN
#    elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#      define RAPIDJSONXML_ENDIAN RAPIDJSONXML_BIGENDIAN
#    else
#      error Unknown machine endianess detected. User needs to define RAPIDJSONXML_ENDIAN.
#    endif // __BYTE_ORDER__
// Detect with GLIBC's endian.h
#  elif defined(__GLIBC__)
#    include <endian.h>
#    if (__BYTE_ORDER == __LITTLE_ENDIAN)
#      define RAPIDJSONXML_ENDIAN RAPIDJSONXML_LITTLEENDIAN
#    elif (__BYTE_ORDER == __BIG_ENDIAN)
#      define RAPIDJSONXML_ENDIAN RAPIDJSONXML_BIGENDIAN
#    else
#      error Unknown machine endianess detected. User needs to define RAPIDJSONXML_ENDIAN.
#   endif // __GLIBC__
// Detect with _LITTLE_ENDIAN and _BIG_ENDIAN macro
#  elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
#    define RAPIDJSONXML_ENDIAN RAPIDJSONXML_LITTLEENDIAN
#  elif defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
#    define RAPIDJSONXML_ENDIAN RAPIDJSONXML_BIGENDIAN
// Detect with architecture macros
#  elif defined(__sparc) || defined(__sparc__) || defined(_POWER) || defined(__powerpc__) || defined(__ppc__) || defined(__hpux) || defined(__hppa) || defined(_MIPSEB) || defined(_POWER) || defined(__s390__)
#    define RAPIDJSONXML_ENDIAN RAPIDJSONXML_BIGENDIAN
#  elif defined(__i386__) || defined(__alpha__) || defined(__ia64) || defined(__ia64__) || defined(_M_IX86) || defined(_M_IA64) || defined(_M_ALPHA) || defined(__amd64) || defined(__amd64__) || defined(_M_AMD64) || defined(__x86_64) || defined(__x86_64__) || defined(_M_X64) || defined(__bfin__)
#    define RAPIDJSONXML_ENDIAN RAPIDJSONXML_LITTLEENDIAN
#  else
#    error Unknown machine endianess detected. User needs to define RAPIDJSONXML_ENDIAN.
#  endif
#endif // RAPIDJSONXML_ENDIAN

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSONXML_ALIGNSIZE

//! Data alignment of the machine.
/*!
    Some machine requires strict data alignment.
    Currently the default uses 4 bytes alignment. User can customize this.
*/
#ifndef RAPIDJSONXML_ALIGN
#define RAPIDJSONXML_ALIGN(x) ((x + 3u) & ~3u)
#endif

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSONXML_SSE2/RAPIDJSONXML_SSE42/RAPIDJSONXML_SIMD

// Enable SSE2 optimization.
//#define RAPIDJSONXML_SSE2

// Enable SSE4.2 optimization.
//#define RAPIDJSONXML_SSE42

#if defined(RAPIDJSONXML_SSE2) || defined(RAPIDJSONXML_SSE42)
#define RAPIDJSONXML_SIMD
#endif

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSONXML_NO_SIZETYPEDEFINE

#ifndef RAPIDJSONXML_NO_SIZETYPEDEFINE
namespace rapidjsonxml {
//! Use 32-bit array/string indices even for 64-bit platform, instead of using size_t.
/*! User may override the SizeType by defining RAPIDJSONXML_NO_SIZETYPEDEFINE.
*/
typedef unsigned SizeType;
} // namespace rapidjsonxml
#endif

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSONXML_ASSERT

//! Assertion.
/*! By default, RapidJsonXml raises an exception for assertion.
    User can override it by defining RAPIDJSONXML_ASSERT(x) macro.
*/
#include "assert_exception.h"
#ifndef RAPIDJSONXML_ASSERT
#include <cassert>
#define RAPIDJSONXML_ASSERT(x) assert(x)
#endif // RAPIDJSONXML_ASSERT

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSONXML_STATIC_ASSERT

// Adopt from boost
#ifndef RAPIDJSONXML_STATIC_ASSERT
//!@cond RAPIDJSONXML_HIDDEN_FROM_DOXYGEN
namespace rapidjsonxml {

template <bool x> struct STATIC_ASSERTION_FAILURE;
template <> struct STATIC_ASSERTION_FAILURE<true> {
    enum { value = 1 };
};
template<int x> struct StaticAssertTest {};
} // namespace rapidjsonxml

#define RAPIDJSONXML_JOIN(X, Y) RAPIDJSONXML_DO_JOIN(X, Y)
#define RAPIDJSONXML_DO_JOIN(X, Y) RAPIDJSONXML_DO_JOIN2(X, Y)
#define RAPIDJSONXML_DO_JOIN2(X, Y) X##Y

#if defined(__GNUC__)
#define RAPIDJSONXML_STATIC_ASSERT_UNUSED_ATTRIBUTE __attribute__((unused))
#else
#define RAPIDJSONXML_STATIC_ASSERT_UNUSED_ATTRIBUTE
#endif
//!@endcond

/*! \def RAPIDJSONXML_STATIC_ASSERT
    \brief (internal) macro to check for conditions at compile-time
    \param x compile-time condition
    \hideinitializer
*/
#define RAPIDJSONXML_STATIC_ASSERT(x) typedef ::rapidjsonxml::StaticAssertTest<\
    sizeof(::rapidjsonxml::STATIC_ASSERTION_FAILURE<bool(x) >)>\
    RAPIDJSONXML_JOIN(StaticAssertTypedef, __LINE__) RAPIDJSONXML_STATIC_ASSERT_UNUSED_ATTRIBUTE
#endif

///////////////////////////////////////////////////////////////////////////////
// Helpers

//!@cond RAPIDJSONXML_HIDDEN_FROM_DOXYGEN

#define RAPIDJSONXML_MULTILINEMACRO_BEGIN do {
#define RAPIDJSONXML_MULTILINEMACRO_END \
} while((void)0, 0)

// adopted from Boost
#define RAPIDJSONXML_VERSION_CODE(x,y,z) \
(((x)*100000) + ((y)*100) + (z))

// token stringification
#define RAPIDJSONXML_STRINGIFY(x) RAPIDJSONXML_DO_STRINGIFY(x)
#define RAPIDJSONXML_DO_STRINGIFY(x) #x

///////////////////////////////////////////////////////////////////////////////
// RAPIDJSONXML_DIAG_PUSH/POP, RAPIDJSONXML_DIAG_OFF

#if defined(__clang__) || (defined(__GNUC__) && RAPIDJSONXML_VERSION_CODE(__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__) >= RAPIDJSONXML_VERSION_CODE(4,2,0))

#define RAPIDJSONXML_PRAGMA(x) _Pragma(RAPIDJSONXML_STRINGIFY(x))
#define RAPIDJSONXML_DIAG_PRAGMA(x) RAPIDJSONXML_PRAGMA(GCC diagnostic x)
#define RAPIDJSONXML_DIAG_OFF(x) \
    RAPIDJSONXML_DIAG_PRAGMA(ignored RAPIDJSONXML_STRINGIFY(RAPIDJSONXML_JOIN(-W,x)))

// push/pop support in Clang and GCC>=4.6
#if defined(__clang__) || (defined(__GNUC__) && RAPIDJSONXML_VERSION_CODE(__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__) >= RAPIDJSONXML_VERSION_CODE(4,6,0))
#define RAPIDJSONXML_DIAG_PUSH RAPIDJSONXML_DIAG_PRAGMA(push)
#define RAPIDJSONXML_DIAG_POP  RAPIDJSONXML_DIAG_PRAGMA(pop)
#else // GCC >= 4.2, < 4.6
#define RAPIDJSONXML_DIAG_PUSH /* ignored */
#define RAPIDJSONXML_DIAG_POP /* ignored */
#endif

#elif defined(_MSC_VER)

// pragma (MSVC specific)
#define RAPIDJSONXML_PRAGMA(x) __pragma(x)
#define RAPIDJSONXML_DIAG_PRAGMA(x) RAPIDJSONXML_PRAGMA(warning(x))

#define RAPIDJSONXML_DIAG_OFF(x) RAPIDJSONXML_DIAG_PRAGMA(disable: x)
#define RAPIDJSONXML_DIAG_PUSH RAPIDJSONXML_DIAG_PRAGMA(push)
#define RAPIDJSONXML_DIAG_POP  RAPIDJSONXML_DIAG_PRAGMA(pop)

#else

#define RAPIDJSONXML_DIAG_OFF(x) /* ignored */
#define RAPIDJSONXML_DIAG_PUSH   /* ignored */
#define RAPIDJSONXML_DIAG_POP    /* ignored */

#endif // RAPIDJSONXML_DIAG_*

//!@endcond

///////////////////////////////////////////////////////////////////////////////
// Allocators and Encodings

#include "allocators.h"
#include "encodings.h"

//! main RapidJsonXml namespace
namespace rapidjsonxml {

///////////////////////////////////////////////////////////////////////////////
//  Stream

/*! \class rapidjsonxml::Stream
    \brief Concept for reading and writing characters.

    For read-only stream, no need to implement PutBegin(), Put(), Flush() and PutEnd().

    For write-only stream, only need to implement Put() and Flush().

\code
concept Stream {
    typename Ch; //!< Character type of the stream.

    //! Read the current character from stream without moving the read cursor.
    Ch Peek() const;

    //! Read the current character from stream and moving the read cursor to next character.
    Ch Take();

    //! Get the current read cursor.
    //! \return Number of characters read from start.
    size_t Tell();

    //! Begin writing operation at the current read pointer.
    //! \return The begin writer pointer.
    Ch* PutBegin();

    //! Write a character.
    void Put(Ch c);

    //! Flush the buffer.
    void Flush();

    //! End the writing operation.
    //! \param begin The begin write pointer returned by PutBegin().
    //! \return Number of characters written.
    size_t PutEnd(Ch* begin);
}
\endcode
*/

//! Provides additional information for stream.
/*!
    By using traits pattern, this type provides a default configuration for stream.
    For custom stream, this type can be specialized for other configuration.
    See TEST(Reader, CustomStringStream) in readertest.cpp for example.
*/
template<typename Stream>
struct StreamTraits {
    //! Whether to make local copy of stream for optimization during parsing.
    /*!
        By default, for safety, streams do not use local copy optimization.
        Stream that can be copied fast should specialize this, like StreamTraits<StringStream>.
    */
    enum { copyOptimization = 0 };
};

//! Put N copies of a character to a stream.
template<typename Stream, typename Ch>
inline void PutN(Stream& stream, Ch c, size_t n) {
    for (size_t i = 0; i < n; i++)
        stream.Put(c);
}

///////////////////////////////////////////////////////////////////////////////
// StringStream

//! Read-only string stream.
/*! \note implements Stream concept
*/
template <typename Encoding>
struct GenericStringStream {
    typedef typename Encoding::Ch Ch;

    GenericStringStream(const Ch *src) : src_(src), head_(src) {}

    Ch Peek() const {
        return *src_;
    }
    Ch Take() {
        return *src_++;
    }
    size_t Tell() const {
        return static_cast<size_t>(src_ - head_);
    }

    Ch* PutBegin() {
        RAPIDJSONXML_ASSERT(false);
        return 0;
    }
    void Put(Ch) {
        RAPIDJSONXML_ASSERT(false);
    }
    void Flush() {
        RAPIDJSONXML_ASSERT(false);
    }
    size_t PutEnd(Ch*) {
        RAPIDJSONXML_ASSERT(false);
        return 0;
    }

    const Ch* src_;     //!< Current read position.
    const Ch* head_;    //!< Original head of the string.
};

template <typename Encoding>
struct StreamTraits<GenericStringStream<Encoding> > {
    enum { copyOptimization = 1 };
};

//! String stream with UTF8 encoding.
typedef GenericStringStream<UTF8<> > StringStream;

///////////////////////////////////////////////////////////////////////////////
// InsituStringStream

//! A read-write string stream.
/*! This string stream is particularly designed for in-situ parsing.
    \note implements Stream concept
*/
template <typename Encoding>
struct GenericInsituStringStream {
    typedef typename Encoding::Ch Ch;

    GenericInsituStringStream(Ch *src) : src_(src), dst_(0), head_(src) {}

    // Read
    Ch Peek() {
        return *src_;
    }
    Ch Take() {
        return *src_++;
    }
    size_t Tell() {
        return static_cast<size_t>(src_ - head_);
    }

    // Write
    void Put(Ch c) {
        RAPIDJSONXML_ASSERT(dst_ != 0);
        *dst_++ = c;
    }

    Ch* PutBegin() {
        return dst_ = src_;
    }
    size_t PutEnd(Ch* begin) {
        return static_cast<size_t>(dst_ - begin);
    }
    void Flush() {}

    Ch* Push(size_t count) {
        Ch* begin = dst_;
        dst_ += count;
        return begin;
    }
    void Pop(size_t count) {
        dst_ -= count;
    }

    Ch* src_;
    Ch* dst_;
    Ch* head_;
};

template <typename Encoding>
struct StreamTraits<GenericInsituStringStream<Encoding> > {
    enum { copyOptimization = 1 };
};

//! Insitu string stream with UTF8 encoding.
typedef GenericInsituStringStream<UTF8<> > InsituStringStream;

///////////////////////////////////////////////////////////////////////////////
// Type

//! Type of JSON value
enum Type {
    kNullType = 0,      //!< null
    kFalseType = 1,     //!< false
    kTrueType = 2,      //!< true
    kObjectType = 3,    //!< object
    kArrayType = 4,     //!< array
    kStringType = 5,    //!< string
    kNumberType = 6     //!< number
};

///////////////////////////////////////////////////////////////////////////////
// Attributes iterator lists (for StartObject and Opentag use)

template <typename Encoding, typename Allocator = MemoryPoolAllocator<> >
class GenericAttribute;

template <typename Encoding, typename Allocator = MemoryPoolAllocator<> >
struct GenericAttributeIteratorPair {
    typedef const GenericAttribute<Encoding, Allocator>* GenericAttributeIterator;
    GenericAttributeIterator begin;
    GenericAttributeIterator end;

    GenericAttributeIteratorPair() : begin(0), end(0) {};
    GenericAttributeIteratorPair(GenericAttributeIterator _begin, GenericAttributeIterator _end) : begin(_begin), end(_end) {};

    bool IsValid() const { return (begin && end); };
};

} // namespace rapidjsonxml

#endif // RAPIDJSONXML_RAPIDJSONXML_H_
