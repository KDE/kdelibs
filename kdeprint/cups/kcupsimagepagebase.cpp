#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file '../../../kdeprint/cups/kcupsimagepagebase.ui'
**
** Created: Thu Mar 8 00:35:18 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "../../../kdeprint/cups/kcupsimagepagebase.h"

#include <qcheckbox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qscrollbar.h>
#include "kprintdialogpage.h"
#include "position.h"
#include "previewimage.h"
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a KCupsImagePageBase which is a child of 'parent', with the 
 *  name 'name'.' 
 */
KCupsImagePageBase::KCupsImagePageBase( QWidget* parent,  const char* name )
    : KPrintDialogPage( parent, name )
{
    if ( !name )
	setName( "KCupsImagePageBase" );
    resize( 348, 433 ); 
    setCaption( i18n( "KPrintDialogPageForm" ) );
    KCupsImagePageBaseLayout = new QVBoxLayout( this ); 
    KCupsImagePageBaseLayout->setSpacing( 6 );
    KCupsImagePageBaseLayout->setMargin( 11 );

    colorbox_ = new QGroupBox( this, "colorbox_" );
    colorbox_->setTitle( i18n( "Color settings" ) );
    colorbox_->setColumnLayout(0, Qt::Vertical );
    colorbox_->layout()->setSpacing( 0 );
    colorbox_->layout()->setMargin( 0 );
    colorbox_Layout = new QHBoxLayout( colorbox_->layout() );
    colorbox_Layout->setAlignment( Qt::AlignTop );
    colorbox_Layout->setSpacing( 19 );
    colorbox_Layout->setMargin( 11 );

    Layout10 = new QVBoxLayout; 
    Layout10->setSpacing( 6 );
    Layout10->setMargin( 0 );

    Layout5 = new QGridLayout; 
    Layout5->setSpacing( 0 );
    Layout5->setMargin( 0 );

    brightnesslabel_ = new QLabel( colorbox_, "brightnesslabel_" );
    brightnesslabel_->setText( i18n( "Brightness:" ) );
    brightnesslabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, brightnesslabel_->sizePolicy().hasHeightForWidth() ) );
    brightnesslabel_->setMinimumSize( QSize( 0, 20 ) );

    Layout5->addWidget( brightnesslabel_, 0, 0 );

    brightnessvalue_ = new QLabel( colorbox_, "brightnessvalue_" );
    brightnessvalue_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, brightnessvalue_->sizePolicy().hasHeightForWidth() ) );
    brightnessvalue_->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    Layout5->addWidget( brightnessvalue_, 0, 1 );

    brightness_ = new QScrollBar( colorbox_, "brightness_" );

    Layout5->addMultiCellWidget( brightness_, 1, 1, 0, 1 );
    Layout10->addLayout( Layout5 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout10->addItem( spacer );

    Layout6 = new QGridLayout; 
    Layout6->setSpacing( 0 );
    Layout6->setMargin( 0 );

    huevalue_ = new QLabel( colorbox_, "huevalue_" );
    huevalue_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, huevalue_->sizePolicy().hasHeightForWidth() ) );
    huevalue_->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    Layout6->addWidget( huevalue_, 0, 1 );

    huelabel_ = new QLabel( colorbox_, "huelabel_" );
    huelabel_->setText( i18n( "Hue:" ) );
    huelabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, huelabel_->sizePolicy().hasHeightForWidth() ) );
    huelabel_->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );
    huelabel_->setMinimumSize( QSize( 0, 20 ) );

    Layout6->addWidget( huelabel_, 0, 0 );

    hue_ = new QScrollBar( colorbox_, "hue_" );

    Layout6->addMultiCellWidget( hue_, 1, 1, 0, 1 );
    Layout10->addLayout( Layout6 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout10->addItem( spacer_2 );

    Layout2 = new QGridLayout; 
    Layout2->setSpacing( 0 );
    Layout2->setMargin( 0 );

    saturationvalue_ = new QLabel( colorbox_, "saturationvalue_" );
    saturationvalue_->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    saturationvalue_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, saturationvalue_->sizePolicy().hasHeightForWidth() ) );

    Layout2->addWidget( saturationvalue_, 0, 1 );

    saturationlabel_ = new QLabel( colorbox_, "saturationlabel_" );
    saturationlabel_->setText( i18n( "Saturation:" ) );
    saturationlabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, saturationlabel_->sizePolicy().hasHeightForWidth() ) );
    saturationlabel_->setMinimumSize( QSize( 0, 20 ) );

    Layout2->addWidget( saturationlabel_, 0, 0 );

    saturation_ = new QScrollBar( colorbox_, "saturation_" );

    Layout2->addMultiCellWidget( saturation_, 1, 1, 0, 1 );
    Layout10->addLayout( Layout2 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout10->addItem( spacer_3 );

    Line1 = new QFrame( colorbox_, "Line1" );
    Line1->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    Layout10->addWidget( Line1 );
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout10->addItem( spacer_4 );

    Layout1 = new QGridLayout; 
    Layout1->setSpacing( 0 );
    Layout1->setMargin( 0 );

    gamma_ = new QScrollBar( colorbox_, "gamma_" );

    Layout1->addMultiCellWidget( gamma_, 1, 1, 0, 1 );

    gammalabel_ = new QLabel( colorbox_, "gammalabel_" );
    gammalabel_->setText( i18n( "Gamma:" ) );
    gammalabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, gammalabel_->sizePolicy().hasHeightForWidth() ) );
    gammalabel_->setMinimumSize( QSize( 0, 20 ) );

    Layout1->addWidget( gammalabel_, 0, 0 );

    gammavalue_ = new QLabel( colorbox_, "gammavalue_" );
    gammavalue_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, gammavalue_->sizePolicy().hasHeightForWidth() ) );
    gammavalue_->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    Layout1->addWidget( gammavalue_, 0, 1 );
    Layout10->addLayout( Layout1 );
    colorbox_Layout->addLayout( Layout10 );

    Layout8_2 = new QVBoxLayout; 
    Layout8_2->setSpacing( 15 );
    Layout8_2->setMargin( 0 );

    preview_ = new PreviewImage( colorbox_, "preview_" );
    preview_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, preview_->sizePolicy().hasHeightForWidth() ) );
    preview_->setMinimumSize( QSize( 130, 160 ) );
    Layout8_2->addWidget( preview_ );

    default_ = new QPushButton( colorbox_, "default_" );
    default_->setText( i18n( "Default configuration" ) );
    Layout8_2->addWidget( default_ );
    colorbox_Layout->addLayout( Layout8_2 );
    KCupsImagePageBaseLayout->addWidget( colorbox_ );

    Layout12 = new QHBoxLayout; 
    Layout12->setSpacing( 6 );
    Layout12->setMargin( 0 );

    sizebox_ = new QGroupBox( this, "sizebox_" );
    sizebox_->setTitle( i18n( "Image size" ) );
    sizebox_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, sizebox_->sizePolicy().hasHeightForWidth() ) );
    sizebox_->setMinimumSize( QSize( 200, 0 ) );
    sizebox_->setColumnLayout(0, Qt::Vertical );
    sizebox_->layout()->setSpacing( 0 );
    sizebox_->layout()->setMargin( 0 );
    sizebox_Layout = new QVBoxLayout( sizebox_->layout() );
    sizebox_Layout->setAlignment( Qt::AlignTop );
    sizebox_Layout->setSpacing( 6 );
    sizebox_Layout->setMargin( 11 );
    QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    sizebox_Layout->addItem( spacer_5 );

    Layout7 = new QGridLayout; 
    Layout7->setSpacing( 4 );
    Layout7->setMargin( 0 );

    ppicheck_ = new QCheckBox( sizebox_, "ppicheck_" );
    ppicheck_->setText( i18n( "Resolution (ppi):" ) );
    ppicheck_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, ppicheck_->sizePolicy().hasHeightForWidth() ) );

    Layout7->addWidget( ppicheck_, 0, 0 );

    ppi_ = new QScrollBar( sizebox_, "ppi_" );

    Layout7->addMultiCellWidget( ppi_, 1, 1, 0, 1 );

    ppivalue_ = new QLabel( sizebox_, "ppivalue_" );
    ppivalue_->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    Layout7->addWidget( ppivalue_, 0, 1 );
    sizebox_Layout->addLayout( Layout7 );
    QSpacerItem* spacer_6 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    sizebox_Layout->addItem( spacer_6 );

    Layout8 = new QGridLayout; 
    Layout8->setSpacing( 4 );
    Layout8->setMargin( 0 );

    scaling_ = new QScrollBar( sizebox_, "scaling_" );

    Layout8->addMultiCellWidget( scaling_, 1, 1, 0, 1 );

    scalingvalue_ = new QLabel( sizebox_, "scalingvalue_" );
    scalingvalue_->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    Layout8->addWidget( scalingvalue_, 0, 1 );

    scalingcheck_ = new QCheckBox( sizebox_, "scalingcheck_" );
    scalingcheck_->setText( i18n( "% of page:" ) );
    scalingcheck_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, scalingcheck_->sizePolicy().hasHeightForWidth() ) );

    Layout8->addWidget( scalingcheck_, 0, 0 );
    sizebox_Layout->addLayout( Layout8 );
    QSpacerItem* spacer_7 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    sizebox_Layout->addItem( spacer_7 );
    Layout12->addWidget( sizebox_ );

    positionbox_ = new QGroupBox( this, "positionbox_" );
    positionbox_->setTitle( i18n( "Image position" ) );
    positionbox_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, positionbox_->sizePolicy().hasHeightForWidth() ) );
    positionbox_->setColumnLayout(0, Qt::Vertical );
    positionbox_->layout()->setSpacing( 0 );
    positionbox_->layout()->setMargin( 0 );
    positionbox_Layout = new QGridLayout( positionbox_->layout() );
    positionbox_Layout->setAlignment( Qt::AlignTop );
    positionbox_Layout->setSpacing( 7 );
    positionbox_Layout->setMargin( 11 );

    pospix_ = new ImagePosition( positionbox_, "pospix_" );
    pospix_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, pospix_->sizePolicy().hasHeightForWidth() ) );

    positionbox_Layout->addWidget( pospix_, 2, 2 );

    Layout19 = new QHBoxLayout; 
    Layout19->setSpacing( 6 );
    Layout19->setMargin( 0 );

    left_ = new QRadioButton( positionbox_, "left_" );
    left_->setText( QString::null );
    left_->setMinimumSize( QSize( 20, 0 ) );
    Layout19->addWidget( left_ );

    wcenter_ = new QRadioButton( positionbox_, "wcenter_" );
    wcenter_->setText( QString::null );
    wcenter_->setMinimumSize( QSize( 20, 0 ) );
    Layout19->addWidget( wcenter_ );

    right_ = new QRadioButton( positionbox_, "right_" );
    right_->setText( QString::null );
    right_->setMinimumSize( QSize( 20, 0 ) );
    Layout19->addWidget( right_ );

    positionbox_Layout->addLayout( Layout19, 1, 2 );

    PixmapLabel1 = new QLabel( positionbox_, "PixmapLabel1" );

    positionbox_Layout->addWidget( PixmapLabel1, 1, 1 );

    Layout20 = new QVBoxLayout; 
    Layout20->setSpacing( 6 );
    Layout20->setMargin( 0 );

    top_ = new QRadioButton( positionbox_, "top_" );
    top_->setText( QString::null );
    Layout20->addWidget( top_ );

    hcenter_ = new QRadioButton( positionbox_, "hcenter_" );
    hcenter_->setText( QString::null );
    Layout20->addWidget( hcenter_ );

    bottom_ = new QRadioButton( positionbox_, "bottom_" );
    bottom_->setText( QString::null );
    Layout20->addWidget( bottom_ );

    positionbox_Layout->addLayout( Layout20, 2, 1 );
    Layout12->addWidget( positionbox_ );
    KCupsImagePageBaseLayout->addLayout( Layout12 );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
KCupsImagePageBase::~KCupsImagePageBase()
{
    // no need to delete child widgets, Qt does it all for us
}

