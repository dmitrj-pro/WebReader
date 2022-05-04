#include "PDF.h"
#include "sha1.hpp"
#include "../Finder.h"
#include "../Base64.h"


#include <Log/Log.h>
#include <Converter/Converter.h>
#include <_Driver/Path.h>
#include "../www/textfill.h"
#include <string.h>
#include "../PositionSaver.h"
#include "../UniCode.h"

using __DP_LIB_NAMESPACE__::ConteinsKey;
using __DP_LIB_NAMESPACE__::Path;
using __DP_LIB_NAMESPACE__::Ifstream;


PDF::PDF(const String & filename):Book(filename) {
	this->id = SHA1::from_file(filename);
	this->name = Path(filename).GetFile();

	Path p {PositionSaver::Get().getCacheDir()};
	p.Append(this->name + ".jpg");
	if (p.IsFile()) {
		this->cover_size = Files::fileSize(p.Get());
		this->cover = new char[this->cover_size+1];
		Ifstream in;
		in.open(p.Get(), std::ios::binary);
		in.read(this->cover, this->cover_size);
		in.close();
	} else {
		Path fileCover = Path{PositionSaver::Get().getCacheDir()};
		fileCover.Append(this->id + "-000001.png");
		if (fileCover.IsFile()) {
			this->cover_size = Files::fileSize(fileCover.Get());
			this->cover = new char[this->cover_size+1];
			Ifstream in;
			in.open(fileCover.Get(), std::ios::binary);
			in.read(this->cover, this->cover_size);
			in.close();
		}
	}
	this->AfterLoadBook();
}
void replaceAll(std::string& str, const std::string& from, const std::string& to);


Request PDF::draw(Request req, const String & _path, MakeRequest make) {
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
	if (path_0 == "SETCOVER") {
		if (ConteinsKey(req->post, "img")) {
			String data = req->post["img"].value;
			auto pos = data.find("base64,");
			if (pos == data.npos)
				return HttpServer::generate404(req->method, req->host, req->path);
			data = data.substr(pos + 7);
			size_t cov_size = 0;
			char * img = base64_decode(data, cov_size);
			if (img != nullptr) {
				if (img[cov_size - 5] == 0x0) {
					// Не получили полностью изображение.
					return HttpServer::generate404(req->method, req->host, req->path);
				}
				if (cov_size <= this->cover_size) {
					delete [] img;
				} else {
					if (this->cover != nullptr)
						delete [] this->cover;
					this->cover = img;
					this->cover_size = cov_size;
					Path save { PositionSaver::Get().getCacheDir() };

					Path p {this->filepath + ".jpg"};
					save.Append(p.GetFile());
					__DP_LIB_NAMESPACE__::Ofstream out;
					out.open(save.Get(), std::ios::binary);
					out.write(img, cov_size);
					out.close();
				}
			}
			return makeRequest();
		}
	}
	if (path_0.size() == 0) {
		if (!ConteinsKey(req->get, "file")) {
			String url = "index.html?file=DOC.pdf";
			DP_LOG_TRACE << "Redirect user to page";
			return makeRedirect(req, url);
		}
		path_0 = "viewer.html";
	}
	if (path_0 == "index.html")
		path_0 = "viewer.html";
	if (path_0 == "DOC.pdf") {
		Request resp = makeRequest();
		resp->headers["Content-Type"] = getMimeType(path_0);
		Path p {this->filepath};
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

	Path _p {"pdf/web"};
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
