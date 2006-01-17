#ifndef KHBOXTEST_H
#define KHBOXTEST_H

#include "khbox.h"
class QPushButton;

class KHBoxTest : public KHBox {
    Q_OBJECT

public:
    KHBoxTest( QWidget* parentWidget );

public Q_SLOTS:
    void slotAdd();
    void slotRemove();

private:
    QPushButton* pbAdd;
    QPushButton* pbRemove;
};


#endif
