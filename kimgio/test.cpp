
#include "kimgio.h"
#include <qimage.h>

int main() {
    kimgioRegister();

    QImage i;
    debug("got %d", i.load("test.jpg"));
	
}
