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
#include "ui_sonnetui.h"

#include "backgroundchecker.h"
#include "speller.h"
#include "filter_p.h"
#include "settings_p.h"

#include <kconfig.h>
#include <kguiitem.h>
#include <klocale.h>
#include <kdebug.h>

#include <QtGui/QListView>
#include <QtGui/QStringListModel>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtCore/QTimer>


namespace Sonnet
{

//to initially disable sorting in the suggestions listview
#define NONSORTINGCOLUMN 2

class Dialog::Private
{
public:
    Ui_SonnetUi ui;
    QStringListModel *suggestionsModel;
    QWidget *wdg;
    QString   originalBuffer;
    BackgroundChecker *checker;

    Word   currentWord;
    QMap<QString, QString> replaceAllMap;
    bool restart;//used when text is distributed across several qtextedits, eg in KAider

    QMap<QString, QString> dictsMap;
};

Dialog::Dialog(BackgroundChecker *checker,
               QWidget *parent)
    : KDialog(parent),
      d(new Private)
{
    setModal(true);
    setCaption(i18nc("@title:window", "Check Spelling"));
    setButtons(Help | Cancel | User1);
    setButtonGuiItem(User1, KGuiItem(i18nc("@action:button", "&Finished")));
    showButtonSeparator(true);

    setDefaultButton(User1);
    d->checker = checker;

    initGui();
    initConnections();
    setMainWidget(d->wdg);
    setHelp(QString(),"sonnet");
}

Dialog::~Dialog()
{
    delete d;
}

void Dialog::initConnections()
{
    connect( d->ui.m_addBtn, SIGNAL(clicked()),
             SLOT(slotAddWord()) );
    connect( d->ui.m_replaceBtn, SIGNAL(clicked()),
             SLOT(slotReplaceWord()) );
    connect( d->ui.m_replaceAllBtn, SIGNAL(clicked()),
             SLOT(slotReplaceAll()) );
    connect( d->ui.m_skipBtn, SIGNAL(clicked()),
             SLOT(slotSkip()) );
    connect( d->ui.m_skipAllBtn, SIGNAL(clicked()),
             SLOT(slotSkipAll()) );
    connect( d->ui.m_suggestBtn, SIGNAL(clicked()),
             SLOT(slotSuggest()) );
    connect( d->ui.m_language, SIGNAL(activated(const QString&)),
             SLOT(slotChangeLanguage(const QString&)) );
    connect( d->ui.m_suggestions, SIGNAL(clicked(QModelIndex)),
	     SLOT(slotSelectionChanged(QModelIndex)) );
    connect( d->checker, SIGNAL(misspelling(const QString&, int)),
             SLOT(slotMisspelling(const QString&, int)) );
    connect( d->checker, SIGNAL(done()),
             SLOT(slotDone()) );
    connect( d->ui.m_suggestions, SIGNAL(doubleClicked(QModelIndex)),
             SLOT( slotReplaceWord() ) );
    connect( this, SIGNAL(user1Clicked()), this, SLOT(slotFinished()) );
    connect( this, SIGNAL(cancelClicked()),this, SLOT(slotCancel()) );
    connect( d->ui.m_replacement, SIGNAL(returnPressed()), this, SLOT(slotReplaceWord()) );
    connect( d->ui.m_autoCorrect, SIGNAL(clicked()),
             SLOT(slotAutocorrect()) );
    // button use by kword/kpresenter
    // hide by default
    d->ui.m_autoCorrect->hide();
}

void Dialog::initGui()
{
    d->wdg = new QWidget(this);
    d->ui.setupUi(d->wdg);

    //d->ui.m_suggestions->setSorting( NONSORTINGCOLUMN );
    d->ui.m_language->clear();
    Speller speller = d->checker->speller();
    d->dictsMap = speller.availableDictionaries();
    QStringList langs = d->dictsMap.keys();
    d->ui.m_language->insertItems(0, langs);
    d->ui.m_language->setCurrentIndex(d->dictsMap.values().indexOf(
                                          speller.language()));
    d->restart = false;

    d->suggestionsModel=new QStringListModel(this);
    d->ui.m_suggestions->setModel(d->suggestionsModel);
}

void Dialog::activeAutoCorrect( bool _active )
{
    if ( _active )
        d->ui.m_autoCorrect->show();
    else
        d->ui.m_autoCorrect->hide();
}

void Dialog::slotAutocorrect()
{
    kDebug();
    emit autoCorrect(d->currentWord.word, d->ui.m_replacement->text() );
    slotReplaceWord();
}

void Dialog::slotFinished()
{
    kDebug();
    emit stop();
    //FIXME: should we emit done here?
    emit done(d->checker->text());
    emit spellCheckStatus(i18n("Spell check stopped."));
    accept();
}

void Dialog::slotCancel()
{
    kDebug();
    emit cancel();
    emit spellCheckStatus(i18n("Spell check canceled."));
    reject();
}

QString Dialog::originalBuffer() const
{
    return d->originalBuffer;
}

QString Dialog::buffer() const
{
    return d->checker->text();
}

void Dialog::setBuffer(const QString &buf)
{
    d->originalBuffer = buf;
    //it is possible to change buffer inside slot connected to done() signal
    d->restart = true;
}


void Dialog::updateDialog( const QString& word )
{
    d->ui.m_unknownWord->setText( word );
    d->ui.m_contextLabel->setText( d->checker->currentContext() );
    const QStringList suggs = d->checker->suggest( word );

    if (suggs.isEmpty())
        d->ui.m_replacement->clear();
    else
        d->ui.m_replacement->setText( suggs.first() );
    fillSuggestions( suggs );
}

void Dialog::show()
{
    kDebug()<<"Showing dialog";
    if (d->originalBuffer.isEmpty())
        d->checker->start();
    else
        d->checker->setText(d->originalBuffer);
}

void Dialog::slotAddWord()
{
   d->checker->addWordToPersonal(d->currentWord.word);
   d->checker->continueChecking();
}

void Dialog::slotReplaceWord()
{
    emit replace( d->currentWord.word, d->currentWord.start,
                  d->ui.m_replacement->text() );
    d->checker->replace(d->currentWord.start,
                        d->currentWord.word,
                        d->ui.m_replacement->text());
    d->checker->continueChecking();
}

void Dialog::slotReplaceAll()
{
    d->replaceAllMap.insert( d->currentWord.word,
                             d->ui.m_replacement->text() );
    slotReplaceWord();
}

void Dialog::slotSkip()
{
    d->checker->continueChecking();
}

void Dialog::slotSkipAll()
{
    //### do we want that or should we have a d->ignoreAll list?
    Speller speller = d->checker->speller();
    speller.addToPersonal(d->currentWord.word);
    d->checker->setSpeller(speller);
    d->checker->continueChecking();
}

void Dialog::slotSuggest()
{
    QStringList suggs = d->checker->suggest( d->ui.m_replacement->text() );
    fillSuggestions( suggs );
}

void Dialog::slotChangeLanguage(const QString &lang)
{
    Speller speller = d->checker->speller();
    QString languageCode = d->dictsMap[lang];
    if (!languageCode.isEmpty()) {
        d->checker->changeLanguage(languageCode);
        slotSuggest();
        emit languageChanged(languageCode);
    }
}

void Dialog::slotSelectionChanged( QModelIndex item )
{
    d->ui.m_replacement->setText( item.data().toString() );
}

void Dialog::fillSuggestions( const QStringList& suggs )
{
    d->suggestionsModel->setStringList(suggs);
}

void Dialog::slotMisspelling(const QString& word, int start)
{
    emit misspelling(word, start);
    //NOTE this is HACK I had to introduce because BackgroundChecker lacks 'virtual' marks on methods
    //this dramatically reduces spellchecking time in Lokalize
    //as this doesn't fetch suggestions for words that are present in msgid
    if (!updatesEnabled())
        return;

    kDebug()<<"Dialog misspelling!!";
    d->currentWord = Word( word, start );
    if ( d->replaceAllMap.contains( word ) ) {
        d->ui.m_replacement->setText( d->replaceAllMap[ word ] );
        slotReplaceWord();
    } else {
        updateDialog( word );
    }
    KDialog::show();
}

void Dialog::slotDone()
{
    kDebug()<<"Dialog done!";
    d->restart=false;
    emit done(d->checker->text());
    if (d->restart)
    {
        d->checker->setText(d->originalBuffer);
        d->restart=false;
    }
    else
    {
        emit spellCheckStatus(i18n("Spell check complete."));
        accept();
    }
}

}

#include "dialog.moc"
