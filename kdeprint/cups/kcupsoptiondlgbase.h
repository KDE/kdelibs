/****************************************************************************
** Form interface generated from reading ui file '../../../kdeprint/cups/kcupsoptiondlgbase.ui'
**
** Created: Thu Mar 8 00:35:20 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KCUPSOPTIONDLGBASE_H
#define KCUPSOPTIONDLGBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;

class KCupsOptionDlgBase : public QDialog
{ 
    Q_OBJECT

public:
    KCupsOptionDlgBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~KCupsOptionDlgBase();

    QGroupBox* serverbox_;
    QLabel* serverlabel_;
    QLabel* portlabel_;
    QLineEdit* server_;
    QLineEdit* port_;
    QGroupBox* userbox_;
    QLabel* userlabel_;
    QLabel* passwordlabel_;
    QLineEdit* login_;
    QLineEdit* password_;
    QCheckBox* anonymous_;
    QPushButton* okbutton_;
    QPushButton* cancelbutton_;

protected:
    QVBoxLayout* KCupsOptionDlgBaseLayout;
    QGridLayout* serverbox_Layout;
    QGridLayout* userbox_Layout;
    QHBoxLayout* Layout1;
};

#endif // KCUPSOPTIONDLGBASE_H
