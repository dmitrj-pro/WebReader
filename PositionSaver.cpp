#include "PositionSaver.h"
#include <Parser/Setting.h>
#include <Converter/Converter.h>
#include <_Driver/Path.h>
#include "Finder.h"
#include <_Driver/Files.h>
#include "EventLooper.h"
#include <_Driver/ServiceMain.h>

using __DP_LIB_NAMESPACE__::String;
using __DP_LIB_NAMESPACE__::List;
using __DP_LIB_NAMESPACE__::trim;
using __DP_LIB_NAMESPACE__::Path;
using __DP_LIB_NAMESPACE__::AllRead;

_PositionSaver::_PositionSaver(const String & config) {
	Path p {__DP_LIB_NAMESPACE__::ServiceSinglton::Get().getExecuteFolder()};
	p.Append(chacheDir);
	chacheDir = p.Get();
	p = Path{__DP_LIB_NAMESPACE__::ServiceSinglton::Get().getExecuteFolder()};
	p.Append(pdftopng);
	pdftopng = p.Get();

	this->filename = config;
	load();
	EventLooper::Get().add_to_loop("PositionSaver", [this]() {
		if (this->changed) {
			this->save();
		}
	});
	Files::MkDir(this->chacheDir);
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

	if (pos.position.find("epub") != pos.position.npos && pos.position.find("[") != pos.position.npos) {
		__DP_LIB_NAMESPACE__::SmartParser par {"${before}[${inner}]${after}"};
		if (par.Check(pos.position))
			pos.position = par.Get("before") + par.Get("after");
	}

	pos.last_time = last_read;
	positions[id] = pos;
	save();
}

String _PositionSaver::getPDFtoPNGPath() {
	Path p { this->pdftopng};
	if (p.IsFile())
		return p.Get();
	else {
		#ifdef DP_WIN
			p = Path{this->pdftopng + ".exe"};
			if (p.IsFile())
				return p.Get();
			else
		#endif
				throw EXCEPTION("PDFtoPNG not found");
	}
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

	ReadN("System.CacheDir", this->chacheDir);
	ReadNType("System.EnablePDFtoPNG", this->pdftopng_enable, bool);
	ReadN("System.PDFtoPNG", this->pdftopng);

	auto books = setting.getFolders<List<String>>("Books");
	for (String id : books) {
		String key = "Books." + id  + ".";
		Position pos;
		ReadN(key + "Position", pos.position);
		ReadNType(key + "LastRead", pos.last_time, unsigned int);
		if (pos.position.find("epub") != pos.position.npos && pos.position.find("[") != pos.position.npos) {
			__DP_LIB_NAMESPACE__::SmartParser par {"${before}[${inner}]${after}"};
			if (par.Check(pos.position))
				pos.position = par.Get("before") + par.Get("after");
		}
		this->positions[id] = pos;
	}
	changed = false;
}

void _PositionSaver::save() {
	__DP_LIB_NAMESPACE__::Setting setting;

	Set("System.CacheDir", this->chacheDir);
	SetType("System.EnablePDFtoPNG", this->pdftopng_enable);
	Set("System.PDFtoPNG", this->pdftopng);

	for (const auto & item : this->positions) {
		String key = "Books." + item.first  + ".";
		Set(key + "Position", item.second.position);
		SetType(key + "LastRead", item.second.last_time);
	}
	__DP_LIB_NAMESPACE__::Ofstream out;
	out.open(this->filename);
	out << setting;
	out.close();
	changed = false;
}

DP_SINGLTONE_CLASS_CPP(_PositionSaver, PositionSaver);
