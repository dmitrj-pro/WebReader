#pragma once

#include <DPLib.conf.h>
#include <functional>
#include <Addon/httpsrv/HttpServer.h>


using __DP_LIB_NAMESPACE__::String;
using __DP_LIB_NAMESPACE__::List;

typedef std::function<Request()> MakeRequest;

class Book {
	protected:
		String name = "";
		String author = "";
		String information = "";
		String published_date = "";
		List<String> categories;
		String folder_category;

		char * cover = nullptr;
		unsigned long long cover_size = 0;

		String id;

		String filepath = "";
	public:
		Book(const String & filename);
		void AfterLoadBook();
		virtual inline String getFilename() { return filepath; }
		virtual inline List<String> getCategories() { return categories; }
		inline String getFolderCategories() { return folder_category; }
		virtual inline String getName() { return name; }
		virtual inline String getAuthor() { return author; }
		virtual inline String getInformation() { return information; }
		virtual inline String getPublishedDate() { return published_date; }
		virtual Request download();

		virtual inline bool hasCover() { return cover != nullptr && cover_size != 0; }
		virtual char * getCover(unsigned long long & size);

		virtual inline String getId() { return id; }

		virtual Request draw(Request req, const String & page, MakeRequest make) = 0;

};

inline Request makeRequest() {
	Request resp(new HttpRequest());
	resp->headers["Cache-Control"] = "no-cache";
	resp->headers["Server"] = "nginx";
	resp->status = 200;
	resp->headers["Content-Type"] = getMimeType("index.html");
	return resp;
}

inline Request makeRedirect(Request , const String & path = "/") {
	Request r = makeRequest();
	r->status = 301;
	//r->headers["Location"] = "http://" + req->host + path;
	r->headers["Location"] = path;
	return r;
}
