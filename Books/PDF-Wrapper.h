#pragma once
#include "Book.h"

class PDFWrapper : public Book {
	private:
		Book * pdf = nullptr;
		Book * pdftopng = nullptr;
		Book * get();
	public:
		PDFWrapper(const String & filename);

		virtual Request draw(Request req, const String & path, MakeRequest make) override;
		virtual Request download() override;
};
