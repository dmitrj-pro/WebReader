#pragma once

#include <DPLib.conf.h>
#include <Generator/CodeGenerator.h>
#include <time.h>

using __DP_LIB_NAMESPACE__::String;
using __DP_LIB_NAMESPACE__::Map;

struct Position{
	String position;
	unsigned int last_time;
};

class _PositionSaver{
	private:
		Map<String, Position> positions;
		String filename;
		void load();
		void save();
	public:
		_PositionSaver(const String & config);
		inline Position getPosition(const String & id) { return positions[id]; }
		// Попытка импорта позиции из моего BookBrowser
		void tryImportForFile(const String & id, const String & filename);
		inline void setPosition(const String & id, const Position & pos) { positions[id] = pos; save(); }
		inline void setPosition(const String & id, const String & pos) {
			Position p;
			p.position = pos;
			p.last_time = time(nullptr);
			positions[id] = p;
			save();
		}

};

DP_SINGLTONE_CLASS(_PositionSaver, PositionSaver, Get, GetRef, Create);
