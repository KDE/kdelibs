
#include "kcolortest.h"
#include <kapp.h>
#include <kpixmapeffect.h>


KColorWidget::KColorWidget(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
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
    image = original;
    warning("Intensity test");
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

int main(int argc, char **argv)
{
    KApplication *app = new KApplication(argc, argv);
    KColorWidget w;
    app->setMainWidget(&w);
    w.show();
    w.doIntensityLoop();
    return(app->exec());
}
