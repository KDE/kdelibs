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
#include "ui_configui.h"

#include "loader.h"
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
    Loader::Ptr loader;
    Ui_KSpell2ConfigUI ui;
    QWidget *wdg;
};

ConfigWidget::ConfigWidget( Loader::Ptr loader, QWidget *parent )
    : QWidget( parent ),d(new Private)
{
    init( loader );
}

ConfigWidget::~ConfigWidget()
{
    delete d;
}

void ConfigWidget::init( Loader::Ptr loader )
{
    d->loader = loader;

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    layout->setSpacing( 0 );
    layout->setObjectName( "KSpell2ConfigUILayout" );
    d->wdg = new QWidget( this );
    d->ui.setupUi( d->wdg );

    //QStringList clients = d->loader->clients();
    d->ui.m_langCombo->insertItems( 0, d->loader->languagesName() );
    setCorrectLanguage( d->loader->languages() );
    //d->ui->m_clientCombo->insertStringList( clients );
    d->ui.m_skipUpperCB->setChecked( !d->loader->settings()->checkUppercase() );
    d->ui.m_skipRunTogetherCB->setChecked( d->loader->settings()->skipRunTogether() );
    QStringList ignoreList = d->loader->settings()->currentIgnoreList();
    ignoreList.sort();
    d->ui.m_ignoreListBox->insertStringList( ignoreList );
    d->ui.m_bgSpellCB->setChecked( d->loader->settings()->backgroundCheckerEnabled() );
    d->ui.m_bgSpellCB->hide();//hidden by default
    connect( d->ui.m_ignoreListBox, SIGNAL(changed()), SLOT(slotChanged()) );

    layout->addWidget( d->wdg );
}

void KSpell2::ConfigWidget::save()
{
    setFromGUI();
    d->loader->settings()->save();
}

void ConfigWidget::setFromGUI()
{
    d->loader->settings()->setDefaultLanguage(
        d->loader->languages()[
            d->loader->languagesName().indexOf(
                d->ui.m_langCombo->currentText() ) ] );
    d->loader->settings()->setCheckUppercase(
        !d->ui.m_skipUpperCB->isChecked() );
    d->loader->settings()->setSkipRunTogether(
        d->ui.m_skipRunTogetherCB->isChecked() );
    d->loader->settings()->setBackgroundCheckerEnabled(
        d->ui.m_bgSpellCB->isChecked() );
}

void ConfigWidget::slotChanged()
{
    d->loader->settings()->setCurrentIgnoreList(
        d->ui.m_ignoreListBox->items() );
}

void ConfigWidget::setCorrectLanguage( const QStringList& langs)
{
    int idx = 0;
    for ( QStringList::const_iterator itr = langs.begin();
          itr != langs.end(); ++itr, ++idx ) {
        if ( *itr == d->loader->settings()->defaultLanguage() )
            d->ui.m_langCombo->setCurrentIndex( idx );
    }
}

void ConfigWidget::setBackgroundCheckingButtonShown( bool b )
{
    d->ui.m_bgSpellCB->setVisible( b );
}

bool ConfigWidget::backgroundCheckingButtonShown() const
{
    return !d->ui.m_bgSpellCB->isHidden();
}

void ConfigWidget::slotDefault()
{
    d->ui.m_skipUpperCB->setChecked( false );
    d->ui.m_skipRunTogetherCB->setChecked( false );
    d->ui.m_bgSpellCB->setChecked( true );
    d->ui.m_ignoreListBox->clear();
}

#include "configwidget.moc"
