/****************************************************************************
** Form interface generated from reading ui file './connectionbase.ui'
**
** Created: Fri Mar 23 15:04:08 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CONNECTIONBASE_H
#define CONNECTIONBASE_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class KComboBox;
class KLineEdit;
class QCheckBox;
class QFrame;
class QLabel;
class QPushButton;

class ConnectionBase : public QWidget
{ 
    Q_OBJECT

public:
    ConnectionBase( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~ConnectionBase();

    KComboBox* pluginCombo;
    KLineEdit* m_host;
    QLabel* TextLabel3;
    KLineEdit* m_port;
    KLineEdit* m_user;
    QLabel* TextLabel4;
    QLabel* TextLabel2;
    KLineEdit* m_password;
    QCheckBox* remember;
    QLabel* TextLabel1;
    QCheckBox* saveConnection;
    QPushButton* createButton;
    QLabel* TextLabel5;
    QFrame* Line2;

public slots:
    virtual void slotCreateConnection();

protected slots:
    virtual void slotDatabaseSelected();

protected:
    QGridLayout* ConnectionBaseLayout;
    QHBoxLayout* Layout5;
    QHBoxLayout* Layout6;
};

#endif // CONNECTIONBASE_H
