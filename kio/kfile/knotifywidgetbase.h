/****************************************************************************
** Form interface generated from reading ui file './knotifywidgetbase.ui'
**
** Created: Mon Aug 12 11:05:42 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KNOTIFYWIDGETBASE_H
#define KNOTIFYWIDGETBASE_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class KComboBox;
class KListView;
class KURLRequester;
class QCheckBox;
class QGroupBox;
class QListViewItem;
class QPushButton;

class KNotifyWidgetBase : public QWidget
{ 
    Q_OBJECT

public:
    KNotifyWidgetBase( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~KNotifyWidgetBase();

    KListView* m_listview;
    QGroupBox* m_actionsBox;
    KURLRequester* m_logfilePath;
    KURLRequester* m_soundPath;
    QCheckBox* m_logToFile;
    KURLRequester* m_executePath;
    QCheckBox* m_playSound;
    QPushButton* m_playButton;
    QCheckBox* m_execute;
    QCheckBox* m_messageBox;
    QCheckBox* m_passivePopup;
    QCheckBox* m_stderr;
    QGroupBox* m_controlsBox;
    QCheckBox* m_affectAllApps;
    QPushButton* m_buttonDisable;
    QPushButton* m_buttonEnable;
    KComboBox* m_comboEnable;
    KComboBox* m_comboDisable;
    QPushButton* m_extension;
    QPushButton* m_playerButton;


protected:
    QVBoxLayout* KNotifyWidgetBaseLayout;
    QVBoxLayout* m_actionsBoxLayout;
    QGridLayout* Layout26;
    QHBoxLayout* Layout25;
    QHBoxLayout* Layout27;
    QHBoxLayout* Layout28;
    QHBoxLayout* Layout29;
    QVBoxLayout* m_controlsBoxLayout;
    QHBoxLayout* Layout12;
    QGridLayout* Layout4;
    QHBoxLayout* Layout8;
};

#endif // KNOTIFYWIDGETBASE_H
