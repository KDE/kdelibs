/*
 ksyntaxhighlighter.cpp

 Copyright (c) 2003 Trolltech AS
 Copyright (c) 2003 Scott Wheeler <wheeler@kde.org>

 This file is part of the KDE libraries

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License version 2 as published by the Free Software Foundation.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.
*/

#include <qcolor.h>
#include <qregexp.h>
#include <qsyntaxhighlighter.h>
#include <qtimer.h>

#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kspell.h>
#include <kapplication.h>

#include "ksyntaxhighlighter.h"

static int dummy, dummy2, dummy3;
static int *Okay = &dummy;
static int *NotOkay = &dummy2;
static int *Ignore = &dummy3;

class KSyntaxHighlighter::KSyntaxHighlighterPrivate
{
public:
    QColor col1, col2, col3, col4, col5;
    SyntaxMode mode;
    bool enabled;
};

class KSpellingHighlighter::KSpellingHighlighterPrivate
{
public:

    KSpellingHighlighterPrivate() :
	alwaysEndsWithSpace( true ),
	intraWordEditing( false ) {}

    QString currentWord;
    int currentPos;
    bool alwaysEndsWithSpace;
    QColor color;
    bool intraWordEditing;
};

class KDictSpellingHighlighter::KDictSpellingHighlighterPrivate
{
public:
    KDictSpellingHighlighterPrivate() :
	spell( 0 ),
	wordCount( 0 ),
	errorCount( 0 ),
	autoReady( false ) {}

    ~KDictSpellingHighlighterPrivate() {
	delete spell;
    }

    static QDict<int>* sDict()
    {
	if (!statDict)
	    statDict = new QDict<int>(50021);
	return statDict;
    }

    QDict<int> autoDict;
    QDict<int> autoIgnoreDict;
    static QObject *sDictionaryMonitor;
    KSpell *spell;
    QTimer *rehighlightRequest;
    QString spellKey;
    int wordCount, errorCount;
    bool active, automatic, autoReady;
private:
    static QDict<int>* statDict;

};

QDict<int>* KDictSpellingHighlighter::KDictSpellingHighlighterPrivate::statDict = 0;


KSyntaxHighlighter::KSyntaxHighlighter( QTextEdit *textEdit,
					  bool colorQuoting,
					  QColor depth0,
					  QColor depth1,
					  QColor depth2,
					  QColor depth3,
					  SyntaxMode mode )
    : QSyntaxHighlighter( textEdit )
{
    d = new KSyntaxHighlighterPrivate();

    d->enabled = colorQuoting;
    d->col1 = depth0;
    d->col2 = depth1;
    d->col3 = depth2;
    d->col4 = depth3;
    d->col5 = depth0;

    d->mode = mode;
}

KSyntaxHighlighter::~KSyntaxHighlighter()
{
    delete d;
}

int KSyntaxHighlighter::highlightParagraph( const QString &text, int )
{
    if (!d->enabled)
	return 0;
    QString simplified = text;
    simplified = simplified.replace( QRegExp( "\\s" ), "" ).replace( "|", ">" );
    while ( simplified.startsWith( ">>>>" ) )
	simplified = simplified.mid(3);
    if	( simplified.startsWith( ">>>" ) || simplified.startsWith( "> >	>" ) )
	setFormat( 0, text.length(), d->col2 );
    else if	( simplified.startsWith( ">>" )	|| simplified.startsWith( "> >"	) )
	setFormat( 0, text.length(), d->col3 );
    else if	( simplified.startsWith( ">" ) )
	setFormat( 0, text.length(), d->col4 );
    else
	setFormat( 0, text.length(), d->col5 );
    return 0;
}

KSpellingHighlighter::KSpellingHighlighter( QTextEdit *textEdit,
					    QColor spellColor,
					    bool colorQuoting,
					    QColor depth0,
					    QColor depth1,
					    QColor depth2,
					    QColor depth3 )
    : KSyntaxHighlighter( textEdit, colorQuoting, depth0, depth1, depth2, depth3 )
{
    d = new KSpellingHighlighterPrivate();

    d->color = spellColor;
}

KSpellingHighlighter::~KSpellingHighlighter()
{
    delete d;
}

int KSpellingHighlighter::highlightParagraph( const QString &text,
					      int paraNo )
{
    if ( paraNo == -2 )
	paraNo = 0;
    // leave #includes, diffs, and quoted replies alone
    QString diffAndCo( ">|" );

    bool isCode = diffAndCo.find(text[0]) != -1;

    if ( !text.endsWith(" ") )
	d->alwaysEndsWithSpace = false;

    KSyntaxHighlighter::highlightParagraph( text, -2 );

    if ( !isCode ) {
        int para, index;
	textEdit()->getCursorPosition( &para, &index );
	int len = text.length();
	if ( d->alwaysEndsWithSpace )
	    len--;

	d->currentPos = 0;
	d->currentWord = "";
	for ( int i = 0; i < len; i++ ) {
	    if ( text[i].isSpace() || text[i] == '-' ) {
		if ( ( para != paraNo ) ||
		    !intraWordEditing() ||
		    ( i - d->currentWord.length() > (uint)index ) ||
		    ( i < index ) ) {
		    flushCurrentWord();
		} else {
		    d->currentWord = "";
		}
		d->currentPos = i + 1;
	    } else {
		d->currentWord += text[i];
	    }
	}
	if ( !text[len - 1].isLetter() ||
	     (uint)( index + 1 ) != text.length() ||
	     para != paraNo )
	    flushCurrentWord();
    }
    return ++paraNo;
}

