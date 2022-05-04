#include "PDF.h"
#include "sha1.hpp"
#include "../Finder.h"
#include "Galery.h"
#include "../Base64.h"
#include "XMLParser.h"
#include "../Zip/Zip.h"


#include <Log/Log.h>
#include <Converter/Converter.h>
#include <_Driver/Path.h>
#include "../www/textfill.h"
#include <string.h>
#include <Parser/SmartParser.h>
#include "../PositionSaver.h"
#include "../UniCode.h"

using __DP_LIB_NAMESPACE__::ConteinsKey;
using __DP_LIB_NAMESPACE__::Path;
using __DP_LIB_NAMESPACE__::Ifstream;
using __DP_LIB_NAMESPACE__::SmartParser;
using __DP_LIB_NAMESPACE__::OStrStream;
using __DP_LIB_NAMESPACE__::ConteinsElement;
using __DP_LIB_NAMESPACE__::endWith;

String toLower(const String & str);


void loopParser(String & folder, String & name, String & cover, String & author, StreamXMLParser * parser) {
	while (!parser->isEnd()) {
		XMLElement el = parser->Next();
		if (el.value.size() > 0) {
			StringStreamXMLParser p {el.value};
			loopParser(folder, name, cover, author, &p);
		}
		if (el.name == "folder")
			folder = el.value;
		if (el.name == "name")
			name = normalizeString(el.value);
		if (el.name == "cover")
			cover = el.value;
		if (el.name == "author")
			author = normalizeString(el.value);
	}
}

Galery::Galery(const String & filename):Book(filename) {
	FileStreamXMLParser parser{filename};
	parser.open();

	String name = "";
	String cover = "";
	String author = "";

	loopParser(folder, name, cover, author, &parser);
	if (folder.size() == 0) {
		Path p {filename.substr(0, filename.size() - String(".galery").size())};
		folder = p.Get();
		this->name = p.GetFile();

	}
	{
		Path p2 {folder};
		if (!p2.IsAbsolute()) {
			Path p3 {filename};
			p3 = Path{p3.GetFolder()};
			p3.Append(p2.Get());
			folder = p3.Get();
		}
	}
	if (!Files::isDir(folder))
		throw EXCEPTION("Fail to load galery from folder " + folder);

	{
		List<String> files = Files::GetFiles(folder);
		for (const String & file : files) {
			String f2 = toLower(file);
			// https://developer.mozilla.org/en-US/docs/Web/HTML/Element/img
			if (endWith(f2, ".jpg") || endWith(f2, ".jpeg") || endWith(f2, ".apng") || endWith(f2, ".jpe") || endWith(f2, ".gif") || endWith(f2, ".jif") || endWith(f2, ".png") || endWith(f2, ".webp") || endWith(f2, ".svg"))
				this->images.push_back(file);
		}

		if (this->images.size() == 0)
			throw EXCEPTION("Galery is empty");
		this->images.sort();
	}

	if (name.size() != 0)
		this->name = name;
	this->author = author;
	{
		SHA1 checksum;
		checksum.update(filename);
		if (!checksum.isFail())
			this->id = checksum.final();
	}
	{
		String cover_path = "";
		Path p {folder};
		if (cover.size() != 0)
			p.Append(cover);
		else
			p.Append(*images.begin());
		if (!p.IsFile())
			throw EXCEPTION("Fail to open cover " + p.Get());
		this->cover_size = Files::fileSize(p.Get());
		this->cover = new char[this->cover_size+1];
		Ifstream in;
		in.open(p.Get(), std::ios::binary);
		in.read(this->cover, this->cover_size);
		in.close();
		if (this->id.size() == 0)
			this->id = SHA1::from_file(p.Get());
	}
	this->AfterLoadBook();
}
void replaceAll(std::string& str, const std::string& from, const std::string& to);


Request Galery::draw(Request req, const String & _path, MakeRequest make) {
	String path_0 = _path;
	String path;
	if (path_0 == "COVER") {
		DP_LOG_TRACE << "Get cover for book " << this->filepath << " (" << this->id << ")";

		Request resp = makeRequest();
		resp->headers["Content-Type"] = getMimeType("img.jpg");
		resp->headers["Content-Disposition"] = "attachment; filename=\"img.jpg\"";
		resp->body_length = this->cover_size;
		resp->body = new char[resp->body_length + 1];
		memcpy(resp->body, this->cover, this->cover_size);
		return resp;
	}
	if (path_0.size() == 0 || path_0 == "index.html") {
		OStrStream out;
		unsigned int pos = 0;
		for (const String & file : this->images) {
			out << (pos == 0 ? "" : ",\n") << findFillText("galery/item_galery.txt", List<String> { file });
			pos ++;
		}
		Position position = PositionSaver::Get().getPosition(this->id);
		String html = findFillText("galery/index.txt", List<String> { position.position.size() == 0 ? "1" : position.position, out.str() } );
		Request resp = make();
		resp->headers["Content-Type"] = getMimeType("index.html");
		resp->body = new char[html.size() + 1];
		strncpy(resp->body, html.c_str(), html.size());
		resp->body_length = html.size();
		return resp;
	}

	if (ConteinsElement(this->images, path_0)) {
		Request resp = makeRequest();
		resp->headers["Content-Type"] = getMimeType(path_0);
		Path p {this->folder};
		p.Append(path_0);
		resp->headers["Content-Disposition"] = "attachment; filename=\"" + p.GetFile() + "\"";
		resp->body_length = Files::fileSize(p.Get());
		resp->body = new char[resp->body_length + 1];
		__DP_LIB_NAMESPACE__::Ifstream in;
		in.open(p.Get(), std::ios::binary);
		in.read(resp->body, resp->body_length);
		in.close();
		return resp;
	}

	DP_LOG_TRACE << "File not found in book. It is reader file?";
	Request resp = make();
	resp->body_length = 0;

	Path _p {"galery"};
	_p.Append(path_0);
	String p = _p.Get();
	#ifdef DP_WIN
		replaceAll(p, "\\", "/");
	#endif

	resp->headers["Content-Type"] = getMimeType(p);
	resp->body = findResourc(p, resp->body_length);
	if (resp->body_length > 0 && resp->body != nullptr) {
		return resp;
	} else {
		String txt = findText(p);
		if (txt.size() < 2)
			return HttpServer::generate404(req->method, req->host, req->path);
		resp->body = new char[txt.size() + 1];
		strncpy(resp->body, txt.c_str(), txt.size());
		resp->body_length = txt.size();
		return resp;
	}
	return HttpServer::generate404(req->method, req->host, req->path);
}

Request Galery::download() {
	ZipWriter zip;
	for (const String & file : this->images) {
		Path p {this->folder};
		p.Append(file);
		zip.addFile(p.Get(), file);
	}
	auto list = zip.save();
	Request resp = makeRequest();
	resp->headers["Content-Type"] = getMimeType("zip.zip");
	resp->headers["Content-Disposition"] = "attachment; filename=\"galery.zip\"";
	resp->body_length = list.size();
	resp->body = new char[resp->body_length + 1];
	for (unsigned int i = 0 ; i < list.size() ; i++)
		resp->body[i] = list[i];
	resp->body[resp->body_length] = 0;
	return resp;
}
