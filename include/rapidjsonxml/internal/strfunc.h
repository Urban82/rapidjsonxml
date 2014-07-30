#ifndef RAPIDJSONXML_INTERNAL_STRFUNC_H_
#define RAPIDJSONXML_INTERNAL_STRFUNC_H_

namespace rapidjsonxml {
namespace internal {

//! Custom strlen() which works on different character types.
/*! \tparam Ch Character type (e.g. char, wchar_t, short)
    \param s Null-terminated input string.
    \return Number of characters in the string.
    \note This has the same semantics as strlen(), the return value is not number of Unicode codepoints.
*/
template <typename Ch>
inline SizeType StrLen(const Ch* s) {
    const Ch* p = s;
    while (*p) ++p;
    return SizeType(p - s);
}

} // namespace internal
} // namespace rapidjsonxml

#endif // RAPIDJSONXML_INTERNAL_STRFUNC_H_
