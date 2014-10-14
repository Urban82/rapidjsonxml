#ifndef RAPIDJSONXML_WRITERXML_H_
#define RAPIDJSONXML_WRITERXML_H_

#include "rapidjsonxml.h"
#include "internal/stack.h"
#include "internal/strfunc.h"
#include "internal/itoa.h"
#include "stringbuffer.h"
#include <cstdio>   // snprintf() or _sprintf_s()
#include <new>      // placement new

#ifdef _MSC_VER
RAPIDJSONXML_DIAG_PUSH
RAPIDJSONXML_DIAG_OFF(4127) // conditional expression is constant
#endif

namespace rapidjsonxml {

//! XML writer
/*! WriterXml implements the concept Handler.
    It generates XML text by events to an output os.

    User may programmatically calls the functions of a writer to generate XML text.

    On the other side, a writer can also be passed to objects that generates events,

    for example Reader::Parse() and Document::Accept().

    \tparam OutputStream Type of output stream.
    \tparam SourceEncoding Encoding of source string.
    \tparam TargetEncoding Encoding of output stream.
    \tparam Allocator Type of allocator for allocating memory of stack.
    \note implements Handler concept
*/
template<typename OutputStream, typename SourceEncoding = UTF8<>, typename TargetEncoding = UTF8<>, typename Allocator = MemoryPoolAllocator<> >
class WriterXml {
public:
    typedef typename SourceEncoding::Ch Ch;
    typedef typename GenericValue<SourceEncoding, Allocator>::ConstAttributeIterator ConstAttributeIterator;
    typedef GenericAttributeIteratorPair<SourceEncoding, Allocator> AttributeIteratorPair;
    typedef AttributeIteratorPair* AttributeIteratorPairList;

    //! Constructor
    /*! \param os Output stream.
        \param allocator User supplied allocator. If it is null, it will create a private one.
        \param levelDepth Initial capacity of stack.
    */
    WriterXml(OutputStream& os, Allocator* allocator = 0, size_t levelDepth = kDefaultLevelDepth) :
        os_(&os), level_stack_(allocator, levelDepth * sizeof(Level)),
        doublePrecision_(kDefaultDoublePrecision), hasRoot_(false),
        lastTag(0), lastTagSize(0), lastAttrib() {}

    WriterXml(Allocator* allocator = 0, size_t levelDepth = kDefaultLevelDepth) :
        os_(0), level_stack_(allocator, levelDepth * sizeof(Level)),
        doublePrecision_(kDefaultDoublePrecision), hasRoot_(false),
        lastTag(0), lastTagSize(0), lastAttrib() {}

    virtual ~WriterXml() {
        delete lastTag;
        lastTag = 0;
        lastTagSize = 0;
        lastAttrib = AttributeIteratorPair();
    }

    //! Reset the writer with a new stream.
    /*!
        This function reset the writer with a new stream and default settings,
        in order to make a WriterXml object reusable for output multiple XMLs.

        \param os New output stream.
        \code
        WriterXml<OutputStream> writer(os1);
        writer.StartObject();
        // ...
        writer.EndObject();

        writer.Reset(os2);
        writer.StartObject();
        // ...
        writer.EndObject();
        \endcode
    */
    void Reset(OutputStream& os) {
        os_ = &os;
        doublePrecision_ = kDefaultDoublePrecision;
        hasRoot_ = false;
        level_stack_.Clear();

        delete lastTag;
        lastTag = 0;
        lastTagSize = 0;

        lastAttrib = AttributeIteratorPair();
    }

    //! Checks whether the output is a complete XML.
    /*!
        A complete XML has a complete root object or array.
    */
    bool IsComplete() const {
        return hasRoot_ && level_stack_.Empty();
    }

    //! Set the number of significant digits for \c double values
    /*! When writing a \c double value to the \c OutputStream, the number
        of significant digits is limited to 6 by default.
        \param p maximum number of significant digits (default: 6)
        \return The WriterXml itself for fluent API.
    */
    WriterXml& SetDoublePrecision(int p = kDefaultDoublePrecision) {
        if (p < 0) p = kDefaultDoublePrecision; // negative precision is ignored
        doublePrecision_ = p;
        return *this;
    }

