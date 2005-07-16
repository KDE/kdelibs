#ifndef KHBOXTEST_H
#define KHBOXTEST_H

#include "kvbox.h"
class QPushButton;

class KHBoxTest : public KVBox {
    Q_OBJECT

public:
    KHBoxTest( QWidget* parentWidget );

public slots:
    void slotAdd();
    void slotRemove();

private:
    QPushButton* pbAdd;
    QPushButton* pbRemove;
};


#endif
