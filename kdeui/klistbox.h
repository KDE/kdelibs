#ifndef KLISTBOX_H
#define KLISTBOX_H

#include <qcursor.h>
#include <qlistbox.h>

class KListBox : public QListBox
{
    Q_OBJECT

public:
    KListBox( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

protected:
    void keyPressEvent(QKeyEvent *e);

protected slots:
    void slotOnItem( QListBoxItem *item );
    void slotOnViewport();

    void slotSettingsChanged(int);

private:
    QCursor oldCursor;
    bool m_bUseSingle;
    bool m_bChangeCursorOverItem;

};

#endif
