/****************************************************************************
** Form interface generated from reading ui file '../../kdeprint/kprintsizecolorpagebase.ui'
**
** Created: Thu Mar 8 00:28:59 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KPRINTSIZECOLORPAGEBASE_H
#define KPRINTSIZECOLORPAGEBASE_H

#include <qvariant.h>
#include "kprintdialogpage.h"
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QComboBox;
class QLabel;
class QRadioButton;

class KPrintSizeColorPageBase : public KPrintDialogPage
{ 
    Q_OBJECT

public:
    KPrintSizeColorPageBase( QWidget* parent = 0, const char* name = 0 );
    ~KPrintSizeColorPageBase();

    QButtonGroup* orientbox_;
    QRadioButton* portrait_;
    QRadioButton* landscape_;
    QLabel* orientpix_;
    QComboBox* size_;
    QLabel* sizelabel_;
    QButtonGroup* colorbox_;
    QLabel* colorpix_;
    QRadioButton* color_;
    QRadioButton* grayscale_;

protected:
    QGridLayout* KPrintSizeColorPageBaseLayout;
    QGridLayout* orientbox_Layout;
    QGridLayout* colorbox_Layout;
};

#endif // KPRINTSIZECOLORPAGEBASE_H
