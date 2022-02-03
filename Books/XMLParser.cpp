#include "XMLParser.h"
#include <_Driver/Path.h>
#include <Types/Exception.h>
#include <Converter/Converter.h>

using __DP_LIB_NAMESPACE__::Path;
using __DP_LIB_NAMESPACE__::toString;
using __DP_LIB_NAMESPACE__::OStrStream;

String XMLElement::XML() const {
    OStrStream out;
    out << "<" << name <<" ";
    for (const auto & it : atributes ) {
        out << it.first << "=" << it.second << " ";
    }
    out << ">";
    out << value << "</" << name << ">\r\n";
    return out.str();
}

XMLElement StreamXMLParser::Next() {
    return NextSkeep("");
}

StreamXMLParser::~StreamXMLParser() {}

XMLElement StreamXMLParser::NextSkeep(const String & skeepName) {
    //String name;
    XMLElement res;
    // -1 - Не инициализированно
    // 10 - Пропуск тега
    // 0 - Чтение имени
    // 1 - чтение тегов
    // 2 - чтение содержимого
    // 3 - чтение конца
    int mode = -1;
    String buf = "";
    bool isString = false;
    int posDel = -1;
    while (!_stream->eof() && !_stream->fail()) {
        char c;
        _stream->get(c);
        if ( c == '\n')
            line++;
        if ((mode == -1 ) && ( c == ' ' || c == '\n' || c == '\r' || c == '\t'))
            continue;
		//if (mode != 10)
		//    res.xml += c;
        if (mode == -1) {
            if ( c != '<') {
                String tt;
                tt += c;
                continue;
                throw EXCEPTION("Unknown symbol '" + tt + "' on line " + toString(line));
            }
            mode ++;
            buf = "";
            continue;
        }
        if (mode == 10) {
            if (c == '"') {
                if (isString)
                    isString = false;
                else
                    isString = true;
            }
            if (!isString && c == '>') {
                mode = -1;
				//res.xml = "";
                continue;
            }
            continue;
        }
        if (mode == 0) {
            if (c == ' ') {
                res.name = buf;
                if (buf == skeepName) {
                    mode = 10;
                    continue;
                }
                buf = "";
                mode ++;
                continue;
            }
            if (c == '?' || c == '!') {
                mode = 10;
                continue;
            }
            if (c == '>') {
                res.name = buf;
                if (buf[0] == '/')
                    break;
                mode = 2;
                buf = "";
                continue;
            }
            if (c == '/') {
                res.name = buf;
                buf = "";
                mode = 3;
                _isEnd = true;
                continue;
            }
            buf += c;
        }
        if (mode == 1) {
            #define ADD_TAG(X) { \
                if (posDel < buf.size()) { \
                String tmp = buf.substr(0, posDel);\
                String val = buf.substr(posDel+1); \
                res.atributes[tmp] = val; \
                buf = ""; \
                } \
            }

            if (!isString && c == '/') {
                ADD_TAG(buf);
                mode = 3;
                continue;
            }
            if (!isString && c == '>' ) {
                ADD_TAG(buf);
                mode = 2;
                continue;
            }
            if (!isString && c == ' ') {
                ADD_TAG(buf);
                continue;
            }
            if (!isString && c == '=') {
                posDel = buf.size();
            }
            if (c == '"') {
                if (isString)
                    isString = false;
                else
                    isString = true;
				continue;
            }
            buf += c;
            continue;
        }

        if (mode == 2) {
            static int tempVal = 0;
            if (c == '"') {
                if (isString)
                    isString = false;
                else
                    isString = true;
            }
            res.value += c;
			//if (!isString && c == '<') {
			if (c == '<') {
				isString = false;
                tempVal = res.value.size();
                buf = "<";
                posDel = 0;
                continue;
            }
            if (!isString && posDel == -2) {
                if (c == '>') {
                    if (buf == res.name) {
                        res.value = res.value.substr(0, tempVal-1);
                        mode = 3;
                        break;
                    }
                }
            }
            if (c == ' ')
                continue;
            if (posDel == -2)
                buf += c;
			if (c == '/' && buf == "<") {
                buf = "";
                posDel = -2;
                continue;
            }
        }

        if (mode == 3) {
            if (c == '>') {
                break;
            }
        }
    }
    if (!_isEnd)
        _isEnd = _stream->eof();
    _current = res;
    return res;
}

FileStreamXMLParser::FileStreamXMLParser(const String & filename) {
    Path p (filename);
    if (!p.IsFile())
        throw EXCEPTION("File '" + filename + "' is not found");
    this->filename = filename;
}
bool FileStreamXMLParser::open() {
    _file.open(filename);
    if (_file.fail())
        return false;
    _stream = &_file;
    return true;
}

void FileStreamXMLParser::close() {
    _file.close();
    _stream = nullptr;
}

StringStreamXMLParser::StringStreamXMLParser(const String & text) {
    _str.str(text);
    _stream = &_str;
    _text = text;
}
