/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2008 John Layt <john@layt.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kcupsoptionspageswidget_p.h"

#include <QCheckBox>
#include <QLabel>
#include <QIcon>
#include <QLayout>
#include <QTimer>
#include <QPrinter>
#include <QPrintEngine>

#include <kcombobox.h>
#include <kdebug.h>
#include <klineedit.h>
#include <klocale.h>
#include <kstandarddirs.h>

/** @internal */
KCupsOptionsPagesWidget::KCupsOptionsPagesWidget( QPrintDialog *parent ) : KCupsOptionsWidget( parent )
{
    ui.setupUi( this );

    //set all the default values
    //TODO restore last used values
    initPagesPerSheet();
    initPagesPerSheetLayout();
    initPageBorder();
    initStartBannerPage();
    initEndBannerPage();
    initPageLabel();
    initMirrorPages();
}

KCupsOptionsPagesWidget::~KCupsOptionsPagesWidget()
{
}

void KCupsOptionsPagesWidget::setupCupsOptions( QStringList &cupsOptions )
{
    switch ( pagesPerSheet() )
    {
        case OnePagePerSheet       :                                                 break; //default
        case TwoPagesPerSheet      : setCupsOption( cupsOptions, "number-up", "2");  break;
        case FourPagesPerSheet     : setCupsOption( cupsOptions, "number-up", "4");  break;
        case SixPagesPerSheet      : setCupsOption( cupsOptions, "number-up", "6");  break;
        case NinePagesPerSheet     : setCupsOption( cupsOptions, "number-up", "9");  break;
        case SixteenPagesPerSheet  : setCupsOption( cupsOptions, "number-up", "16"); break;
    }

    switch ( pagesPerSheetLayout() )
    {
        case LeftToRightTopToBottom :                                                            break; //default
        case LeftToRightBottomToTop : setCupsOption( cupsOptions, "number-up-layout", "lrbt" );  break;
        case RightToLeftTopToBottom : setCupsOption( cupsOptions, "number-up-layout", "rltb" );  break;
        case RightToLeftBottomToTop : setCupsOption( cupsOptions, "number-up-layout", "rlbt" );  break;
        case BottomToTopLeftToRight : setCupsOption( cupsOptions, "number-up-layout", "btlr" );  break;
        case BottomToTopRightToLeft : setCupsOption( cupsOptions, "number-up-layout", "btrl" );  break;
        case TopToBottomLeftToRight : setCupsOption( cupsOptions, "number-up-layout", "tblr" );  break;
        case TopToBottomRightToLeft : setCupsOption( cupsOptions, "number-up-layout", "tbrl" );  break;
    }

    switch ( pageBorder() )
    {
        case NoBorder        :                                                              break; //default
        case SingleLine      : setCupsOption( cupsOptions, "page-border", "single" );       break;
        case SingleThickLine : setCupsOption( cupsOptions, "page-border", "single-thick" ); break;
        case DoubleLine      : setCupsOption( cupsOptions, "page-border", "double" );       break;
        case DoubleThickLine : setCupsOption( cupsOptions, "page-border", "double-thick" ); break;
    }

    if ( startBannerPage() != NoBanner || endBannerPage() != NoBanner ) {

        QString startBanner, endBanner;

        switch ( startBannerPage() )
        {
            case NoBanner     : startBanner = "none";         break;
            case Standard     : startBanner = "standard";     break;
            case Unclassified : startBanner = "unclassified"; break;
            case Confidential : startBanner = "confidential"; break;
            case Classified   : startBanner = "classified";   break;
            case Secret       : startBanner = "secret";       break;
            case TopSecret    : startBanner = "topsecret";    break;
        }

        switch ( endBannerPage() )
        {
            case NoBanner     : endBanner = "none";         break;
            case Standard     : endBanner = "standard";     break;
            case Unclassified : endBanner = "unclassified"; break;
            case Confidential : endBanner = "confidential"; break;
            case Classified   : endBanner = "classified";   break;
            case Secret       : endBanner = "secret";       break;
            case TopSecret    : endBanner = "topsecret";    break;
        }

        setCupsOption( cupsOptions, "job-sheets", startBanner + ',' + endBanner );

    }

    if ( !pageLabel().isEmpty() ) {
        setCupsOption( cupsOptions, "page-label", pageLabel() );
    }

    if ( mirrorPages() ) {
        setCupsOption( cupsOptions, "mirror", "" );
    }
}

