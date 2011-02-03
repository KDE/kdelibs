#include <qimage.h>
#include <qstring.h>

#include <kimageeffect.h>

#include <ksvgiconengine.h>
#include <ksvgiconpainter.h>

#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char **argv)
{
	if(argc < 5)
	{
		cout << "Usage : ksvgtopng width height svgfilename outputfilename" << endl;
		cout << "Please use full path name for svgfilename" << endl;
		return -1;
	}

	int width = atoi(argv[1]);
	int height = atoi(argv[2]);

	QImage *img = 0;

	KSVGIconEngine *svgEngine = new KSVGIconEngine();

	if(svgEngine->load(width, height, argv[3]))
	{
		img = svgEngine->painter()->image();
/*		
		// Apply icon sharpening
		double factor = 0;

		if(width == 16)
			factor = 30;
		else if(width == 32)
			factor = 20;
		else if(width == 48)
			factor = 10;
		else if(width == 64)
			factor = 5;

		*img = KImageEffect::sharpen(*img, factor);		
*/
	}
	else
		img = new QImage();

	delete svgEngine;

	img->save(argv[4], "PNG");

	delete img;

	return 0;
}
