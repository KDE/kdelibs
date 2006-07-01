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

#include <QtCore/QHash>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtGui/QKeyEvent>
#include <QtGui/QTextEdit>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kspell.h>

#include "ksyntaxhighlighter.h"

static int dummy, dummy2, dummy3, dummy4;
static int *Okay = &dummy;
static int *NotOkay = &dummy2;
static int *Ignore = &dummy3;
static int *Unknown = &dummy4;
static const int tenSeconds = 10*1000;

class KSyntaxHighlighter::Private
{
  public:
    QColor mColor1;
    QColor mColor2;
    QColor mColor3;
    QColor mColor4;
    QColor mColor5;
    SyntaxMode mMode;
    bool mEnabled;
};

class KSpellingHighlighter::Private
{
  public:

    Private( KSpellingHighlighter *parent )
      : mParent( parent ),
        mAlwaysEndsWithSpace( true ),
        mIntraWordEditing( false )
    {
    }

    void flushCurrentWord();

    KSpellingHighlighter *mParent;
    QString mCurrentWord;
    int mCurrentPos;
    bool mAlwaysEndsWithSpace;
    bool mIntraWordEditing;
    QColor mColor;
};

void KSpellingHighlighter::Private::flushCurrentWord()
{
  while ( mCurrentWord[ 0 ].isPunct() ) {
    mCurrentWord = mCurrentWord.mid( 1 );
    mCurrentPos++;
  }

  QChar ch;
  while ( !mCurrentWord.isEmpty() && ( ch = mCurrentWord[(int) mCurrentWord.length() - 1] ).isPunct() &&
          ch != '(' && ch != '@' )
    mCurrentWord.truncate( mCurrentWord.length() - 1 );

  if ( !mCurrentWord.isEmpty() ) {
    if ( mParent->isMisspelled( mCurrentWord ) ) {
      mParent->setFormat( mCurrentPos, mCurrentWord.length(), mColor );
//      mParent->setMisspelled( mCurrentPos, mCurrentWord.length(), true );
    }
  }

  mCurrentWord = "";
}

class KDictSpellingHighlighter::Private
{
  public:
    Private()
      : mDict( 0 ),
        mSpell( 0 ),
        mSpellConfig( 0 ),
        mRehighlightRequest( 0 ),
        mWordCount( 0 ),
        mErrorCount( 0 ),
        mAutoReady( false ),
        mGlobalConfig( true ),
        mSpellReady( false )
    {
    }

    ~Private()
    {
      delete mRehighlightRequest;
      mRehighlightRequest = 0;

      delete mSpell;
      mSpell = 0;

      delete mDict;
      mDict = 0;
    }

    static QHash<QString, int*>* sDict()
    {
      if ( !statDict )
        statDict = new QHash<QString, int*>();

      return statDict;
    }

    QHash<QString, int*>* mDict;
    QHash<QString, int*> mAutoDict;
    QHash<QString, int*> mAutoIgnoreDict;
    static QObject *sDictionaryMonitor;
    KSpell *mSpell;
    KSpellConfig *mSpellConfig;
    QTimer *mRehighlightRequest;
    QTimer *mSpellTimeout;
    QString mSpellKey;
    int mWordCount;
    int mErrorCount;
    int mChecksRequested;
    int mChecksDone;
    int mDisablePercentage;
    int mDisableWordCount;
    bool mCompleteRehighlightRequired;
    bool mActive;
    bool mAutomatic;
    bool mAutoReady;
    bool mGlobalConfig;
    bool mSpellReady;

  private:
    static QHash<QString,int*>* statDict;
};

QHash<QString, int*>* KDictSpellingHighlighter::Private::statDict = 0;

KSyntaxHighlighter::KSyntaxHighlighter( QTextEdit *textEdit,
                                        bool colorQuoting,
                                        const QColor& depth0,
                                        const QColor& depth1,
                                        const QColor& depth2,
                                        const QColor& depth3,
                                        SyntaxMode mode )
  : QSyntaxHighlighter( textEdit ),
    d( new Private )
{
  d->mEnabled = colorQuoting;
  d->mColor1 = depth0;
  d->mColor2 = depth1;
  d->mColor3 = depth2;
  d->mColor4 = depth3;
  d->mColor5 = depth0;

  d->mMode = mode;
}

KSyntaxHighlighter::~KSyntaxHighlighter()
{
  delete d;
}

