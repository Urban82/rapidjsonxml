#ifndef RAPIDJSONXML_MEMORYSTREAM_H_
#define RAPIDJSONXML_MEMORYSTREAM_H_

#include "rapidjsonxml.h"

namespace rapidjsonxml {

//! Represents an in-memory input byte stream.
/*!
	This class is mainly for being wrapped by EncodedInputStream or AutoUTFInputStream.

	It is similar to FileReadBuffer but the source is an in-memory buffer instead of a file.

	Differences between MemoryStream and StringStream:
	1. StringStream has encoding but MemoryStream is a byte stream.
	2. MemoryStream needs size of the source buffer and the buffer don't need to be null terminated. StringStream assume null-terminated string as source.
	3. MemoryStream supports Peek4() for encoding detection. StringStream is specified with an encoding so it should not have Peek4().
	\note implements Stream concept
*/
struct MemoryStream {
	typedef char Ch; // byte

	MemoryStream(const Ch *src, size_t size) : src_(src), begin_(src), end_(src + size), size_(size) {}

	Ch Peek() const { return *src_; }
	Ch Take() { return (src_ == end_) ? '\0' : *src_++; }
	size_t Tell() const { return static_cast<size_t>(src_ - begin_); }

	Ch* PutBegin() { RAPIDJSONXML_ASSERT(false); return 0; }
	void Put(Ch) { RAPIDJSONXML_ASSERT(false); }
	void Flush() { RAPIDJSONXML_ASSERT(false); }
	size_t PutEnd(Ch*) { RAPIDJSONXML_ASSERT(false); return 0; }

	// For encoding detection only.
	const Ch* Peek4() const {
		return Tell() + 4 <= size_ ? src_ : 0;
	}

	const Ch* src_;		//!< Current read position.
	const Ch* begin_;	//!< Original head of the string.
	const Ch* end_;		//!< End of stream.
	size_t size_;		//!< Size of the stream.
};

} // namespace rapidjsonxml

#endif // RAPIDJSONXML_MEMORYBUFFER_H_
