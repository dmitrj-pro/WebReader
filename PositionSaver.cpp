#include "PositionSaver.h"
#include <Parser/Setting.h>
#include <Converter/Converter.h>
#include <_Driver/Path.h>
#include "Finder.h"
#include <_Driver/Files.h>

using __DP_LIB_NAMESPACE__::String;
using __DP_LIB_NAMESPACE__::List;
using __DP_LIB_NAMESPACE__::trim;
using __DP_LIB_NAMESPACE__::Path;
using __DP_LIB_NAMESPACE__::AllRead;

_PositionSaver::_PositionSaver(const String & config) {
	this->filename = config;
	load();
}

void _PositionSaver::tryImportForFile(const String & id, const String & filename) {
	Path p {filename + ".position"};
	if (!p.IsFile())
		return;
	auto last_read = Files::fileModTime(p.Get());
	auto pos = positions[id];
	if (pos.last_time >= last_read)
		return;
	pos.position = trim(AllRead(p.Get()));
	pos.last_time = last_read;
	positions[id] = pos;
	save();
}

#define Read(X, Var) \
	if ( setting.Conteins(X)) { \
		Var = trim(setting.get(X)); \
	} else { \
		String tmp = "Paramet '"; \
		tmp += X; \
		tmp += "' is not found."; \
		throw EXCEPTION (tmp); \
	}

#define ReadType(X, Var, Type) \
	if ( setting.Conteins(X)) { \
		Var =  __DP_LIB_NAMESPACE__::parse<Type>( trim(setting.get(X))); \
	} else { \
		String tmp = "Paramet '"; \
		tmp += X; \
		tmp += "' is not found."; \
		throw EXCEPTION (tmp); \
	}

#define ReadN(X, Var) \
	if ( setting.Conteins(X)) \
		Var = trim(setting.get(X));

#define ReadNType(X, Var, Type) \
	if ( setting.Conteins(X)) \
		Var = __DP_LIB_NAMESPACE__::parse<Type>(trim(setting.get(X)));

#define Set(X, Var) \
	setting.add(X, Var);

#define SetType(X, Var) \
	setting.add(X, __DP_LIB_NAMESPACE__::toString(Var));

void _PositionSaver::load() {
	__DP_LIB_NAMESPACE__::Ifstream in;
	in.open(this->filename);
	if (in.fail())
		return;
	__DP_LIB_NAMESPACE__::Setting setting;
	in * setting;
	in.close();

	auto books = setting.getFolders<List<String>>("Books");
	for (String id : books) {
		String key = "Books." + id  + ".";
		Position pos;
		ReadN(key + "Position", pos.position);
		ReadNType(key + "LastRead", pos.last_time, unsigned int);
		this->positions[id] = pos;
	}
}

void _PositionSaver::save() {
	__DP_LIB_NAMESPACE__::Setting setting;
	for (const auto & item : this->positions) {
		String key = "Books." + item.first  + ".";
		Set(key + "Position", item.second.position);
		SetType(key + "LastRead", item.second.last_time);
	}
	__DP_LIB_NAMESPACE__::Ofstream out;
	out.open(this->filename);
	out << setting;
	out.close();
}

DP_SINGLTONE_CLASS_CPP(_PositionSaver, PositionSaver);
