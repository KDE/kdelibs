/****************************************************************************
** Form interface generated from reading ui file '../../../kdeprint/cups/kcupshpgl2pagebase.ui'
**
** Created: Thu Mar 8 00:35:19 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KCUPSHPGL2PAGEBASE_H
#define KCUPSHPGL2PAGEBASE_H

#include <qvariant.h>
#include "kprintdialogpage.h"
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QGroupBox;
class QLabel;
class QSpinBox;

class KCupsHPGL2PageBase : public KPrintDialogPage
{ 
    Q_OBJECT

public:
    KCupsHPGL2PageBase( QWidget* parent = 0, const char* name = 0 );
    ~KCupsHPGL2PageBase();

    QGroupBox* hpglbox_;
    QCheckBox* black_;
    QCheckBox* fit_;
    QLabel* penwidthlabel_;
    QSpinBox* penwidth_;
    QLabel* unitlabel_;

protected:
    QVBoxLayout* KCupsHPGL2PageBaseLayout;
    QVBoxLayout* hpglbox_Layout;
    QHBoxLayout* Layout5;
};

#endif // KCUPSHPGL2PAGEBASE_H
