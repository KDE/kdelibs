#ifndef FORM1_H
#define FORM1_H

#include <QtCore/QStringList>
#include <qvariant.h>
#include <QtGui/QWidget>
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QGroupBox;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;

class KHistoryCombo;
class KLineEdit;


class Form1 : public QWidget
{
    Q_OBJECT

public:
    Form1( QWidget* parent = 0 );
    ~Form1();

    QGroupBox* GroupBox1;
    QLabel* TextLabel1;
    KLineEdit* LineEdit1;
    QPushButton* PushButton1;
    QPushButton* PushButton1_4;
    QListWidget* ListBox1;
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
