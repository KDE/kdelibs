/**
 * dialog.cpp
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 * Copyright (C)  2009  Michel Ludwig <michel.ludwig@kdemail.net>
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
#include <kmessagebox.h>
#include <kprogressdialog.h>
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

class ReadOnlyStringListModel: public QStringListModel
{
public:
    ReadOnlyStringListModel(QObject* parent):QStringListModel(parent){}
    Qt::ItemFlags flags(const QModelIndex& index) const {Q_UNUSED(index); return Qt::ItemIsEnabled | Qt::ItemIsSelectable;}
};

class Dialog::Private
{
public:
    Ui_SonnetUi ui;
    ReadOnlyStringListModel *suggestionsModel;
    QWidget *wdg;
    KProgressDialog *progressDialog;
    QString   originalBuffer;
    BackgroundChecker *checker;

    Word   currentWord;
    QMap<QString, QString> replaceAllMap;
    bool restart;//used when text is distributed across several qtextedits, eg in KAider

    QMap<QString, QString> dictsMap;

    int progressDialogTimeout;
    bool showCompletionMessageBox;
    bool spellCheckContinuedAfterReplacement;
    bool canceled;

    void deleteProgressDialog(bool directly)
    {
      if (progressDialog)
      {
        progressDialog->hide();
        if (directly)
        {
          delete progressDialog;
        }
        else
        {
          progressDialog->deleteLater();
        }
        progressDialog = NULL;
      }
    }
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

    setDefaultButton(User1);
    d->checker = checker;

    d->canceled = false;
    d->showCompletionMessageBox = false;
    d->spellCheckContinuedAfterReplacement = true;
    d->progressDialogTimeout = -1;
    d->progressDialog = NULL;

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
    setGuiEnabled(false);

    //d->ui.m_suggestions->setSorting( NONSORTINGCOLUMN );
    updateDictionaryComboBox();
    d->restart = false;

    d->suggestionsModel=new ReadOnlyStringListModel(this);
    d->ui.m_suggestions->setModel(d->suggestionsModel);
}

void Dialog::activeAutoCorrect( bool _active )
{
    if ( _active )
        d->ui.m_autoCorrect->show();
    else
        d->ui.m_autoCorrect->hide();
}

void Dialog::showProgressDialog(int timeout)
{
  d->progressDialogTimeout = timeout;
}

void Dialog::showSpellCheckCompletionMessage( bool b )
{
  d->showCompletionMessageBox = b;
}

void Dialog::setSpellCheckContinuedAfterReplacement( bool b )
{
  d->spellCheckContinuedAfterReplacement = b;
}

void Dialog::slotAutocorrect()
{
    setGuiEnabled(false);
    setProgressDialogVisible(true);
    kDebug();
    emit autoCorrect(d->currentWord.word, d->ui.m_replacement->text() );
    slotReplaceWord();
}

void Dialog::setGuiEnabled(bool b)
{
  d->wdg->setEnabled(b);
}

void Dialog::setProgressDialogVisible(bool b)
{
  if (!b)
  {
    d->deleteProgressDialog(true);
  }
  else if(d->progressDialogTimeout >= 0)
  {
    if (d->progressDialog)
    {
      return;
    }
    d->progressDialog = new KProgressDialog(this, i18nc("@title:window", "Check Spelling"),
                                                    i18nc("progress label", "Spell checking in progress..."));
    d->progressDialog->setModal(true);
    d->progressDialog->setAutoClose(false);
    d->progressDialog->setAutoReset(false);
    // create an 'indefinite' progress box as we currently cannot get progress feedback from
    // the speller
    d->progressDialog->progressBar()->reset();
    d->progressDialog->progressBar()->setRange(0, 0);
    d->progressDialog->progressBar()->setValue(0);
    connect(d->progressDialog, SIGNAL(cancelClicked()), this, SLOT(slotCancel()));
    d->progressDialog->setMinimumDuration(d->progressDialogTimeout);
  }
}

void Dialog::slotFinished()
{
    kDebug();
    setProgressDialogVisible(false);
    emit stop();
    //FIXME: should we emit done here?
    emit done(d->checker->text());
    emit spellCheckStatus(i18n("Spell check stopped."));
    accept();
}

void Dialog::slotCancel()
{
    kDebug();
    d->canceled = true;
    d->deleteProgressDialog(false); // this method can be called in response to
                                    // pressing 'Cancel' on the dialog
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

void Dialog::updateDictionaryComboBox()
{
    d->ui.m_language->clear();
    Speller speller = d->checker->speller();
    d->dictsMap = speller.availableDictionaries();
    QStringList langs = d->dictsMap.keys();
    d->ui.m_language->insertItems(0, langs);
    d->ui.m_language->setCurrentIndex(d->dictsMap.values().indexOf(
                                          speller.language()));
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
    d->canceled = false;
    updateDictionaryComboBox();
    if (d->originalBuffer.isEmpty())
    {
        d->checker->start();
    }
    else
    {
        d->checker->setText(d->originalBuffer);
    }
    setProgressDialogVisible(true);
}

void Dialog::slotAddWord()
{
   setGuiEnabled(false);
   setProgressDialogVisible(true);
   d->checker->addWordToPersonal(d->currentWord.word);
   d->checker->continueChecking();
}

void Dialog::slotReplaceWord()
{
    setGuiEnabled(false);
    setProgressDialogVisible(true);
    QString replacementText = d->ui.m_replacement->text();
    emit replace( d->currentWord.word, d->currentWord.start,
                  replacementText );

    if( d->spellCheckContinuedAfterReplacement ) {
      d->checker->replace(d->currentWord.start,
                          d->currentWord.word,
                          replacementText);
      d->checker->continueChecking();
    }
    else {
      d->checker->stop();
    }
}

void Dialog::slotReplaceAll()
{
    setGuiEnabled(false);
    setProgressDialogVisible(true);
    d->replaceAllMap.insert( d->currentWord.word,
                             d->ui.m_replacement->text() );
    slotReplaceWord();
}

void Dialog::slotSkip()
{
    setGuiEnabled(false);
    setProgressDialogVisible(true);
    d->checker->continueChecking();
}

void Dialog::slotSkipAll()
{
    setGuiEnabled(false);
    setProgressDialogVisible(true);
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

void Dialog::slotSelectionChanged(const QModelIndex &item)
{
    d->ui.m_replacement->setText( item.data().toString() );
}

void Dialog::fillSuggestions( const QStringList& suggs )
{
    d->suggestionsModel->setStringList(suggs);
}

void Dialog::slotMisspelling(const QString& word, int start)
{
    setGuiEnabled(true);
    setProgressDialogVisible(false);
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
    d->restart=false;
    QString currentLanguage = d->checker->speller().language();
    emit done(d->checker->text());
    if (d->restart)
    {
        if (currentLanguage != d->checker->speller().language())
        {
          updateDictionaryComboBox();
        }
        d->checker->setText(d->originalBuffer);
        d->restart=false;
    }
    else
    {
        setProgressDialogVisible(false);
        kDebug()<<"Dialog done!";
        emit spellCheckStatus(i18n("Spell check complete."));
        accept();
        if(!d->canceled && d->showCompletionMessageBox)
        {
          KMessageBox::information(this, i18n("Spell check complete."), i18nc("@title:window", "Check Spelling"));
        }
    }
}

}

#include "dialog.moc"
