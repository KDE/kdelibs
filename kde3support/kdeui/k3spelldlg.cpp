/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>
   Copyright (C) 2000 Rik Hemsley <rik@kde.org>
   Copyright (C) 2000-2001 Wolfram Diestel <wolfram@steloj.de>
   Copyright (C) 2003 Zack Rusin <zack@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "k3spelldlg.h"

#include <QProgressBar>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "k3sconfig.h"
#include "ui_k3spellui.h"
#include <kconfiggroup.h>
#include <kdebug.h>

//to initially disable sorting in the suggestions listview
#define NONSORTINGCOLUMN 2

class K3SpellDlg::K3SpellDlgPrivate {
public:
  Ui_K3SpellUI   ui;
  K3SpellConfig* spellConfig;
};

K3SpellDlg::K3SpellDlg( QWidget * parent, bool _progressbar, bool _modal )
  : KDialog(parent)
    , d(0),progressbar( false )
{
  setCaption( i18n("Check Spelling") );
  setButtons( Help|Cancel|User1 );
  setModal( _modal );
  setDefaultButton( Cancel );
  setButtonText( Cancel, i18n("&Finished") );

  KConfigGroup cg( KGlobal::config(),"K3Spell" );
  kDebug() << (cg.readEntry("K3Spell_DoSpellChecking"));
  kDebug() << "dospellchecking is " << cg.readEntry("K3Spell_DoSpellChecking", false);
  if ( (cg.readEntry("K3Spell_DoSpellChecking", false) == true) )  //dospellcheck ?
  {
    Q_UNUSED( _progressbar );
    d = new K3SpellDlgPrivate;
    d->ui.setupUi(this);

    connect( d->ui.m_replaceBtn, SIGNAL(clicked()),
             this, SLOT(replace()));
    connect( this, SIGNAL(ready(bool)),
             d->ui.m_replaceBtn, SLOT(setEnabled(bool)) );

    connect( d->ui.m_replaceAllBtn, SIGNAL(clicked()), this, SLOT(replaceAll()));
    connect(this, SIGNAL(ready(bool)), d->ui.m_replaceAllBtn, SLOT(setEnabled(bool)));

    connect( d->ui.m_skipBtn, SIGNAL(clicked()), this, SLOT(ignore()));
    connect( this, SIGNAL(ready(bool)), d->ui.m_skipBtn, SLOT(setEnabled(bool)));

    connect( d->ui.m_skipAllBtn, SIGNAL(clicked()), this, SLOT(ignoreAll()));
    connect( this, SIGNAL(ready(bool)), d->ui.m_skipAllBtn, SLOT(setEnabled(bool)));

    connect( d->ui.m_addBtn, SIGNAL(clicked()), this, SLOT(add()));
    connect( this, SIGNAL(ready(bool)), d->ui.m_addBtn, SLOT(setEnabled(bool)));

    connect( d->ui.m_suggestBtn, SIGNAL(clicked()), this, SLOT(suggest()));
    connect( this, SIGNAL(ready(bool)), d->ui.m_suggestBtn, SLOT(setEnabled(bool)) );
    d->ui.m_suggestBtn->hide();

    connect(this, SIGNAL(user1Clicked()), this, SLOT(stop()));

    connect( d->ui.m_replacement, SIGNAL(textChanged(const QString &)),
             SLOT(textChanged(const QString &)) );

    connect( d->ui.m_replacement, SIGNAL(returnPressed()),   SLOT(replace()) );
    connect( d->ui.m_suggestions, SIGNAL(selectionChanged(Q3ListViewItem*)),
             SLOT(slotSelectionChanged(Q3ListViewItem*)) );

    connect( d->ui.m_suggestions, SIGNAL( doubleClicked ( Q3ListViewItem *, const QPoint &, int ) ),
             SLOT( replace() ) );
    d->spellConfig = new K3SpellConfig( 0, 0 , false );
    d->spellConfig->fillDicts( d->ui.m_language );
    connect( d->ui.m_language, SIGNAL(activated(int)),
    	   d->spellConfig, SLOT(sSetDictionary(int)) );
    connect( d->spellConfig, SIGNAL(configChanged()),
           SLOT(slotConfigChanged()) );

    setHelp( "spelldlg", "k3spell" );
    setMinimumSize( sizeHint() );
    emit ready( false );
  }
}

K3SpellDlg::~K3SpellDlg()
{
  delete d;
}

QString K3SpellDlg::replacement() const
{
    return newword;
}

