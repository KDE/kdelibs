/*
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
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
*/

#include "k3syntaxhighlighter.h"

#include <QColor>
#include <QtCore/QRegExp>
#include <Qt3Support/Q3SyntaxHighlighter>
#include <QtCore/QTimer>

#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <k3sconfig.h>
#include <k3spell.h>
#include <Qt3Support/Q3Dict>
#include <QKeyEvent>

#include <kconfiggroup.h>
#include <fixx11h.h>

static int dummy, dummy2, dummy3, dummy4;
static int *Okay = &dummy;
static int *NotOkay = &dummy2;
static int *Ignore = &dummy3;
static int *Unknown = &dummy4;
static const int tenSeconds = 10*1000;

class K3SyntaxHighlighter::K3SyntaxHighlighterPrivate
{
public:
    QColor col1, col2, col3, col4, col5;
    SyntaxMode mode;
    bool enabled;
};

class K3SpellingHighlighter::K3SpellingHighlighterPrivate
{
public:

    K3SpellingHighlighterPrivate() :
	alwaysEndsWithSpace( true ),
	intraWordEditing( false ) {}

    QString currentWord;
    int currentPos;
    bool alwaysEndsWithSpace;
    QColor color;
    bool intraWordEditing;
};

class K3DictSpellingHighlighter::K3DictSpellingHighlighterPrivate
{
public:
    K3DictSpellingHighlighterPrivate() :
        mDict( 0 ),
	spell( 0 ),
        mSpellConfig( 0 ),
        rehighlightRequest( 0 ),
	wordCount( 0 ),
	errorCount( 0 ),
	autoReady( false ),
        globalConfig( true ),
	spellReady( false ) {}

    ~K3DictSpellingHighlighterPrivate() {
	delete rehighlightRequest;
	delete spell;
    }

    static Q3Dict<int>* sDict()
    {
	if (!statDict)
	    statDict = new Q3Dict<int>(50021);
	return statDict;
    }

    Q3Dict<int>* mDict;
    Q3Dict<int> autoDict;
    Q3Dict<int> autoIgnoreDict;
    static QObject *sDictionaryMonitor;
    K3Spell *spell;
    K3SpellConfig *mSpellConfig;
    QTimer *rehighlightRequest, *spellTimeout;
    QString spellKey;
    int wordCount, errorCount;
    int checksRequested, checksDone;
    int disablePercentage;
    int disableWordCount;
    bool completeRehighlightRequired;
    bool active, automatic, autoReady;
    bool globalConfig, spellReady;
private:
    static Q3Dict<int>* statDict;

};

Q3Dict<int>* K3DictSpellingHighlighter::K3DictSpellingHighlighterPrivate::statDict = 0;


K3SyntaxHighlighter::K3SyntaxHighlighter( Q3TextEdit *textEdit,
					  bool colorQuoting,
					  const QColor& depth0,
					  const QColor& depth1,
					  const QColor& depth2,
					  const QColor& depth3,
					  SyntaxMode mode )
    : Q3SyntaxHighlighter( textEdit ),d(new K3SyntaxHighlighterPrivate())
{

    d->enabled = colorQuoting;
    d->col1 = depth0;
    d->col2 = depth1;
    d->col3 = depth2;
    d->col4 = depth3;
    d->col5 = depth0;

    d->mode = mode;
}

K3SyntaxHighlighter::~K3SyntaxHighlighter()
{
    delete d;
}

int K3SyntaxHighlighter::highlightParagraph( const QString &text, int )
{
    if (!d->enabled) {
	setFormat( 0, text.length(), textEdit()->viewport()->paletteForegroundColor() );
	return 0;
    }

    QString simplified = text;
    simplified = simplified.replace( QRegExp( "\\s" ), QString() ).replace( '|', QLatin1String(">") );
    while ( simplified.startsWith( QLatin1String(">>>>") ) )
	simplified = simplified.mid(3);
    if	( simplified.startsWith( QLatin1String(">>>") ) || simplified.startsWith( QString::fromLatin1("> >	>") ) )
	setFormat( 0, text.length(), d->col2 );
    else if	( simplified.startsWith( QLatin1String(">>") ) || simplified.startsWith( QString::fromLatin1("> >") ) )
	setFormat( 0, text.length(), d->col3 );
    else if	( simplified.startsWith( QLatin1String(">") ) )
	setFormat( 0, text.length(), d->col4 );
    else
	setFormat( 0, text.length(), d->col5 );
    return 0;
}

