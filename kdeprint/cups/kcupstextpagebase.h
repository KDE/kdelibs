/****************************************************************************
** Form interface generated from reading ui file '../../../kdeprint/cups/kcupstextpagebase.ui'
**
** Created: Thu Mar 8 00:35:19 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KCUPSTEXTPAGEBASE_H
#define KCUPSTEXTPAGEBASE_H

#include <qvariant.h>
#include "kprintdialogpage.h"
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class ColumnPreview;
class MarginPreview;
class QButtonGroup;
class QCLineEdit;
class QCheckBox;
class QComboBox;
class QFrame;
class QGroupBox;
class QLabel;
class QRadioButton;
class QSpinBox;

class KCupsTextPageBase : public KPrintDialogPage
{ 
    Q_OBJECT

public:
    KCupsTextPageBase( QWidget* parent = 0, const char* name = 0 );
    ~KCupsTextPageBase();

    QGroupBox* formatbox_;
    QSpinBox* lpi_;
    QLabel* lpilabel_;
    QSpinBox* cpi_;
    QLabel* cpilabel_;
    QFrame* separator_;
    QLabel* columnlabel_;
    QSpinBox* column_;
    ColumnPreview* columnpreview_;
    QButtonGroup* prettybox_;
    QRadioButton* prettyon_;
    QRadioButton* prettyoff_;
    QLabel* prettypix_;
    QGroupBox* marginbox_;
    QLabel* unitslabel_;
    QCLineEdit* left_;
    QLabel* bottomlabel_;
    QCLineEdit* bottom_;
    QCheckBox* margincustom_;
    QLabel* leftlabel_;
    QComboBox* units_;
    QLabel* rightlabel_;
    QCLineEdit* right_;
    QCLineEdit* top_;
    QLabel* toplabel_;
    MarginPreview* marginpreview_;

protected:
    QVBoxLayout* KCupsTextPageBaseLayout;
    QHBoxLayout* Layout7;
    QVBoxLayout* formatbox_Layout;
    QGridLayout* Layout4;
    QHBoxLayout* Layout8;
    QVBoxLayout* Layout7_2;
    QHBoxLayout* prettybox_Layout;
    QVBoxLayout* Layout6;
    QHBoxLayout* marginbox_Layout;
    QGridLayout* Layout7_3;
    QGridLayout* Layout6_2;
};

#endif // KCUPSTEXTPAGEBASE_H
