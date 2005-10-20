#ifndef KCOLORTEST_H
#define KCOLORTEST_H

#include <qwidget.h>
#include <qimage.h>
#include <kpixmap.h>

class KColorWidget : public QWidget
{
public:
    KColorWidget(QWidget *parent=0, const char *name=0);
    void doIntensityLoop();
protected:
    void paintEvent(QPaintEvent *ev);
private:
    QImage image, original;
    KPixmap pixmap;

};

#endif
