#include "kblendtest.h"
#include <kapp.h>
#include <kpixmapeffect.h>
#include <qpainter.h>
#include <qdatetime.h>
#include <qstring.h>

int cols = 3, rows = 3; // how many 

KBlendWidget::KBlendWidget(QWidget *parent, const char *name)
  :QWidget(parent, name)
{

    int x = 0, y = 0;

    bgnd = qRgb(0, 70, 70);

    image = QImage("testimage.png");

    resize(image.width()*2+60, image.height()+40);
    setBackgroundColor(bgnd);
}
  
void KBlendWidget::paintEvent(QPaintEvent */*ev*/)
{
    QTime time;
    int it, ft;
    QString say;

    image = QImage("testimage.png");

    int x = 0, y = 0;

    QPainter p(this);
    p.setPen(Qt::white);

    time.start();
    it = time.elapsed();
    KPixmapEffect::blend(image, 0.1, bgnd, KPixmapEffect::DiagonalGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Vertical";
    p.drawImage(20, 20, image);
    p.drawText(5 , 15, say); 

    image = QImage("testimage.png");

    it = time.elapsed();
    KPixmapEffect::blend(image, 0.1, bgnd, KPixmapEffect::CrossDiagonalGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Horizontal";
    p.drawImage(40+image.width(), 20, image);
    p.drawText(15+image.width() , 15, say); 
    p.end();

}

int main(int argc, char **argv)
{
    KApplication *app = new KApplication(argc, argv);
    KBlendWidget w;
    app->setMainWidget(&w);
    w.show();
    return(app->exec());
}
    
