#include "WebReader.h"
#include <Parser/SmartParser.h>
#include "www/textfill.h"
#include <string.h>
#include "Finder.h"
#include <Converter/Converter.h>
#include "Books/EPub.h"
#include "Books/PDF-Wrapper.h"
#include "Books/Galery.h"
#include "UniCode.h"
#include <_Driver/Path.h>

using __DP_LIB_NAMESPACE__::startWithN;
using __DP_LIB_NAMESPACE__::endWithN;
using __DP_LIB_NAMESPACE__::SmartParser;
using __DP_LIB_NAMESPACE__::Path;
using __DP_LIB_NAMESPACE__::ConteinsKey;
using __DP_LIB_NAMESPACE__::OStrStream;
using __DP_LIB_NAMESPACE__::Vector;
using __DP_LIB_NAMESPACE__::parse;
using __DP_LIB_NAMESPACE__::toString;

void WebReader::startWebServer() {
	List<String> folders;
	folders.push_back(this->base_path);
	while (folders.size() > 0) {
		String folder = folders.front();
		DP_LOG_DEBUG << "Watch folder " << folder;
		folders.pop_front();
		auto list = Files::GetFiles(folder);
		for (auto file : list) {
			Path p {folder};
			p.Append(file);
			if (Files::isDir(p.Get())) {
				folders.push_back(p.Get());
				continue;
			}
			if (endWithN(file, ".epub")) {
				Book * b = new EPub(p.Get());
				this->books[b->getId()] = b;
			}
			if (endWithN(file, ".pdf")) {
				Book * b = new PDFWrapper(p.Get());
				this->books[b->getId()] = b;
			}
			if (endWithN(file, ".galery")) {
				Book * b = new Galery(p.Get());
				this->books[b->getId()] = b;
			}
		}
	}
	DP_LOG_INFO << "All book loaded";

	srv.add_route("*", "/index.html", "GET", [this](Request r) { return this->mainPage(r); });
	srv.add_route("*", "/", "GET", [this](Request r) { return this->mainPage(r); });
	srv.add_route("*", "*", "POST", [this](Request r) { return this->defaultRoutePost(r); });
	srv.add_route("*", "*", "GET", [this](Request r) { return this->defaultRoute(r); });
	srv.listen(this->web_host, this->web_port);
}

Request WebReader::defaultRoutePost(Request req) {
	if (req->path.size() > 0) {
		if (__DP_LIB_NAMESPACE__::startWithN(req->path, "/book/"))
			return routeBookPost(req);
	} else
		return HttpServer::generate404(req->method, req->host, req->path);
}

Request WebReader::defaultRoute(Request req) {
	if (req->path.size() > 0) {
		if (__DP_LIB_NAMESPACE__::startWithN(req->path, "/book/"))
			return routeBook(req);
		if (__DP_LIB_NAMESPACE__::startWithN(req->path, "/info/"))
			return routeInfoBook(req);
		if (__DP_LIB_NAMESPACE__::startWithN(req->path, "/cover/")) {
			String id = req->path.substr(7);
			id = id.substr(0, id.size() - 4);
			DP_LOG_TRACE << "Get cover for " << id;
			if (!ConteinsKey(this->books, id))
				return HttpServer::generate404(req->method, req->host, req->path);
			Request resp = makeRequest();
			unsigned long long l;
			resp->body = this->books[id]->getCover(l);
			resp->body_length = l;
			String type = getMimeType(req->path);
			resp->headers["Content-Type"] = type;
			return resp;
		}
		if (__DP_LIB_NAMESPACE__::startWithN(req->path, "/download/")) {
			String id = req->path.substr(10);
			DP_LOG_TRACE << "Download book " << id;
			if (!ConteinsKey(this->books, id))
				return HttpServer::generate404(req->method, req->host, req->path);

			return this->books[id]->download();
		}
		Request resp = makeRequest();
		resp->body_length = 0;
		String filename = req->path.substr(1);

		resp->headers["Content-Type"] = getMimeType(filename);
		resp->body = findResourc(filename, resp->body_length);
		if (resp->body_length > 0 && resp->body != nullptr) {
			return resp;
		} else {
			String txt = findText(filename);
			if (txt.size() < 2)
				return HttpServer::generate404(req->method, req->host, req->path);
			resp->body = new char[txt.size() + 1];
			strncpy(resp->body, txt.c_str(), txt.size());
			resp->body_length = txt.size();
			return resp;
		}
	} else
		return HttpServer::generate404(req->method, req->host, req->path);
}

