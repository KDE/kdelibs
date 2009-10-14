/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2009 John Layt <john@layt.net>
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

#include "kcupsoptionssettingswidget_p.h"

#include <QPrinter>
#include <QPrintDialog>
#include <QPrintEngine>
#include <QLabel>
#include <QBoxLayout>
#include <QFrame>

#include <klocale.h>

/** @internal */
KCupsOptionsSettingsWidget::KCupsOptionsSettingsWidget( QPrintDialog *parent ) : KCupsOptionsWidget( parent )
{
    //set all the default values
    //TODO restore last used values
    initPageSet();
    setSystemSelectsPages( false );
}

KCupsOptionsSettingsWidget::~KCupsOptionsSettingsWidget()
{
}

void KCupsOptionsSettingsWidget::setupCupsOptions( QStringList &cupsOptions )
{
    if ( m_dialog->printRange() != QAbstractPrintDialog::Selection && pageSet() != AllPages ) {
        //If the application is selecting pages and the first page number is even then need to adjust the odd-even accordingly
        if ( !m_systemSelectsPages && m_dialog->printRange() == QAbstractPrintDialog::PageRange && ( m_dialog->fromPage() % 2 == 0 ) ) {
            if ( pageSet() == OddPages ) setCupsOption( cupsOptions, "page-set", "even" );
            if ( pageSet() == EvenPages ) setCupsOption( cupsOptions, "page-set", "odd" );
        } else {
            if ( pageSet() == OddPages ) setCupsOption( cupsOptions, "page-set", "odd" );
            if ( pageSet() == EvenPages ) setCupsOption( cupsOptions, "page-set", "even" );
        }
    }

    if ( m_systemSelectsPages && m_dialog->printRange() == QAbstractPrintDialog::PageRange ) {
        setCupsOption( cupsOptions, "page-ranges", QString("%1-%2").arg( m_dialog->fromPage() ).arg( m_dialog->toPage() ) );
    }
}

void KCupsOptionsSettingsWidget::initPageSet()
{
    m_pageSetCombo = new QComboBox( m_dialog );
    m_pageSetCombo->addItem( i18n("All Pages"), QVariant( AllPages ) );
    m_pageSetCombo->addItem( i18n("Odd Pages"), QVariant( OddPages ) );
    m_pageSetCombo->addItem( i18n("Even Pages"), QVariant( EvenPages ) );

    setPageSet( AllPages );

    QWidget *settingsWidget = m_dialog->findChild<QWidget*>("gbPrintRange");
    if ( settingsWidget ) {
        QFrame *pageSetLine = new QFrame();
        pageSetLine->setObjectName( QString::fromUtf8("pageSetLine") );
        pageSetLine->setFrameShape( QFrame::HLine );
        pageSetLine->setFrameShadow( QFrame::Sunken );

        QLabel *pageSetLabel = new QLabel( i18n("Page Set") );
        pageSetLabel->setObjectName( QString::fromUtf8("pageSetLabel") );

        QHBoxLayout *pageSetLayout = new QHBoxLayout();
        pageSetLayout->setObjectName( QString::fromUtf8("pageSetLayout") );
        pageSetLayout->addWidget( pageSetLabel );
        pageSetLayout->addWidget( m_pageSetCombo );

        QBoxLayout *settingsLayout = static_cast<QBoxLayout*>( settingsWidget->layout() );
        settingsLayout->insertWidget( 3, pageSetLine );
        settingsLayout->insertLayout( 4, pageSetLayout );
    }
}

void KCupsOptionsSettingsWidget::setPageSet( KCupsOptionsSettingsWidget::PageSet pageSet  )
{
    m_pageSetCombo->setCurrentIndex( m_pageSetCombo->findData( QVariant( pageSet ) ) );
}

KCupsOptionsSettingsWidget::PageSet KCupsOptionsSettingsWidget::pageSet() const
{
    return (KCupsOptionsSettingsWidget::PageSet) m_pageSetCombo->itemData( m_pageSetCombo->currentIndex() ).toInt();
}

void KCupsOptionsSettingsWidget::setSystemSelectsPages( bool serverSelectsPages )
{
    m_systemSelectsPages = serverSelectsPages;
}

bool KCupsOptionsSettingsWidget::systemSelectsPages() const
{
    return m_systemSelectsPages;
}
