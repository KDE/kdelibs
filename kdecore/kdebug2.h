#ifndef __KDEBUG2_H__
#define __KDEBUG2_H__

#include <iostream>
#include <qstring.h>
#include <kdebug.h>
#include <stdio.h>

class kdbgstream;
class kndbgstream;
typedef kdbgstream & (*KDBGFUNC)(kdbgstream &); // manipulator function
typedef kndbgstream & (*KNDBGFUNC)(kndbgstream &); // manipulator function

#define k_lineinfo "[" << __FILE__ << ":" << __LINE__ << "] "

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
    void flush();
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

inline kdbgstream &endl( kdbgstream &s) { s << "\n"; return s; }
inline kdbgstream &flush( kdbgstream &s) { s.flush(); return s; }
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

inline kndbgstream &endl( kndbgstream & s) { return s; }
inline kndbgstream &flush( kndbgstream & s) { return s; }
inline kndbgstream &perror( kndbgstream & s) { return s; }

#ifndef NDEBUG
kdbgstream kdDebug(int area = 0);
kdbgstream kdDebug(bool cond, int area = 0);
#else
inline kndbgstream kdDebug(int = 0) { return kndbgstream(); }
inline kndbgstream kdDebug(bool , int  = 0) { return kndbgstream(); }
#endif
kdbgstream kdError(int area = 0);

#endif
