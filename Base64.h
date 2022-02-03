#pragma once
#include <DPLib.conf.h>

using __DP_LIB_NAMESPACE__::String;

String base64_encode(const unsigned char *src, size_t len);

char * base64_encode(const unsigned char *src, size_t len, size_t & olength);

inline String base64_encode(const String & str) {
	return base64_encode((unsigned char *)str.c_str(), str.size());
}

std::string base64_decode(const void* data, const size_t len);
char * base64_decode(const void* data, const size_t len,  size_t & olength);

inline String base64_decode(const String & str) {
	return base64_decode((void *) str.c_str(), str.size());
}

inline char * base64_decode(const String & str, size_t & olength) {
	return base64_decode((void *) str.c_str(), str.size(), olength);
}
