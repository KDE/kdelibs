#include "kgradienttest.h"
#include <kapp.h>
#include <kpixmapeffect.h>
#include <qpainter.h>
#include <qdatetime.h>
#include <qstring.h>

void KGradientWidget::paintEvent(QPaintEvent *ev)
{
    QTime time;
    int it, ft;
    QString say;
 
    pix.resize(width()/2, height()/3);
    QPainter p(this);
    p.setPen(Qt::white);

    // draw once, so that the benchmarking be fair :-)
    pix.gradientFill(Qt::black, Qt::red, KPixmap::Vertical);

    // vertical
    time.start();
    it = time.elapsed();
    pix.gradientFill(Qt::black, Qt::red, KPixmap::Vertical);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms";
    p.drawPixmap(0, 0, pix);
    p.drawText(5, 15, say);

    // horizontal
    it = time.elapsed();
    pix.gradientFill(Qt::black, Qt::red, KPixmap::Horizontal);
    ft = time.elapsed() ;
    say.setNum( ft - it); say += " ms";
    p.drawPixmap(width()/2, 0, pix);
    p.drawText(5+width()/2, 15, say);


    // diagonal
    it = time.elapsed();
    pix.gradientFill(Qt::black, Qt::red, KPixmap::Diagonal);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms";
    p.drawPixmap(0, height()/3, pix);
    p.drawText(5, 15+height()/3, say);

    debug ("get here?");


    // crossdiagonal
    it = time.elapsed();
    pix.gradientFill(Qt::black, Qt::red, KPixmap::CrossDiagonal);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms";
    p.drawPixmap(width()/2, height()/3, pix);
    p.drawText(5+width()/2, 15+height()/3, say);

    // pyramidal
    it = time.elapsed();
    KPixmapEffect::apply(pix, Qt::black, Qt::red, 
			 KPixmapEffect::PyramidGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms";
    p.drawPixmap(0, 2*height()/3, pix);
    p.drawText(5, 15+2*height()/3, say);

    // rectangular
    it = time.elapsed();
    KPixmapEffect::apply(pix, Qt::black, Qt::red, 
			 KPixmapEffect::RectangleGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms";
    p.drawPixmap(0+width()/2, 2*height()/3, pix);
    p.drawText(5+width()/2, 15+2*height()/3, say);

}

int main(int argc, char **argv)
{
    KApplication *app = new KApplication(argc, argv);
    KGradientWidget w;
    w.resize(500, 750);
    app->setMainWidget(&w);
    w.show();
    return(app->exec());
}
    
