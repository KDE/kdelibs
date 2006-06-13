//
// Simple little hack to show off blending effects.
//
// Copyright Cristian Tibirna <tibirna@kde.org>
// Licensed under the Artistic License.

#ifndef KBLEND_TEST_H
#define KBLEND_TEST_H

#include <qwidget.h>
#include <qimage.h>

class KBlendWidget : public QWidget
{
public:
    KBlendWidget(QWidget *parent=0);
protected:
    void paintEvent(QPaintEvent *ev);
private:
    QImage image, original;
    QColor bgnd;
};

#endif