    //! \see SetDoublePrecision()
    int GetDoublePrecision() const {
        return doublePrecision_;
    }

    /*!@name Implementation of Handler
        \see Handler
    */
    //@{

    bool Null() {
        Prefix(kNullType);
        return WriteNull();
    }
    bool Bool(bool b) {
        Prefix(b ? kTrueType : kFalseType);
        return WriteBool(b);
    }
    bool Int(int i) {
        Prefix(kNumberType);
        return WriteInt(i);
    }
    bool Uint(unsigned u) {
        Prefix(kNumberType);
        return WriteUint(u);
    }
    bool Int64(int64_t i64) {
        Prefix(kNumberType);
        return WriteInt64(i64);
    }
    bool Uint64(uint64_t u64) {
        Prefix(kNumberType);
        return WriteUint64(u64);
    }

    //! Writes the given \c double value to the stream
    /*!
        The number of significant digits (the precision) to be written
        can be set by \ref SetDoublePrecision() for the WriterXml:
        \code
        WriterXml<...> writer(...);
        writer.SetDoublePrecision(12).Double(M_PI);
        \endcode
        \param d The value to be written.
        \return Whether it is succeed.
    */
    bool Double(double d) {
        Prefix(kNumberType);
        return WriteDouble(d);
    }

    bool String(const Ch* str, SizeType length, bool copy = false) {
        (void)copy;
        Prefix(kStringType);
        return WriteString(str, length);
    }

    bool StartObject(const AttributeIteratorPair attribs) {
        Prefix(kObjectType, attribs);
        new (level_stack_.template Push<Level>()) Level(false);
        return true;
    }

    bool EndObject(SizeType memberCount = 0) {
        (void)memberCount;
        RAPIDJSONXML_ASSERT(level_stack_.GetSize() >= sizeof(Level));
        RAPIDJSONXML_ASSERT(!level_stack_.template Top<Level>()->inArray);
        level_stack_.template Pop<Level>(1);
        if (level_stack_.Empty()) // end of json text
            os_->Flush();
        return true;
    }

    bool StartArray() {
        Prefix(kArrayType);
        Level* level = new (level_stack_.template Push<Level>()) Level(true);
        // Copy last tag reference
        level->tag = lastTag;
        level->tagSize = lastTagSize;
        level->attrib = lastAttrib;
        // Reset last tag reference
        lastTag = 0;
        lastTagSize = 0;
        lastAttrib = AttributeIteratorPair();
        return true;
    }

    bool EndArray(SizeType elementCount = 0) {
        (void)elementCount;
        RAPIDJSONXML_ASSERT(level_stack_.GetSize() >= sizeof(Level));
        RAPIDJSONXML_ASSERT(level_stack_.template Top<Level>()->inArray);
        level_stack_.template Pop<Level>(1);
        if (level_stack_.Empty()) // end of json text
            os_->Flush();
        return true;
    }

    bool OpenTag(const Ch* str, SizeType length, const AttributeIteratorPairList attribs_list, bool copy = false) {
        (void)copy;
        os_->Put('<');
        if(!WriteString(str, length))
            return false;
        if (attribs_list) {
            AttributeIteratorPairList a = attribs_list;
            while (a->IsValid()) {
                for (ConstAttributeIterator it = a->begin; it != a->end; ++it) {
                    os_->Put(' ');
                    WriteString(it->GetName(), it->GetNameLength());
                    os_->Put('=');
                    os_->Put('"');
                    WriteString(it->GetValue(), it->GetValueLength());
                    os_->Put('"');
                }
                ++a;
            }
        }
        os_->Put('>');

        // Save last tag
        delete lastTag;
        lastTag = strndup(str, length);
        lastTagSize = length;
        if (attribs_list) {
            lastAttrib = *attribs_list;
        }
        else {
            lastAttrib = AttributeIteratorPair();
        }

        return true;
    }

    bool CloseTag(const Ch* str, SizeType length, bool copy = false) {
        (void)copy;
        os_->Put('<'); os_->Put('/');
        if(!WriteString(str, length))
            return false;
        os_->Put('>');
        return true;
    }
    //@}

    /*! @name Convenience extensions */
    //@{

