#ifndef KDUALCOLORTEST_H
#define KDUALCOLORTEST_H

#include <qlabel.h>
#include <kdualcolorbutton.h>

class KDualColorWidget : public QWidget
{
    Q_OBJECT
public:
    KDualColorWidget(QWidget *parent=0);
protected Q_SLOTS:
    void slotFgChanged(const QColor &c);
    void slotBgChanged(const QColor &c);
    void slotCurrentChanged(KDualColorButton::DualColor current);
protected:
    QLabel *lbl;
};

#endif
