#pragma once
#include "Book.h"

class PDF : public Book {
	private:

	public:
		PDF(const String & filename);

		virtual Request draw(Request req, const String & path, MakeRequest make) override;
};
