#include <iostream>
#include <qstring.h>
#include <kdebug.h>
#include <stdio.h>

enum DebugLevels { KDEBUG_INFO= 0, KDEBUG_WARN= 1, KDEBUG_ERROR= 2, KDEBUG_FATAL= 3 };

extern void kDebugBackend( unsigned short nLevel, unsigned short nArea,
			   const char * pFormat, va_list arguments );

class kdbgstream;
typedef kdbgstream & (*KDBGFUNC)(kdbgstream &);// manipulator function

class kdbgstream {
 public:
    kdbgstream(unsigned int _area, unsigned int _level) : area(_area), level(_level) {}
    ~kdbgstream() {
	if (!output.isEmpty()) {
	    cerr << "ASSERT: debug output not ended with \\n" << endl;
	    *this << "\n";
	}
    }
    kdbgstream &operator<<(int i)  { QString tmp; tmp.setNum(i); output += tmp; return *this; }
    void flush() {
	if (output.isEmpty())
	    return;
	output.truncate(output.length() - 1);
	kDebugBackend( level, area, "%s", output.utf8().data());
	output = QString::null;
    }
    kdbgstream &operator<<(const QString& string) {
	output += string;
	if (output.at(output.length() -1 ) == '\n')
	    flush();
	return *this;
    }
    kdbgstream &operator<<(const char *string) {
	output += QString::fromUtf8(string);
	if (output.at(output.length() - 1) == '\n')
	    flush();
	return *this;
    }
    kdbgstream& operator<<(KDBGFUNC f) { return (*f)(*this); }

 private:
    QString output;
    unsigned int area, level;
};

kdbgstream kdDebug(int area = 0) { return kdbgstream(area, KDEBUG_INFO); }
kdbgstream kdError(int area = 0) { return kdbgstream(area, KDEBUG_ERROR); }
kdbgstream &endl( kdbgstream & s) { s << "\n"; return s; }
kdbgstream &flush( kdbgstream & s) { s.flush(); return s; }
