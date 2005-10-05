/**
 * dialog.cpp
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
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
#include "dialog.h"
#include "kspell2ui.h"

#include "backgroundchecker.h"
#include "broker.h"
#include "filter.h"
#include "dictionary.h"
#include "settings.h"

#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>

#include <q3listview.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qtimer.h>
#include <q3dict.h>

namespace KSpell2
{

//to initially disable sorting in the suggestions listview
#define NONSORTINGCOLUMN 2

class Dialog::Private
{
public:
    KSpell2UI *ui;
    QString   originalBuffer;
    BackgroundChecker *checker;

    Word   currentWord;
    QMap<QString, QString> replaceAllMap;
};

Dialog::Dialog( BackgroundChecker *checker,
                QWidget *parent, const char *name )
    : KDialogBase( parent, name, true,
                   i18n( "Check Spelling" ),
                   Help|Cancel|User1, Cancel,  true,
                   i18n( "&Finished" ) )
{
    d = new Private;

    d->checker = checker;

    initGui();
    initConnections();
    setMainWidget( d->ui );
}

Dialog::~Dialog()
{
    delete d;
}

void Dialog::initConnections()
{
    connect( d->ui->m_addBtn, SIGNAL(clicked()),
             SLOT(slotAddWord()) );
    connect( d->ui->m_replaceBtn, SIGNAL(clicked()),
             SLOT(slotReplaceWord()) );
    connect( d->ui->m_replaceAllBtn, SIGNAL(clicked()),
             SLOT(slotReplaceAll()) );
    connect( d->ui->m_skipBtn, SIGNAL(clicked()),
             SLOT(slotSkip()) );
    connect( d->ui->m_skipAllBtn, SIGNAL(clicked()),
             SLOT(slotSkipAll()) );
    connect( d->ui->m_suggestBtn, SIGNAL(clicked()),
             SLOT(slotSuggest()) );
    connect( d->ui->m_language, SIGNAL(activated(const QString&)),
             SLOT(slotChangeLanguage(const QString&)) );
    connect( d->ui->m_suggestions, SIGNAL(selectionChanged(Q3ListViewItem*)),
             SLOT(slotSelectionChanged(Q3ListViewItem*)) );
    connect( d->checker, SIGNAL(misspelling(const QString&, int)),
             SIGNAL(misspelling(const QString&, int)) );
    connect( d->checker, SIGNAL(misspelling(const QString&, int)),
             SLOT(slotMisspelling(const QString&, int)) );
    connect( d->checker, SIGNAL(done()),
             SLOT(slotDone()) );
    connect( d->ui->m_suggestions, SIGNAL(doubleClicked(Q3ListViewItem*, const QPoint&, int)),
             SLOT( slotReplaceWord() ) );
    connect( this, SIGNAL(user1Clicked()), this, SLOT(slotFinished()) );
    connect( this, SIGNAL(cancelClicked()),this, SLOT(slotCancel()) );
    connect( d->ui->m_replacement, SIGNAL(returnPressed()), this, SLOT(slotReplaceWord()) );
    connect( d->ui->m_autoCorrect, SIGNAL(clicked()),
             SLOT(slotAutocorrect()) );
    // button use by kword/kpresenter
    // hide by default
    d->ui->m_autoCorrect->hide();
}

void Dialog::initGui()
{
    d->ui = new KSpell2UI( this );
    d->ui->m_suggestions->setSorting( NONSORTINGCOLUMN );
    d->ui->m_language->clear();
    d->ui->m_language->insertStringList( d->checker->broker()->languages() );
    for ( int i = 0; !d->ui->m_language->text( i ).isNull(); ++i ) {
        QString ct = d->ui->m_language->text( i );
        if ( ct == d->checker->broker()->settings()->defaultLanguage() ) {
            d->ui->m_language->setCurrentItem( i );
            break;
        }
    }
}

void Dialog::activeAutoCorrect( bool _active )
{
    if ( _active )
        d->ui->m_autoCorrect->show();
    else
        d->ui->m_autoCorrect->hide();
}

void Dialog::slotAutocorrect()
{
    kdDebug()<<"void Dialog::slotAutocorrect()\n";
    emit autoCorrect(d->currentWord.word, d->ui->m_replacement->text() );
    slotReplaceWord();
}

void Dialog::slotFinished()
{
    kdDebug()<<"void Dialog::slotFinished() \n";
    emit stop();
    //FIXME: should we emit done here?
    emit done( d->checker->filter()->buffer() );
    accept();
}

void Dialog::slotCancel()
{
    kdDebug()<<"void Dialog::slotCancel() \n";
    emit cancel();
    reject();
}

QString Dialog::originalBuffer() const
{
    return d->originalBuffer;
}

QString Dialog::buffer() const
{
    return d->checker->filter()->buffer();
}

void Dialog::setBuffer( const QString& buf )
{
    d->originalBuffer = buf;
}

void Dialog::setFilter( Filter *filter )
{
    filter->setBuffer( d->checker->filter()->buffer() );
    d->checker->setFilter( filter );
}

void Dialog::updateDialog( const QString& word )
{
    d->ui->m_unknownWord->setText( word );
    d->ui->m_contextLabel->setText( d->checker->filter()->context() );
    QStringList suggs = d->checker->suggest( word );
    d->ui->m_replacement->setText( suggs.first() );
    fillSuggestions( suggs );
}

void Dialog::show()
{
    kdDebug()<<"Showing dialog"<<endl;
    if ( d->originalBuffer.isEmpty() )
        d->checker->start();
    else
        d->checker->checkText( d->originalBuffer );
}

void Dialog::slotAddWord()
{
   d->checker->addWord( d->currentWord.word ); 
   d->checker->continueChecking();
}

void Dialog::slotReplaceWord()
{
    emit replace( d->currentWord.word, d->currentWord.start,
                  d->ui->m_replacement->text() );
    d->checker->filter()->replace( d->currentWord, d->ui->m_replacement->text() );
    d->checker->continueChecking();
}

void Dialog::slotReplaceAll()
{
    d->replaceAllMap.insert( d->currentWord.word,
                             d->ui->m_replacement->text() );
    slotReplaceWord();
}

void Dialog::slotSkip()
{
    d->checker->continueChecking();
}

void Dialog::slotSkipAll()
{
    //### do we want that or should we have a d->ignoreAll list?
    d->checker->broker()->settings()->addWordToIgnore( d->ui->m_replacement->text() );
    d->checker->continueChecking();
}

void Dialog::slotSuggest()
{
    QStringList suggs = d->checker->suggest( d->ui->m_replacement->text() );
    fillSuggestions( suggs );
}

void Dialog::slotChangeLanguage( const QString& lang )
{
    d->checker->changeLanguage( lang );
    slotSuggest();
}

void Dialog::slotSelectionChanged( Q3ListViewItem *item )
{
    d->ui->m_replacement->setText( item->text( 0 ) );
}

void Dialog::fillSuggestions( const QStringList& suggs )
{
    d->ui->m_suggestions->clear();
    for ( QStringList::ConstIterator it = suggs.begin(); it != suggs.end(); ++it ) {
        new Q3ListViewItem( d->ui->m_suggestions, d->ui->m_suggestions->firstChild(),
                           *it );
    }
}

void Dialog::slotMisspelling(const QString& word, int start )
{
    kdDebug()<<"Dialog misspelling!!"<<endl;
    d->currentWord = Word( word, start );
    if ( d->replaceAllMap.contains( word ) ) {
        d->ui->m_replacement->setText( d->replaceAllMap[ word ] );
        slotReplaceWord();
    } else {
        updateDialog( word );
    }
    KDialogBase::show();
}

void Dialog::slotDone()
{
    kdDebug()<<"Dialog done!"<<endl;
    emit done( d->checker->filter()->buffer() );
    accept();
}

}

#include "dialog.moc"
