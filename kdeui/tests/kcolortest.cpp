#include "kcolortest.h"
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kimageeffect.h>

#include <stdio.h>

#include <QDesktopWidget>
#include <QPainter>
#include <QTime>

bool fullscreen = false, oldway = false, intvsfade = false;
int max = 3; // how many steps

KColorWidget::KColorWidget(QWidget *parent)
    : QWidget(parent), m_bFirst(true)
{

  if (fullscreen || intvsfade) {
    QPixmap shot = QPixmap::grabWindow(QApplication::desktop()->winId());
    original = shot.toImage();
  }
  else
    original = QImage("testimage.png");
  resize(original.width(), original.height());
  // to be sure ...
  original = original.convertToFormat(QImage::Format_ARGB32);
}

void KColorWidget::paintEvent(QPaintEvent *)
{
  if(m_bFirst) {
    m_bFirst = false;
    doIntensityLoop();
  } else {
    QPainter p(this);
    p.drawPixmap(0, 0, pixmap);
  }
}

void KColorWidget::doIntensityLoop()
{
    int count;
    int start, stop;
    QTime t;
    QPainter p(this);

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
	pixmap = QPixmap::fromImage(image);
	p.drawPixmap(0, 0, pixmap);
      }
      stop = t.elapsed();
      qDebug ("Total fullscreen %s dim time for %d steps : %f s",
	       oldway?"(antonio)":"(intensity)", count, (stop - start)*1e-3);

      if (intvsfade) {
	image = original; image.detach();
	start = t.elapsed();
	for(count=0; count < max; ++count){
	  KImageEffect::fade(image, 1./max, Qt::black);
	  pixmap = QPixmap::fromImage(image);
	  p.drawPixmap(0, 0, pixmap);
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
        pixmap = QPixmap::fromImage(image);
        p.drawPixmap(0, 0, pixmap);
      }

      for(count=0; count < max; ++count){
        KImageEffect::intensity(image, -1./max);
        pixmap = QPixmap::fromImage(image);
        p.drawPixmap(0, 0, pixmap);
      }

      image = original; image.detach();
      qDebug("Red channel intensity test");
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, -1./max, KImageEffect::Red);
        pixmap = QPixmap::fromImage(image);
        p.drawPixmap(0, 0, pixmap);
      }
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, 1./max, KImageEffect::Red);
        pixmap = QPixmap::fromImage(image);
        p.drawPixmap(0, 0, pixmap);
      }

      image = original; image.detach();
      qDebug("Green channel intensity test");
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, -1./max, KImageEffect::Green);
        pixmap = QPixmap::fromImage(image);
        p.drawPixmap(0, 0, pixmap);
      }
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, 1./max, KImageEffect::Green);
        pixmap = QPixmap::fromImage(image);
        p.drawPixmap(0, 0, pixmap);
      }

      image = original; image.detach();
      qDebug("Blue channel intensity test");
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, -1./max, KImageEffect::Blue);
        pixmap = QPixmap::fromImage(image);
        p.drawPixmap(0, 0, pixmap);
      }
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, 1./max, KImageEffect::Blue);
        pixmap = QPixmap::fromImage(image);
        p.drawPixmap(0, 0, pixmap);
      }
    }
}

int main(int argc, char **argv)
{

    if (argc > 1) {
      if (!strcmp(argv[1], "--fullscreen"))
	{
	  fullscreen = true;
	  if ((argc > 2) && !strcmp(argv[2], "old_way"))
	    oldway = true;
	}
      else if (!strcmp(argv[1], "--int_vs_fade")) {
	intvsfade = fullscreen = true;
	oldway = false;
      }
      else
	printf("Usage: %s [int_vs_fade | fullscreen [old_way]]\n ", argv[0]);
    }

    static KCmdLineOptions options[] =
    {
        { "fullscreen <old_way>", "", "test" },
        { "int_vs_fade", "", 0 },
        KCmdLineLastOption
    };
    KAboutData about("KColorTest", "KColorTest", "version");
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication *app = new KApplication();
    KColorWidget w;
    w.show();
    w.doIntensityLoop();
    return(app->exec());
}