void KSyntaxHighlighter::highlightBlock ( const QString & text )
{
  if ( !d->mEnabled ) {
    //reset color.
    //setFormat( 0, text.length(), document()->paletteForegroundColor() );
    setCurrentBlockState( 0 );
    return;
  }

  QString simplified = text;
  simplified = simplified.replace( QRegExp( "\\s" ), QString() ).replace( '|', QLatin1String(">") );
  while ( simplified.startsWith( QLatin1String(">>>>") ) )
    simplified = simplified.mid( 3 );

  if ( simplified.startsWith( QLatin1String(">>>") ) || simplified.startsWith( QString::fromLatin1("> >	>") ) )
    setFormat( 0, text.length(), d->mColor2 );
  else if ( simplified.startsWith( QLatin1String(">>") ) || simplified.startsWith( QString::fromLatin1("> >") ) )
    setFormat( 0, text.length(), d->mColor3 );
  else if ( simplified.startsWith( QLatin1String(">") ) )
    setFormat( 0, text.length(), d->mColor4 );
  else
    setFormat( 0, text.length(), d->mColor5 );

  setCurrentBlockState( 0 );
}

KSpellingHighlighter::KSpellingHighlighter( QTextEdit *textEdit,
                                            const QColor& spellColor,
                                            bool colorQuoting,
                                            const QColor& depth0,
                                            const QColor& depth1,
                                            const QColor& depth2,
                                            const QColor& depth3 )
  : KSyntaxHighlighter( textEdit, colorQuoting, depth0, depth1, depth2, depth3 ),
    d( new Private( this ) )
{
  d->mColor = spellColor;
}

KSpellingHighlighter::~KSpellingHighlighter()
{
  delete d;
}

void KSpellingHighlighter::highlightBlock ( const QString& )
{
#if 0
  if ( paraNo == -2 )
    paraNo = 0;

  // leave #includes, diffs, and quoted replies alone
  QString diffAndCo( ">|" );

  bool isCode = diffAndCo.find( text[ 0 ] ) != -1;

  if ( !text.endsWith( " " ) )
    d->mAlwaysEndsWithSpace = false;

  KSyntaxHighlighter::highlightBlock ( text );

  if ( !isCode ) {
    int para, index;
    document ()->getCursorPosition( &para, &index );
    int len = text.length();
    if ( d->mAlwaysEndsWithSpace )
      len--;

    d->mCurrentPos = 0;
    d->mCurrentWord = "";
    for ( int i = 0; i < len; i++ ) {
      if ( !text[i].isLetter() && (!(text[i] == '\'')) ) {
        if ( ( para != paraNo ) || !d->mIntraWordEditing() ||
             ( i - d->mCurrentWord.length() > index ) || ( i < index ) ) {
          flushCurrentWord();
        } else {
          d->mCurrentWord = "";
        }

        d->mCurrentPos = i + 1;
      } else {
        d->mCurrentWord += text[ i ];
      }
    }

    if ( ( len > 0 && !text[ len - 1 ].isLetter() ) ||
         ( index + 1 ) != text.length() ||
          para != paraNo )
      flushCurrentWord();
  }

  return ++paraNo;
#endif
}

QStringList KSpellingHighlighter::personalWords()
{
  QStringList list;
  list.append( "KMail" );
  list.append( "KOrganizer" );
  list.append( "KAddressBook" );
  list.append( "KHTML" );
  list.append( "KIO" );
  list.append( "KJS" );
  list.append( "Konqueror" );
  list.append( "KSpell" );
  list.append( "Kontact" );
  list.append( "Qt" );

  return list;
}



QObject *KDictSpellingHighlighter::Private::sDictionaryMonitor = 0;

