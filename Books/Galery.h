#pragma once
#include "Book.h"

class Galery : public Book {
	private:
		String folder = "";
		List<String> images;
	public:
		Galery(const String & filename);

		virtual Request download() override;

		virtual Request draw(Request req, const String & path, MakeRequest make) override;
};