void KCupsOptionsPagesWidget::initPagesPerSheet()
{
    setPagesPerSheet( OnePagePerSheet );
    ui.oneUpRadioButton->setIcon( QIcon( KStandardDirs::locate( "data", "kdeui/pics/kdeprint_nup1.png" ) ) );
    ui.twoUpRadioButton->setIcon( QIcon( KStandardDirs::locate( "data", "kdeui/pics/kdeprint_nup2.png" ) ) );
    ui.fourUpRadioButton->setIcon( QIcon( KStandardDirs::locate( "data", "kdeui/pics/kdeprint_nup4.png" ) ) );
    ui.sixUpRadioButton->setIcon( QIcon( KStandardDirs::locate( "data", "kdeui/pics/kdeprint_nupother.png" ) ) );
    ui.nineUpRadioButton->setIcon( QIcon( KStandardDirs::locate( "data", "kdeui/pics/kdeprint_nupother.png" ) ) );
    ui.sixteenUpRadioButton->setIcon( QIcon( KStandardDirs::locate( "data", "kdeui/pics/kdeprint_nupother.png" ) ) );
}

void KCupsOptionsPagesWidget::setPagesPerSheet( KCupsOptionsPagesWidget::PagesPerSheet pagesPerSheet  )
{
    switch ( pagesPerSheet )
    {
        case OnePagePerSheet      : ui.oneUpRadioButton->setChecked(true);     break;
        case TwoPagesPerSheet     : ui.twoUpRadioButton->setChecked(true);     break;
        case FourPagesPerSheet    : ui.fourUpRadioButton->setChecked(true);    break;
        case SixPagesPerSheet     : ui.sixUpRadioButton->setChecked(true);     break;
        case NinePagesPerSheet    : ui.nineUpRadioButton->setChecked(true);    break;
        case SixteenPagesPerSheet : ui.sixteenUpRadioButton->setChecked(true); break;
        default                   : ui.oneUpRadioButton->setChecked(true);     break;
    }
}

KCupsOptionsPagesWidget::PagesPerSheet KCupsOptionsPagesWidget::pagesPerSheet() const
{
    if ( ui.oneUpRadioButton->isChecked() )     return OnePagePerSheet;
    if ( ui.twoUpRadioButton->isChecked() )     return TwoPagesPerSheet;
    if ( ui.fourUpRadioButton->isChecked() )    return FourPagesPerSheet;
    if ( ui.sixUpRadioButton->isChecked() )     return SixPagesPerSheet;
    if ( ui.nineUpRadioButton->isChecked() )    return NinePagesPerSheet;
    if ( ui.sixteenUpRadioButton->isChecked() ) return SixteenPagesPerSheet;
    return OnePagePerSheet;
}

void KCupsOptionsPagesWidget::initPagesPerSheetLayout()
{
    ui.pagesPerSheetLayoutCombo->addItem( i18n("Left to Right, Top to Bottom"), QVariant( LeftToRightTopToBottom ) );
    ui.pagesPerSheetLayoutCombo->addItem( i18n("Left to Right, Bottom to Top"), QVariant( LeftToRightBottomToTop ) );
    ui.pagesPerSheetLayoutCombo->addItem( i18n("Right to Left, Bottom to Top"), QVariant( RightToLeftBottomToTop ) );
    ui.pagesPerSheetLayoutCombo->addItem( i18n("Right to Left, Top to Bottom"), QVariant( RightToLeftTopToBottom ) );
    ui.pagesPerSheetLayoutCombo->addItem( i18n("Bottom to Top, Left to Right"), QVariant( BottomToTopLeftToRight ) );
    ui.pagesPerSheetLayoutCombo->addItem( i18n("Bottom to Top, Right to Left"), QVariant( BottomToTopRightToLeft ) );
    ui.pagesPerSheetLayoutCombo->addItem( i18n("Top to Bottom, Left to Right"), QVariant( TopToBottomLeftToRight ) );
    ui.pagesPerSheetLayoutCombo->addItem( i18n("Top to Bottom, Right to Left"), QVariant( TopToBottomRightToLeft ) );

    setPagesPerSheetLayout( LeftToRightTopToBottom );
}

void KCupsOptionsPagesWidget::setPagesPerSheetLayout( KCupsOptionsPagesWidget::PagesPerSheetLayout pagesPerSheetLayout  )
{
    ui.pagesPerSheetLayoutCombo->setCurrentIndex( ui.pagesPerSheetLayoutCombo->findData( QVariant( pagesPerSheetLayout ) ) );
}

KCupsOptionsPagesWidget::PagesPerSheetLayout KCupsOptionsPagesWidget::pagesPerSheetLayout() const
{
    return (KCupsOptionsPagesWidget::PagesPerSheetLayout) ui.pagesPerSheetLayoutCombo->itemData( ui.pagesPerSheetLayoutCombo->currentIndex() ).toInt();
}

