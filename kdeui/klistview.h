#ifndef KLISTVIEW_H
#define KLISTVIEW_H

#include <qcursor.h>
#include <qlistview.h>

class KListView : public QListView
{
    Q_OBJECT

public:
    KListView( QWidget *parent = 0, const char *name = 0 );

protected slots:
    void slotOnItem( QListViewItem *item );
    void slotOnViewport();

    void slotSettingsChanged(int);

private:
    QCursor oldCursor;
    bool m_bUseSingle;
    bool m_bChangeCursorOverItem;

};

#endif
