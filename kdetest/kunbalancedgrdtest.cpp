#include "kunbalancedgrdtest.h"
#include <kapp.h>
#include <kpixmapeffect.h>
#include <qpainter.h>
#include <qstring.h>
#include <qlayout.h>

int cols = 3, rows = 3; // how many 


KGradientWidget::KGradientWidget(QWidget *parent, const char *name)
  :QWidget(parent, name)
{
  time.start();
  setMinimumSize(250 * cols, 250 * rows);
}

void KGradientWidget::paintEvent(QPaintEvent *ev)
{
    int it, ft;
    QString say;

    QColor ca = Qt::black, cb = Qt::cyan;

    int x = 0, y = 0;

    pix.resize(width()/cols, height()/rows);
    QPainter p(this);
    p.setPen(Qt::white);

    // draw once, so that the benchmarking be fair :-)
    KPixmapEffect::unbalancedGradient(pix,ca, cb, 
				      KPixmapEffect::VerticalGradient,
				      balance);

    // vertical
    time.start();
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix,ca, cb, 
				      KPixmapEffect::VerticalGradient,
				      balance);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Vertical";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5 + (x++)*width()/cols, 15+y*height()/rows, say); // augment x

    // horizontal
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix,ca, cb, 
				      KPixmapEffect::HorizontalGradient,
				      balance);
    ft = time.elapsed() ;
    say.setNum( ft - it); say += " ms, Horizontal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // elliptic
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix, ca, cb, 
				      KPixmapEffect::EllipticGradient,
				      balance);
    ft = time.elapsed() ;
    say.setNum( ft - it); say += " ms, Elliptic";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    y++; // next row
    x = 0; // reset the columns

    // diagonal
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix,ca, cb, 
				      KPixmapEffect::DiagonalGradient,
				      balance);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Diagonal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // crossdiagonal
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix,ca, cb,
				      KPixmapEffect::CrossDiagonalGradient,
				      balance);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, CrossDiagonal";
    p.drawPixmap(width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);


    y++; // next row
    x = 0; // reset the columns

    // pyramidal
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix, ca, cb, 
				      KPixmapEffect::PyramidGradient,
				      balance);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Pyramid";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // rectangular
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix, ca, cb, 
				      KPixmapEffect::RectangleGradient,
				      balance);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Rectangle";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/rows, 15+y*height()/rows, say);

    // crosspipe
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix, ca, cb, 
				      KPixmapEffect::PipeCrossGradient,
				      balance);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, PipeCross";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/rows, 15+y*height()/rows, say);

}

myTopWidget::myTopWidget (QWidget *parent, const char *name=0)
  :QWidget(parent, name)
{
  QGridLayout *lay = new QGridLayout (this, 2, 2, 0);

  grds = new KGradientWidget(this);
  lay->addMultiCellWidget(grds, 0, 0 ,0, 1);

  bLabel = new QLabel("Balance: 0.00", this);
  lay->addWidget(bLabel, 1, 0);

  bSlider = new QSlider ( -100, 100, 1, 10, QSlider::Horizontal, this);
  lay->addWidget(bSlider, 1, 1);

  connect(bSlider, SIGNAL(valueChanged(int)), this, SLOT(rebalance(int)));

  rebalance(30);
}

void myTopWidget::rebalance(int a)
{
  QString val; val.sprintf("Balance: %3.2f", (float)a/100);

  bLabel->setText(val);
  grds->setBalance(a);
  grds->repaint(false);
}

int main(int argc, char **argv)
{
    KApplication *app = new KApplication(argc, argv);
    myTopWidget w;
    app->setMainWidget(&w);
    w.show();
    return(app->exec());
}
    
#include "kunbalancedgrdtest.moc"
