#include "kgradienttest.h"
#include <kapp.h>
#include <qpainter.h>
#include <qdatetime.h>
#include <qstring.h>

void KGradientWidget::paintEvent(QPaintEvent *ev)
{
    QTime time;
    int it, ft;
    QString say;
 
    pix.resize(width()/2, height()/2);
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
    p.drawText(30, 30, say);

    // horizontal
    it = time.elapsed();
    pix.gradientFill(Qt::black, Qt::red, KPixmap::Horizontal);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms";
    p.drawPixmap(0, height()/2, pix);
    p.drawText(30, 30+height()/2, say);

    //diagonal
    it = time.elapsed();
    pix.gradientFill(Qt::black, Qt::red, KPixmap::Diagonal);
    ft = time.elapsed() ;
    say.setNum( ft - it); say += " ms";
    p.drawPixmap(width()/2, 0, pix);
    p.drawText(30+width()/2, 30, say);

    //pyramidal
    it = time.elapsed();
    pix.gradientFill(Qt::black, Qt::red, KPixmap::Pyramidal);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms";
    p.drawPixmap(width()/2, height()/2, pix);
    p.drawText(30+width()/2, 30+height()/2, say);

}

int main(int argc, char **argv)
{
    KApplication *app = new KApplication(argc, argv);
    KGradientWidget w;
    w.resize(600, 600);
    app->setMainWidget(&w);
    w.show();
    return(app->exec());
}
    
