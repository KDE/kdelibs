/**
 * highlighter.cpp
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 * Copyright (C)  2006  Laurent Montel <montel@kde.org>
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

#include "highlighter.h"
#include "highlighter.moc"
#include "broker.h"
#include "dictionary.h"
#include "settings.h"

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include <qtextedit.h>
#include <qtimer.h>
#include <qcolor.h>
#include <QHash>
#include <QTextCursor>

namespace KSpell2 {

class Highlighter::Private
{
public:
    Filter     *filter;
    Broker::Ptr broker;
    Dictionary *dict;
    QHash<QString, Dictionary*>dictCache;
    QTextEdit *edit;
    bool active;
    bool automatic;
    bool completeRehighlightRequired;
    bool intraWordEditing;
    int disablePercentage;
    int disableWordCount;
    int wordCount, errorCount;
    QTimer *rehighlightRequest;
    QColor spellColor;
};

Highlighter::Highlighter( QTextEdit *textEdit,
                          const QString& configFile,
                          Filter *filter, const QColor& _col)
    : QSyntaxHighlighter( textEdit ),d(new Private)
{
    d->filter = filter;
    d->edit = textEdit;
    d->active = true;
    d->automatic = true;
    d->wordCount = 0;
    d->errorCount = 0;
    d->intraWordEditing = false;
    d->completeRehighlightRequired = false;

    d->spellColor = _col.isValid() ? _col : Qt::red;

    textEdit->installEventFilter( this );
    textEdit->viewport()->installEventFilter( this );

    if ( !configFile.isEmpty() )
        d->broker = Broker::openBroker( KSharedConfig::openConfig( configFile ).data() );
    else
        d->broker = Broker::openBroker();

    d->filter->setSettings( d->broker->settings() );
    d->dict   = d->broker->dictionary();
    Q_ASSERT( d->dict );
    d->dictCache.insert( d->broker->settings()->defaultLanguage(),
                         d->dict );

    d->disablePercentage = d->broker->settings()->disablePercentageWordError();

    d->disableWordCount = d->broker->settings()->disableWordErrorCount();

    //Add kde personal word
    const QStringList l = Highlighter::personalWords();
    for ( QStringList::ConstIterator it = l.begin(); it != l.end(); ++it ) {
        d->dict->addToSession( *it );
    }
    d->rehighlightRequest = new QTimer(this);
    connect( d->rehighlightRequest, SIGNAL( timeout() ),
	     this, SLOT( slotRehighlight() ));
    d->completeRehighlightRequired = true;
    d->rehighlightRequest->setInterval(0);
    d->rehighlightRequest->setSingleShot(true);
    d->rehighlightRequest->start();
}

Highlighter::~Highlighter()
{
    delete d;
}

void Highlighter::slotRehighlight()
{
    kDebug(0) << "Highlighter::slotRehighlight()" << endl;
    if (d->completeRehighlightRequired) {
#ifdef __GNUC__
#warning "Use qt-copy for rehighlight"
#endif
	// Qt 4.2 function, see patch: http://websvn.kde.org/trunk/qt-copy/patches/0112-ksyntaxhighlighter-rehighlight-document.diff
    rehighlight(); 

    } else {
	//rehighlight the current para only (undo/redo safe)
        QTextCursor cursor = d->edit->textCursor();
        cursor.insertText( "" );
    }
    //if (d->checksDone == d->checksRequested)
    //d->completeRehighlightRequired = false;
    QTimer::singleShot( 0, this, SLOT( slotAutoDetection() ));
}


QStringList Highlighter::personalWords()
{
    QStringList l;
    l.append( "KMail" );
    l.append( "KOrganizer" );
    l.append( "KAddressBook" );
    l.append( "KHTML" );
    l.append( "KIO" );
    l.append( "KJS" );
    l.append( "Konqueror" );
    l.append( "KSpell" );
    l.append( "Kontact" );
    l.append( "Qt" );
    return l;
}

bool Highlighter::automatic() const
{
    return d->automatic;
}

bool Highlighter::intraWordEditing() const
{
    return d->intraWordEditing;
}

void Highlighter::setIntraWordEditing( bool editing )
{
    d->intraWordEditing = editing;
}


void Highlighter::setAutomatic( bool automatic )
{
    if ( automatic  == d->automatic )
        return;

    d->automatic = automatic;
    if ( d->automatic )
        slotAutoDetection();
}

void Highlighter::slotAutoDetection()
{
    bool savedActive = d->active;

    if ( d->automatic ) {
	// tme = Too many errors
        bool tme = ( d->wordCount >= d->disableWordCount ) && ( d->errorCount * 100 >= d->disablePercentage * d->wordCount );
	if ( d->active && tme )
	    d->active = false;
	else if ( !d->active && !tme )
	    d->active = true;
    }
    if ( d->active != savedActive ) {
	if ( d->wordCount > 1 )
	    if ( d->active )
		emit activeChanged( i18n("As-you-type spell checking enabled.") );
	    else
		emit activeChanged( i18n( "Too many misspelled words. "
					  "As-you-type spell checking disabled." ) );
	d->completeRehighlightRequired = true;
	d->rehighlightRequest->setInterval(100);
        d->rehighlightRequest->setSingleShot(true);
        kDebug()<<" Highlighter::slotAutoDetection :"<<d->active<<endl;
    }

}

void Highlighter::setActive( bool active )
{
    if ( active == d->active )
        return;
    d->active = active;
#ifdef __GNUC__
#warning "Use qt-copy for rehighlight"
#endif
    // Qt 4.2 function, see patch: http://websvn.kde.org/trunk/qt-copy/patches/0112-ksyntaxhighlighter-rehighlight-document.diff
    rehighlight();
    

    if ( d->active )
        emit activeChanged( i18n("As-you-type spell checking enabled.") );
    else
        emit activeChanged( i18n("As-you-type spell checking disabled.") );
}

bool Highlighter::isActive() const
{
    return d->active;
}

void Highlighter::highlightBlock ( const QString & text )
{
    if ( text.isEmpty() || !d->active)
        return;
    QTextCursor cursor = d->edit->textCursor();
    int index = cursor.position();

    const int lengthPosition = text.length() - 1;

    if ( index != lengthPosition ||
         ( lengthPosition > 0 && !text[lengthPosition-1].isLetter() ) ) {
        d->filter->setBuffer( text );
        Word w = d->filter->nextWord();
        while ( !w.end ) {
            ++d->wordCount;
            if ( !d->dict->check( w.word ) ) {
                ++d->errorCount;
                setMisspelled( w.start, w.word.length() );
            } else
                unsetMisspelled( w.start, w.word.length() );
            w = d->filter->nextWord();
        }
    }
    //QTimer::singleShot( 0, this, SLOT(checkWords()) );
    setCurrentBlockState ( 0 );
}

Filter *Highlighter::currentFilter() const
{
    return d->filter;
}

void Highlighter::setCurrentFilter( Filter *filter )
{
    d->filter = filter;
    d->filter->setSettings( d->broker->settings() );
}

QString Highlighter::currentLanguage() const
{
    return d->dict->language();
}

void Highlighter::setCurrentLanguage( const QString& lang )
{
    if ( !d->dictCache.find( lang ) ) {
        Dictionary *dict = d->broker->dictionary( lang );
        if ( dict ) {
            d->dictCache.insert( lang, dict );
        } else {
            kDebug()<<"No dictionary for \""
                     <<lang
                     <<"\" staying with the current language."
                     <<endl;
            return;
        }
    }
    d->dict = d->dictCache[lang];
    d->wordCount = 0;
    d->errorCount = 0;
    if ( d->automatic )
        slotAutoDetection();
}

void Highlighter::setMisspelled( int start, int count )
{
    setFormat( start , count, d->spellColor );
}

void Highlighter::unsetMisspelled( int start, int count )
{
    setFormat( start, count, Qt::black );
}

bool Highlighter::eventFilter( QObject *o, QEvent *e)
{
#if 0
    if (o == textEdit() && (e->type() == QEvent::FocusIn)) {
        if ( d->globalConfig ) {
            QString skey = spellKey();
            if ( d->spell && d->spellKey != skey ) {
                d->spellKey = skey;
                KDictSpellingHighlighter::dictionaryChanged();
            }
        }
    }
#endif
    if (o == d->edit  && (e->type() == QEvent::KeyPress)) {
	QKeyEvent *k = static_cast<QKeyEvent *>(e);
	//d->autoReady = true;
	if (d->rehighlightRequest->isActive()) // try to stay out of the users way
	    d->rehighlightRequest->start( 500 );
	if ( k->key() == Qt::Key_Enter ||
	     k->key() == Qt::Key_Return ||
	     k->key() == Qt::Key_Up ||
	     k->key() == Qt::Key_Down ||
	     k->key() == Qt::Key_Left ||
	     k->key() == Qt::Key_Right ||
	     k->key() == Qt::Key_PageUp ||
	     k->key() == Qt::Key_PageDown ||
	     k->key() == Qt::Key_Home ||
	     k->key() == Qt::Key_End ||
	     (( Qt::ButtonState(k->modifiers()) & Qt::ControlModifier ) &&
	      ((k->key() == Qt::Key_A) ||
	       (k->key() == Qt::Key_B) ||
	       (k->key() == Qt::Key_E) ||
	       (k->key() == Qt::Key_N) ||
	       (k->key() == Qt::Key_P))) ) {
	    if ( intraWordEditing() ) {
		setIntraWordEditing( false );
		d->completeRehighlightRequired = true;
		d->rehighlightRequest->setInterval(500);
                d->rehighlightRequest->setSingleShot(true);
                d->rehighlightRequest->start();
	    }
#if 0
	    if (d->checksDone != d->checksRequested) {
		// Handle possible change of paragraph while
		// words are pending spell checking
		d->completeRehighlightRequired = true;
		d->rehighlightRequest->start( 500, true );
	    }
#endif
	} else {
	    setIntraWordEditing( true );
	}
	if ( k->key() == Qt::Key_Space ||
	     k->key() == Qt::Key_Enter ||
	     k->key() == Qt::Key_Return ) {
	    QTimer::singleShot( 0, this, SLOT( slotAutoDetection() ));
	}
    }

    else if ( o == d->edit->viewport() &&
	 ( e->type() == QEvent::MouseButtonPress )) {
	//d->autoReady = true;
	if ( intraWordEditing() ) {
	    setIntraWordEditing( false );
	    d->completeRehighlightRequired = true;
	    d->rehighlightRequest->setInterval(0);
            d->rehighlightRequest->setSingleShot(true);
            d->rehighlightRequest->start();
	}
    }
    return false;
}


/*
void Highlighter::checkWords()
{
    Word w = d->filter->nextWord();
    if ( !w.end ) {
        if ( !d->dict->check( w.word ) ) {
            setFormat( w.start, w.word.length(),
                       Qt::red );
        }
    }
}*/

}
