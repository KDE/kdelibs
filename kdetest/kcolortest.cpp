
#include "kcolortest.h"
#include <kapp.h>
#include <kpixmapeffect.h>


KColorWidget::KColorWidget(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    image = QImage("testimage.png");
    resize(image.width(), image.height());
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
    for(count=0; count < 20; ++count){
        KPixmapEffect::brighten(image, .05);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               Qt::CopyROP, true);
    }
    for(count=0; count < 20; ++count){
        KPixmapEffect::dim(image, .05);
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
