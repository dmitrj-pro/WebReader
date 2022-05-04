#include "PDFtoPNG.h"
#include "sha1.hpp"
#include "../Finder.h"
#include "../Base64.h"
#include <_Driver/Files.h>
#include <Parser/SmartParser.h>
#include <Converter/Converter.h>


#include <Log/Log.h>
#include <Converter/Converter.h>
#include <_Driver/Path.h>
#include <_Driver/Application.h>
#include "../www/textfill.h"
#include <string.h>
#include "../PositionSaver.h"
#include "../UniCode.h"

using __DP_LIB_NAMESPACE__::ConteinsKey;
using __DP_LIB_NAMESPACE__::Path;
using __DP_LIB_NAMESPACE__::Ifstream;
using __DP_LIB_NAMESPACE__::OStrStream;
using __DP_LIB_NAMESPACE__::ConteinsElement;
using __DP_LIB_NAMESPACE__::Application;
using __DP_LIB_NAMESPACE__::startWith;


PDFtoPNG::PDFtoPNG(const String & filename):Book(filename) {
	if (!PositionSaver::Get().getEnablePDFtoPNG())
		return;

	this->id = SHA1::from_file(filename);
	this->name = Path{filename}.GetFile();


	// Сразу перекодируем PDF
	Path fileCover {PositionSaver::Get().getCacheDir()};
	fileCover.Append(this->id);

	List<String> files = Files::GetFiles(fileCover.GetFolder());
	for (const String & file : files) {
		if (startWith(file, this->id))
			this->files.push_back(file);
	}
	if (this->files.size() == 0) {
		DP_LOG_INFO << "Convert PDF " << filepath << " to png";
		Application app {PositionSaver::Get().getPDFtoPNGPath()};
		app << this->filepath;
		app << fileCover.Get();
		app.ExecAll();

		List<String> files = Files::GetFiles(fileCover.GetFolder());
		for (const String & file : files) {
			if (startWith(file, this->id))
				this->files.push_back(file);
		}
	}

	this->files.sort();
	if (this->files.size() > 0)
		extracted = true;

	fileCover = Path{fileCover.GetFolder()};
	fileCover.Append(this->id + "-000001.png");
	this->AfterLoadBook();

	if (fileCover.IsFile()) {
		this->cover_size = Files::fileSize(fileCover.Get());
		this->cover = new char[this->cover_size+1];
		Ifstream in;
		in.open(fileCover.Get(), std::ios::binary);
		in.read(this->cover, this->cover_size);
		in.close();
	}
}
void replaceAll(std::string& str, const std::string& from, const std::string& to);


Request PDFtoPNG::draw(Request req, const String & _path, MakeRequest make) {
	if (!this->extracted)
		return HttpServer::generate404(req->method, req->host, req->path);
	String path_0 = _path;
	String path;
	if (path_0 == "COVER") {
		DP_LOG_TRACE << "Get cover for book " << this->filepath << " (" << this->id << ")";

		Request resp = makeRequest();
		resp->headers["Content-Type"] = getMimeType("img.png");
		resp->headers["Content-Disposition"] = "attachment; filename=\"img.png\"";
		resp->body_length = this->cover_size;
		resp->body = new char[resp->body_length + 1];
		memcpy(resp->body, this->cover, this->cover_size);
		return resp;
	}
	if (!extracted)
		throw EXCEPTION("Fail: pdf not decoded");

	if (path_0.size() == 0 || path_0 == "index.html") {
		OStrStream out;
		unsigned int pos = 0;
		for (const String & file : this->files) {
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
	if (ConteinsElement(this->files, path_0)) {
		Request resp = makeRequest();
		resp->headers["Content-Type"] = getMimeType(path_0);
		Path p {PositionSaver::Get().getCacheDir()};
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