KDictSpellingHighlighter::KDictSpellingHighlighter( QTextEdit *textEdit,
                                                    bool spellCheckingActive,
                                                    bool autoEnable,
                                                    const QColor& spellColor,
                                                    bool colorQuoting,
                                                    const QColor& depth0,
                                                    const QColor& depth1,
                                                    const QColor& depth2,
                                                    const QColor& depth3,
                                                    KSpellConfig *spellConfig )
  : KSpellingHighlighter( textEdit, spellColor, colorQuoting, depth0, depth1, depth2, depth3 ),
    d( new Private )
{
  d->mSpellConfig = spellConfig;
  d->mGlobalConfig = ( !spellConfig );
  d->mAutomatic = autoEnable;
  d->mActive = spellCheckingActive;
  d->mChecksRequested = 0;
  d->mChecksDone = 0;
  d->mCompleteRehighlightRequired = false;

  KConfigGroup cg( KGlobal::config(), "KSpell" );
  d->mDisablePercentage = cg.readEntry( "KSpell_AsYouTypeDisablePercentage", 42 );
  d->mDisablePercentage = qMin( d->mDisablePercentage, 101 );
  d->mDisableWordCount = cg.readEntry( "KSpell_AsYouTypeDisableWordCount", 100 );

  textEdit->installEventFilter( this );
  textEdit->viewport()->installEventFilter( this );

  d->mRehighlightRequest = new QTimer(this);
  connect( d->mRehighlightRequest, SIGNAL( timeout() ),
           this, SLOT( slotRehighlight() ));

  d->mSpellTimeout = new QTimer(this);
  connect( d->mSpellTimeout, SIGNAL( timeout() ),
           this, SLOT( slotKSpellNotResponding() ));

  if ( d->mGlobalConfig ) {
    d->mSpellKey = spellKey();

    if ( !d->sDictionaryMonitor )
      d->sDictionaryMonitor = new QObject();

  } else {
    d->mDict = new QHash<QString,int*>();
    connect( d->mSpellConfig, SIGNAL( configChanged() ),
             this, SLOT( slotLocalSpellConfigChanged() ) );
  }

  slotDictionaryChanged();
  startTimer( 2 * 1000 );
}

KDictSpellingHighlighter::~KDictSpellingHighlighter()
{
  delete d;
}

void KDictSpellingHighlighter::slotSpellReady( KSpell *spell )
{
  kDebug(0) << "KDictSpellingHighlighter::slotSpellReady( " << spell << " )" << endl;
  if ( d->mGlobalConfig ) {
    connect( d->sDictionaryMonitor, SIGNAL( destroyed()),
             this, SLOT( slotDictionaryChanged() ));
  }

  if ( spell != d->mSpell ) {
    delete d->mSpell;
    d->mSpell = spell;
  }

  d->mSpellReady = true;
  const QStringList l = KSpellingHighlighter::personalWords();
  for ( QStringList::ConstIterator it = l.begin(); it != l.end(); ++it )
    d->mSpell->addPersonal( *it );

  connect( spell, SIGNAL( misspelling( const QString &, const QStringList &, unsigned int )),
           this, SLOT( slotMisspelling( const QString &, const QStringList &, unsigned int )));
  connect( spell, SIGNAL( corrected( const QString &, const QString &, unsigned int )),
           this, SLOT( slotCorrected( const QString &, const QString &, unsigned int )));

  d->mChecksRequested = 0;
  d->mChecksDone = 0;
  d->mCompleteRehighlightRequired = true;
  d->mRehighlightRequest->setSingleShot( true );
  d->mRehighlightRequest->start( 0 );
}

bool KDictSpellingHighlighter::isMisspelled( const QString &word )
{
  if ( !d->mSpellReady )
    return false;

  // This debug is expensive, only enable it locally
  //kDebug(0) << "KDictSpellingHighlighter::isMisspelled( \"" << word << "\" )" << endl;
  // Normally isMisspelled would look up a dictionary and return
  // true or false, but kspell is asynchronous and slow so things
  // get tricky...
  // For auto detection ignore signature and reply prefix
  if ( !d->mAutoReady )
    d->mAutoIgnoreDict.insert( word, Ignore );

  // "dict" is used as a cache to store the results of KSpell
  QHash<QString,int*>* dict = ( d->mGlobalConfig ? d->sDict() : d->mDict );
  if ( !dict->isEmpty() && (*dict)[ word ] == NotOkay ) {
    if ( d->mAutoReady && ( d->mAutoDict[ word ] != NotOkay )) {
      if ( !d->mAutoIgnoreDict[ word ] )
        ++d->mErrorCount;
      d->mAutoDict.insert( word, NotOkay );
    }

    return d->mActive;
  }

  if ( !dict->isEmpty() && (*dict)[ word ] == Okay ) {
    if ( d->mAutoReady && !d->mAutoDict[ word ] ) {
      d->mAutoDict.insert( word, Okay );
    }

    return false;
  }

  if ( (dict->isEmpty() || !((*dict)[word])) && d->mSpell ) {
    //int para, index;
    //document ()->getCursorPosition( &para, &index );
    ++d->mWordCount;
    dict->insert( word, Unknown );
    ++d->mChecksRequested;
    //if (currentParagraph() != para)
    //    d->completeRehighlightRequired = true;
    d->mSpellTimeout->setSingleShot( true );
    d->mSpellTimeout->start( tenSeconds );
    d->mSpell->checkWord( word, false );
  }

  return false;
}

bool KSpellingHighlighter::intraWordEditing() const
{
  return d->mIntraWordEditing;
}