K3SpellingHighlighter::K3SpellingHighlighter( Q3TextEdit *textEdit,
					    const QColor& spellColor,
					    bool colorQuoting,
					    const QColor& depth0,
					    const QColor& depth1,
					    const QColor& depth2,
					    const QColor& depth3 )
    : K3SyntaxHighlighter( textEdit, colorQuoting, depth0, depth1, depth2, depth3 ),d(new K3SpellingHighlighterPrivate())
{

    d->color = spellColor;
}

K3SpellingHighlighter::~K3SpellingHighlighter()
{
    delete d;
}

int K3SpellingHighlighter::highlightParagraph( const QString &text,
					      int paraNo )
{
    if ( paraNo == -2 )
	paraNo = 0;
    // leave #includes, diffs, and quoted replies alone
    QString diffAndCo( ">|" );

    bool isCode = diffAndCo.contains(text[0]);

    if ( !text.endsWith(' ') )
	d->alwaysEndsWithSpace = false;

    K3SyntaxHighlighter::highlightParagraph( text, -2 );

    if ( !isCode ) {
        int para, index;
	textEdit()->getCursorPosition( &para, &index );
	int len = text.length();
	if ( d->alwaysEndsWithSpace )
	    len--;

	d->currentPos = 0;
	d->currentWord = "";
	for ( int i = 0; i < len; i++ ) {
	    if ( !text[i].isLetter() && (!(text[i] == '\'')) ) {
		if ( ( para != paraNo ) ||
		    !intraWordEditing() ||
		    ( i - d->currentWord.length() > index ) ||
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
	if ( ( len > 0 && !text[len - 1].isLetter() ) ||
	     ( index + 1 ) != text.length() ||
	     para != paraNo )
	    flushCurrentWord();
    }
    return ++paraNo;
}

QStringList K3SpellingHighlighter::personalWords()
{
    QStringList l;
    l.append( "KMail" );
    l.append( "KOrganizer" );
    l.append( "KAddressBook" );
    l.append( "KHTML" );
    l.append( "KIO" );
    l.append( "KJS" );
    l.append( "Konqueror" );
    l.append( "K3Spell" );
    l.append( "Kontact" );
    l.append( "Qt" );
    return l;
}

void K3SpellingHighlighter::flushCurrentWord()
{
    while ( d->currentWord[0].isPunct() ) {
	d->currentWord = d->currentWord.mid( 1 );
	d->currentPos++;
    }

    QChar ch;
    while ( !d->currentWord.isEmpty() && ( ch = d->currentWord[(int) d->currentWord.length() - 1] ).isPunct() &&
	     ch != '(' && ch != '@' )
	d->currentWord.truncate( d->currentWord.length() - 1 );

    if ( !d->currentWord.isEmpty() ) {
	if ( isMisspelled( d->currentWord ) ) {
	    setFormat( d->currentPos, d->currentWord.length(), d->color );
//	    setMisspelled( d->currentPos, d->currentWord.length(), true );
	}
    }
    d->currentWord = "";
}

QObject *K3DictSpellingHighlighter::K3DictSpellingHighlighterPrivate::sDictionaryMonitor = 0;

K3DictSpellingHighlighter::K3DictSpellingHighlighter( Q3TextEdit *textEdit,
						    bool spellCheckingActive ,
						    bool autoEnable,
						    const QColor& spellColor,
						    bool colorQuoting,
						    const QColor& depth0,
						    const QColor& depth1,
						    const QColor& depth2,
						    const QColor& depth3,
                                                    K3SpellConfig *spellConfig )
    : K3SpellingHighlighter( textEdit, spellColor,
			    colorQuoting, depth0, depth1, depth2, depth3 ),d(new K3DictSpellingHighlighterPrivate())
{

    d->mSpellConfig = spellConfig;
    d->globalConfig = ( !spellConfig );
    d->automatic = autoEnable;
    d->active = spellCheckingActive;
    d->checksRequested = 0;
    d->checksDone = 0;
    d->completeRehighlightRequired = false;

    KConfigGroup cg( KSharedConfig::openConfig(), "K3Spell" );
    d->disablePercentage = cg.readEntry( "K3Spell_AsYouTypeDisablePercentage", QVariant(42 )).toInt();
    d->disablePercentage = qMin( d->disablePercentage, 101 );
    d->disableWordCount = cg.readEntry( "K3Spell_AsYouTypeDisableWordCount", QVariant(100 )).toInt();

    textEdit->installEventFilter( this );
    textEdit->viewport()->installEventFilter( this );

    d->rehighlightRequest = new QTimer(this);
    connect( d->rehighlightRequest, SIGNAL(timeout()),
	     this, SLOT(slotRehighlight()));
    d->spellTimeout = new QTimer(this);
    connect( d->spellTimeout, SIGNAL(timeout()),
	     this, SLOT(slotK3SpellNotResponding()));

    if ( d->globalConfig ) {
        d->spellKey = spellKey();

        if ( !d->sDictionaryMonitor )
            d->sDictionaryMonitor = new QObject();
    }
    else {
        d->mDict = new Q3Dict<int>(4001);
        connect( d->mSpellConfig, SIGNAL(configChanged()),
                 this, SLOT(slotLocalSpellConfigChanged()) );
    }

    slotDictionaryChanged();
}

K3DictSpellingHighlighter::~K3DictSpellingHighlighter()
{
    delete d->spell;
    d->spell = 0;
    delete d->mDict;
    d->mDict = 0;
    delete d;
}

void K3DictSpellingHighlighter::slotSpellReady( K3Spell *spell )
{
    kDebug(0) << "KDictSpellingHighlighter::slotSpellReady( " << spell << " )";
    if ( d->globalConfig ) {
        connect( d->sDictionaryMonitor, SIGNAL(destroyed()),
                 this, SLOT(slotDictionaryChanged()));
    }
    if ( spell != d->spell )
    {
        delete d->spell;
        d->spell = spell;
    }
    d->spellReady = true;
    const QStringList l = K3SpellingHighlighter::personalWords();
    for ( QStringList::ConstIterator it = l.begin(); it != l.end(); ++it ) {
        d->spell->addPersonal( *it );
    }
    connect( spell, SIGNAL(misspelling(QString,QStringList,uint)),
	     this, SLOT(slotMisspelling(QString,QStringList,uint)));
    connect( spell, SIGNAL(corrected(QString,QString,uint)),
	     this, SLOT(slotCorrected(QString,QString,uint)));
    d->checksRequested = 0;
    d->checksDone = 0;
    d->completeRehighlightRequired = true;
    d->rehighlightRequest->start( 0, true );
}

bool K3DictSpellingHighlighter::isMisspelled( const QString &word )
{
    if (!d->spellReady)
	return false;

    // This debug is expensive, only enable it locally
    //kDebug(0) << "KDictSpellingHighlighter::isMisspelled( \"" << word << "\" )";
    // Normally isMisspelled would look up a dictionary and return
    // true or false, but kspell is asynchronous and slow so things
    // get tricky...
    // For auto detection ignore signature and reply prefix
    if ( !d->autoReady )
	d->autoIgnoreDict.replace( word, Ignore );

    // "dict" is used as a cache to store the results of K3Spell
    Q3Dict<int>* dict = ( d->globalConfig ? d->sDict() : d->mDict );
    if ( !dict->isEmpty() && (*dict)[word] == NotOkay ) {
	if ( d->autoReady && ( d->autoDict[word] != NotOkay )) {
	    if ( !d->autoIgnoreDict[word] )
		++d->errorCount;
	    d->autoDict.replace( word, NotOkay );
	}

	return d->active;
    }
    if ( !dict->isEmpty() && (*dict)[word] == Okay ) {
	if ( d->autoReady && !d->autoDict[word] ) {
	    d->autoDict.replace( word, Okay );
	}
	return false;
    }

    if ((dict->isEmpty() || !((*dict)[word])) && d->spell ) {
	int para, index;
	textEdit()->getCursorPosition( &para, &index );
	++d->wordCount;
	dict->replace( word, Unknown );
	++d->checksRequested;
	if (currentParagraph() != para)
	    d->completeRehighlightRequired = true;
	d->spellTimeout->start( tenSeconds, true );
	d->spell->checkWord( word, false );
    }
    return false;
}

bool K3SpellingHighlighter::intraWordEditing() const
{
    return d->intraWordEditing;
}

void K3SpellingHighlighter::setIntraWordEditing( bool editing )
{
    d->intraWordEditing = editing;
}

void K3DictSpellingHighlighter::slotMisspelling (const QString &originalWord, const QStringList &suggestions,
                                                unsigned int pos)
{
    Q_UNUSED( suggestions );
    // kDebug() << suggestions.join( " " ).toLatin1();
    if ( d->globalConfig )
        d->sDict()->replace( originalWord, NotOkay );
    else
        d->mDict->replace( originalWord, NotOkay );

    //Emit this baby so that apps that want to have suggestions in a popup over
    //the misspelled word can catch them.
    emit newSuggestions( originalWord, suggestions, pos );
}

void K3DictSpellingHighlighter::slotCorrected(const QString &word,
					     const QString &,
					     unsigned int)

{
    Q3Dict<int>* dict = ( d->globalConfig ? d->sDict() : d->mDict );
    if ( !dict->isEmpty() && (*dict)[word] == Unknown ) {
        dict->replace( word, Okay );
    }
    ++d->checksDone;
    if (d->checksDone == d->checksRequested) {
	d->spellTimeout->stop();
      slotRehighlight();
    } else {
	d->spellTimeout->start( tenSeconds, true );
    }
}

void K3DictSpellingHighlighter::dictionaryChanged()
{
    QObject *oldMonitor = K3DictSpellingHighlighterPrivate::sDictionaryMonitor;
    K3DictSpellingHighlighterPrivate::sDictionaryMonitor = new QObject();
    K3DictSpellingHighlighterPrivate::sDict()->clear();
    delete oldMonitor;
}

void K3DictSpellingHighlighter::restartBackgroundSpellCheck()
{
    kDebug(0) << "KDictSpellingHighlighter::restartBackgroundSpellCheck()";
    slotDictionaryChanged();
}

void K3DictSpellingHighlighter::setActive( bool active )
{
    if ( active == d->active )
        return;

    d->active = active;
    rehighlight();
    if ( d->active )
        emit activeChanged( i18n("As-you-type spell checking enabled.") );
    else
        emit activeChanged( i18n("As-you-type spell checking disabled.") );
}

bool K3DictSpellingHighlighter::isActive() const
{
    return d->active;
}

void K3DictSpellingHighlighter::setAutomatic( bool automatic )
{
    if ( automatic == d->automatic )
        return;

    d->automatic = automatic;
    if ( d->automatic )
        slotAutoDetection();
}

bool K3DictSpellingHighlighter::automatic() const
{
    return d->automatic;
}

void K3DictSpellingHighlighter::slotRehighlight()
{
    kDebug(0) << "KDictSpellingHighlighter::slotRehighlight()";
    if (d->completeRehighlightRequired) {
	rehighlight();
    } else {
	int para, index;
	textEdit()->getCursorPosition( &para, &index );
	//rehighlight the current para only (undo/redo safe)
	textEdit()->insertAt( "", para, index );
    }
    if (d->checksDone == d->checksRequested)
	d->completeRehighlightRequired = false;
    QTimer::singleShot( 0, this, SLOT(slotAutoDetection()));
}

void K3DictSpellingHighlighter::slotDictionaryChanged()
{
    delete d->spell;
    d->spellReady = false;
    d->wordCount = 0;
    d->errorCount = 0;
    d->autoDict.clear();

    d->spell = new K3Spell( 0, i18n( "Incremental Spellcheck" ), this,
		SLOT(slotSpellReady(K3Spell*)), d->mSpellConfig );
}

void K3DictSpellingHighlighter::slotLocalSpellConfigChanged()
{
    kDebug(0) << "KDictSpellingHighlighter::slotSpellConfigChanged()";
    // the spell config has been changed, so we have to restart from scratch
    d->mDict->clear();
    slotDictionaryChanged();
}

QString K3DictSpellingHighlighter::spellKey()
{
    KSharedConfig::openConfig()->reparseConfiguration();
    KConfigGroup cg( KSharedConfig::openConfig(), "K3Spell" );
    QString key;
    key += QString::number( cg.readEntry( "K3Spell_NoRootAffix", QVariant(0 )).toInt());
    key += '/';
    key += QString::number( cg.readEntry( "K3Spell_RunTogether", QVariant(0 )).toInt());
    key += '/';
    key += cg.readEntry( "K3Spell_Dictionary", "" );
    key += '/';
    key += QString::number( cg.readEntry( "K3Spell_DictFromList", QVariant(false )).toInt());
    key += '/';
    key += QString::number( cg.readEntry( "K3Spell_Encoding", QVariant(KS_E_ASCII )).toInt());
    key += '/';
    key += QString::number( cg.readEntry( "K3Spell_Client", QVariant(KS_CLIENT_ISPELL )).toInt());
    return key;
}


// Automatic spell checking support
// In auto spell checking mode disable as-you-type spell checking
// iff more than one third of words are spelt incorrectly.
//
// Words in the signature and reply prefix are ignored.
// Only unique words are counted.

void K3DictSpellingHighlighter::slotAutoDetection()
{
    if ( !d->autoReady )
	return;

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
	if ( d->wordCount > 1 ) {
	    if ( d->active )
		emit activeChanged( i18n("As-you-type spell checking enabled.") );
	    else
		emit activeChanged( i18n( "Too many misspelled words. "
					  "As-you-type spell checking disabled." ) );
	}
	d->completeRehighlightRequired = true;
	d->rehighlightRequest->start( 100, true );
    }
}

void K3DictSpellingHighlighter::slotK3SpellNotResponding()
{
    static int retries = 0;
    if (retries < 10) {
        if ( d->globalConfig )
	    K3DictSpellingHighlighter::dictionaryChanged();
	else
	    slotLocalSpellConfigChanged();
    } else {
	setAutomatic( false );
	setActive( false );
    }
    ++retries;
}

bool K3DictSpellingHighlighter::eventFilter( QObject *o, QEvent *e)
{
    if (o == textEdit() && (e->type() == QEvent::FocusIn)) {
        if ( d->globalConfig ) {
            QString skey = spellKey();
            if ( d->spell && d->spellKey != skey ) {
                d->spellKey = skey;
                K3DictSpellingHighlighter::dictionaryChanged();
            }
        }
    }

    if (o == textEdit() && (e->type() == QEvent::KeyPress)) {
	QKeyEvent *k = static_cast<QKeyEvent *>(e);
	d->autoReady = true;
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
	     (( k->state() & Qt::ControlModifier ) &&
	      ((k->key() == Qt::Key_A) ||
	       (k->key() == Qt::Key_B) ||
	       (k->key() == Qt::Key_E) ||
	       (k->key() == Qt::Key_N) ||
	       (k->key() == Qt::Key_P))) ) {
	    if ( intraWordEditing() ) {
		setIntraWordEditing( false );
		d->completeRehighlightRequired = true;
		d->rehighlightRequest->start( 500, true );
	    }
	    if (d->checksDone != d->checksRequested) {
		// Handle possible change of paragraph while
		// words are pending spell checking
		d->completeRehighlightRequired = true;
		d->rehighlightRequest->start( 500, true );
	    }
	} else {
	    setIntraWordEditing( true );
	}
	if ( k->key() == Qt::Key_Space ||
	     k->key() == Qt::Key_Enter ||
	     k->key() == Qt::Key_Return ) {
	    QTimer::singleShot( 0, this, SLOT(slotAutoDetection()));
	}
    }

    else if ( o == textEdit()->viewport() &&
	 ( e->type() == QEvent::MouseButtonPress )) {
	d->autoReady = true;
	if ( intraWordEditing() ) {
	    setIntraWordEditing( false );
	    d->completeRehighlightRequired = true;
	    d->rehighlightRequest->start( 0, true );
	}
    }

    return false;
}

#include "moc_k3syntaxhighlighter.cpp"
