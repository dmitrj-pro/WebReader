#include "WebReader.h"
#include "Books/EPub.h"
#include "EventLooper.h"
#include "PositionSaver.h"
#include <algorithm>

SortedElement::CompareElement SortedElement::LessFunc = [](const SortedElement& s1, const SortedElement & s2) {
	return s1.readPosition.last_time > s2.readPosition.last_time;
};
SortedElement::CompareElement SortedElement::EqualFunc = [](const SortedElement& s1, const SortedElement & s2) {
	return s1.readPosition.last_time == s2.readPosition.last_time;
};

List<SortedElement> WebReader::getBooksSortedByLastDate(const String & filter){
	List<SortedElement> res;
	auto & saver = PositionSaver::Get();
	for (auto pair: this->books) {
		if (filter.size() == 0 || (pair.second->getFolderCategories() == filter || __DP_LIB_NAMESPACE__::ConteinsElement(pair.second->getCategories(), filter))) {
			SortedElement e;
			e.id = pair.first;
			e.book = pair.second;
			e.readPosition = saver.getPosition(e.id);
			res.push_back(e);
		}
	}
	res.sort();
	return res;
}

void WebReader::preStart() {
	EventLooper::Create(new _EventLooper());
}

void WebReader::consoleMain() {
	startWebServer();
	SetNeedToExit(true);
}

DP_ADD_MAIN_FUNCTION(new WebReader())
