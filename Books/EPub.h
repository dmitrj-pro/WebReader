#pragma once
#include "Book.h"
#include "../Zip/Zip.h"

using __DP_LIB_NAMESPACE__::Map;
using __DP_LIB_NAMESPACE__::Vector;

class EPub : public Book {
	private:
		ZipReader zip;

		String cover_id = "";

		String navigation_file = "";

		Map<String, String> resources;
		Vector<String> resource_position;

		String resource_basepath = "OEBPS";
		String root_path = "";
	public:
		EPub(const String & filename);

		virtual Request draw(Request req, const String & path, MakeRequest make) override;
};
