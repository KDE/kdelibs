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
    slide = new KDoubleNumInput("Desaturate: ", 
						 0, 1, 0.1, 0.9, "","", 
						 true, this, "desat");
    connect(slide,SIGNAL(valueChanged(double)), this, SLOT(change(double)));
    
    resize(image.width()*2, image.height() + slide->height());
    slide->setGeometry(0, image.height(), image.width()*2, slide->height());
}

void KDesatWidget::change(double) { repaint(); }
    
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
    image = KImageEffect::desaturate(image, slide->value());
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

