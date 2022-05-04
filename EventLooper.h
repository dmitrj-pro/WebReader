#pragma once

#include <_Driver/ThreadWorker.h>
#include <mutex>
#include <functional>
#include <DPLib.conf.h>
#include <Generator/CodeGenerator.h>

using __DP_LIB_NAMESPACE__::String;
using __DP_LIB_NAMESPACE__::Pair;
using __DP_LIB_NAMESPACE__::List;

typedef std::function<void()> EventFunc;
typedef Pair<String, EventFunc> Event;

class _EventLooper{
	private:
		unsigned int sleep_micro_ms = 100;
		// 5 minut
		unsigned int sleep_ms = 1000 * 60 * 5;

		List<Event> everyLoop;
		List<Event> oneLoop;
		List<Event> oneLoop2;
		std::mutex locker;
		__DP_LIB_NAMESPACE__::Thread thread;

		void push_event(List<Event> & list, const Event & e);
		bool exists(List<Event> & list, const String & name);
		void remove_event(List<Event> & list, const String & name);
	public:
		_EventLooper();
		void loop();
		inline void add_to_loop(const String & name, EventFunc func) {
			Event e;
			e.first = name;
			e.second = func;
			push_event(everyLoop, e);
		}
		inline void add_to_one_shot(const String & name, EventFunc func) {
			Event e;
			e.first = name;
			e.second = func;
			if (exists(oneLoop2, name))
				remove_event(oneLoop2, name);
			if (!exists(oneLoop, name))
				push_event(oneLoop, e);
		}
		inline bool exists_shot(const String & name) {
			return exists(oneLoop, name) && exists(oneLoop2, name);
		}
		inline bool exists_loop(const String & name) {
			return  exists(everyLoop, name);
		}
};

DP_SINGLTONE_CLASS(_EventLooper, EventLooper, Get, GetRef, Create)
