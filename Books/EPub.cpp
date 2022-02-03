#include "EPub.h"
#include <Log/Log.h>
#include <Parser/SmartParser.h>
#include "XMLParser.h"
#include <Converter/Converter.h>
#include <_Driver/Path.h>
#include "../www/textfill.h"
#include <string.h>
#include "../EventLooper.h"
#include "../PositionSaver.h"
#include "../UniCode.h"
#include "sha1.hpp"

using __DP_LIB_NAMESPACE__::startWithN;
using __DP_LIB_NAMESPACE__::SmartParser;
using __DP_LIB_NAMESPACE__::ConteinsKey;
using __DP_LIB_NAMESPACE__::Path;

#define SAVE_OPERATION(OP)\
	try {\
		OP;\
	} catch(__DP_LIB_NAMESPACE__::LineException e) { \
		DP_LOG_FATAL << e.toString(); \
		return; \
	} catch (std::exception e) { \
		DP_LOG_FATAL << e.what(); \
		return; \
	} catch(...) { \
		DP_LOG_FATAL << "Fail to open epub file"; \
		return; \
	}

XMLElement getElementByName(StreamXMLParser & parser, const String & name) {
	SmartParser p{"${name}.${all}"};
	String find = name;
	String all = "";
	if (p.Check(name)) {
		find = p.Get("name");
		all = p.Get("all");
	}
	while (!parser.isEnd()) {
		XMLElement el = parser.Next();
		if (el.name == find) {
			if (all.size() == 0)
				return el;
			StringStreamXMLParser par {el.value};
			return getElementByName(par, all);
		}
	}
	return XMLElement{};
}

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from)) != std::string::npos)
		str.replace(start_pos, from.length(), to);
}

EPub::EPub(const String & filename):Book(filename) {
	DP_LOG_DEBUG << "Try open file " << filename;
	SAVE_OPERATION(zip.open(filename);)

	if (zip.hasFile("mimetype")) {
		String txt = "";
		SAVE_OPERATION(txt = zip.getAsText("mimetype");)
		if (!startWithN(txt, "application/epub+zip")) {
			DP_LOG_WARNING << "Type of file is " << txt;
		}
	} else {
		DP_LOG_WARNING << "Epub has not file mimetype";
	}

	if (!zip.hasFile("META-INF/container.xml")) {
		DP_LOG_FATAL << "Epub has not file META-INF/container.xml";
		return;
	}


	String txt = "";
	SAVE_OPERATION(txt = zip.getAsText("META-INF/container.xml");)
	{
		StringStreamXMLParser parser{txt};
		XMLElement el = getElementByName(parser, "container.rootfiles.rootfile");
		if (el.name.size() == 0) {
			DP_LOG_FATAL << "EPub has not rootfile";
			return;
		}
		if (!ConteinsKey(el.atributes, "full-path")) {
			DP_LOG_FATAL << "Root file attribute in unknown format: " << el.XML();
			return;
		}
		{
			Path p{el.atributes["full-path"]};
			this->resource_basepath = p.GetFolder();
			this->root_path = p.GetFile();
		}

		SAVE_OPERATION(txt = zip.getAsText(el.atributes["full-path"]);)
	}
	{
		StringStreamXMLParser parser = StringStreamXMLParser{txt};
		txt = getElementByName(parser, "package").value;
	}

	StringStreamXMLParser parser = StringStreamXMLParser{txt};

	while (!parser.isEnd()) {
		XMLElement el = parser.Next();
		if (el.name == "metadata") {
			StringStreamXMLParser inner {el.value};
			while (!inner.isEnd()) {
				XMLElement e = inner.Next();
				if (e.name == "dc:title") {
					this->name = normalizeString(e.value);
				}
				if (e.name == "dc:identifier") {
					this->id = normalizeString(e.value);
				}
				/*
				 * <dc:creator>Анеко Юсаги</dc:creator>
					<dc:contributor>Минами Сэйра</dc:contributor>
					<dc:date>2020-06-09</dc:date>
				*/
				if (e.name == "meta") {
					if (e.atributes["name"] == "cover") {
						cover_id = e.atributes["content"];
					}
				}
				if (e.name == "dc:creator") {
					this->author = normalizeString(e.value);
				}
				if (e.name == "dc:description")
					this->information = e.value;
				if (e.name == "dc:date")
					this->published_date = e.value;
			}
		}
		if (this->id.size() == 0)
			this->id = SHA1::from_file(filename);


		if (el.name == "manifest") {
			StringStreamXMLParser inner {el.value};
			while (!inner.isEnd()) {
				XMLElement e = inner.Next();
				if (e.name == "item") {
					this->resources[e.atributes["id"]] = e.atributes["href"];
					if (cover_id.size() == 0 && startWithN(e.atributes["media-type"], "image")) {
						cover_id = e.atributes["id"];
					}
				}
			}
		}
		if (el.name == "spine") {
			this->navigation_file = el.atributes["toc"];
			StringStreamXMLParser inner {el.value};
			while (!inner.isEnd()) {
				XMLElement e = inner.Next();
				if (e.name == "itemref") {
					resource_position.push_back(e.atributes["idref"]);
				}
			}
		}
	}
	DP_LOG_TRACE << "Readed information about " << this->author << " - " << this->name << " (" << this->id << ")";
	{
		String path = this->cover_id;
		if (ConteinsKey(this->resources, path)) {
			path = this->resources[path];
		}
		Path p {this->resource_basepath};
		p.Append(path);
		path = p.Get();
		#ifdef DP_WIN
			replaceAll(path, "\\", "/");
		#endif
		DP_LOG_DEBUG << "Extract cover " << path;
		if (zip.hasFile(path)) {
			cover = zip.getAsBin(path, cover_size);
		}
	}
	zip.close();
	this->AfterLoadBook();
}