QStringList KSpellingHighlighter::personalWords()
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

void KSpellingHighlighter::flushCurrentWord()
{
    while ( d->currentWord[0].isPunct() ) {
	d->currentWord = d->currentWord.mid( 1 );
	d->currentPos++;
    }

    QChar ch;
    while ( ( ch = d->currentWord[(int) d->currentWord.length() - 1] ).isPunct() &&
	     ch != '(' && ch != '@' )
	d->currentWord.truncate( d->currentWord.length() - 1 );

    if ( !d->currentWord.isEmpty() ) {
	if ( isMisspelled( d->currentWord ) )
	    setFormat( d->currentPos, d->currentWord.length(), d->color );
//	    setMisspelled( d->currentPos, d->currentWord.length(), true );
    }
    d->currentWord = "";
}

QObject *KDictSpellingHighlighter::KDictSpellingHighlighterPrivate::sDictionaryMonitor = 0;

KDictSpellingHighlighter::KDictSpellingHighlighter( QTextEdit *textEdit,
						    bool spellCheckingActive ,
						    bool autoEnable,
						    QColor spellColor,
						    bool colorQuoting,
						    QColor depth0,
						    QColor depth1,
						    QColor depth2,
						    QColor depth3 )
    : KSpellingHighlighter( textEdit, spellColor,
			    colorQuoting, depth0, depth1, depth2, depth3 )
{
    d = new KDictSpellingHighlighterPrivate();

    d->automatic = autoEnable;
    d->active = spellCheckingActive;

    textEdit->installEventFilter( this );
    textEdit->viewport()->installEventFilter( this );

    d->rehighlightRequest = new QTimer();
    connect( d->rehighlightRequest, SIGNAL( timeout() ),
	     this, SLOT( slotRehighlight() ));

    d->spellKey = spellKey();

    if ( !d->sDictionaryMonitor )
	d->sDictionaryMonitor = new QObject();

    slotDictionaryChanged();
    startTimer( 2 * 1000 );
}

KDictSpellingHighlighter::~KDictSpellingHighlighter()
{
    delete d->spell;
    d->spell = 0;
    delete d;
}

void KDictSpellingHighlighter::slotSpellReady( KSpell *spell )
{
    connect( d->sDictionaryMonitor, SIGNAL( destroyed()),
	     this, SLOT( slotDictionaryChanged() ));
    if ( spell != d->spell )
    {
        delete d->spell;
        d->spell = spell;
    }
    const QStringList l = KSpellingHighlighter::personalWords();
    for ( QStringList::ConstIterator it = l.begin(); it != l.end(); ++it ) {
        d->spell->addPersonal( *it );
    }
    connect( spell, SIGNAL( misspelling( const QString &, const QStringList &, unsigned int )),
	     this, SLOT( slotMisspelling( const QString &, const QStringList &, unsigned int )));
    d->rehighlightRequest->start( 0, true );
}

bool KDictSpellingHighlighter::isMisspelled( const QString &word )
{
    // Normally isMisspelled would look up a dictionary and return
    // true or false, but kspell is asynchronous and slow so things
    // get tricky...

    // For auto detection ignore signature and reply prefix
    if ( !d->autoReady )
	d->autoIgnoreDict.replace( word, Ignore );

    // "dict" is used as a cache to store the results of KSpell
    if ( !d->sDict()->isEmpty() && (*d->sDict())[word] == NotOkay ) {
	if ( d->autoReady && ( d->autoDict[word] != NotOkay )) {
	    if ( !d->autoIgnoreDict[word] ) {
		++d->errorCount;
		if ( d->autoDict[word] != Okay )
		    ++d->wordCount;
	    }
	    d->autoDict.replace( word, NotOkay );
	}
	return d->active;
    }
    if ( !d->sDict()->isEmpty() && (*d->sDict())[word] == Okay ) {
	if ( d->autoReady && !d->autoDict[word] ) {
	    d->autoDict.replace( word, Okay );
	    if ( !d->autoIgnoreDict[word] )
		++d->wordCount;
	}
	return false;
    }

    // there is no 'spelt correctly' signal so default to Okay
    d->sDict()->replace( word, Okay );

    // yes I tried checkWord, the docs lie and it didn't give useful signals :-(
    if ( d->spell )
	d->spell->check( word, false );
    return false;
}

bool KSpellingHighlighter::intraWordEditing() const
{
    return d->intraWordEditing;
}

