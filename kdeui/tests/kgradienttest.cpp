//
// Copyright Daniel M. Duley <mosfet@kde.org>
// Licensed under the Artistic License
#include "kgradienttest.h"
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kpixmapeffect.h>

#include <QDateTime>
#include <QPainter>

int cols = 3, rows = 3; // how many

void KGradientWidget::paintEvent(QPaintEvent */*ev*/)
{
    QTime time;
    int it, ft;
    QString say;

    QColor ca = Qt::black, cb = Qt::blue;

    int x = 0, y = 0;

    pix = QPixmap(width()/cols, height()/rows);
    QPainter p(this);
    p.setPen(Qt::white);

    // draw once, so that the benchmarking be fair :-)
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::VerticalGradient);

    // vertical
    time.start();
    it = time.elapsed();
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::VerticalGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Vertical";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5 + (x++)*width()/cols, 15+y*height()/rows, say); // augment x

    // horizontal
    it = time.elapsed();
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::HorizontalGradient);
    ft = time.elapsed() ;
    say.setNum( ft - it); say += " ms, Horizontal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // elliptic
    it = time.elapsed();
    KPixmapEffect::gradient(pix, ca, cb, KPixmapEffect::EllipticGradient);
    ft = time.elapsed() ;
    say.setNum( ft - it); say += " ms, Elliptic";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    y++; // next row
    x = 0; // reset the columns

    // diagonal
    it = time.elapsed();
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::DiagonalGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Diagonal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // crossdiagonal
    it = time.elapsed();
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::CrossDiagonalGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, CrossDiagonal";
    p.drawPixmap(width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    y++; // next row
    x = 0; // reset the columns

    // pyramidal
    it = time.elapsed();
    KPixmapEffect::gradient(pix, ca, cb, KPixmapEffect::PyramidGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Pyramid";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // pattern
    it = time.elapsed();
    KPixmapEffect::gradient(pix, ca, cb, KPixmapEffect::RectangleGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Rectangle";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/rows, 15+y*height()/rows, say);

    // crosspipe
    it = time.elapsed();
    KPixmapEffect::gradient(pix, ca, cb, KPixmapEffect::PipeCrossGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, PipeCross";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/rows, 15+y*height()/rows, say);
}

int main(int argc, char **argv)
{
    KAboutData about("KGradientTest", "KGradientTest", "version");
    KCmdLineArgs::init(argc, argv, &about);

    KApplication *app=new KApplication();

    KGradientWidget w;
    w.resize(250 * cols, 250 * rows);
    w.show();
    return(app->exec());
}