Request EPub::draw(Request req, const String & _path, MakeRequest make) {
	String path_0 = _path;
	String path;
	if (path_0 == "COVER") {
		DP_LOG_TRACE << "Get cover for book " << this->name << " (" << this->id << ")";
		String path = this->cover_id;
		if (ConteinsKey(this->resources, path)) {
			path_0 = this->resources[path];
		}
	}
	if (path_0.size() == 0) {
		if (!ConteinsKey(req->get, "epub")) {
			Position pos = PositionSaver::Get().getPosition(this->id);
			String url = "index.html?epub=.&goto=";
			url += pos.position;
			DP_LOG_TRACE << "Redirect user to page";
			return makeRedirect(req, url);
		}
		path_0 = "index.html";
	}
	if (ConteinsKey(this->resources, path_0)) {
		path = this->resources[path_0];
	}
	path = path_0;

	Path p {this->resource_basepath};
	p.Append(path);
	path = p.Get();
	#ifdef DP_WIN
		replaceAll(path, "\\", "/");
	#endif
	DP_LOG_DEBUG << "Request get " << path;
	zip.reopen();
	if (!EventLooper::Get().exists_shot(zip.Filename())) {
		EventLooper::Get().add_to_one_shot(zip.Filename(), [this]() {
			this->zip.close();
		});
	}
	if (zip.hasFile(path)) {
		Request req = make();
		unsigned long long size = 0;
		req->body= zip.getAsBin(path, size);
		req->body_length = size;
		return req;
	} else {
		DP_LOG_TRACE << "File not found in book. It is reader file?";
		Request resp = make();
		resp->body_length = 0;

		if (path_0 == "rootpath") {
			DP_LOG_TRACE << "Request rootpath";
			String txt = this->root_path;
			resp->body = new char[txt.size() + 1];
			strncpy(resp->body, txt.c_str(), txt.size());
			resp->body_length = txt.size();
			return resp;
		}

		Path _p {"epub"};
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
	}
		return HttpServer::generate404(req->method, req->host, req->path);
}
