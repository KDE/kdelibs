#ifndef _ITEMCONTAINERTEST_H
#define _ITEMCONTAINERTEST_H

#include <qwidget.h>

class KIconView;
class KListView;
class KListBox;
class QButtonGroup;
class QLabel;

class TopLevel : public QWidget
{
    Q_OBJECT
public:

    TopLevel( QWidget *parent=0, const char *name=0 );

    enum ViewID { IconView, ListView, ListBox };
    enum ModeID { NoSelection, Single, Multi, Extended };

public slots:
    //void slotSwitchView( int id );
    void slotSwitchMode( int id ); 

    void slotIconViewExec( QIconViewItem* item );
    void slotListViewExec( QListViewItem* item ); 
    void slotListBoxExec( QListBoxItem* item );
    void slotToggleSingleColumn( bool b );

    void slotClicked( QIconViewItem* ) { qDebug("CLICK");}
    void slotDoubleClicked( QIconViewItem* ) { qDebug("DOUBLE CLICK");}
protected:
    KIconView* m_pIconView;
    KListView* m_pListView;
    KListBox* m_pListBox;

    QButtonGroup* m_pbgView;
    QButtonGroup* m_pbgMode;
    QLabel* m_plblWidget;
    QLabel* m_plblSignal;
    QLabel* m_plblItem;
};

#endif