Request WebReader::routeBook(Request req) {
	String book_id = "";
	String page = "";
	{
		SmartParser parser {"/book/${id}/${page:null<string>}"};
		SmartParser parser2 {"/book/${id}"};
		SmartParser parser3 {"/book/${id}/POSITION/"};
		SmartParser parser4 {"/book/${id}/${type}/POSITION/"};
		if (parser2.Check(req->path)) {
			book_id = parser2.Get("id");
		}
		if (parser.Check(req->path)) {
			book_id = parser.Get("id");
			page = parser.Get("page");
		}

		if (parser4.Check(req->path)) {
			if (ConteinsKey(req->get, "pos")) {
				DP_LOG_TRACE << "Set position for " << book_id << ": " << req->get["pos"];
				PositionSaver::Get().setPosition(parser4.Get("id"), req->get["pos"]);
				return makeRequest();
			}
			auto pos = PositionSaver::Get().getPosition(parser4.Get("id"));
			String html = "{ \"position\": \"" + pos.position + "\" }";
			Request resp = makeRequest();
			resp->body = new char[html.size() + 1];
			strncpy(resp->body, html.c_str(), html.size());
			resp->body_length = html.size();
			return resp;
		}
		if (parser3.Check(req->path)) {
			if (ConteinsKey(req->get, "pos")) {
				DP_LOG_TRACE << "Set position for " << book_id << ": " << req->get["pos"];
				PositionSaver::Get().setPosition(parser3.Get("id"), req->get["pos"]);
				return makeRequest();
			}
			auto pos = PositionSaver::Get().getPosition(parser3.Get("id"));
			String html = "{ \"position\": \"" + pos.position + "\" }";
			Request resp = makeRequest();
			resp->body = new char[html.size() + 1];
			strncpy(resp->body, html.c_str(), html.size());
			resp->body_length = html.size();
			return resp;
		}
	}
	DP_LOG_TRACE << "routeBook about " << book_id;
	if (ConteinsKey(this->books, book_id)) {
		return this->books[book_id]->draw(req, page, makeRequest);
	} else
		return HttpServer::generate404(req->method, req->host, req->path);
}

Request WebReader::routeBookPost(Request req) {
	String book_id = "";
	String page = "";
	{
		SmartParser parser {"/book/${id}/${page:null<string>}"};
		if (parser.Check(req->path)) {
			book_id = parser.Get("id");
			page = parser.Get("page");
		}
	}
	DP_LOG_TRACE << "routeBook about " << book_id;
	if (ConteinsKey(this->books, book_id)) {
		return this->books[book_id]->draw(req, page, makeRequest);
	} else
		return HttpServer::generate404(req->method, req->host, req->path);
}

Request WebReader::routeInfoBook(Request req) {
	String book_id = "";
	{
		SmartParser parser2 {"/info/${id}/"};
		if (parser2.Check(req->path))
			book_id = parser2.Get("id");
	}
	DP_LOG_TRACE << "routeInfoBook about " << book_id;

	if (book_id.size() == 0 || !ConteinsKey(this->books, book_id))
		return HttpServer::generate404(req->method, req->host, req->path);

	Book * b = this->books[book_id];

	String html = findFillText("template.txt", List<String>({"Books", makeMenu(), b->getName(), findFillText("info/index.txt", List<String>{
																 "cover/" + book_id + ".jpg",
																 b->getAuthor(),
																 b->getName(),
																 b->getPublishedDate(),
																 b->getId(),
																 b->getId(),
																 b->getInformation(),
																 b->getId()
															 }), ""}));
	Request resp = makeRequest();
	resp->body = new char[html.size() + 1];
	strncpy(resp->body, html.c_str(), html.size());
	resp->body_length = html.size();
	return resp;
}