    //! Writes the given \c double value to the stream (explicit precision)
    /*!
        The currently set double precision is ignored in favor of the explicitly
        given precision for this value.
        \see Double(), SetDoublePrecision(), GetDoublePrecision()
        \param d The value to be written
        \param precision The number of significant digits for this value
        \return Whether it is succeeded.
    */
    bool Double(double d, int precision) {
        int oldPrecision = GetDoublePrecision();
        SetDoublePrecision(precision);
        bool ret = Double(d);
        SetDoublePrecision(oldPrecision);
        return ret;
    }

    //! Simpler but slower overload.
    bool String(const Ch* str) {
        return String(str, internal::StrLen(str));
    }

    //@}

protected:
    //! Information for each nested level
    struct Level {
        Level(bool inArray_) : valueCount(0), inArray(inArray_), tag(0), tagSize(0), attrib() {}
        ~Level() { delete tag; }
        size_t valueCount;  //!< number of values in this level
        bool inArray;       //!< true if in array, otherwise in object
        const Ch* tag;
        SizeType tagSize;
        AttributeIteratorPair attrib;
    };

    static const size_t kDefaultLevelDepth = 32;

    bool WriteNull()  {
        os_->Put('n');
        os_->Put('u');
        os_->Put('l');
        os_->Put('l');
        return true;
    }

    bool WriteBool(bool b)  {
        if (b) {
            os_->Put('t');
            os_->Put('r');
            os_->Put('u');
            os_->Put('e');
        }
        else {
            os_->Put('f');
            os_->Put('a');
            os_->Put('l');
            os_->Put('s');
            os_->Put('e');
        }
        return true;
    }

    bool WriteInt(int i) {
        char buffer[11];
        const char* end = internal::i32toa(i, buffer);
        for (const char* p = buffer; p != end; ++p)
            os_->Put(*p);
        return true;
    }

    bool WriteUint(unsigned u) {
        char buffer[10];
        const char* end = internal::u32toa(u, buffer);
        for (const char* p = buffer; p != end; ++p)
            os_->Put(*p);
        return true;
    }

    bool WriteInt64(int64_t i64) {
        char buffer[21];
        const char* end = internal::i64toa(i64, buffer);
        for (const char* p = buffer; p != end; ++p)
            os_->Put(*p);
        return true;
    }

    bool WriteUint64(uint64_t u64) {
        char buffer[20];
        const char* end = internal::u64toa(u64, buffer);
        for (const char* p = buffer; p != end; ++p)
            os_->Put(*p);
        return true;
    }

#ifdef _MSC_VER
#define RAPIDJSONXML_SNPRINTF sprintf_s
#else
#define RAPIDJSONXML_SNPRINTF snprintf
#endif

    //! \todo Optimization with custom double-to-string converter.
    bool WriteDouble(double d) {
        char buffer[100];
        int ret = RAPIDJSONXML_SNPRINTF(buffer, sizeof(buffer), "%.*g", doublePrecision_, d);
        RAPIDJSONXML_ASSERT(ret >= 1);
        for (int i = 0; i < ret; i++)
            os_->Put(buffer[i]);
        return true;
    }
#undef RAPIDJSONXML_SNPRINTF

