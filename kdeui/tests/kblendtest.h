//
// Simple little hack to show off blending effects.
//
// (C) KDE Artistic Cristian Tibirna <tibirna@kde.org>
//

#ifndef KBLEND_TEST_H
#define KBLEND_TEST_H

#include <qwidget.h>
#include <qimage.h>

class KBlendWidget : public QWidget
{
public:
    KBlendWidget(QWidget *parent=0, const char *name=0);
protected:
    void paintEvent(QPaintEvent *ev);
private:
    QImage image, original;
    QColor bgnd;
};

#endif