void KSpellingHighlighter::setIntraWordEditing( bool editing )
{
    d->intraWordEditing = editing;
}

void KDictSpellingHighlighter::slotMisspelling (const QString &originalWord, const QStringList &suggestions, unsigned int)
{
    Q_UNUSED( suggestions );
    // kdDebug() << suggestions.join( " " ).latin1() << endl;
    d->sDict()->replace( originalWord, NotOkay );

    // this is slow but since kspell is async this will have to do for now
    d->rehighlightRequest->start( 0, true );
}

void KDictSpellingHighlighter::dictionaryChanged()
{
    QObject *oldMonitor = KDictSpellingHighlighterPrivate::sDictionaryMonitor;
    KDictSpellingHighlighterPrivate::sDictionaryMonitor = new QObject();
    KDictSpellingHighlighterPrivate::sDict()->clear();
    delete oldMonitor;
}

void KDictSpellingHighlighter::slotRehighlight()
{
    rehighlight();
    QTimer::singleShot( 0, this, SLOT( slotAutoDetection() ));
}

void KDictSpellingHighlighter::slotDictionaryChanged()
{
    delete d->spell;
    d->spell = 0;
    d->wordCount = 0;
    d->errorCount = 0;
    d->autoDict.clear();

    // SCOTT: Hmm, this looks suspecious.  Shouldn't this be d->spell = ...?

    d->spell = new KSpell( 0, i18n( "Incremental Spellcheck - KMail" ), this,
		SLOT( slotSpellReady( KSpell * ) ));
}

QString KDictSpellingHighlighter::spellKey()
{
    KConfig *config = KGlobal::config();
    KConfigGroupSaver cs( config, "KSpell" );
    config->reparseConfiguration();
    QString key;
    key += QString::number( config->readNumEntry( "KSpell_NoRootAffix", 0 ));
    key += '/';
    key += QString::number( config->readNumEntry( "KSpell_RunTogether", 0 ));
    key += '/';
    key += config->readEntry( "KSpell_Dictionary", "" );
    key += '/';
    key += QString::number( config->readNumEntry( "KSpell_DictFromList", false ));
    key += '/';
    key += QString::number( config->readNumEntry( "KSpell_Encoding", KS_E_ASCII ));
    key += '/';
    key += QString::number( config->readNumEntry( "KSpell_Client", KS_CLIENT_ISPELL ));
    return key;
}


// Automatic spell checking support
// In auto spell checking mode disable as-you-type spell checking
// iff more than one third of words are spelt incorrectly.
//
// Words in the signature and reply prefix are ignored.
// Only unique words are counted.

void KDictSpellingHighlighter::slotAutoDetection()
{
    if ( !d->autoReady )
	return;

    bool savedActive = d->active;

    if ( d->automatic ) {
	if ( d->active && ( d->errorCount * 3 >= d->wordCount ))
	    d->active = false;
	else if ( !d->active && ( d->errorCount * 3 < d->wordCount ))
	    d->active = true;
    }
    if ( d->active != savedActive ) {
	if ( d->wordCount > 1 )
	    if ( d->active )
		emit activeChanged( i18n("As-you-type spell checking enabled.") );
	    else
		emit activeChanged( i18n( "Too many misspelled words. "
					  "As-you-type spell checking disabled." ) );
	d->rehighlightRequest->start( 100, true );
    }
}

bool KDictSpellingHighlighter::eventFilter( QObject *o, QEvent *e)
{
    if (o == textEdit() && (e->type() == QEvent::FocusIn)) {
	if ( d->spell && d->spellKey != spellKey() ) {
	    d->spellKey = spellKey();
	    KDictSpellingHighlighter::dictionaryChanged();
	}
    }

    if (o == textEdit() && (e->type() == QEvent::KeyPress)) {
	QKeyEvent *k = static_cast<QKeyEvent *>(e);
	d->autoReady = true;
	if ( k->key() == Key_Enter ||
	     k->key() == Key_Return ||
	     k->key() == Key_Up ||
	     k->key() == Key_Down ||
	     k->key() == Key_Left ||
	     k->key() == Key_Right ||
	     k->key() == Key_PageUp ||
	     k->key() == Key_PageDown ||
	     k->key() == Key_Home ) {
	    if ( intraWordEditing() ) {
		setIntraWordEditing( false );
		d->rehighlightRequest->start( 0, true );
	    }
	} else {
	    setIntraWordEditing( true );
	}
	if ( k->key() == Key_Space ||
	     k->key() == Key_Enter ||
	     k->key() == Key_Return ) {
	    QTimer::singleShot( 0, this, SLOT( slotAutoDetection() ));
	}
    }

    if ( o == textEdit()->viewport() &&
	 ( e->type() == QEvent::MouseButtonPress )) {
	if ( intraWordEditing() ) {
	    setIntraWordEditing( false );
	    d->rehighlightRequest->start( 0, true );
	}
    }

    return false;
}

#include "ksyntaxhighlighter.moc"
