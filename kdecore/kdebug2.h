#include <iostream>
#include <qstring.h>
#include <kdebug.h>
#include <stdio.h>

enum DebugLevels { KDEBUG_INFO= 0, KDEBUG_WARN= 1, KDEBUG_ERROR= 2, KDEBUG_FATAL= 3 };

extern void kDebugBackend( unsigned short nLevel, unsigned short nArea,
			   const char * pFormat, va_list arguments );

class kdbgstream;
class kndbgstream;
typedef kdbgstream & (*KDBGFUNC)(kdbgstream &); // manipulator function
typedef kndbgstream & (*KNDBGFUNC)(kndbgstream &); // manipulator function

class kdbgstream {
 public:
    kdbgstream(unsigned int _area, unsigned int _level, bool _print = true) :area(_area), level(_level),  print(_print) {}
    ~kdbgstream() {
	if (!output.isEmpty()) {
	    cerr << "ASSERT: debug output not ended with \\n" << endl;
	    *this << "\n";
	}
    }
    kdbgstream &operator<<(int i)  {
	if (!print) return *this;
	QString tmp; tmp.setNum(i); output += tmp;
	return *this;
    }
    void flush() {
	if (output.isEmpty() || !print)
	    return;
	output.truncate(output.length() - 1);
	kDebugBackend( level, area, "%s", output.utf8().data());
	output = QString::null;
    }
    kdbgstream &operator<<(const QString& string) {
	if (!print) return *this;
	output += string;
	if (output.at(output.length() -1 ) == '\n')
	    flush();
	return *this;
    }
    kdbgstream &operator<<(const char *string) {
	if (!print) return *this;
	output += QString::fromUtf8(string);
	if (output.at(output.length() - 1) == '\n')
	    flush();
	return *this;
    }
    kdbgstream& operator<<(KDBGFUNC f) {
	if (!print) return *this;
	return (*f)(*this);
    }

 private:
    QString output;
    unsigned int area, level;
    bool print;
};

kdbgstream &endl( kdbgstream &s) { s << "\n"; return s; }
kdbgstream &flush( kdbgstream &s) { s.flush(); return s; }
kdbgstream &perror( kdbgstream &s);

class kndbgstream {
 public:
    kndbgstream() {}
    ~kndbgstream() {}
    kndbgstream &operator<<(int )  { return *this; }
    void flush() {}
    kndbgstream &operator<<(const QString& ) {return *this;}
    kndbgstream &operator<<(const char *) {return *this;}
    kndbgstream& operator<<(KNDBGFUNC) { return *this; }
};

kndbgstream &endl( kndbgstream & s) { return s; }
kndbgstream &flush( kndbgstream & s) { return s; }
kndbgstream &perror( kndbgstream & s) { return s; }

#ifndef NDEBUG
kdbgstream kdDebug(int area = 0) { return kdbgstream(area, KDEBUG_INFO); }
kdbgstream kdDebug(bool cond, int area = 0) { if (cond) return kdbgstream(area, KDEBUG_INFO); else return kdbgstream(0, 0, false); }
#else
kndbgstream kdDebug(int = 0) { return kndbgstream(); }
kdbgstream kdDebug(bool , int  = 0) { return kndbgstream(); }
#endif
kdbgstream kdError(int area = 0) { return kdbgstream(area, KDEBUG_ERROR); }
