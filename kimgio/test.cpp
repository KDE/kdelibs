// This library is distributed under the conditions of the GNU LGPL.
#include "kimgio.h"
#include <qimage.h>
#include <kinstance.h>

int main() {
    // needed for KStdDirs
    KInstance ins("testkimgio");

    kimgioRegister();

    QImage i;
    debug("got %d", i.load("test.jpg"));
	
}
