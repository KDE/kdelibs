#include "kdesattest.h"
#include <kapp.h>
#include <kimageeffect.h>
#include <qpainter.h>
#include <qdatetime.h>
#include <qstring.h>

int cols = 3, rows = 3; // how many

KDesatWidget::KDesatWidget(QWidget *parent, const char *name)
  :QWidget(parent, name)
{

    image = QImage("testimage.png");
    slide = new KIntNumInput("Desaturate: ",
			     0, 1000, 1, 700, "", 10,  
			     true, this, "desat");
    connect(slide,SIGNAL(valueChanged(int)), this, SLOT(change(int)));

    resize(image.width()*2, image.height() + slide->height());
    slide->setGeometry(0, image.height(), image.width()*2, slide->height());
}

void KDesatWidget::change(int) { 
    desat_value = slide->value() / 1000.;
    repaint(); 
}

void KDesatWidget::paintEvent(QPaintEvent */*ev*/)
{
    QTime time;
    int it, ft;
    QString say;

    QPainter p(this);
    p.setPen(Qt::black);

    // original image
    time.start();
    it = time.elapsed();
    image = QImage("testimage.png");
    p.drawImage(0, 0, image);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Vertical";
    p.drawText(5 , 15, say);

    // desaturated image
    it = time.elapsed();
    image = KImageEffect::desaturate(image, desat_value);
    p.drawImage(image.width(), 0, image);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Horizontal";
    p.drawText(15+image.width() , 15, say);
}

int main(int argc, char **argv)
{
    KApplication *app = new KApplication(argc, argv, "KDesatTest");
    KDesatWidget w;
    app->setMainWidget(&w);
    w.show();
    return(app->exec());
}

#include "kdesattest.moc"
