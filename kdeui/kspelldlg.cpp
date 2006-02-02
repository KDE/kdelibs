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

#include <QProgressBar>

#include <kconfig.h>
#include <klocale.h>

#include "ksconfig.h"
#include "kspelldlg.h"
#include "kspellui.h"

//to initially disable sorting in the suggestions listview
#define NONSORTINGCOLUMN 2

class KSpellDlg::KSpellDlgPrivate {
public:
  Ui_KSpellUI   ui;
  KSpellConfig* spellConfig;
};

KSpellDlg::KSpellDlg( QWidget * parent, bool _progressbar, bool _modal )
  : KDialog(parent, i18n("Check Spelling"), Help|Cancel|User1)
    , d(0),progressbar( false )
{
  setModal( _modal );
  setDefaultButton( Cancel );
  enableButtonSeparator( true );
  setButtonText( Cancel, i18n("&Finished") );

  KConfigGroup cg( KGlobal::config(),"KSpell" );
  kDebug() << (cg.readEntry("KSpell_DoSpellChecking")) << endl;
  kDebug() << "dospellchecking is " << cg.readEntry("KSpell_DoSpellChecking", false)  << endl;
  if ( (cg.readEntry("KSpell_DoSpellChecking", false) == true) )  //dospellcheck ?
  {
    Q_UNUSED( _progressbar );
    d = new KSpellDlgPrivate;
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
    d->spellConfig = new KSpellConfig( 0, 0 , false );
    d->spellConfig->fillDicts( d->ui.m_language );
    connect( d->ui.m_language, SIGNAL(activated(int)),
    	   d->spellConfig, SLOT(sSetDictionary(int)) );
    connect( d->spellConfig, SIGNAL(configChanged()),
           SLOT(slotConfigChanged()) );

    setHelp( "spelldlg", "kspell" );
    setMinimumSize( sizeHint() );
    emit ready( false );
  }
}

KSpellDlg::~KSpellDlg()
{
  delete d;
}

void
KSpellDlg::init( const QString & _word, QStringList * _sugg )
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
  kDebug(750) << "KSpellDlg::init [" << word << "]" << endl;

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
KSpellDlg::init( const QString& _word, QStringList* _sugg,
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

  kDebug(750) << "KSpellDlg::init [" << word << "]" << endl;

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

void
KSpellDlg::slotProgress( unsigned int p )
{
  if (!progressbar)
    return;

  progbar->setValue( (int) p );
}

void
KSpellDlg::textChanged( const QString & )
{
  d->ui.m_replaceBtn->setEnabled( true );
  d->ui.m_replaceAllBtn->setEnabled( true );
  d->ui.m_suggestBtn->setEnabled( true );
}

void
KSpellDlg::slotSelectionChanged( Q3ListViewItem* item )
{
  if ( item )
    d->ui.m_replacement->setText( item->text( 0 ) );
}

/*
  exit functions
  */

void
KSpellDlg::closeEvent( QCloseEvent * )
{
  cancel();
}

void
KSpellDlg::done( int result )
{
  emit command( result );
}
void
KSpellDlg::ignore()
{
  newword = word;
  done( KS_IGNORE );
}

void
KSpellDlg::ignoreAll()
{
  newword = word;
  done( KS_IGNOREALL );
}

void
KSpellDlg::add()
{
  newword = word;
  done( KS_ADD );
}


void
KSpellDlg::cancel()
{
  newword = word;
  done( KS_CANCEL );
}

void
KSpellDlg::replace()
{
  newword = d->ui.m_replacement->text();
  done( KS_REPLACE );
}

void
KSpellDlg::stop()
{
  newword = word;
  done( KS_STOP );
}

void
KSpellDlg::replaceAll()
{
  newword = d->ui.m_replacement->text();
  done( KS_REPLACEALL );
}

void
KSpellDlg::suggest()
{
  newword = d->ui.m_replacement->text();
  done( KS_SUGGEST );
}

void
KSpellDlg::slotConfigChanged()
{
  d->spellConfig->writeGlobalSettings();
  done( KS_CONFIG );
}

#include "kspelldlg.moc"
