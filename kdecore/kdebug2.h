#include <iostream>
#include <qstring.h>

class kdbgstream {
 public:
    kdbgstream(int area, int level) {}
    ~kdbgstream() { cout << "destr " << output.ascii() << endl; }
    kdbgstream &operator<<(int i)  { QString tmp; tmp.setNum(i); output += tmp; return *this; }
 private:
    QString output;
};

kdbgstream kdbg(int area = 0) { return kdbgstream(area, 0); }


