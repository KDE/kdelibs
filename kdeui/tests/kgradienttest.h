//
// Simple little hack to show off new diagonal gradients.
//
// (C) KDE Artistic Daniel M. Duley <mosfet@kde.org>
//

#ifndef KGRADIENT_TEST_H
#define KGRADIENT_TEST_H

#include <qwidget.h>
#include <kpixmap.h>

class KGradientWidget : public QWidget
{
public:
    KGradientWidget(QWidget *parent=0)
        : QWidget(parent){;}
protected:
    void paintEvent(QPaintEvent *ev);
private:
    KPixmap pix;
};

#endif
