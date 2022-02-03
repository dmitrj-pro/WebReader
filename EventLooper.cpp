#include "EventLooper.h"
#include <_Driver/ServiceMain.h>
#include <Log/Log.h>

_EventLooper::_EventLooper() {
	thread = __DP_LIB_NAMESPACE__::Thread([this](){
		this->loop();
	});
	thread.start();
}

void _EventLooper::loop() {
	while (!__DP_LIB_NAMESPACE__::ServiceSinglton::Get().NeedToExit()) {
		unsigned int sleep = 0;
		while (sleep < sleep_ms && !__DP_LIB_NAMESPACE__::ServiceSinglton::Get().NeedToExit()) {
			__DP_LIB_NAMESPACE__::ServiceSinglton::Get().LoopWait(sleep_micro_ms);
			sleep += sleep_micro_ms;
		}
		if (__DP_LIB_NAMESPACE__::ServiceSinglton::Get().NeedToExit())
			return;
		locker.lock();
		for (auto func : everyLoop) {
			try {
				func.second();
			} catch (__DP_LIB_NAMESPACE__::LineException e) {
				DP_LOG_ERROR << "Error in event loop (" << func.first << "): " << e.toString();
			} catch(...) {
				DP_LOG_ERROR << "Unknown error in event loop (" << func.first << ")";
			}
		}
		for (auto func : oneLoop2) {
			try {
				func.second();
			} catch (__DP_LIB_NAMESPACE__::LineException e) {
				DP_LOG_ERROR << "Error in event loop (" << func.first << "): " << e.toString();
			} catch(...) {
				DP_LOG_ERROR << "Unknown error in event loop (" << func.first << ")";
			}
		}
		oneLoop2.clear();
		oneLoop2 = oneLoop;
		oneLoop.clear();
		locker.unlock();
	}
}

void _EventLooper::push_event(List<Event> & list, const Event & e) {
	locker.lock();
	for (const Event & i : list) {
		if (i.first == e.first) {
			locker.unlock();
			return;
		}
	}
	list.push_back(e);
	locker.unlock();
}

void _EventLooper::remove_event(List<Event> & list, const String & name) {
	locker.lock();
	for (auto it = list.begin(); it != list.end(); it++) {
		if (it->first == name) {
			list.erase(it);
			locker.unlock();
			return;
		}
	}
	locker.unlock();
}

bool _EventLooper::exists(List<Event> & list, const String & name) {
	for (const Event & i : list)
		if (i.first == name)
			return true;
	return false;
}

DP_SINGLTONE_CLASS_CPP(_EventLooper, EventLooper);
