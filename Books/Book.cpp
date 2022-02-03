#include "Book.h"
#include <cstring>
#include <_Driver/Path.h>
#include "../UniCode.h"
#include "../PositionSaver.h"
#include "../Finder.h"

using __DP_LIB_NAMESPACE__::Path;

char * Book::getCover(unsigned long long & size) {
	if (cover_size == 0 || cover == nullptr)
		return nullptr;
	char * res = new char [cover_size];
	memcpy(res, cover, cover_size);
	size = cover_size;
	return res;
}

Book::Book(const String & filename) :filepath(filename) {
	Path p { filename};
	p = Path{p.GetFolder()};
	folder_category = normalizeString(p.GetFile());
}

void Book::AfterLoadBook() {
	PositionSaver::Get().tryImportForFile(this->id, this->filepath);
	if (this->author.size() > 0)
		this->categories.push_back(author);
}

Request Book::download() {
	Request resp = makeRequest();
	resp->headers["Content-Type"] = getMimeType(filepath);
	Path p {filepath};
	resp->headers["Content-Disposition"] = "attachment; filename=\"" + p.GetFile() + "\"";
	resp->body_length = Files::fileSize(filepath);
	resp->body = new char[resp->body_length + 1];
	__DP_LIB_NAMESPACE__::Ifstream in;
	in.open(filepath, std::ios::binary);
	in.read(resp->body, resp->body_length);
	in.close();
	return resp;
}
