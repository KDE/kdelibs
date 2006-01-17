/****************************************************************************
** Form interface generated from reading ui file './kcompletiontest.ui'
**
** Created: Wed Nov 15 20:12:56 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef FORM1_H
#define FORM1_H

#include <qstringlist.h>
#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class Q3GroupBox;
class QLabel;
class Q3ListBox;
class Q3ListBoxItem;
class QPushButton;

class KHistoryCombo;
class KLineEdit;


class Form1 : public QWidget
{
    Q_OBJECT

public:
    Form1( QWidget* parent = 0, const char* name = 0 );
    ~Form1();

    Q3GroupBox* GroupBox1;
    QLabel* TextLabel1;
    KLineEdit* LineEdit1;
    QPushButton* PushButton1;
    QPushButton* PushButton1_4;
    Q3ListBox* ListBox1;
    QPushButton* PushButton1_3;
    QPushButton* PushButton1_2;

    KLineEdit* edit;
    KHistoryCombo *combo;

protected Q_SLOTS:
    void slotList();
    void slotAdd();
    void slotRemove();
    void slotHighlighted( const QString& );

protected:
    QStringList defaultItems() const;

    QVBoxLayout* Form1Layout;
    QVBoxLayout* GroupBox1Layout;
    QVBoxLayout* Layout9;
    QHBoxLayout* Layout1;
    QHBoxLayout* Layout2;
    QHBoxLayout* Layout3;
    QHBoxLayout* Layout8;
    QVBoxLayout* Layout7;
};

#endif // FORM1_H
