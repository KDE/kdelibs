#include "kdualcolortest.h"
#include <kdualcolorbutton.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <klocale.h>
#include <qlayout.h>
#include <qpalette.h>
//Added by qt3to4:
#include <QLabel>
#include <QHBoxLayout>

KDualColorWidget::KDualColorWidget(QWidget *parent)
    : QWidget(parent)
{
    lbl = new QLabel("Testing, testing, 1, 2, 3...", this);
    KDualColorButton *colorBtn =
        new KDualColorButton(lbl->colorGroup().text(),
                             lbl->colorGroup().background(), this);
    connect(colorBtn, SIGNAL(fgChanged(const QColor &)),
            SLOT(slotFgChanged(const QColor &)));
    connect(colorBtn, SIGNAL(bgChanged(const QColor &)),
            SLOT(slotBgChanged(const QColor &)));
    connect(colorBtn, SIGNAL(currentChanged(KDualColorButton::DualColor)),
            SLOT(slotCurrentChanged(KDualColorButton::DualColor)));
    
    QHBoxLayout *layout = new QHBoxLayout(this, 5);
    layout->addWidget(colorBtn, 0);
    layout->addWidget(lbl, 1);
    layout->activate();
    resize(sizeHint());
}

void KDualColorWidget::slotFgChanged(const QColor &c)
{
    QPalette p = lbl->palette();
    p.setColor(QColorGroup::Text, c);
    lbl->setPalette(p);
}

void KDualColorWidget::slotBgChanged(const QColor &c)
{
    QPalette p = lbl->palette();
    QBrush b(c, Qt::SolidPattern);
    p.setBrush(QColorGroup::Background, b);
    setPalette(p);
}

void KDualColorWidget::slotCurrentChanged(KDualColorButton::DualColor current)
{
    if(current == KDualColorButton::Foreground)
        qDebug("Foreground Button Selected.");
    else
        qDebug("Background Button Selected.");
}

int main(int argc, char **argv)
{
    KAboutData about("KDualColorTest", "KDualColorTest", "version");
    KCmdLineArgs::init(argc, argv, &about);

    KApplication *app=new KApplication();

    KDualColorWidget w;
    app->setMainWidget(&w);
    w.show();
    return(app->exec());
}

#include "kdualcolortest.moc"