void KSpellingHighlighter::setIntraWordEditing( bool editing )
{
  d->mIntraWordEditing = editing;
}

void KDictSpellingHighlighter::slotMisspelling (const QString &originalWord, const QStringList &suggestions,
                                                unsigned int pos)
{
  Q_UNUSED( suggestions );

  if ( d->mGlobalConfig )
    d->sDict()->insert( originalWord, NotOkay );
  else
    d->mDict->insert( originalWord, NotOkay );

  //Emit this baby so that apps that want to have suggestions in a popup over
  //the misspelled word can catch them.
  emit newSuggestions( originalWord, suggestions, pos );
}

void KDictSpellingHighlighter::slotCorrected( const QString &word, const QString&, unsigned int )

{
  QHash<QString,int*>* dict = ( d->mGlobalConfig ? d->sDict() : d->mDict );
  if ( !dict->isEmpty() && (*dict)[ word ] == Unknown )
    dict->insert( word, Okay );

  ++d->mChecksDone;

  if ( d->mChecksDone == d->mChecksRequested ) {
    d->mSpellTimeout->stop();
    slotRehighlight();
  } else {
    d->mSpellTimeout->setSingleShot( true );
    d->mSpellTimeout->start( tenSeconds );
  }
}

void KDictSpellingHighlighter::dictionaryChanged()
{
  QObject *oldMonitor = Private::sDictionaryMonitor;
  Private::sDictionaryMonitor = new QObject();
  Private::sDict()->clear();
  delete oldMonitor;
}

void KDictSpellingHighlighter::restartBackgroundSpellCheck()
{
  kDebug(0) << "KDictSpellingHighlighter::restartBackgroundSpellCheck()" << endl;
  slotDictionaryChanged();
}

void KDictSpellingHighlighter::setActive( bool active )
{
  if ( active == d->mActive )
    return;

  d->mActive = active;
  //rehighlight();

  if ( d->mActive )
    emit activeChanged( i18n("As-you-type spell checking enabled.") );
  else
    emit activeChanged( i18n("As-you-type spell checking disabled.") );
}

bool KDictSpellingHighlighter::isActive() const
{
  return d->mActive;
}

void KDictSpellingHighlighter::setAutomatic( bool automatic )
{
  if ( automatic == d->mAutomatic )
    return;

  d->mAutomatic = automatic;
  if ( d->mAutomatic )
    slotAutoDetection();
}

bool KDictSpellingHighlighter::automatic() const
{
  return d->mAutomatic;
}

void KDictSpellingHighlighter::slotRehighlight()
{
  kDebug(0) << "KDictSpellingHighlighter::slotRehighlight()" << endl;
  if ( d->mCompleteRehighlightRequired) {
    //rehighlight();
  } else {
    //int para, index;
    //document ()->getCursorPosition( &para, &index );
    //rehighlight the current para only (undo/redo safe)
    //document ()->insertAt( "", para, index );
  }

  if ( d->mChecksDone == d->mChecksRequested )
    d->mCompleteRehighlightRequired = false;

  QTimer::singleShot( 0, this, SLOT( slotAutoDetection() ) );
}

void KDictSpellingHighlighter::slotDictionaryChanged()
{
  delete d->mSpell;
  d->mSpellReady = false;
  d->mWordCount = 0;
  d->mErrorCount = 0;
  d->mAutoDict.clear();

  d->mSpell = new KSpell( 0, i18n( "Incremental Spellcheck" ), this,
                          SLOT( slotSpellReady( KSpell* ) ), d->mSpellConfig );
}

void KDictSpellingHighlighter::slotLocalSpellConfigChanged()
{
  kDebug(0) << "KDictSpellingHighlighter::slotSpellConfigChanged()" << endl;
  // the spell config has been changed, so we have to restart from scratch
  d->mDict->clear();
  slotDictionaryChanged();
}

