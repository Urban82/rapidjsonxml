#ifndef RAPIDJSONXML_PRETTYWRITER_H_
#define RAPIDJSONXML_PRETTYWRITER_H_

#include "writerjson.h"

#ifdef __GNUC__
RAPIDJSONXML_DIAG_PUSH
RAPIDJSONXML_DIAG_OFF(effc++)
#endif

namespace rapidjsonxml {

//! WriterJson with indentation and spacing.
/*!
    \tparam OutputStream Type of ouptut os.
    \tparam SourceEncoding Encoding of source string.
    \tparam TargetEncoding Encoding of output stream.
    \tparam Allocator Type of allocator for allocating memory of stack.
*/
template<typename OutputStream, typename SourceEncoding = UTF8<>, typename TargetEncoding = UTF8<>, typename Allocator = MemoryPoolAllocator<> >
class PrettyWriterJson : public WriterJson<OutputStream, SourceEncoding, TargetEncoding, Allocator> {
public:
    typedef WriterJson<OutputStream, SourceEncoding, TargetEncoding, Allocator> Base;
    typedef typename Base::Ch Ch;

    //! Constructor
    /*! \param os Output stream.
        \param allocator User supplied allocator. If it is null, it will create a private one.
        \param levelDepth Initial capacity of stack.
    */
    PrettyWriterJson(OutputStream& os, Allocator* allocator = 0, size_t levelDepth = Base::kDefaultLevelDepth) :
        Base(os, allocator, levelDepth), indentChar_(' '), indentCharCount_(4) {}

    //! Overridden for fluent API, see \ref WriterJson::SetDoublePrecision()
    PrettyWriterJson& SetDoublePrecision(int p) {
        Base::SetDoublePrecision(p);
        return *this;
    }

    //! Set custom indentation.
    /*! \param indentChar       Character for indentation. Must be whitespace character (' ', '\\t', '\\n', '\\r').
        \param indentCharCount  Number of indent characters for each indentation level.
        \note The default indentation is 4 spaces.
    */
    PrettyWriterJson& SetIndent(Ch indentChar, unsigned indentCharCount) {
        RAPIDJSONXML_ASSERT(indentChar == ' ' || indentChar == '\t' || indentChar == '\n' || indentChar == '\r');
        indentChar_ = indentChar;
        indentCharCount_ = indentCharCount;
        return *this;
    }

    /*! @name Implementation of Handler
        \see Handler
    */
    //@{

    bool Null() {
        PrettyPrefix(kNullType);
        return Base::WriteNull();
    }
    bool Bool(bool b) {
        PrettyPrefix(b ? kTrueType : kFalseType);
        return Base::WriteBool(b);
    }
    bool Int(int i) {
        PrettyPrefix(kNumberType);
        return Base::WriteInt(i);
    }
    bool Uint(unsigned u) {
        PrettyPrefix(kNumberType);
        return Base::WriteUint(u);
    }
    bool Int64(int64_t i64) {
        PrettyPrefix(kNumberType);
        return Base::WriteInt64(i64);
    }
    bool Uint64(uint64_t u64) {
        PrettyPrefix(kNumberType);
        return Base::WriteUint64(u64);
    }
    bool Double(double d) {
        PrettyPrefix(kNumberType);
        return Base::WriteDouble(d);
    }

    bool String(const Ch* str, SizeType length, bool copy = false) {
        (void)copy;
        PrettyPrefix(kStringType);
        return Base::WriteString(str, length);
    }

    bool StartObject() {
        PrettyPrefix(kObjectType);
        new (Base::level_stack_.template Push<typename Base::Level>()) typename Base::Level(false);
        return Base::WriteStartObject();
    }

