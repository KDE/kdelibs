/****************************************************************************
** Form interface generated from reading ui file '../../kdeprint/kprintcopiespagebase.ui'
**
** Created: Thu Mar 8 00:28:58 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KPRINTCOPIESPAGEBASE_H
#define KPRINTCOPIESPAGEBASE_H

#include <qvariant.h>
#include "kprintdialogpage.h"
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QSpinBox;

class KPrintCopiesPageBase : public KPrintDialogPage
{ 
    Q_OBJECT

public:
    KPrintCopiesPageBase( QWidget* parent = 0, const char* name = 0 );
    ~KPrintCopiesPageBase();

    QGroupBox* copiesbox__2;
    QLabel* copieslabel_;
    QSpinBox* copies_;
    QLabel* copiespix_;
    QCheckBox* collate_;
    QCheckBox* reverse_;
    QButtonGroup* pagebox__2;
    QRadioButton* all_;
    QRadioButton* current_;
    QRadioButton* range_;
    QLineEdit* rangeedit_;
    QLabel* rangeexpl_;
    QLabel* pagelabel_;
    QComboBox* pagecombo_;

protected:
    QGridLayout* KPrintCopiesPageBaseLayout;
    QVBoxLayout* copiesbox__2Layout;
    QHBoxLayout* Layout5;
    QHBoxLayout* Layout4;
    QVBoxLayout* Layout2;
    QVBoxLayout* pagebox__2Layout;
    QVBoxLayout* Layout10;
    QHBoxLayout* Layout8;
    QHBoxLayout* Layout1;
};

#endif // KPRINTCOPIESPAGEBASE_H
