#include <iostream>
#include <qstring.h>
#include <kdebug.h>
#include <stdio.h>

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
	fprintf(stdout, "%s", output.utf8().data());
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

kdbgstream kdbg(int area = 0) { return kdbgstream(area, 0); }
kdbgstream &endl( kdbgstream & s) { s << "\n"; return s; }
kdbgstream &flush( kdbgstream & s) { s.flush(); return s; }
