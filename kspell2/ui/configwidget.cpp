/**
 * configwidget.cpp
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "configwidget.h"
#include "configui.h"

#include "broker.h"
#include "settings.h"

#include <keditlistbox.h>
#include <kcombobox.h>
#include <klocale.h>

#include <qcheckbox.h>
#include <qlayout.h>

using namespace KSpell2;

class ConfigWidget::Private
{
public:
    Broker::Ptr broker;
    KSpell2ConfigUI *ui;
};

ConfigWidget::ConfigWidget( Broker *broker, QWidget *parent )
    : QWidget( parent ),d(new Private)
{
    init( broker );
}

ConfigWidget::~ConfigWidget()
{
    delete d;
}

void ConfigWidget::init( Broker *broker )
{
    d->broker = broker;

    QVBoxLayout *layout = new QVBoxLayout( this, 0, 0, "KSpell2ConfigUILayout");
    d->ui = new KSpell2ConfigUI( this );

    QStringList langs = d->broker->languages();
    //QStringList clients = d->broker->clients();
    d->ui->m_langCombo->insertStringList( langs );
    setCorrectLanguage( langs );
    //d->ui->m_clientCombo->insertStringList( clients );
    d->ui->m_skipUpperCB->setChecked( !d->broker->settings()->checkUppercase() );
    d->ui->m_skipRunTogetherCB->setChecked( d->broker->settings()->skipRunTogether() );
    QStringList ignoreList = d->broker->settings()->currentIgnoreList();
    ignoreList.sort();
    d->ui->m_ignoreListBox->insertStringList( ignoreList );
    d->ui->m_bgSpellCB->setChecked( d->broker->settings()->backgroundCheckerEnabled() );
    d->ui->m_bgSpellCB->hide();//hidden by default
    connect( d->ui->m_ignoreListBox, SIGNAL(changed()), SLOT(slotChanged()) );

    layout->addWidget( d->ui );
}

void KSpell2::ConfigWidget::save()
{
    setFromGUI();
    d->broker->settings()->save();
}

void ConfigWidget::setFromGUI()
{
    d->broker->settings()->setDefaultLanguage(
        d->ui->m_langCombo->currentText() );
    d->broker->settings()->setCheckUppercase(
        !d->ui->m_skipUpperCB->isChecked() );
    d->broker->settings()->setSkipRunTogether(
        d->ui->m_skipRunTogetherCB->isChecked() );
    d->broker->settings()->setBackgroundCheckerEnabled(
        d->ui->m_bgSpellCB->isChecked() );
}

void ConfigWidget::slotChanged()
{
    d->broker->settings()->setCurrentIgnoreList(
        d->ui->m_ignoreListBox->items() );
}

void ConfigWidget::setCorrectLanguage( const QStringList& langs)
{
    int idx = 0;
    for ( QStringList::const_iterator itr = langs.begin();
          itr != langs.end(); ++itr, ++idx ) {
        if ( *itr == d->broker->settings()->defaultLanguage() )
            d->ui->m_langCombo->setCurrentItem( idx );
    }
}

void ConfigWidget::setBackgroundCheckingButtonShown( bool b )
{
    d->ui->m_bgSpellCB->setShown( b );
}

bool ConfigWidget::backgroundCheckingButtonShown() const
{
    return d->ui->m_bgSpellCB->isShown();
}

void ConfigWidget::slotDefault()
{
    d->ui->m_skipUpperCB->setChecked( false );
    d->ui->m_skipRunTogetherCB->setChecked( false );
    d->ui->m_bgSpellCB->setChecked( true );
    d->ui->m_ignoreListBox->clear();
}

#include "configwidget.moc"