Request WebReader::mainPage(Request req) {
	OStrStream vec;
	unsigned int pos = 0;
	String filter = "";
	if (ConteinsKey(req->get, "filter"))
		filter = req->get["filter"];
	UInt all_pages;
	UInt page_num = 0;
	UInt limit = 20;
	if (ConteinsKey(req->get, "limit"))
		limit = parse<UInt>(req->get["limit"]);
	if (ConteinsKey(req->get, "page"))
		page_num = parse<UInt>(req->get["page"]);
	auto sorted = getBooksSortedByLastDate(all_pages, page_num, limit, filter);

	for (auto it : sorted) {
		vec << findFillText("list/element.txt", List<String>{
							  it.book->getAuthor().size() == 0 ? "..." : it.book->getAuthor(),
							  it.id,
							  it.book->getName(),
							  it.book->hasCover() ? "cover/" + it.id + ".jpg" : "assets/img/books.jpg",
							  it.id,
							  it.id
						  });
		pos ++;
	}
	/*String encode = "utf-8";
	#ifdef DP_WIN
		//encode = "cp1251";
	#endif*/
	String pagination = "";
	{
		OStrStream pages;
		if (page_num != 0)
			pages << findFillText("list/pages_prev.txt", List<String> {
									 "?limit=" + toString(limit) + "&page=" + toString(page_num-1)  + (filter.size() > 0 ? ("&filter=" + filter) : "")
								  });
		for (UInt i = 0 ; i < all_pages; i++)
			pages << findFillText("list/pages_item.txt", List<String> {
									 "?limit=" + toString(limit) + "&page=" + toString(i)  + (filter.size() > 0 ? ("&filter=" + filter) : ""),
									  i == page_num ? findText("list/pages_selected.txt") :  findText("list/pages_unselected.txt"),
									  toString(i)
								  });
		if ((page_num + 1) < all_pages)
			pages << findFillText("list/pages_next.txt", List<String> {
									 "?limit=" + toString(limit) + "&page=" + toString(page_num+1) + (filter.size() > 0 ? ("&filter=" + filter) : "")
								  });
		OStrStream limiter;
		List<unsigned short> l { 1, 2, 3, 5, 10, 15, 20, 30, 50, 75, 100};
		bool lim_exists = false;
		for (unsigned short lim : l) {
			if (lim == limit)
				lim_exists = true;
			limiter << findFillText("list/pages_limit_item.txt", List<String> {
										lim == limit ? findText("list/pages_limit_item_selected.txt") : findText("list/pages_limit_item_unselected.txt"),
										toString(lim)
									});
		}
		if (!lim_exists)
			limiter << findFillText("limit/pages_limit_item.txt", List<String> {
										findText("limit/pages_limit_item_selected.txt"),
										toString(limit)
									});
		pagination = findFillText("list/pages.txt", List<String> {
									  pages.str(),
									  limiter.str()
								  });
	}
	String html = findFillText("template.txt", List<String>({"Books", makeMenu(), "List", findFillText("list/index.txt", List<String>{
																 vec.str()
															 }), pagination}));
	Request resp = makeRequest();
	resp->body = new char[html.size() + 1];
	strncpy(resp->body, html.c_str(), html.size());
	resp->body_length = html.size();
	return resp;
}

String WebReader::makeMenu() {
	List<String> cats;
	for (const auto & it : this->books)
		if (!__DP_LIB_NAMESPACE__::ConteinsElement(cats, it.second->getFolderCategories()))
			cats.push_back(it.second->getFolderCategories());
	cats.sort();
	OStrStream out;
	for (const String & it : cats)
		out << findFillText("menu/item.txt", List<String>({
															  it,
															  it
														  }));
	cats.clear();
	for (const auto & it : this->books) {
		for (const String & i : it.second->getCategories())
			if (!__DP_LIB_NAMESPACE__::ConteinsElement(cats, i))
				cats.push_back(i);
	}
	cats.sort();
	for (const String & it : cats)
		out << findFillText("menu/item.txt", List<String>({
															  it,
															  it
														  }));
	return out.str();
}
