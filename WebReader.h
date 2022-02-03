#pragma once
#include <_Driver/ServiceMain.h>
#include <Addon/httpsrv/HttpPathRouter.h>
#include "Books/Book.h"
#include <Types/SmartPtr.h>
#include <_Driver/Path.h>
#include "PositionSaver.h"
#include <functional>

using __DP_LIB_NAMESPACE__::List;
using __DP_LIB_NAMESPACE__::Map;
using __DP_LIB_NAMESPACE__::SmartPtr;

struct SortedElement{
	String id;
	Book * book;
	Position readPosition;

	typedef std::function<bool(const SortedElement&, const SortedElement&)> CompareElement;
	static CompareElement LessFunc;
	static CompareElement EqualFunc;
	inline friend bool operator< (const SortedElement& s1, const SortedElement & s2){
		return SortedElement::LessFunc(s1, s2);
	}
	inline friend bool operator== (const SortedElement& s1, const SortedElement & s2){
		return SortedElement::EqualFunc(s1, s2);
	}

};

class WebReader: public __DP_LIB_NAMESPACE__::ServiceMain {
	private:
		HttpHostPathRouterServer srv;
		Map<String, Book *> books;

		String web_host = "127.0.0.1";
		unsigned short web_port = 12390;
		String base_path = "./";
		String position_file = "positions.txt";

		List<SortedElement> getBooksSortedByLastDate(const String & filter = "");
		String makeMenu();
	public:
		WebReader():__DP_LIB_NAMESPACE__::ServiceMain("WebReader") {}
		void consoleMain();
		void preStart();
		void startWebServer();
		virtual void MainLoop() override {
			PositionSaver::Create(new _PositionSaver(this->position_file));
			consoleMain();
		}
		inline void onFailArg() { }

		inline void SetWebHost(const String & host) {
			this->web_host = host;
		}
		inline void SetWebPort(unsigned short host) {
			this->web_port = host;
		}
		inline void SetBasePath(const String & host) {
			this->base_path = host;
		}
		inline void SetLogLevel(const String & host) {
			__DP_LIB_NAMESPACE__::log.SetUserLogLevel(__DP_LIB_NAMESPACE__::LogLevel::parseLogLevel(host));
			__DP_LIB_NAMESPACE__::log.SetLibLogLevel(__DP_LIB_NAMESPACE__::LogLevel::DPInfo);
		}
		inline void SetPathToPositions(const String & host) {
			this->position_file = host;
		}


		virtual void PreStart() override {
			preStart();

			__DP_LIB_NAMESPACE__::Path p {this->GetPathToFile()};
			p = __DP_LIB_NAMESPACE__::Path{p.GetFolder()};
			p.Append(this->position_file);
			position_file = p.Get();

			DP_SM_addArgumentHelp1(String, &WebReader::SetWebHost, &WebReader::onFailArg, "Set listen host", "-h", "--h", "-host", "--host");
			DP_SM_addArgumentHelp1(unsigned short, &WebReader::SetWebPort, &WebReader::onFailArg, "Set listen port", "-p", "--p", "-port", "--port");
			DP_SM_addArgumentHelp1(String, &WebReader::SetBasePath, &WebReader::onFailArg, "Set basepath for books", "-b", "--b", "-basepath", "--basepath");
			DP_SM_addArgumentHelp1(String, &WebReader::SetLogLevel, &WebReader::onFailArg, "Set log level application", "-l", "--l", "-log", "--log");
			DP_SM_addArgumentHelp1(String, &WebReader::SetPathToPositions, &WebReader::onFailArg, "Set path to position file", "-position", "--position");
			AddHelp(std::cout, [this]() {
				std::cout << "WebReader support next types: epub, pdf, Galery (for comics read)\n";
				std::cout << "WebReader scan basepath and automatic add finded books to library\n";
				std::cout << "\nFor read galery (comics) need create folder ex. 'Konosuba part 1' in basepath or subdir in basepath\n";
				std::cout << "Create file next to the created folder the same name as the created directory with type '.galery'. Example 'Konosuba part 1.galery'.\n";
				std::cout << "Optional: Write information about galery in xml format:\n";
				std::cout << "<galery>\n";
				std::cout << "	<name>KonoSuba Part 1</name>\n";
				std::cout << "	<cover>Image 1.jpg</cover>\n";
				std::cout << "	<author>Natsume Akatsuki</author>\n";
				std::cout << "</galery>\n";

				std::cout << "\nIf option cover is empty, then selected first image from galery for book cover.\n";
				this->SetNeedToExit();
			}, "help", "-help", "--help", "?");
		}
		virtual void MainExit() override {
			srv.exit();
		}


		Request defaultRoutePost(Request r);
		Request defaultRoute(Request r);
		Request mainPage(Request r);
		Request routeBook(Request req);
		Request routeBookPost(Request req);
		Request routeInfoBook(Request req);
};
