#include <qimage.h>
#include <qstring.h>

#include "ksvgiconengine.h"
#include "ksvgiconpainter.h"

#include <iostream.h>

int main(int argc, char **argv)
{
	if(argc < 5)
		return -1;
	
	int width = atoi(argv[1]);
	int height = atoi(argv[2]);

	QImage *img = 0;
	
	KSVGIconEngine *svgEngine = new KSVGIconEngine();

	if(svgEngine->load(width, height, argv[3]))
		img = svgEngine->painter()->image();
	else
		img = new QImage();

	delete svgEngine;
	
	img->save(argv[4], "PNG");

	delete img;

	return 0;
}