void KCupsOptionsPagesWidget::initPageBorder()
{
    ui.pageBorderCombo->addItem( i18nc("No border line", "None"), NoBorder );
    ui.pageBorderCombo->addItem( i18n("Single Line"),       SingleLine );
    ui.pageBorderCombo->addItem( i18n("Single Thick Line"), SingleThickLine );
    ui.pageBorderCombo->addItem( i18n("Double Line"),       DoubleLine );
    ui.pageBorderCombo->addItem( i18n("Double Thick Line"), DoubleThickLine );

    setPageBorder( NoBorder );
}

void KCupsOptionsPagesWidget::setPageBorder( KCupsOptionsPagesWidget::PageBorder pageBorder  )
{
    ui.pageBorderCombo->setCurrentIndex( ui.pageBorderCombo->findData( QVariant( pageBorder ) ) );
}

KCupsOptionsPagesWidget::PageBorder KCupsOptionsPagesWidget::pageBorder() const
{
    return (KCupsOptionsPagesWidget::PageBorder) ui.pageBorderCombo->itemData( ui.pageBorderCombo->currentIndex() ).toInt();
}

void KCupsOptionsPagesWidget::initStartBannerPage()
{
    ui.startBannerPageCombo->addItem( i18nc("Banner page", "None"),         NoBanner );
    ui.startBannerPageCombo->addItem( i18nc("Banner page", "Standard"),     Standard );
    ui.startBannerPageCombo->addItem( i18nc("Banner page", "Unclassified"), Unclassified );
    ui.startBannerPageCombo->addItem( i18nc("Banner page", "Confidential"), Confidential );
    ui.startBannerPageCombo->addItem( i18nc("Banner page", "Classified"),   Classified );
    ui.startBannerPageCombo->addItem( i18nc("Banner page", "Secret"),       Secret );
    ui.startBannerPageCombo->addItem( i18nc("Banner page", "Top Secret"),   TopSecret );

    setStartBannerPage( NoBanner );
}

void KCupsOptionsPagesWidget::setStartBannerPage( KCupsOptionsPagesWidget::BannerPage bannerPage  )
{
    ui.startBannerPageCombo->setCurrentIndex( ui.startBannerPageCombo->findData( QVariant( bannerPage ) ) );
}

KCupsOptionsPagesWidget::BannerPage KCupsOptionsPagesWidget::startBannerPage() const
{
    return (KCupsOptionsPagesWidget::BannerPage) ui.startBannerPageCombo->itemData( ui.startBannerPageCombo->currentIndex() ).toInt();
}

void KCupsOptionsPagesWidget::initEndBannerPage()
{
    ui.endBannerPageCombo->addItem( i18nc("Banner page", "None"),         NoBanner );
    ui.endBannerPageCombo->addItem( i18nc("Banner page", "Standard"),     Standard );
    ui.endBannerPageCombo->addItem( i18nc("Banner page", "Unclassified"), Unclassified );
    ui.endBannerPageCombo->addItem( i18nc("Banner page", "Confidential"), Confidential );
    ui.endBannerPageCombo->addItem( i18nc("Banner page", "Classified"),   Classified );
    ui.endBannerPageCombo->addItem( i18nc("Banner page", "Secret"),       Secret );
    ui.endBannerPageCombo->addItem( i18nc("Banner page", "Top Secret"),   TopSecret );

    setEndBannerPage( NoBanner );
}

void KCupsOptionsPagesWidget::setEndBannerPage( KCupsOptionsPagesWidget::BannerPage bannerPage )
{
    ui.endBannerPageCombo->setCurrentIndex( ui.endBannerPageCombo->findData( QVariant( bannerPage ) ) );
}

KCupsOptionsPagesWidget::BannerPage KCupsOptionsPagesWidget::endBannerPage() const
{
    return (KCupsOptionsPagesWidget::BannerPage) ui.endBannerPageCombo->itemData( ui.endBannerPageCombo->currentIndex() ).toInt();
}

void KCupsOptionsPagesWidget::initPageLabel()
{
    setPageLabel( "" );
}

void KCupsOptionsPagesWidget::setPageLabel( QString label )
{
    ui.pageLabelLineEdit->insert( label );
}

QString KCupsOptionsPagesWidget::pageLabel() const
{
    return ui.pageLabelLineEdit->text();
}

void KCupsOptionsPagesWidget::initMirrorPages()
{
    setMirrorPages( false );
}

void KCupsOptionsPagesWidget::setMirrorPages( bool mirror )
{
    ui.mirrorPagesCheckBox->setChecked( mirror );
}

bool KCupsOptionsPagesWidget::mirrorPages() const
{
    return ui.mirrorPagesCheckBox->isChecked();
}

#include "kcupsoptionspageswidget_p.moc"