    bool WriteString(const Ch* str, SizeType length)  {
        static const char hexDigits[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
        static const char escape[256] = {
#define Z16 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
#define E16 '#','#','#','#','#','#','#','#','#','#','#','#','#','#','#','#'
            //0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
            E16, E16,                                                                     // 00~1F
            0,   0,   0,   0,   0,   0, '#',   0,   0,   0,   0,   0,   0,   0,   0,   0, // 20
            Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16               // 30~FF
#undef Z16
#undef E16
        };

        GenericStringStream<SourceEncoding> is(str);
        while (is.Tell() < length) {
            const Ch c = is.Peek();
            if (!TargetEncoding::supportUnicode && (unsigned)c >= 0x80) {
                // Unicode escaping
                unsigned codepoint;
                if (!SourceEncoding::Decode(is, &codepoint))
                    return false;
                os_->Put('&');
                os_->Put('#');
                os_->Put('x');
                if (codepoint <= 0xD7FF || (codepoint >= 0xE000 && codepoint <= 0xFFFF)) {
                    os_->Put(hexDigits[(codepoint >> 12) & 15]);
                    os_->Put(hexDigits[(codepoint >>  8) & 15]);
                    os_->Put(hexDigits[(codepoint >>  4) & 15]);
                    os_->Put(hexDigits[(codepoint      ) & 15]);
                }
                else if (codepoint >= 0x010000 && codepoint <= 0x10FFFF) {
                    // Surrogate pair
                    unsigned s = codepoint - 0x010000;
                    unsigned lead = (s >> 10) + 0xD800;
                    unsigned trail = (s & 0x3FF) + 0xDC00;
                    os_->Put(hexDigits[(lead >> 12) & 15]);
                    os_->Put(hexDigits[(lead >>  8) & 15]);
                    os_->Put(hexDigits[(lead >>  4) & 15]);
                    os_->Put(hexDigits[(lead      ) & 15]);
                    os_->Put(hexDigits[(trail >> 12) & 15]);
                    os_->Put(hexDigits[(trail >>  8) & 15]);
                    os_->Put(hexDigits[(trail >>  4) & 15]);
                    os_->Put(hexDigits[(trail      ) & 15]);
                }
                else
                    return false; // invalid code point
                os_->Put(';');
            }
            else if ((sizeof(Ch) == 1 || (unsigned)c < 256) && escape[(unsigned char)c])  {
                is.Take();
                os_->Put('&');
                os_->Put(escape[(unsigned char)c]);
                if (escape[(unsigned char)c] == '#') {
                    os_->Put('x');
                    os_->Put('0');
                    os_->Put('0');
                    os_->Put(hexDigits[(unsigned char)c >> 4]);
                    os_->Put(hexDigits[(unsigned char)c & 0xF]);
                }
                os_->Put(';');
            }
            else
                Transcoder<SourceEncoding, TargetEncoding>::Transcode(is, *os_);
        }
        return true;
    }

    void Prefix(Type type, const AttributeIteratorPair attribs = AttributeIteratorPair()) {
        (void)type;
        if (level_stack_.GetSize() != 0) { // this value is not at root
            Level* level = level_stack_.template Top<Level>();
            if (level->inArray && level->valueCount > 0) {
                CloseTag(level->tag, level->tagSize);

                AttributeIteratorPair attribs_list[3];
                attribs_list[0] = level->attrib;
                attribs_list[1] = attribs;

                OpenTag(level->tag, level->tagSize, attribs_list, false);
            }
            level->valueCount++;
        }
        else {
            RAPIDJSONXML_ASSERT(type == kObjectType || type == kArrayType);
            RAPIDJSONXML_ASSERT(!hasRoot_); // Should only has one and only one root.
            hasRoot_ = true;
        }
    }

    OutputStream* os_;
    internal::Stack<Allocator> level_stack_;
    int doublePrecision_;
    bool hasRoot_;

    static const int kDefaultDoublePrecision = 6;

    Ch* lastTag;
    SizeType lastTagSize;
    AttributeIteratorPair lastAttrib;

private:
    // Prohibit copy constructor & assignment operator.
    WriterXml(const WriterXml&);
    WriterXml& operator=(const WriterXml&);
};

// Full specialization for StringStream to prevent memory copying

template<>
inline bool WriterXml<StringBuffer>::WriteInt(int i) {
    char *buffer = os_->Push(11);
    const char* end = internal::i32toa(i, buffer);
    os_->Pop(11 - (end - buffer));
    return true;
}

template<>
inline bool WriterXml<StringBuffer>::WriteUint(unsigned u) {
    char *buffer = os_->Push(10);
    const char* end = internal::u32toa(u, buffer);
    os_->Pop(10 - (end - buffer));
    return true;
}

template<>
inline bool WriterXml<StringBuffer>::WriteInt64(int64_t i64) {
    char *buffer = os_->Push(21);
    const char* end = internal::i64toa(i64, buffer);
    os_->Pop(21 - (end - buffer));
    return true;
}

template<>
inline bool WriterXml<StringBuffer>::WriteUint64(uint64_t u) {
    char *buffer = os_->Push(20);
    const char* end = internal::u64toa(u, buffer);
    os_->Pop(20 - (end - buffer));
    return true;
}

} // namespace rapidjsonxml

#ifdef _MSC_VER
RAPIDJSONXML_DIAG_POP
#endif

#endif // RAPIDJSONXML_WRITERXML_H_
