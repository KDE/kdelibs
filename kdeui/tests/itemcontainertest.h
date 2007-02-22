/*
* Tests the item container widgets K3IconView, K3ListView, KListWidget
*
* Copyright 2000 by Michael Reiher <michael.reiher@gmx.de>
*
* Licensed under the GNU General Public License version 2
*/
#ifndef _ITEMCONTAINERTEST_H
#define _ITEMCONTAINERTEST_H

#include <QtGui/QWidget>

class K3IconView;
class K3ListView;
class KListWidget;
class QButtonGroup;
class QLabel;

class TopLevel : public QWidget
{
    Q_OBJECT
public:

    TopLevel( QWidget *parent=0 );

    enum ViewID { IconView, ListView, ListBox };
    enum ModeID { NoSelection, Single, Multi, Extended };

public Q_SLOTS:
    //void slotSwitchView( int id );
    void slotSwitchMode( int id ); 

    void slotIconViewExec( Q3IconViewItem* item );
    void slotListViewExec( Q3ListViewItem* item ); 
    void slotListBoxExec( QListWidgetItem* item );
    void slotToggleSingleColumn( bool b );

    void slotClicked( Q3IconViewItem* ) { qDebug("CLICK");}
    void slotDoubleClicked( Q3IconViewItem* ) { qDebug("DOUBLE CLICK");}
protected:
    K3IconView* m_pIconView;
    K3ListView* m_pListView;
    KListWidget* m_pListBox;

    QButtonGroup* m_pbgView;
    QButtonGroup* m_pbgMode;
    QLabel* m_plblWidget;
    QLabel* m_plblSignal;
    QLabel* m_plblItem;
};

#endif
