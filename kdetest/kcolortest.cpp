
#include "kcolortest.h"
#include <kapp.h>
#include <kpixmapeffect.h>
#include <stdio.h>
#include <qdatetime.h>

bool fullscreen = false;

KColorWidget::KColorWidget(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    if (fullscreen) {
      QPixmap shot = QPixmap::grabWindow(QApplication::desktop()->winId());
      original = shot.convertToImage();
    }
    else
      original = QImage("testimage.png");
    resize(original.width(), original.height());
}

void KColorWidget::paintEvent(QPaintEvent *ev)
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

    image = original;
    warning("Intensity test");


    if (fullscreen){
      start = t.elapsed();
      for(count=0; count < 10; ++count){
	KPixmapEffect::intensity(image, .03*count, false);
	pixmap.convertFromImage(image);
	image = original;
	bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
	       Qt::CopyROP, true);
      }
      stop = t.elapsed();
      warning ("Total fullscreen dim time for %d steps : %f s", 
	       count, (stop - start)*1e-3);
    }
    
    else {
      for(count=0; count < 20; ++count){
        KPixmapEffect::intensity(image, .05, true);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               Qt::CopyROP, true);
      }
      
      for(count=0; count < 20; ++count){
        KPixmapEffect::intensity(image, .05, false);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               Qt::CopyROP, true);
      }
      
      image = original;
      warning("Red channel intensity test");
      for(count=0; count < 20; ++count){
        KPixmapEffect::channelIntensity(image, .05, KPixmapEffect::Red,
                                        true);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               Qt::CopyROP, true);
      }
      for(count=0; count < 20; ++count){
        KPixmapEffect::channelIntensity(image, .05, KPixmapEffect::Red,
                                        false);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               Qt::CopyROP, true);
      }
      
      image = original;
      warning("Green channel intensity test");
      for(count=0; count < 20; ++count){
        KPixmapEffect::channelIntensity(image, .05, KPixmapEffect::Green,
                                        true);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               Qt::CopyROP, true);
      }
      for(count=0; count < 20; ++count){
        KPixmapEffect::channelIntensity(image, .05, KPixmapEffect::Green,
                                        false);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               Qt::CopyROP, true);
      }
      
      image = original;
      warning("Blue channel intensity test");
      for(count=0; count < 20; ++count){
        KPixmapEffect::channelIntensity(image, .05, KPixmapEffect::Blue,
                                        true);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               Qt::CopyROP, true);
      }
      for(count=0; count < 20; ++count){
        KPixmapEffect::channelIntensity(image, .05, KPixmapEffect::Blue,
                                        false);
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
	fullscreen = true;
      else
	printf("Usage: %s [fullscreen]", argv[0]);
    }
    KApplication *app = new KApplication(argc, argv);
    KColorWidget w;
    app->setMainWidget(&w);
    w.show();
    w.doIntensityLoop();
    return(app->exec());
}
