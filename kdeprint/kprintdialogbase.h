/****************************************************************************
** Form interface generated from reading ui file '../../kdeprint/kprintdialogbase.ui'
**
** Created: Thu Mar 8 00:28:58 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KPRINTDIALOGBASE_H
#define KPRINTDIALOGBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;

class KPrintDialogBase : public QDialog
{ 
    Q_OBJECT

public:
    KPrintDialogBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~KPrintDialogBase();

    QGroupBox* printerbox_;
    QLabel* type_;
    QLabel* namelabel_;
    QLabel* location_;
    QLabel* TextLabel3;
    QLabel* TextLabel6;
    QLabel* TextLabel5;
    QLabel* comment_;
    QLabel* state_;
    QLabel* TextLabel4;
    QComboBox* dest_;
    QCheckBox* outputtofile_;
    QLineEdit* outputfilename_;
    QPushButton* browsebutton_;
    QPushButton* propbutton_;
    QPushButton* defaultbutton_;
    QCheckBox* preview_;
    QPushButton* optionsbutton_;
    QPushButton* okbutton_;
    QPushButton* cancelbutton_;

protected slots:
    virtual void collateClicked();
    virtual void optionsClicked();
    virtual void outputFileBrowseClicked();
    virtual void printerSelected(int);
    virtual void propertyClicked();
    virtual void rangeEntered();
    virtual void setDefaultClicked();

protected:
    QVBoxLayout* KPrintDialogBaseLayout;
    QGridLayout* printerbox_Layout;
    QGridLayout* Layout20;
    QHBoxLayout* Layout16;
    QVBoxLayout* Layout5;
    QHBoxLayout* Layout13;
};

#endif // KPRINTDIALOGBASE_H
