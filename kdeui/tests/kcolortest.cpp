
#include "kcolortest.h"
#include <kapplication.h>
#include <kimageeffect.h>
#include <stdio.h>
#include <qdatetime.h>

bool fullscreen = false, oldway = false, intvsfade = false;
int max = 20; // how many steps

KColorWidget::KColorWidget(QWidget *parent, const char *name)
    : QWidget(parent, name)
{

  if (fullscreen || intvsfade) {
    QPixmap shot = QPixmap::grabWindow(QApplication::desktop()->winId());
    original = shot.convertToImage();
  }
  else
    original = QImage("testimage.png");
  resize(original.width(), original.height());
}

void KColorWidget::paintEvent(QPaintEvent *)
{
  if(!pixmap.isNull())
    bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
	   Qt::CopyROP, true);
}

void KColorWidget::doIntensityLoop()
{
    int count;

    int start, stop;
    QTime t;

    t.start();

    image = original; image.detach();

    if (fullscreen){
      start = t.elapsed();
      for(count=0; count < max; ++count){
	if (!oldway)
	  KImageEffect::intensity(image, -1./max);
	else {
	  uint *qptr=(uint *)image.bits();
	  QRgb qrgb;
	  int size=pixmap.width()*pixmap.height();
	  for (int i=0;i<size; i++, qptr++)
	    {
	      qrgb=*(QRgb *)qptr;
	      *qptr=qRgb((int)(qRed(qrgb)*1./max),
			 (int)(qGreen(qrgb)*1./max),
			 (int)(qBlue(qrgb)*1./max));
	    }
	}
	pixmap.convertFromImage(image);
	bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
	       Qt::CopyROP, true);
      }
      stop = t.elapsed();
      qDebug ("Total fullscreen %s dim time for %d steps : %f s",
	       oldway?"(antonio)":"(intensity)", count, (stop - start)*1e-3);

      if (intvsfade) {
	image = original; image.detach();
	start = t.elapsed();
	for(count=0; count < max; ++count){
	  KImageEffect::fade(image, 1./max, black);
	  pixmap.convertFromImage(image);
	  bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
		 Qt::CopyROP, true);
	}
      }
      stop = t.elapsed();
      qDebug ("Total fullscreen (fade) dim time for %d steps : %f s",
	       count, (stop - start)*1e-3);

    }

    else {
      image = original; image.detach();
      qDebug("Intensity test");
      for(count=0; count < max; ++count){
        KImageEffect::intensity(image, 1./max);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               Qt::CopyROP, true);
      }

      for(count=0; count < max; ++count){
        KImageEffect::intensity(image, -1./max);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               Qt::CopyROP, true);
      }

      image = original; image.detach();
      qDebug("Red channel intensity test");
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, -1./max, KImageEffect::Red);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               Qt::CopyROP, true);
      }
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, 1./max, KImageEffect::Red);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               Qt::CopyROP, true);
      }

      image = original; image.detach();
      qDebug("Green channel intensity test");
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, -1./max, KImageEffect::Green);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               Qt::CopyROP, true);
      }
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, 1./max, KImageEffect::Green);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               Qt::CopyROP, true);
      }

      image = original; image.detach();
      qDebug("Blue channel intensity test");
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, -1./max, KImageEffect::Blue);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               Qt::CopyROP, true);
      }
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, 1./max, KImageEffect::Blue);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               Qt::CopyROP, true);
      }
    }
}

int main(int argc, char **argv)
{
    if (argc > 1) {
      if (!strcmp(argv[1], "fullscreen"))
	{
	  fullscreen = true;
	  if (!strcmp(argv[2], "old_way"))
	    oldway = true;
	}
      else if (!strcmp(argv[1], "int_vs_fade")) {
	intvsfade = fullscreen = true;
	oldway = false;
      }
      else
	printf("Usage: %s [int_vs_fade | fullscreen [old_way]]\n ", argv[0]);
    }
    KApplication *app = new KApplication(argc, argv, "KColorTest");
    KColorWidget w;
    app->setMainWidget(&w);
    w.show();
    w.doIntensityLoop();
    return(app->exec());
}
