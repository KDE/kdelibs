//
// Simple little hack to show off new diagonal gradients.
//
// (C) KDE Artistic Daniel M. Duley <mosfet@kde.org>
//

#ifndef KHASH_TEST_H
#define KHASH_TEST_H

#include <qwidget.h>
#include <kpixmap.h>

class KHashWidget : public QWidget
{
    Q_OBJECT

public:
    KHashWidget(QWidget *parent=0, const char *name=0)
        : QWidget(parent, name){;}
protected:
    void paintEvent(QPaintEvent *ev);
private:
    KPixmap pix;
};

#endif
