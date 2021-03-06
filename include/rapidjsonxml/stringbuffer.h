#ifndef RAPIDJSONXML_STRINGBUFFER_H_
#define RAPIDJSONXML_STRINGBUFFER_H_

#include "rapidjsonxml.h"
#include "internal/stack.h"

namespace rapidjsonxml {

//! Represents an in-memory output stream.
/*!
    \tparam Encoding Encoding of the stream.
    \tparam Allocator type for allocating memory buffer.
    \note implements Stream concept
*/
template <typename Encoding, typename Allocator = CrtAllocator>
struct GenericStringBuffer {
    typedef typename Encoding::Ch Ch;

    GenericStringBuffer(Allocator* allocator = 0, size_t capacity = kDefaultCapacity) : stack_(allocator, capacity) {}

    void Put(Ch c) {
        *stack_.template Push<Ch>() = c;
    }
    void Flush() {}

    void Clear() {
        stack_.Clear();
    }
    Ch* Push(size_t count) {
        return stack_.template Push<Ch>(count);
    }
    void Pop(size_t count) {
        stack_.template Pop<Ch>(count);
    }

    const Ch* GetString() const {
        // Push and pop a null terminator. This is safe.
        *stack_.template Push<Ch>() = '\0';
        stack_.template Pop<Ch>(1);

        return stack_.template Bottom<Ch>();
    }

    size_t GetSize() const {
        return stack_.GetSize();
    }

    static const size_t kDefaultCapacity = 256;
    mutable internal::Stack<Allocator> stack_;
};

//! String buffer with UTF8 encoding
typedef GenericStringBuffer<UTF8<> > StringBuffer;

//! Implement specialized version of PutN() with memset() for better performance.
template<>
inline void PutN(GenericStringBuffer<UTF8<> >& stream, char c, size_t n) {
    memset(stream.stack_.Push<char>(n), c, n * sizeof(c));
}

} // namespace rapidjsonxml

#endif // RAPIDJSONXML_STRINGBUFFER_H_
