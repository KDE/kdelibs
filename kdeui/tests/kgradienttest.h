//
// Copyright Daniel M. Duley <mosfet@kde.org>
// Licensed under the Artistic License.

#ifndef KGRADIENT_TEST_H
#define KGRADIENT_TEST_H

#include <qwidget.h>
#include <qpixmap.h>

class KGradientWidget : public QWidget
{
public:
    KGradientWidget(QWidget *parent=0)
        : QWidget(parent){;}
protected:
    void paintEvent(QPaintEvent *ev);
private:
    QPixmap pix;
};

#endif
