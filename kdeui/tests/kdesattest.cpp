#include "kdesattest.h"
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kimageeffect.h>
#include <qpainter.h>
#include <qdatetime.h>
#include <qstring.h>
//Added by qt3to4:
#include <QPaintEvent>

int cols = 3, rows = 3; // how many

KDesatWidget::KDesatWidget(QWidget *parent)
  :QWidget(parent)
{

    image = QImage("testimage.png");
    slide = new KDoubleNumInput(0, 700, 700, this, 1, 2);
    slide->setRange(0, 1, 0.001);
    slide->setLabel("Desaturate: ", Qt::AlignVCenter | Qt::AlignLeft);
    connect(slide,SIGNAL(valueChanged(double)), this, SLOT(change(double)));

    resize(image.width()*2, image.height() + slide->height());
    slide->setGeometry(0, image.height(), image.width()*2, slide->height());
}

void KDesatWidget::change(double) {
    desat_value = slide->value();
    repaint(false);
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
    KAboutData about("KDesatTest", "KDesatTest", "version");
    KCmdLineArgs::init(argc, argv, &about);

    KApplication *app=new KApplication();

    KDesatWidget w;
    app->setMainWidget(&w);
    w.show();
    return(app->exec());
}

#include "kdesattest.moc"