QString KDictSpellingHighlighter::spellKey()
{
  KGlobal::config()->reparseConfiguration();
  KConfigGroup cg( KGlobal::config(), "KSpell" );

  QString key;
  key += QString::number( cg.readEntry( "KSpell_NoRootAffix", 0 ) );
  key += '/';
  key += QString::number( cg.readEntry( "KSpell_RunTogether", 0 ) );
  key += '/';
  key += cg.readEntry( "KSpell_Dictionary", "" );
  key += '/';
  key += QString::number( int(cg.readEntry( "KSpell_DictFromList", false )) );
  key += '/';
  key += QString::number( cg.readEntry( "KSpell_Encoding", int(KS_E_ASCII) ) );
  key += '/';
  key += QString::number( cg.readEntry( "KSpell_Client", int(KS_CLIENT_ISPELL) ) );

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
  if ( !d->mAutoReady )
    return;

  bool savedActive = d->mActive;

  if ( d->mAutomatic ) {
    // tme = Too many errors
    bool tme = ( d->mWordCount >= d->mDisableWordCount ) && ( d->mErrorCount * 100 >= d->mDisablePercentage * d->mWordCount );

    if ( d->mActive && tme )
      d->mActive = false;
    else if ( !d->mActive && !tme )
      d->mActive = true;
  }

  if ( d->mActive != savedActive ) {
    if ( d->mWordCount > 1 )
      if ( d->mActive )
        emit activeChanged( i18n("As-you-type spell checking enabled.") );
      else
        emit activeChanged( i18n( "Too many misspelled words. "
                                  "As-you-type spell checking disabled." ) );

    d->mCompleteRehighlightRequired = true;
    d->mRehighlightRequest->setSingleShot( true );
    d->mRehighlightRequest->start( 100 );
  }
}

void KDictSpellingHighlighter::slotKSpellNotResponding()
{
  static int retries = 0;
  if ( retries < 10 ) {
    if ( d->mGlobalConfig )
      KDictSpellingHighlighter::dictionaryChanged();
    else
      slotLocalSpellConfigChanged();
  } else {
    setAutomatic( false );
    setActive( false );
  }

  ++retries;
}

bool KDictSpellingHighlighter::eventFilter( QObject *object, QEvent *event )
{
  if ( object == document () && (event->type() == QEvent::FocusIn) ) {
    if ( d->mGlobalConfig ) {
      QString skey = spellKey();
      if ( d->mSpell && d->mSpellKey != skey ) {
        d->mSpellKey = skey;
        KDictSpellingHighlighter::dictionaryChanged();
      }
    }
  }

  if ( object == document () && (event->type() == QEvent::KeyPress) ) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>( event );
    d->mAutoReady = true;

    if ( d->mRehighlightRequest->isActive() ) { // try to stay out of the users way
      d->mRehighlightRequest->setSingleShot( false );
      d->mRehighlightRequest->start( 500 );
    }

    if ( keyEvent->key() == Qt::Key_Enter ||
         keyEvent->key() == Qt::Key_Return ||
         keyEvent->key() == Qt::Key_Up ||
         keyEvent->key() == Qt::Key_Down ||
         keyEvent->key() == Qt::Key_Left ||
         keyEvent->key() == Qt::Key_Right ||
         keyEvent->key() == Qt::Key_PageUp ||
         keyEvent->key() == Qt::Key_PageDown ||
         keyEvent->key() == Qt::Key_Home ||
         keyEvent->key() == Qt::Key_End ||
         ((keyEvent->modifiers() & Qt::ControlModifier) &&
         ((keyEvent->key() == Qt::Key_A) ||
         (keyEvent->key() == Qt::Key_B) ||
         (keyEvent->key() == Qt::Key_E) ||
         (keyEvent->key() == Qt::Key_N) ||
         (keyEvent->key() == Qt::Key_P) ) ) ) {

      if ( intraWordEditing() ) {
        setIntraWordEditing( false );
        d->mCompleteRehighlightRequired = true;
        d->mRehighlightRequest->setSingleShot( true );
        d->mRehighlightRequest->start( 500 );
      }

      if ( d->mChecksDone != d->mChecksRequested ) {
        // Handle possible change of paragraph while
        // words are pending spell checking
        d->mCompleteRehighlightRequired = true;
        d->mRehighlightRequest->setSingleShot( true );
        d->mRehighlightRequest->start( 500 );
      }
    } else {
      setIntraWordEditing( true );
    }

    if ( keyEvent->key() == Qt::Key_Space ||
         keyEvent->key() == Qt::Key_Enter ||
         keyEvent->key() == Qt::Key_Return ) {
      QTimer::singleShot( 0, this, SLOT( slotAutoDetection() ) );
    }
  } else if ( /*o == document ()->viewport() &&*/
            ( event->type() == QEvent::MouseButtonPress )) {
    d->mAutoReady = true;
    if ( intraWordEditing() ) {
      setIntraWordEditing( false );
      d->mCompleteRehighlightRequired = true;
      d->mRehighlightRequest->setSingleShot( true );
      d->mRehighlightRequest->start( 0 );
    }
  }

  return false;
}

#include "ksyntaxhighlighter.moc"
