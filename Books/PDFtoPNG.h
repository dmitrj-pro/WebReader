#pragma once
#include "Book.h"

class PDFtoPNG : public Book {
	private:
		bool extracted = false;
		List<String> files;
	public:
		PDFtoPNG(const String & filename);

		virtual Request draw(Request req, const String & path, MakeRequest make) override;
};
