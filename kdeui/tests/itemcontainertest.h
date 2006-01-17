#ifndef _ITEMCONTAINERTEST_H
#define _ITEMCONTAINERTEST_H

#include <qwidget.h>

class KIconView;
class KListView;
class KListBox;
class Q3ButtonGroup;
class QLabel;

class TopLevel : public QWidget
{
    Q_OBJECT
public:

    TopLevel( QWidget *parent=0, const char *name=0 );

    enum ViewID { IconView, ListView, ListBox };
    enum ModeID { NoSelection, Single, Multi, Extended };

public Q_SLOTS:
    //void slotSwitchView( int id );
    void slotSwitchMode( int id ); 

    void slotIconViewExec( Q3IconViewItem* item );
    void slotListViewExec( Q3ListViewItem* item ); 
    void slotListBoxExec( Q3ListBoxItem* item );
    void slotToggleSingleColumn( bool b );

    void slotClicked( Q3IconViewItem* ) { qDebug("CLICK");}
    void slotDoubleClicked( Q3IconViewItem* ) { qDebug("DOUBLE CLICK");}
protected:
    KIconView* m_pIconView;
    KListView* m_pListView;
    KListBox* m_pListBox;

    Q3ButtonGroup* m_pbgView;
    Q3ButtonGroup* m_pbgMode;
    QLabel* m_plblWidget;
    QLabel* m_plblSignal;
    QLabel* m_plblItem;
};

#endif