    bool EndObject(SizeType memberCount = 0) {
        (void)memberCount;
        RAPIDJSONXML_ASSERT(Base::level_stack_.GetSize() >= sizeof(typename Base::Level));
        RAPIDJSONXML_ASSERT(!Base::level_stack_.template Top<typename Base::Level>()->inArray);
        bool empty = Base::level_stack_.template Pop<typename Base::Level>(1)->valueCount == 0;

        if (!empty) {
            Base::os_->Put('\n');
            WriteIndent();
        }
        if (!Base::WriteEndObject())
            return false;
        if (Base::level_stack_.Empty()) // end of json text
            Base::os_->Flush();
        return true;
    }

    bool StartArray() {
        PrettyPrefix(kArrayType);
        new (Base::level_stack_.template Push<typename Base::Level>()) typename Base::Level(true);
        return Base::WriteStartArray();
    }

    bool EndArray(SizeType memberCount = 0) {
        (void)memberCount;
        RAPIDJSONXML_ASSERT(Base::level_stack_.GetSize() >= sizeof(typename Base::Level));
        RAPIDJSONXML_ASSERT(Base::level_stack_.template Top<typename Base::Level>()->inArray);
        bool empty = Base::level_stack_.template Pop<typename Base::Level>(1)->valueCount == 0;

        if (!empty) {
            Base::os_->Put('\n');
            WriteIndent();
        }
        if (!Base::WriteEndArray())
            return false;
        if (Base::level_stack_.Empty()) // end of json text
            Base::os_->Flush();
        return true;
    }

    bool OpenTag(const Ch* str, SizeType length, bool copy = false) {
        (void)copy;
        PrettyPrefix(kStringType);
        if(!Base::WriteString(str, length))
            return false;
        Base::os_->Put(':');
        return true;
    }
    //@}

    /*! @name Convenience extensions */
    //@{

    //! Simpler but slower overload.
    bool String(const Ch* str) {
        return String(str, internal::StrLen(str));
    }

    //! Overridden for fluent API, see \ref WriterJson::Double()
    bool Double(double d, int precision) {
        int oldPrecision = Base::GetDoublePrecision();
        SetDoublePrecision(precision);
        bool ret = Double(d);
        SetDoublePrecision(oldPrecision);
        return ret;
    }

    //@}
protected:
    void PrettyPrefix(Type type) {
        (void)type;
        if (Base::level_stack_.GetSize() != 0) { // this value is not at root
            typename Base::Level* level = Base::level_stack_.template Top<typename Base::Level>();

            if (level->inArray) {
                if (level->valueCount > 0) {
                    Base::os_->Put(','); // add comma if it is not the first element in array
                    Base::os_->Put('\n');
                }
                else
                    Base::os_->Put('\n');
                WriteIndent();
            }
            else { // in object
                if (level->valueCount > 0) {
                    if (level->valueCount % 2 == 0) {
                        Base::os_->Put(',');
                        Base::os_->Put('\n');
                    }
                    else {
                        Base::os_->Put(' ');
                    }
                }
                else
                    Base::os_->Put('\n');

                if (level->valueCount % 2 == 0)
                    WriteIndent();
            }
            if (!level->inArray && level->valueCount % 2 == 0)
                RAPIDJSONXML_ASSERT(type == kStringType); // if it's in object, then even number should be a name
            level->valueCount++;
        }
        else {
            RAPIDJSONXML_ASSERT(type == kObjectType || type == kArrayType);
            RAPIDJSONXML_ASSERT(!Base::hasRoot_); // Should only has one and only one root.
            Base::hasRoot_ = true;
        }
    }

    void WriteIndent()  {
        size_t count = (Base::level_stack_.GetSize() / sizeof(typename Base::Level)) * indentCharCount_;
        PutN(*Base::os_, indentChar_, count);
    }

    Ch indentChar_;
    unsigned indentCharCount_;

private:
    // Prohibit copy constructor & assignment operator.
    PrettyWriterJson(const PrettyWriterJson&);
    PrettyWriterJson& operator=(const PrettyWriterJson&);
};

} // namespace rapidjsonxml

#ifdef __GNUC__
RAPIDJSONXML_DIAG_POP
#endif

#endif // RAPIDJSONXML_RAPIDJSONXML_H_
