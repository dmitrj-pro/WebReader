#include "PDF-Wrapper.h"
#include "PDF.h"
#include "PDFtoPNG.h"
#include "../PositionSaver.h"
#include <Parser/SmartParser.h>

using __DP_LIB_NAMESPACE__::startWith;

Book * PDFWrapper::get() {
	return PositionSaver::Get().getEnablePDFtoPNG() ? pdftopng : pdf;
}

PDFWrapper::PDFWrapper(const String & filename) :Book(filename) {
	pdftopng = new PDFtoPNG(filename);
	pdf = new PDF(filename);

	Book * b = get();
	this->categories = b->getCategories();
	this->folder_category = b->getFolderCategories();
	this->name = b->getName();
	this->author = b->getAuthor();
	this->information = b->getInformation();
	this->published_date = b->getPublishedDate();
	this->id = b->getId();
	this->cover = b->getCover(cover_size);

}

Request PDFWrapper::draw(Request req, const String & path, MakeRequest make) {
	Book * b = get();
	String p = path;
	if (startWith(path, "pdf/")) {
		p = path.substr(4);
		b = this->pdf;
	}
	if (startWith(path, "png/")) {
		p = path.substr(4);
		b = this->pdftopng;
	}
	Request r = b->draw(req, p, make);
	if (b->hasCover() && (!this->hasCover()))
		this->cover = b->getCover(cover_size);
	return r;
}

Request PDFWrapper::download() {
	return get()->download();
}
