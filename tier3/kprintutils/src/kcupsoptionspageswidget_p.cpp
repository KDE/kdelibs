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
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QIcon>
#include <QLayout>
#include <QTimer>
#include <QPrinter>
#include <QPrintEngine>
#include <qstandardpaths.h>

#include <klocalizedstring.h>

/** @internal */
KCupsOptionsPagesWidget::KCupsOptionsPagesWidget( QPrintDialog *parent ) : KCupsOptionsWidget( parent )
{
    ui.setupUi( this );

    //set all the default values
    //TODO restore last used values
    initPageBorder();
    initPageLabel();
    initMirrorPages();
}

KCupsOptionsPagesWidget::~KCupsOptionsPagesWidget()
{
}

void KCupsOptionsPagesWidget::setupCupsOptions( QStringList &cupsOptions )
{
    switch ( pageBorder() )
    {
        case NoBorder        :                                                              break; //default
        case SingleLine      : setCupsOption( cupsOptions, "page-border", "single" );       break;
        case SingleThickLine : setCupsOption( cupsOptions, "page-border", "single-thick" ); break;
        case DoubleLine      : setCupsOption( cupsOptions, "page-border", "double" );       break;
        case DoubleThickLine : setCupsOption( cupsOptions, "page-border", "double-thick" ); break;
    }

    if ( !pageLabel().isEmpty() ) {
        setCupsOption( cupsOptions, "page-label", pageLabel() );
    }

    if ( mirrorPages() ) {
        setCupsOption( cupsOptions, "mirror", "" );
    }
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

