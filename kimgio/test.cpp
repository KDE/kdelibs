
#include "kimgio.h"
#include <qimage.h>

int main(int argc, char **argv) {
    kimgioRegister();
    
    QImage i;
    debug("got %d", i.load("test.jpg"));
	
}
