//
// Simple little hack to show off new diagonal gradients.
//
// (C) KDE Artistic Daniel M. Duley <mosfet@kde.org>
//

#ifndef __KGRADIENT_TEST_H
#define __KGRADIENT_TEST_H

#include <qwidget.h>

class KGradientWidget : public QWidget
{
public:
    KGradientWidget(QWidget *parent=0, const char *name=0)
        : QWidget(parent, name){;}
protected:
    void paintEvent(QPaintEvent *ev);
};

#endif
