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

#include "kcupsoptionsjobwidget_p.h"

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QTime>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPrinter>
#include <QPrintEngine>

#include <kcombobox.h>
#include <klineedit.h>
#include <klocale.h>
#include <kdatetime.h>
//#include <kdebug.h>

/** @internal */
KCupsOptionsJobWidget::KCupsOptionsJobWidget( QPrintDialog *parent ) : KCupsOptionsWidget( parent )
{
    ui.setupUi( this );
    ui.jobOptionsTable->setFont(KGlobalSettings::smallestReadableFont());

    //set all the default values
    //TODO restore last used values
    initJobHold();
    initJobBilling();
    initJobPriority();
    initJobOptions();
}

KCupsOptionsJobWidget::~KCupsOptionsJobWidget()
{
}

void KCupsOptionsJobWidget::setupCupsOptions( QStringList &cupsOptions )
{
    switch ( jobHold() )
    {
        case NoHold       : break; //default
        case Indefinite   : setCupsOption( cupsOptions, "job-hold-until", "indefinite"   ); break;
        case DayTime      : setCupsOption( cupsOptions, "job-hold-until", "day-time"     ); break;
        case Night        : setCupsOption( cupsOptions, "job-hold-until", "night"        ); break;
        case SecondShift  : setCupsOption( cupsOptions, "job-hold-until", "second-shift" ); break;
        case ThirdShift   : setCupsOption( cupsOptions, "job-hold-until", "third-shift"  ); break;
        case Weekend      : setCupsOption( cupsOptions, "job-hold-until", "weekend"      ); break;
        case SpecificTime : //CUPS expects the time in UTC, user has entered in local time, so get the UTS equivalent
                            KDateTime localDateTime = KDateTime::currentLocalDateTime();
                            //Check if time is for tomorrow in case of DST change overnight
                            if ( jobHoldTime() < localDateTime.time() ) localDateTime.addDays(1);
                            localDateTime.setTime( jobHoldTime() );
                            setCupsOption( cupsOptions, "job-hold-until", localDateTime.toUtc().time().toString("HH:mm") );
                            break;
    }

    if ( !jobBilling().isEmpty() ) {
        setCupsOption( cupsOptions, "job-billing", jobBilling() );
    }

    if ( jobPriority() != 50 ) {
        setCupsOption( cupsOptions, "job-priority", QString( jobPriority() ) );
    }

    //cupsOptions << jobOptions();
}

void KCupsOptionsJobWidget::initJobHold()
{
    ui.jobHoldComboBox->addItem( i18n("Print Immediately"),             QVariant( NoHold ) );
    ui.jobHoldComboBox->addItem( i18n("Hold Indefinitely"),             QVariant( Indefinite ) );
    ui.jobHoldComboBox->addItem( i18n("Day (06:00 to 17:59)"),          QVariant( DayTime ) );
    ui.jobHoldComboBox->addItem( i18n("Night (18:00 to 05:59)"),        QVariant( Night ) );
    ui.jobHoldComboBox->addItem( i18n("Second Shift (16:00 to 23:59)"), QVariant( SecondShift ) );
    ui.jobHoldComboBox->addItem( i18n("Third Shift (00:00 to 07:59)"),  QVariant( ThirdShift ) );
    ui.jobHoldComboBox->addItem( i18n("Weekend (Saturday to Sunday)"),  QVariant( Weekend ) );
    ui.jobHoldComboBox->addItem( i18n("Specific Time"),                 QVariant( SpecificTime ) );

    connect( ui.jobHoldComboBox, SIGNAL( currentIndexChanged(int) ), this, SLOT( toggleJobHoldTime() ) );

    setJobHold( NoHold );
    setJobHoldTime( QTime( 0, 0 ) );
    toggleJobHoldTime();
}

void KCupsOptionsJobWidget::setJobHold( KCupsOptionsJobWidget::JobHoldUntil jobHold  )
{
    ui.jobHoldComboBox->setCurrentIndex( ui.jobHoldComboBox->findData( QVariant( jobHold ) ) );
}

KCupsOptionsJobWidget::JobHoldUntil KCupsOptionsJobWidget::jobHold() const
{
    return (KCupsOptionsJobWidget::JobHoldUntil) ui.jobHoldComboBox->itemData( ui.jobHoldComboBox->currentIndex() ).toInt();
}

void KCupsOptionsJobWidget::toggleJobHoldTime()
{
    if ( jobHold() == SpecificTime ) {
        ui.jobHoldTimeEdit->setEnabled(true);
    } else {
        ui.jobHoldTimeEdit->setEnabled(false);
    }
}

void KCupsOptionsJobWidget::setJobHoldTime( QTime holdUntilTime  )
{
    ui.jobHoldTimeEdit->setTime( holdUntilTime );
}

QTime KCupsOptionsJobWidget::jobHoldTime() const
{
    return ui.jobHoldTimeEdit->time();
}

void KCupsOptionsJobWidget::initJobBilling()
{
    setJobBilling( "" );
}

void KCupsOptionsJobWidget::setJobBilling( const QString jobBilling )
{
    ui.jobBillingLineEdit->insert( jobBilling );
}

QString KCupsOptionsJobWidget::jobBilling() const
{
    return ui.jobBillingLineEdit->text();
}

void KCupsOptionsJobWidget::initJobPriority()
{
    setJobPriority( 50 );
}

void KCupsOptionsJobWidget::setJobPriority( int jobPriority  )
{
    ui.jobPrioritySpinBox->setValue( jobPriority );
}

int KCupsOptionsJobWidget::jobPriority() const
{
    return ui.jobPrioritySpinBox->value();
}

void KCupsOptionsJobWidget::initJobOptions()
{
    ui.jobOptionsTable->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
}

void KCupsOptionsJobWidget::setJobOptions( QStringList jobOptions  )
{
    for ( int x = 1; x <= ui.jobOptionsTable->rowCount() && jobOptions.size() <= x*2; x++ ) {
        if ( !ui.jobOptionsTable->item( x, 1 )->text().isEmpty() ) {
            ui.jobOptionsTable->item( x, 1 )->setText( jobOptions.at( (x*2)-1 ) );
            ui.jobOptionsTable->item( x, 2 )->setText( jobOptions.at( (x*2) ) );
        }
    }
}

QStringList KCupsOptionsJobWidget::jobOptions() const
{
    QStringList optionList;

    for ( int x = 1; x <= ui.jobOptionsTable->rowCount(); x++ ) {
                                           //TODO this cause a crash in text
        if ( !ui.jobOptionsTable->item(x,1)->text().isEmpty() ) {
            optionList << ui.jobOptionsTable->item(x,1)->text() << ui.jobOptionsTable->item(x,2)->text();
        }
    }

    return optionList;
}

#include "kcupsoptionsjobwidget_p.moc"
