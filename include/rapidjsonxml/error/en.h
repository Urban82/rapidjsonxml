#ifndef RAPIDJSONXML_ERROR_EN_H__
#define RAPIDJSONXML_ERROR_EN_H__

#include "error.h"

namespace rapidjsonxml {

//! Maps error code of parsing into error message.
/*!
	\param parseErrorCode Error code obtained in parsing.
	\return the error message.
	\note User can make a copy of this function for localization.
		Using switch-case is safer for future modification of error codes.
*/
inline const RAPIDJSONXML_ERROR_CHARTYPE* GetParseError_En(ParseErrorCode parseErrorCode) {
	switch (parseErrorCode) {
		case kParseErrorNone:							return RAPIDJSONXML_ERROR_STRING("No error.");

		case kParseErrorDocumentEmpty:					return RAPIDJSONXML_ERROR_STRING("The document is empty.");
		case kParseErrorDocumentRootNotObjectOrArray:	return RAPIDJSONXML_ERROR_STRING("The document root must be either object or array.");
		case kParseErrorDocumentRootNotSingular:		return RAPIDJSONXML_ERROR_STRING("The document root must not follow by other values.");
	
		case kParseErrorValueInvalid:					return RAPIDJSONXML_ERROR_STRING("Invalid value.");
	
		case kParseErrorObjectMissName:					return RAPIDJSONXML_ERROR_STRING("Missing a name for object member.");
		case kParseErrorObjectMissColon:				return RAPIDJSONXML_ERROR_STRING("Missing a colon after a name of object member.");
		case kParseErrorObjectMissCommaOrCurlyBracket:	return RAPIDJSONXML_ERROR_STRING("Missing a comma or '}' after an object member.");
	
		case kParseErrorArrayMissCommaOrSquareBracket:	return RAPIDJSONXML_ERROR_STRING("Missing a comma or ']' after an array element.");

		case kParseErrorStringUnicodeEscapeInvalidHex:	return RAPIDJSONXML_ERROR_STRING("Incorrect hex digit after \\u escape in string.");
		case kParseErrorStringUnicodeSurrogateInvalid:	return RAPIDJSONXML_ERROR_STRING("The surrogate pair in string is invalid.");
		case kParseErrorStringEscapeInvalid:			return RAPIDJSONXML_ERROR_STRING("Invalid escape character in string.");
		case kParseErrorStringMissQuotationMark:		return RAPIDJSONXML_ERROR_STRING("Missing a closing quotation mark in string.");
		case kParseErrorStringInvalidEncoding:			return RAPIDJSONXML_ERROR_STRING("Invalid encoding in string.");

		case kParseErrorNumberTooBig:					return RAPIDJSONXML_ERROR_STRING("Number too big to be stored in double.");
		case kParseErrorNumberMissFraction:				return RAPIDJSONXML_ERROR_STRING("Miss fraction part in number.");
		case kParseErrorNumberMissExponent:				return RAPIDJSONXML_ERROR_STRING("Miss exponent in number.");

		case kParseErrorTermination:					return RAPIDJSONXML_ERROR_STRING("Terminate parsing due to Handler error.");
		case kParseErrorUnspecificSyntaxError:			return RAPIDJSONXML_ERROR_STRING("Unspecific syntax error.");

		default:
			return RAPIDJSONXML_ERROR_STRING("Unknown error.");
	}
}

} // namespace rapidjsonxml

#endif // RAPIDJSONXML_ERROR_EN_H__
