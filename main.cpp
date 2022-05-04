#include "WebReader.h"
#include "Books/EPub.h"
#include "EventLooper.h"
#include "PositionSaver.h"
#include <algorithm>
#ifdef DP_WIN
	#include <windows.h>
#endif

SortedElement::CompareElement SortedElement::LessFunc = [](const SortedElement& s1, const SortedElement & s2) {
	return s1.readPosition.last_time > s2.readPosition.last_time;
};
SortedElement::CompareElement SortedElement::EqualFunc = [](const SortedElement& s1, const SortedElement & s2) {
	return s1.readPosition.last_time == s2.readPosition.last_time;
};

List<SortedElement> WebReader::getBooksSortedByLastDate(UInt & all_pages, UInt page_num, UInt limit,const String & filter){
	if (limit == 0)
		limit = 1;
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
	List<SortedElement> res2;
	UInt min = page_num * limit;
	UInt max = min + limit;
	all_pages = 0;
	UInt i = 0;
	for (const SortedElement & el : res) {
		if (!(i >= min && i < max)) {
			i++;
			continue;
		}
		i++;
		res2.push_back(el);
	}

	all_pages = (i % limit == 0 ) ? (i / limit) : (i / limit + 1);
	return res2;
}

void WebReader::preStart() {
	EventLooper::Create(new _EventLooper());
}

void WebReader::consoleMain() {
	startWebServer();
	SetNeedToExit(true);
}

DP_ADD_MAIN_FUNCTION_WITH_NETWORK(new WebReader())
