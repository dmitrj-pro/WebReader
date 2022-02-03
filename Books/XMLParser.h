#pragma once
#include <DPLib.conf.h>

using __DP_LIB_NAMESPACE__::String;
using __DP_LIB_NAMESPACE__::Ifstream;
using __DP_LIB_NAMESPACE__::Map;
using __DP_LIB_NAMESPACE__::IStrStream;

struct XMLElement{
	//String xml = "";
    String name = "";
    String value = "";
    Map<String, String> atributes;
	String XML() const;
};

class StreamXMLParser{
    private:
        XMLElement _current;
        bool _isEnd = false;
        int line = -1;
    protected:
        std::istream * _stream;
    public:
        virtual ~StreamXMLParser();
        inline XMLElement current() const { return _current; }
        inline bool isEnd() const { return _isEnd; }
        XMLElement Next();
        XMLElement NextSkeep(const String & name);
};

class FileStreamXMLParser: public StreamXMLParser {
    private:
        String filename = "";
        Ifstream _file;
    public:
        FileStreamXMLParser(const String & filename);
        bool open();
        void close();
};

class StringStreamXMLParser: public StreamXMLParser {
    private:
        IStrStream _str;
        String _text;
    public:
        StringStreamXMLParser(const String & text);
};