void
K3SpellDlg::init( const QString & _word, QStringList * _sugg )
{
  sugg = _sugg;
  word = _word;

  d->ui.m_suggestions->clear();
  d->ui.m_suggestions->setSorting( NONSORTINGCOLUMN );
  for ( QStringList::Iterator it = _sugg->begin(); it != _sugg->end(); ++it ) {
    Q3ListViewItem *item = new Q3ListViewItem( d->ui.m_suggestions,
                                             d->ui.m_suggestions->lastItem() );
    item->setText( 0, *it );
  }
  kDebug(750) << "K3SpellDlg::init [" << word << "]";

  emit ready( true );

  d->ui.m_unknownWord->setText( _word );

  if ( sugg->count() == 0 ) {
    d->ui.m_replacement->setText( _word );
    d->ui.m_replaceBtn->setEnabled( false );
    d->ui.m_replaceAllBtn->setEnabled( false );
    d->ui.m_suggestBtn->setEnabled( false );
  } else {
    d->ui.m_replacement->setText( (*sugg)[0] );
    d->ui.m_replaceBtn->setEnabled( true );
    d->ui.m_replaceAllBtn->setEnabled( true );
    d->ui.m_suggestBtn->setEnabled( false );
    d->ui.m_suggestions->setSelected( d->ui.m_suggestions->firstChild(), true );
  }
}

void
K3SpellDlg::init( const QString& _word, QStringList* _sugg,
                 const QString& context )
{
  sugg = _sugg;
  word = _word;

  d->ui.m_suggestions->clear();
  d->ui.m_suggestions->setSorting( NONSORTINGCOLUMN );
  for ( QStringList::Iterator it = _sugg->begin(); it != _sugg->end(); ++it ) {
      Q3ListViewItem *item = new Q3ListViewItem( d->ui.m_suggestions,
                                               d->ui.m_suggestions->lastItem() );
      item->setText( 0, *it );
  }

  kDebug(750) << "K3SpellDlg::init [" << word << "]";

  emit ready( true );

  d->ui.m_unknownWord->setText( _word );
  d->ui.m_contextLabel->setText( context );

  if ( sugg->count() == 0 ) {
    d->ui.m_replacement->setText( _word );
    d->ui.m_replaceBtn->setEnabled( false );
    d->ui.m_replaceAllBtn->setEnabled( false );
    d->ui.m_suggestBtn->setEnabled( false );
  } else {
    d->ui.m_replacement->setText( (*sugg)[0] );
    d->ui.m_replaceBtn->setEnabled( true );
    d->ui.m_replaceAllBtn->setEnabled( true );
    d->ui.m_suggestBtn->setEnabled( false );
    d->ui.m_suggestions->setSelected( d->ui.m_suggestions->firstChild(), true );
  }
}

void K3SpellDlg::standby()
{
    ready( false );
}

void
K3SpellDlg::slotProgress( unsigned int p )
{
  if (!progressbar)
    return;

  progbar->setValue( (int) p );
}

void
K3SpellDlg::textChanged( const QString & )
{
  d->ui.m_replaceBtn->setEnabled( true );
  d->ui.m_replaceAllBtn->setEnabled( true );
  d->ui.m_suggestBtn->setEnabled( true );
}

void
K3SpellDlg::slotSelectionChanged( Q3ListViewItem* item )
{
  if ( item )
    d->ui.m_replacement->setText( item->text( 0 ) );
}

/*
  exit functions
  */

void
K3SpellDlg::closeEvent( QCloseEvent * )
{
  cancel();
}

void
K3SpellDlg::done( int result )
{
  emit command( result );
}
void
K3SpellDlg::ignore()
{
  newword = word;
  done( KS_IGNORE );
}

void
K3SpellDlg::ignoreAll()
{
  newword = word;
  done( KS_IGNOREALL );
}

void
K3SpellDlg::add()
{
  newword = word;
  done( KS_ADD );
}


void
K3SpellDlg::cancel()
{
  newword = word;
  done( KS_CANCEL );
}

void
K3SpellDlg::replace()
{
  newword = d->ui.m_replacement->text();
  done( KS_REPLACE );
}

void
K3SpellDlg::stop()
{
  newword = word;
  done( KS_STOP );
}

void
K3SpellDlg::replaceAll()
{
  newword = d->ui.m_replacement->text();
  done( KS_REPLACEALL );
}

void
K3SpellDlg::suggest()
{
  newword = d->ui.m_replacement->text();
  done( KS_SUGGEST );
}

void
K3SpellDlg::slotConfigChanged()
{
  d->spellConfig->writeGlobalSettings();
  done( KS_CONFIG );
}

#include "k3spelldlg.moc"
