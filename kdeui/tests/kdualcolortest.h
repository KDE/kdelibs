#ifndef __KDUALCOLORTEST_H
#define __KDUALCOLORTEST_H

#include <qlabel.h>
#include <kdualcolorbutton.h>

class KDualColorWidget : public QWidget
{
    Q_OBJECT
public:
    KDualColorWidget(QWidget *parent=0, const char *name=0);
protected slots:
    void slotFgChanged(const QColor &c);
    void slotBgChanged(const QColor &c);
    void slotCurrentChanged(KDualColorButton::DualColor current);
protected:
    QLabel *lbl;
};

#endif
