#include "kgradienttest.h"
#include <kapp.h>
#include <kpixmapeffect.h>
#include <qpainter.h>
#include <qdatetime.h>
#include <qstring.h>

int cols = 3, rows = 3; // how many

void KGradientWidget::paintEvent(QPaintEvent */*ev*/)
{
    QTime time;
    int it, ft;
    QString say;

    QColor ca = Qt::black, cb = Qt::blue;

    int x = 0, y = 0;

    pix.resize(width()/cols, height()/rows);
    QPainter p(this);
    p.setPen(Qt::white);

    // draw once, so that the benchmarking be fair :-)
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::VerticalGradient);

    // vertical
    time.start();
    it = time.elapsed();
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::VerticalGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say = "Vertical";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5 + (x++)*width()/cols, 15+y*height()/rows, say); // augment x

    // horizontal
    it = time.elapsed();
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::HorizontalGradient);
    ft = time.elapsed() ;
    say.setNum( ft - it); say = "Horizontal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // elliptic
    it = time.elapsed();
    KPixmapEffect::gradient(pix, ca, cb, KPixmapEffect::EllipticGradient);
    ft = time.elapsed() ;
    say.setNum( ft - it); say = "Elliptic";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    y++; // next row
    x = 0; // reset the columns

    // diagonal
    it = time.elapsed();
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::DiagonalGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say = "Diagonal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // crossdiagonal
    it = time.elapsed();
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::CrossDiagonalGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say = "CrossDiagonal";
    p.drawPixmap(width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    y++; // next row
    x = 0; // reset the columns

    // pyramidal
    it = time.elapsed();
    KPixmapEffect::gradient(pix, ca, cb, KPixmapEffect::PyramidGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say = "Pyramid";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // pattern
    it = time.elapsed();
    KPixmapEffect::gradient(pix, ca, cb, KPixmapEffect::RectangleGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say = "Rectangle";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/rows, 15+y*height()/rows, say);

    // crosspipe
    it = time.elapsed();
    KPixmapEffect::gradient(pix, ca, cb, KPixmapEffect::PipeCrossGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say = "PipeCross";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/rows, 15+y*height()/rows, say);
}

int main(int argc, char **argv)
{
    KApplication *app = new KApplication(argc, argv, "KGradientTest");
    KGradientWidget w;
    w.resize(250 * cols, 250 * rows);
    app->setMainWidget(&w);
    w.show();
    return(app->exec());
}

