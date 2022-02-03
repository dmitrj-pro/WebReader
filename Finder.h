#pragma once

#include <list>
#include <DPLib.conf.h>

using std::list;
using __DP_LIB_NAMESPACE__::String;
using __DP_LIB_NAMESPACE__::Int;

namespace Files {
	list <String> GetFiles(const String & filder);
	bool isDir(const String & folder);

	bool MkDir(const String & folder);
	Int fileSize(const String & file);

	bool CopyFile(const String & from, const String & to);

	list<String> SetFilter(const list<String> & data, const String& filter = "${name}.ovpn");

	String GetPath(const String & file);
	String GetName(const String & file);
	bool RemoveFile(const String & path);
	bool RemoveDir(const String & path);
	unsigned int fileModTime(const String & path);
}
