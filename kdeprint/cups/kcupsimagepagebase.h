/****************************************************************************
** Form interface generated from reading ui file '../../../kdeprint/cups/kcupsimagepagebase.ui'
**
** Created: Thu Mar 8 00:35:18 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KCUPSIMAGEPAGEBASE_H
#define KCUPSIMAGEPAGEBASE_H

#include <qvariant.h>
#include "kprintdialogpage.h"
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class ImagePosition;
class PreviewImage;
class QCheckBox;
class QFrame;
class QGroupBox;
class QLabel;
class QPushButton;
class QRadioButton;
class QScrollBar;

class KCupsImagePageBase : public KPrintDialogPage
{ 
    Q_OBJECT

public:
    KCupsImagePageBase( QWidget* parent = 0, const char* name = 0 );
    ~KCupsImagePageBase();

    QGroupBox* colorbox_;
    QLabel* brightnesslabel_;
    QLabel* brightnessvalue_;
    QScrollBar* brightness_;
    QLabel* huevalue_;
    QLabel* huelabel_;
    QScrollBar* hue_;
    QLabel* saturationvalue_;
    QLabel* saturationlabel_;
    QScrollBar* saturation_;
    QFrame* Line1;
    QScrollBar* gamma_;
    QLabel* gammalabel_;
    QLabel* gammavalue_;
    PreviewImage* preview_;
    QPushButton* default_;
    QGroupBox* sizebox_;
    QCheckBox* ppicheck_;
    QScrollBar* ppi_;
    QLabel* ppivalue_;
    QScrollBar* scaling_;
    QLabel* scalingvalue_;
    QCheckBox* scalingcheck_;
    QGroupBox* positionbox_;
    ImagePosition* pospix_;
    QRadioButton* left_;
    QRadioButton* wcenter_;
    QRadioButton* right_;
    QLabel* PixmapLabel1;
    QRadioButton* top_;
    QRadioButton* hcenter_;
    QRadioButton* bottom_;

protected:
    QVBoxLayout* KCupsImagePageBaseLayout;
    QHBoxLayout* colorbox_Layout;
    QVBoxLayout* Layout10;
    QGridLayout* Layout5;
    QGridLayout* Layout6;
    QGridLayout* Layout2;
    QGridLayout* Layout1;
    QVBoxLayout* Layout8_2;
    QHBoxLayout* Layout12;
    QVBoxLayout* sizebox_Layout;
    QGridLayout* Layout7;
    QGridLayout* Layout8;
    QGridLayout* positionbox_Layout;
    QHBoxLayout* Layout19;
    QVBoxLayout* Layout20;
};

#endif // KCUPSIMAGEPAGEBASE_H
