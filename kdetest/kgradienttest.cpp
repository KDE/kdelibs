#include "kgradienttest.h"
#include <kapp.h>
#include <kpixmap.h>
#include <qpainter.h>

void KGradientWidget::paintEvent(QPaintEvent *ev)
{
    KPixmap pm;
    pm.resize(width(), height());
    pm.gradientFill(Qt::red, Qt::black, KPixmap::Diagonal);
    QPainter p(this);
    p.drawPixmap(0, 0, pm);
}

int main(int argc, char **argv)
{
    KApplication *app = new KApplication(argc, argv);
    KGradientWidget w;
    w.resize(300, 300);
    app->setMainWidget(&w);
    w.show();
    return(app->exec());
}
    
