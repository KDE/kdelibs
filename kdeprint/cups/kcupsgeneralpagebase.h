/****************************************************************************
** Form interface generated from reading ui file '../../../kdeprint/cups/kcupsgeneralpagebase.ui'
**
** Created: Thu Mar 8 00:35:17 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KCUPSGENERALPAGEBASE_H
#define KCUPSGENERALPAGEBASE_H

#include <qvariant.h>
#include "kprintdialogpage.h"
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QComboBox;
class QGroupBox;
class QLabel;
class QRadioButton;

class KCupsGeneralPageBase : public KPrintDialogPage
{ 
    Q_OBJECT

public:
    KCupsGeneralPageBase( QWidget* parent = 0, const char* name = 0 );
    ~KCupsGeneralPageBase();

    QComboBox* source_;
    QLabel* sizelabel_;
    QComboBox* size_;
    QLabel* typelabel_;
    QComboBox* type_;
    QLabel* sourcelabel_;
    QButtonGroup* nupbox_;
    QRadioButton* nup1_;
    QRadioButton* nup2_;
    QRadioButton* nup4_;
    QLabel* nuppix_;
    QButtonGroup* orientbox_;
    QRadioButton* portrait_;
    QRadioButton* landscape_;
    QRadioButton* reverse_;
    QRadioButton* reverseportrait_;
    QLabel* orientpix_;
    QButtonGroup* duplexbox_;
    QRadioButton* normal_;
    QRadioButton* book_;
    QRadioButton* tablet_;
    QLabel* duplexpix_;
    QGroupBox* bannerbox_;
    QLabel* startbannerlabel_;
    QComboBox* startbanner_;
    QLabel* stopbannerlabel_;
    QComboBox* stopbanner_;

protected slots:
    virtual void orientationChanged(int);
    virtual void nupChanged(int);
    virtual void duplexChanged(int);

protected:
    QVBoxLayout* KCupsGeneralPageBaseLayout;
    QGridLayout* Layout12;
    QGridLayout* Layout15;
    QHBoxLayout* nupbox_Layout;
    QVBoxLayout* Layout14;
    QHBoxLayout* orientbox_Layout;
    QVBoxLayout* Layout12_2;
    QHBoxLayout* duplexbox_Layout;
    QVBoxLayout* Layout13;
    QVBoxLayout* bannerbox_Layout;
    QHBoxLayout* Layout6;
    QHBoxLayout* Layout7;
};

#endif // KCUPSGENERALPAGEBASE_H
