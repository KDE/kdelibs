#ifndef KLISTBOX_H
#define KLISTBOX_H

#include <qcursor.h>
#include <qlistbox.h>

class KListBox : public QListBox
{
    Q_OBJECT

public:
    KListBox( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

protected slots:
    void slotOnItem( QListBoxItem *item );
    void slotOnViewport();

private:
    void checkClickMode();
    QCursor oldCursor;
    bool useDouble;

};

#endif
