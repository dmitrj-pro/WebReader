#include "UniCode.h"
#include "Base64.h"

//#include <iostream>


String toBase64(const String & str0) {
	std::u16string str = wstrFroStr(str0);
	char * data = new char [sizeof(char16_t) * str.size() + 2];
//std::cout << (str.size() + 1) << " " << str.size() << "\n";
	const char16_t * arr = str.c_str();
//	for (int i =0; i < str.size(); i++) {
//		std::cout << ((int) arr[i]) << " ";
//	}
//	std::cout << "\n";
	int pos = 0;
	for (unsigned int i = 0; i < str.size(); i++) {
		const char * t2 = (char *) (&(arr[i]));
		for (unsigned int j =0; j< sizeof(char16_t); j++) {
			data[pos++] = t2[j];
		}
	}
//	for (int i = 0; i < sizeof(char16_t) * str.size(); i++) {
//		std::cout << ((int) data[i]) << " ";
//	}
//	std::cout << "\n";
	String res = base64_encode((unsigned char * )data, sizeof(char16_t) * str.size());
	delete [] data;
	return res;
}

String fromBase64(const String & str){
	String data = base64_decode(str);
	char16_t * res = new char16_t[data.size()/sizeof(char16_t) + 2];
	for (unsigned int i = 0; i < data.size(); i++) {
		char * e2 = (char *) (&res[i/sizeof(char16_t)]);
		e2[i % sizeof(char16_t)] = data[i];
		
	}
	res[(data.size()/sizeof(char16_t))] = 0;
	std::u16string tt = res;
//std::cout << tt.size() << (data.size()/sizeof(char16_t)) << "\n";
	String ret = strFromWStr(tt);
	delete []res;
	return ret;
}
