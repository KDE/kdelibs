#ifndef KICONVIEW_H
#define KICONVIEW_H

#include <qcursor.h>
#include <qiconview.h>

class KIconView : public QIconView
{
    Q_OBJECT

public:
    KIconView( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

protected slots:
    void slotOnItem( QIconViewItem *item );
    void slotOnViewport();

private:
    void checkClickMode();
    QCursor oldCursor;
    bool useDouble;

};

#endif
