#ifndef KCOLORTEST_H
#define KCOLORTEST_H

#include <qwidget.h>
#include <qimage.h>
#include <qpixmap.h>

class KColorWidget : public QWidget
{
public:
    KColorWidget(QWidget *parent=0);
    void doIntensityLoop();
protected:
    void paintEvent(QPaintEvent *ev);
private:
    QImage image, original;
    QPixmap pixmap;
    bool m_bFirst;

};

#endif
