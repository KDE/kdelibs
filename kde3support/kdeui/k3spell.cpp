/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>
   Copyright (C) 2000-2001 Wolfram Diestel <wolfram@steloj.de>
   Copyright (C) 2003 Zack Rusin <zack@kde.org>
   Copyright (C) 2007-2008 Kevin Kofler <Kevin@tigcc.ticalc.org>

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

#include "k3spell.h"

#include <config.h>

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h> // atoi

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif


#include <QtGui/QApplication>
#include <QtCore/QTextCodec>
#include <QtCore/QTimer>

#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
#include "k3sconfig.h"
#include "k3spelldlg.h"
#include <kprocess.h>
#include <QTextStream>

#define MAXLINELENGTH 10000
#undef IGNORE //fix possible conflict

enum {
  GOOD=     0,
  IGNORE=   1,
  REPLACE=  2,
  MISTAKE=  3
};

enum checkMethod { Method1 = 0, Method2 };

struct BufferedWord
{
  checkMethod method;
  QString word;
  bool useDialog;
  bool suggest;
};

class K3Spell::K3SpellPrivate
{
public:
  bool endOfResponse;
  bool m_bIgnoreUpperWords;
  bool m_bIgnoreTitleCase;
  bool m_bNoMisspellingsEncountered;
  SpellerType type;
  K3Spell* suggestSpell;
  bool checking;
  QList<BufferedWord> unchecked;
  QTimer *checkNextTimer;
  bool aspellV6;
  QTextCodec* m_codec;
  QString convertQByteArray( const QByteArray& b )
  {
      QTextCodec* originalCodec = QTextCodec::codecForCStrings();
      QTextCodec::setCodecForCStrings( m_codec );
      QString s( b );
      QTextCodec::setCodecForCStrings( originalCodec );
      return s;
  }
  QByteArray convertQString( const QString& s )
  {
      QTextCodec* originalCodec = QTextCodec::codecForCStrings();
      QTextCodec::setCodecForCStrings( m_codec );
      QByteArray b = s.toAscii();
      QTextCodec::setCodecForCStrings( originalCodec );
      return b;
  }
};

//TODO
//Parse stderr output
//e.g. -- invalid dictionary name

/*
  Things to put in K3SpellConfigDlg:
    make root/affix combinations that aren't in the dictionary (-m)
    don't generate any affix/root combinations (-P)
    Report  run-together  words   with   missing blanks as spelling errors.  (-B)
    default dictionary (-d [dictionary])
    personal dictionary (-p [dictionary])
    path to ispell -- NO: ispell should be in $PATH
    */


//  Connects a slot to KProcess's output signal
#define OUTPUT(x) (connect (proc, SIGNAL (readyReadStandardOutput()), this, SLOT (x())))

// Disconnect a slot from...
#define NOOUTPUT(x) (disconnect (proc, SIGNAL (readyReadStandardOutput()), this, SLOT (x())))



K3Spell::K3Spell( QWidget *_parent, const QString &_caption,
		QObject *obj, const char *slot, K3SpellConfig *_ksc,
		bool _progressbar, bool _modal )
{
  initialize( _parent, _caption, obj, slot, _ksc,
              _progressbar, _modal, Text );
}

K3Spell::K3Spell( QWidget *_parent, const QString &_caption,
		QObject *obj, const char *slot, K3SpellConfig *_ksc,
		bool _progressbar, bool _modal, SpellerType type )
{
  initialize( _parent, _caption, obj, slot, _ksc,
              _progressbar, _modal, type );
}

K3Spell::spellStatus K3Spell::status() const
{
    return m_status;
}

void K3Spell::hide() { ksdlg->hide(); }

QStringList K3Spell::suggestions() const
{
    return sugg;
}

int K3Spell::dlgResult () const
{
    return dlgresult;
}

int K3Spell::heightDlg() const { return ksdlg->height(); }
int K3Spell::widthDlg() const { return ksdlg->width(); }

QString K3Spell::intermediateBuffer() const
{
    return K3Spell::newbuffer;
}

// Check if aspell is at least version 0.6
static bool determineASpellV6()
{
  QString result;
  FILE *fs = popen("aspell -v", "r");
  if (fs)
  {
    // Close textstream before we close fs
    {
      QTextStream ts(fs, QIODevice::ReadOnly);
      result = ts.readAll().trimmed();
    }
    pclose(fs);
  }

  QRegExp rx("Aspell (\\d.\\d)");
  if (rx.indexIn(result) != -1)
  {
     float version = rx.cap(1).toFloat();
     return (version >= 0.6);
  }
  return false;
}


void
K3Spell::startIspell()
  //trystart = {0,1,2}
{
  if ((trystart == 0) && (ksconfig->client() == KS_CLIENT_ASPELL))
     d->aspellV6 = determineASpellV6();

  kDebug(750) << "Try #" << trystart;

  if ( trystart > 0 ) {
    proc->reset();
  }

  switch ( ksconfig->client() )
  {
  case KS_CLIENT_ISPELL:
    *proc << "ispell";
    kDebug(750) << "Using ispell";
    break;
  case KS_CLIENT_ASPELL:
    *proc << "aspell";
    kDebug(750) << "Using aspell";
    break;
  case KS_CLIENT_HSPELL:
    *proc << "hspell";
    kDebug(750) << "Using hspell";
    break;
  case KS_CLIENT_ZEMBEREK:
    *proc << "zpspell";
    kDebug(750) << "Using zemberek(zpspell)";
    break;
  case KS_CLIENT_HUNSPELL:
    *proc << "hunspell";
    kDebug(750) << "Using hunspell";
    break;
  }

  // Hunspell doesn't need all of these options, but it'll ignore those it doesn't understand.
  if ( ksconfig->client() == KS_CLIENT_ISPELL || ksconfig->client() == KS_CLIENT_ASPELL || ksconfig->client() == KS_CLIENT_HUNSPELL )
  {
    *proc << "-a" << "-S";

    switch ( d->type )
    {
    case HTML:
      //Debian uses an ispell version that has the -h option instead.
      //Not sure what they did, but the preferred spell checker
      //on that platform is aspell anyway, so use -H untill I'll come
      //up with something better.
      *proc << "-H";
      break;
    case TeX:
      //same for aspell and ispell
      *proc << "-t";
      break;
    case Nroff:
      //only ispell and hunspell support
      if ( ksconfig->client() == KS_CLIENT_ISPELL || ksconfig->client() == KS_CLIENT_HUNSPELL )
        *proc << "-n";
      break;
    case Text:
    default:
      //nothing
      break;
    }
    if (ksconfig->noRootAffix())
    {
      *proc<<"-m";
    }
    if (ksconfig->runTogether())
    {
      *proc << "-B";
    }
    else
    {
      *proc << "-C";
    }


    if (trystart<2)
    {
      if (! ksconfig->dictionary().isEmpty())
      {
        kDebug(750) << "using dictionary [" << ksconfig->dictionary() << "]";
        *proc << "-d";
        *proc << ksconfig->dictionary();
      }
    }

  //Note to potential debuggers:  -Tlatin2 _is_ being added on the
  //  _first_ try.  But, some versions of ispell will fail with this
  // option, so k3spell tries again without it.  That's why as 'ps -ax'
  // shows "ispell -a -S ..." withou the "-Tlatin2" option.

    if ( ksconfig->client() == KS_CLIENT_HUNSPELL && trystart<1 ) {
      // Note: This sets I/O encoding. Hunspell correctly handles dictionary encoding != I/O encoding.
      // It will be faster if the I/O encoding matches the dictionary encoding, but using UTF-8 is always safe.
      switch ( ksconfig->encoding() )
      {
      case KS_E_LATIN1:
	*proc << "-i" << "ISO-8859-1";
	break;
      case KS_E_LATIN2:
	*proc << "-i" << "ISO-8859-2";
	break;
      case KS_E_LATIN3:
	*proc << "-i" << "ISO-8859-3";
        break;
      case KS_E_LATIN4:
	*proc << "-i" << "ISO-8859-4";
        break;
      case KS_E_LATIN5:
	*proc << "-i" << "ISO-8859-5";
        break;
      case KS_E_LATIN7:
	*proc << "-i" << "ISO-8859-7";
        break;
      case KS_E_LATIN8:
	*proc << "-i" << "ISO-8859-8";
        break;
      case KS_E_LATIN9:
	*proc << "-i" << "ISO-8859-9";
        break;
      case KS_E_LATIN13:
	*proc << "-i" << "ISO-8859-13";
        break;
      case KS_E_LATIN15:
	*proc << "-i" << "ISO-8859-15";
        break;
      case KS_E_UTF8:
	*proc << "-i" << "UTF-8";
        break;
      case KS_E_KOI8R:
	*proc << "-i" << "KOI8-R";
        break;
      case KS_E_KOI8U:
	*proc << "-i" << "KOI8-U";
        break;
      case KS_E_CP1251:
	*proc << "-i" << "CP1251";
        break;
      case KS_E_CP1255:
	*proc << "-i" << "CP1255";
        break;
      default:
        break;
      }
    } else if ( trystart<1 ) {
      switch ( ksconfig->encoding() )
      {
      case KS_E_LATIN1:
	*proc << "-Tlatin1";
	break;
      case KS_E_LATIN2:
	*proc << "-Tlatin2";
	break;
      case KS_E_LATIN3:
        *proc << "-Tlatin3";
        break;

        // add the other charsets here
      case KS_E_LATIN4:
      case KS_E_LATIN5:
      case KS_E_LATIN7:
      case KS_E_LATIN8:
      case KS_E_LATIN9:
      case KS_E_LATIN13:
	// will work, if this is the default charset in the dictionary
	kError(750) << "charsets ISO-8859-4, -5, -7, -8, -9 and -13 not supported yet" << endl;
	break;
      case KS_E_LATIN15: // ISO-8859-15 (Latin 9)
        if (ksconfig->client() == KS_CLIENT_ISPELL)
        {
          /*
           * As far as I know, there are no ispell dictionary using ISO-8859-15
           * but users have the tendency to select this encoding instead of ISO-8859-1
           * So put ispell in ISO-8859-1 (Latin 1) mode.
           */
          *proc << "-Tlatin1";
        }
        else
          kError(750) << "ISO-8859-15 not supported for aspell yet." << endl;
        break;
      case KS_E_UTF8:
        *proc << "-Tutf8";
        if (ksconfig->client() == KS_CLIENT_ASPELL)
          *proc << "--encoding=utf-8";
        break;
      case KS_E_KOI8U:
	*proc << "-w'"; // add ' as a word char
	break;
      default:
        break;
      }
    }

  // -a : pipe mode
  // -S : sort suggestions by probable correctness
  }
  else       // hspell and Zemberek(zpspell) doesn't need all the rest of the options
    *proc << "-a";

  if (trystart == 0) //don't connect these multiple times
  {
    connect( proc, SIGNAL(readyReadStandardError()),
             this, SLOT(ispellErrors()) );

    connect( proc, SIGNAL(finished(int, QProcess::ExitStatus)),
             this, SLOT(ispellExit ()) );

    proc->setOutputChannelMode( KProcess::SeparateChannels );
    proc->setNextOpenMode( QIODevice::ReadWrite | QIODevice::Text );

    OUTPUT(K3Spell2);
  }

  proc->start();
  if ( !proc->waitForStarted() )
  {
    m_status = Error;
    QTimer::singleShot( 0, this, SLOT(emitDeath()));
  }
}

void
K3Spell::ispellErrors(  )
{
  // buffer[buflen-1] = '\0';
  //  kDebug(750) << "ispellErrors [" << buffer << "]\n";
}

void K3Spell::K3Spell2( )

{
  QString line;

  kDebug(750) << "K3Spell::K3Spell2";

  trystart = maxtrystart;  //We've officially started ispell and don't want
                           //to try again if it dies.

  QByteArray data;
  qint64 read = proc->readLine(data.data(),data.count());
  if ( read == -1 )
  {
     QTimer::singleShot( 0, this, SLOT(emitDeath()) );
     return;
  }
  line = d->convertQByteArray( data );

  if ( !line.startsWith('@') ) //@ indicates that ispell is working fine
  {
     QTimer::singleShot( 0, this, SLOT(emitDeath()) );
     return;
  }

  //We want to recognize KDE in any text!
  if ( !ignore("kde") )
  {
     kDebug(750) << "@KDE was false";
     QTimer::singleShot( 0, this, SLOT(emitDeath()) );
     return;
  }

  //We want to recognize linux in any text!
  if ( !ignore("linux") )
  {
     kDebug(750) << "@Linux was false";
     QTimer::singleShot( 0, this, SLOT(emitDeath()) );
     return;
  }

  NOOUTPUT( K3Spell2 );

  m_status = Running;
  emit ready( this );
}

void
K3Spell::setUpDialog( bool reallyuseprogressbar )
{
  if ( dialogsetup )
    return;

  //Set up the dialog box
  ksdlg = new K3SpellDlg( parent, progressbar && reallyuseprogressbar, modaldlg );
  ksdlg->setCaption( caption );

  connect( ksdlg, SIGNAL(command(int)),
           this, SLOT(slotStopCancel(int)) );
  connect( this, SIGNAL(progress(unsigned int)),
	   ksdlg, SLOT(slotProgress(unsigned int)) );

  if ( modaldlg )
    ksdlg->setFocus();
  dialogsetup = true;
}

bool K3Spell::addPersonal( const QString & word )
{
  QString qs = word.simplified();

  //we'll let ispell do the work here b/c we can
  if ( qs.indexOf(' ') != -1 || qs.isEmpty() )    // make sure it's a _word_
    return false;

  qs.prepend( "*" );
  personaldict = true;

  return proc->write( d->convertQString( qs ) );
}

bool K3Spell::writePersonalDictionary()
{
  return proc->write( QByteArray( "#" ) );
}

bool K3Spell::ignore( const QString & word )
{
  QString qs = word.simplified();

  //we'll let ispell do the work here b/c we can
  if ( qs.indexOf (' ') != -1 || qs.isEmpty() )    // make sure it's a _word_
    return false;

  qs.prepend( "@" );

  return proc->write( d->convertQString( qs ) );
}

bool
K3Spell::cleanFputsWord( const QString & s )
{
  QString qs(s);
  bool empty = true;

  for( int i = 0; i < qs.length(); i++ )
  {
    //we need some punctuation for ornaments
    if ( (qs[i] != '\'' && qs[i] != '\"' && qs[i] != '-'
         && qs[i].isPunct()) || qs[i].isSpace() )
    {
      qs.remove(i,1);
      i--;
    } else {
      if ( qs[i].isLetter() )
        empty=false;
    }
  }

  // don't check empty words, otherwise synchronization will lost
  if (empty)
    return false;

  return proc->write( d->convertQString( QString('^'+qs+'\n') ) );
}

bool
K3Spell::cleanFputs( const QString & s )
{
  QString qs(s);
  unsigned l = qs.length();

  // some uses of '$' (e.g. "$0") cause ispell to skip all following text
  for( unsigned int i = 0; i < l; ++i )
  {
    if( qs[i] == '$' )
      qs[i] = ' ';
  }

  if ( l<MAXLINELENGTH )
  {
    if ( qs.isEmpty() )
      qs="";
    return proc->write( d->convertQString('^'+qs+'\n') );
  }
  else
    return proc->write( d->convertQString( "^\n" ) );
}

bool K3Spell::checkWord( const QString & buffer, bool _usedialog )
{
  if (d->checking) { // don't check multiple words simultaneously
    BufferedWord bufferedWord;
    bufferedWord.method = Method1;
    bufferedWord.word = buffer;
    bufferedWord.useDialog = _usedialog;
    d->unchecked.append( bufferedWord );
    return true;
  }
  d->checking = true;
  QString qs = buffer.simplified();

  if ( qs.indexOf (' ') != -1 || qs.isEmpty() ) {   // make sure it's a _word_
    d->checkNextTimer->setInterval(0);
    d->checkNextTimer->setSingleShot(true);
    d->checkNextTimer->start();
    return false;
  }
  ///set the dialog signal handler
  dialog3slot = SLOT(checkWord3());

  usedialog = _usedialog;
  setUpDialog( false );
  if ( _usedialog )
  {
    emitProgress();
  }
  else
    ksdlg->hide();

  QByteArray data;
  while (proc->readLine( data.data(), data.count() ) != -1 )
      ; // eat spurious blanks

  OUTPUT(checkWord2);
  //  connect (this, SIGNAL (dialog3()), this, SLOT (checkWord3()));

  proc->write( d->convertQString( QString( "%" ) ) ); // turn off terse mode
  proc->write( d->convertQString( buffer ) ); // send the word to ispell

  return true;
}

bool K3Spell::checkWord( const QString & buffer, bool _usedialog, bool suggest )
{
  if (d->checking) { // don't check multiple words simultaneously
    BufferedWord bufferedWord;
    bufferedWord.method = Method2;
    bufferedWord.word = buffer;
    bufferedWord.useDialog = _usedialog;
    bufferedWord.suggest = suggest;
    d->unchecked.append( bufferedWord );
    return true;
  }
  d->checking = true;
  QString qs = buffer.simplified();

  if ( qs.indexOf (' ') != -1 || qs.isEmpty() ) {   // make sure it's a _word_
    d->checkNextTimer->setInterval(0);
    d->checkNextTimer->setSingleShot(true);
    d->checkNextTimer->start();
    return false;
  }

  ///set the dialog signal handler
  if ( !suggest ) {
    dialog3slot = SLOT(checkWord3());
    usedialog = _usedialog;
    setUpDialog( false );
    if ( _usedialog )
    {
      emitProgress();
    }
    else
      ksdlg->hide();
  }

  QByteArray data;
  while (proc->readLine( data.data(), data.count() ) != -1 ) ; // eat spurious blanks

  OUTPUT(checkWord2);
  //  connect (this, SIGNAL (dialog3()), this, SLOT (checkWord3()));

  proc->write( d->convertQString( QString( "%" ) ) ); // turn off terse mode
  proc->write( d->convertQString( buffer ) ); // send the word to ispell

  return true;
}

void K3Spell::checkWord2(  )
{
  QString word;
  QString line;
  line = d->convertQByteArray( proc->readLine() ); //get ispell's response

/* ispell man page: "Each sentence of text input is terminated with an
   additional blank line,  indicating that ispell has completed processing
   the input line."
   <sanders>
   But there can be multiple lines returned in the case of an error,
   in this case we should consume all the output given otherwise spell checking
   can get out of sync.
   </sanders>
*/
  QByteArray data;
  while (proc->readLine( data.data(), data.count() ) != -1 ) ; // eat spurious blanks
  NOOUTPUT(checkWord2);

  bool mistake = ( parseOneResponse(line, word, sugg) == MISTAKE );
  if ( mistake && usedialog )
  {
    cwword = word;
    dialog( word, sugg, SLOT(checkWord3()) );
    d->checkNextTimer->setInterval(0);
    d->checkNextTimer->setSingleShot(true);
    d->checkNextTimer->start();
    return;
  }
  else if( mistake )
  {
    emit misspelling( word, sugg, lastpos );
  }

  //emits a "corrected" signal _even_ if no change was made
  //so that the calling program knows when the check is complete
  emit corrected( word, word, 0L );
  d->checkNextTimer->setInterval(0);
  d->checkNextTimer->setSingleShot(true);
  d->checkNextTimer->start();
}

void K3Spell::checkNext()
{
// Queue words to prevent kspell from turning into a fork bomb
  d->checking = false;
  if (!d->unchecked.empty()) {
    BufferedWord buf = d->unchecked.front();
    d->unchecked.pop_front();

    if (buf.method == Method1)
      checkWord( buf.word, buf.useDialog );
    else
      checkWord( buf.word, buf.useDialog, buf.suggest );
  }
}

void K3Spell::suggestWord()
{
  QString word;
  QString line;
  line = d->convertQByteArray( proc->readLine() ); //get ispell's response

/* ispell man page: "Each sentence of text input is terminated with an
   additional blank line,  indicating that ispell has completed processing
   the input line." */
  QByteArray data;
  while (proc->readLine( data.data(), data.count() ) != -1 ) ; // eat spurious blanks

  NOOUTPUT(checkWord2);

  bool mistake = ( parseOneResponse(line, word, sugg) == MISTAKE );
  if ( mistake && usedialog )
  {
    cwword=word;
    dialog( word, sugg, SLOT(checkWord3()) );
    return;
  }
}

void K3Spell::checkWord3()
{
  disconnect( this, SIGNAL(dialog3()), this, SLOT(checkWord3()) );

  emit corrected( cwword, replacement(), 0L );
}

QString K3Spell::funnyWord( const QString & word )
  // composes a guess from ispell to a readable word
  // e.g. "re+fry-y+ies" -> "refries"
{
  QString qs;
  for( int i=0; i<word.size(); i++ )
  {
    if (word [i]=='+')
      continue;
    if (word [i]=='-')
    {
      QString shorty;
      int j, k;

      for( j = i+1; j < word.size() && word[j] != '+' && word[j] != '-'; j++ )
        shorty += word[j];

      i = j-1;

      if ( !( k = qs.lastIndexOf(shorty) ) || k != -1 )
        qs.remove( k, shorty.length() );
      else
      {
        qs += '-';
        qs += shorty;  //it was a hyphen, not a '-' from ispell
      }
    }
    else
      qs += word[i];
  }

  return qs;
}


int K3Spell::parseOneResponse( const QString &buffer, QString &word, QStringList & sugg )
  // buffer is checked, word and sugg are filled in
  // returns
  //   GOOD    if word is fine
  //   IGNORE  if word is in ignorelist
  //   REPLACE if word is in replacelist
  //   MISTAKE if word is misspelled
{
  word = "";
  posinline=0;

  sugg.clear();

  if ( buffer[0] == '*' || buffer[0] == '+' || buffer[0] == '-' )
  {
    return GOOD;
  }

  if ( buffer[0] == '&' || buffer[0] == '?' || buffer[0] == '#' )
  {
    int i,j;


    word = buffer.mid( 2, buffer.indexOf( ' ', 3 ) -2 );
    //check() needs this
    orig=word;

    if( d->m_bIgnoreTitleCase && word == word.toUpper() )
      return IGNORE;

    if( d->m_bIgnoreUpperWords && word[0] == word[0].toUpper() )
    {
      QString text = word[0] + word.right( word.length()-1 ).toLower();
      if( text == word )
        return IGNORE;
    }

    /////// Ignore-list stuff //////////
    //We don't take advantage of ispell's ignore function because
    //we can't interrupt ispell's output (when checking a large
    //buffer) to add a word to _it's_ ignore-list.
    if ( ignorelist.indexOf( word.toLower() ) != -1 )
      return IGNORE;

    //// Position in line ///
    QString qs2;

    if ( buffer.indexOf( ':' ) != -1 )
      qs2 = buffer.left( buffer.indexOf(':') );
    else
      qs2 = buffer;

    posinline = qs2.right( qs2.length()-qs2.lastIndexOf(' ') ).toInt()-1;

    ///// Replace-list stuff ////
    QStringList::Iterator it = replacelist.begin();
    for( ;it != replacelist.end(); ++it, ++it ) // Skip two entries at a time.
    {
      if ( word == *it ) // Word matches
      {
        ++it;
        word = *it;   // Replace it with the next entry
        return REPLACE;
      }
    }

    /////// Suggestions //////
    if ( buffer[0] != '#' )
    {
      QString qs = buffer.mid( buffer.indexOf(':')+2, buffer.length() );
      qs += ',';
      sugg.clear();
      i = j = 0;

      while( i < qs.length() )
      {
        QString temp = qs.mid( i, (j=qs.indexOf(',',i)) - i );
        sugg.append( funnyWord(temp) );

        i=j+2;
      }
    }

    if ( (sugg.count()==1) && (sugg.first() == word) )
      return GOOD;

    return MISTAKE;
  }

  if ( buffer.isEmpty() ) {
      kDebug(750) << "Got an empty response: ignoring";
      return GOOD;
  }

  kError(750) << "HERE?: [" << buffer << "]" << endl;
  kError(750) << "Please report this to zack@kde.org" << endl;
  kError(750) << "Thank you!" << endl;

  emit done( false );
  emit done( K3Spell::origbuffer );
  return MISTAKE;
}

bool K3Spell::checkList (QStringList *_wordlist, bool _usedialog)
  // prepare check of string list
{
  wordlist=_wordlist;
  if ((totalpos=wordlist->count())==0)
    return false;
  wlIt = wordlist->begin();
  usedialog=_usedialog;

  // prepare the dialog
  setUpDialog();

  //set the dialog signal handler
  dialog3slot = SLOT (checkList4 ());

  proc->write(QByteArray( '%' ) ); // turn off terse mode & check one word at a time

  //lastpos now counts which *word number* we are at in checkListReplaceCurrent()
  lastpos = -1;
  checkList2();

  // when checked, KProcess calls checkList3a
  OUTPUT(checkList3a);

  return true;
}

void K3Spell::checkList2 ()
  // send one word from the list to KProcess
  // invoked first time by checkList, later by checkListReplaceCurrent and checkList4
{
  // send next word
  if (wlIt != wordlist->end())
  {
    kDebug(750) << "KS::cklist2 " << lastpos << ": " << *wlIt;

    d->endOfResponse = false;
    bool put;
    lastpos++; offset=0;
    put = cleanFputsWord (*wlIt);
    ++wlIt;

    // when cleanFPutsWord failed (e.g. on empty word)
    // try next word; may be this is not good for other
    // problems, because this will make read the list up to the end
    if (!put) {
      checkList2();
    }
  }
  else
    // end of word list
  {
    NOOUTPUT(checkList3a);
    ksdlg->hide();
    emit done(true);
  }
}

void K3Spell::checkList3a ()
  // invoked by KProcess, when data from ispell are read
{
  //kDebug(750) << "start of checkList3a";

  // don't read more data, when dialog is waiting
  // for user interaction
  if ( dlgon ) {
    //kDebug(750) << "dlgon: don't read more data";
    return;
  }

  int e;
  qint64 tempe;

  QString word;
  QString line;

  do
  {
    QByteArray data;
    tempe = proc->readLine( data.data(), data.count() ); //get ispell's response

    //kDebug(750) << "checkList3a: read bytes [" << tempe << "]";
    line = d->convertQByteArray( data );

    if ( tempe == 0 ) {
      d->endOfResponse = true;
      //kDebug(750) << "checkList3a: end of resp";
    } else if ( tempe>0 ) {
      if ( (e=parseOneResponse( line, word, sugg ) ) == MISTAKE ||
           e==REPLACE )
      {
        dlgresult=-1;

        if ( e == REPLACE )
        {
          QString old = *(--wlIt); ++wlIt;
          dlgreplacement = word;
          checkListReplaceCurrent();
          // inform application
          emit corrected( old, *(--wlIt), lastpos ); ++wlIt;
        }
        else if( usedialog )
        {
          cwword = word;
          dlgon = true;
          // show the dialog
          dialog( word, sugg, SLOT(checkList4()) );
          return;
        }
        else
        {
          d->m_bNoMisspellingsEncountered = false;
          emit misspelling( word, sugg, lastpos );
        }
      }

    }
    emitProgress (); //maybe

    // stop when empty line or no more data
  } while (tempe > 0);

  //kDebug(750) << "checkList3a: exit loop with [" << tempe << "]";

  // if we got an empty line, t.e. end of ispell/aspell response
  // and the dialog isn't waiting for user interaction, send next word
  if (d->endOfResponse && !dlgon) {
    //kDebug(750) << "checkList3a: send next word";
    checkList2();
  }
}

void K3Spell::checkListReplaceCurrent()
{

  // go back to misspelled word
  wlIt--;

  QString s = *wlIt;
  s.replace(posinline+offset,orig.length(),replacement());
  offset += replacement().length()-orig.length();
  wordlist->insert (wlIt, s);
  wlIt = wordlist->erase (wlIt);
  // wlIt now points to the word after the repalced one

}

void K3Spell::checkList4 ()
  // evaluate dialog return, when a button was pressed there
{
  dlgon=false;
  QString old;

  disconnect (this, SIGNAL (dialog3()), this, SLOT (checkList4()));

  //others should have been processed by dialog() already
  switch (dlgresult)
  {
  case KS_REPLACE:
  case KS_REPLACEALL:
    kDebug(750) << "KS: cklist4: lastpos: " << lastpos;
    old = *(--wlIt);
    ++wlIt;
    // replace word
    checkListReplaceCurrent();
    emit corrected( old, *(--wlIt), lastpos );
    ++wlIt;
    break;
  case KS_CANCEL:
    ksdlg->hide();
    emit done( false );
    return;
  case KS_STOP:
    ksdlg->hide();
    emit done( true );
    return;
  case KS_CONFIG:
    ksdlg->hide();
    emit done( false );
    //check( origbuffer.mid( lastpos ), true );
    //trystart = 0;
    //proc->disconnect();
    //proc->kill();
    //delete proc;
    //proc = new KProcess( codec );
    //startIspell();
    return;
  };

  // read more if there is more, otherwise send next word
  if (!d->endOfResponse) {
    //kDebug(750) << "checkList4: read more from response";
    checkList3a();
  }
}

bool K3Spell::check( const QString &_buffer, bool _usedialog )
{
  QString qs;

  usedialog = _usedialog;
  setUpDialog();
  //set the dialog signal handler
  dialog3slot = SLOT(check3());

  kDebug(750) << "KS: check";
  origbuffer = _buffer;
  if ( ( totalpos = origbuffer.length() ) == 0 )
  {
    emit done( origbuffer );
    return false;
  }


  // Torben: I corrected the \n\n problem directly in the
  //         origbuffer since I got errors otherwise
  if ( !origbuffer.endsWith("\n\n" ) )
  {
    if (origbuffer.at(origbuffer.length()-1)!='\n')
    {
      origbuffer+='\n';
      origbuffer+='\n'; //shouldn't these be removed at some point?
    }
    else
      origbuffer+='\n';
  }

  newbuffer = origbuffer;

  // KProcess calls check2 when read from ispell
  OUTPUT( check2 );
  proc->write( QByteArray( "!" ) );

  //lastpos is a position in newbuffer (it has offset in it)
  offset = lastlastline = lastpos = lastline = 0;

  emitProgress();

  // send first buffer line
  int i = origbuffer.indexOf( '\n', 0 ) + 1;
  qs = origbuffer.mid( 0, i );
  cleanFputs( qs );

  lastline=i; //the character position, not a line number

  if ( usedialog )
  {
    emitProgress();
  }
  else
    ksdlg->hide();

  return true;
}

int K3Spell::lastPosition() const
{
    return lastpos;
}


void K3Spell::check2()
  // invoked by KProcess when read from ispell
{
  int e;
  qint64 tempe;
  QString word;
  QString line;
  static bool recursive = false;
  if (recursive &&
      !ksdlg )
  {
      return;
  }
  recursive = true;

  do
  {
    QByteArray data;
    tempe = proc->readLine( data.data(), data.count() ); //get ispell's response
    line = d->convertQByteArray( data );
    //kDebug(750) << "K3Spell::check2 (" << tempe << "b)";

    if ( tempe>0 )
    {
      if ( ( e=parseOneResponse (line, word, sugg) )==MISTAKE ||
           e==REPLACE)
      {
        dlgresult=-1;

        // for multibyte encoding posinline needs correction
        if ((ksconfig->encoding() == KS_E_UTF8) && !d->aspellV6) {
          // kDebug(750) << "line: " << origbuffer.mid(lastlastline,
          // lastline-lastlastline) << endl;
          // kDebug(750) << "posinline uncorr: " << posinline;

          // convert line to UTF-8, cut at pos, convert back to UCS-2
          // and get string length
          posinline = (QString::fromUtf8(
                         origbuffer.mid(lastlastline,lastline-lastlastline).toUtf8(),
                         posinline)).length();
          // kDebug(750) << "posinline corr: " << posinline;
        }

        lastpos = posinline+lastlastline+offset;

        //orig is set by parseOneResponse()

        if (e==REPLACE)
        {
          dlgreplacement=word;
          emit corrected( orig, replacement(), lastpos );
          offset += replacement().length()-orig.length();
          newbuffer.replace( lastpos, orig.length(), word );
        }
        else  //MISTAKE
        {
          cwword = word;
          //kDebug(750) << "(Before dialog) word=[" << word << "] cwword =[" << cwword << "]\n";
          if ( usedialog ) {
            // show the word in the dialog
            dialog( word, sugg, SLOT(check3()) );
          } else {
            // No dialog, just emit misspelling and continue
            d->m_bNoMisspellingsEncountered = false;
            emit misspelling( word, sugg, lastpos );
            dlgresult = KS_IGNORE;
            check3();
          }
          recursive = false;
          return;
        }
      }

    }

    emitProgress(); //maybe

  } while( tempe>0 );

  if ( tempe == -1 ) { //we were called, but no data seems to be ready...
    // Make sure we don't get called directly again and make sure we do get
    // called when new data arrives.
    NOOUTPUT( check2 );
//     proc->enableReadSignals(true);
    OUTPUT( check2 );
    recursive = false;
    return;
  }

//   proc->ackRead();

  //If there is more to check, then send another line to ISpell.
  if ( lastline < origbuffer.length() )
  {
    int i;
    QString qs;

    //kDebug(750) << "[EOL](" << tempe << ")[" << temp << "]";

    lastpos = (lastlastline=lastline) + offset; //do we really want this?
    i = origbuffer.indexOf('\n', lastline) + 1;
    qs = origbuffer.mid( lastline, i-lastline );
    cleanFputs( qs );
    lastline = i;
    recursive = false;
    return;
  }
  else
    //This is the end of it all
  {
    ksdlg->hide();
    //      kDebug(750) << "check2() done";
    newbuffer.truncate( newbuffer.length()-2 );
    emitProgress();
    emit done( newbuffer );
  }
  recursive = false;
}

void K3Spell::check3 ()
  // evaluates the return value of the dialog
{
  disconnect (this, SIGNAL (dialog3()), this, SLOT (check3()));
  kDebug(750) << "check3 [" << cwword << "] [" << replacement() << "] " << dlgresult;

  //others should have been processed by dialog() already
  switch (dlgresult)
  {
  case KS_REPLACE:
  case KS_REPLACEALL:
    offset+=replacement().length()-cwword.length();
    newbuffer.replace (lastpos, cwword.length(),
                       replacement());
    emit corrected (dlgorigword, replacement(), lastpos);
    break;
  case KS_CANCEL:
    //      kDebug(750) << "canceled\n";
    ksdlg->hide();
    emit done( origbuffer );
    return;
  case KS_CONFIG:
    ksdlg->hide();
    emit done( origbuffer );
    KMessageBox::information( 0, i18n("You have to restart the dialog for changes to take effect") );
    //check( origbuffer.mid( lastpos ), true );
    return;
  case KS_STOP:
    ksdlg->hide();
    //buffer=newbuffer);
    emitProgress();
    emit done (newbuffer);
    return;
  };

//   proc->ackRead();
}

void
K3Spell::slotStopCancel (int result)
{
  if (dialogwillprocess)
    return;

  kDebug(750) << "K3Spell::slotStopCancel [" << result << "]";

  if (result==KS_STOP || result==KS_CANCEL)
    if (!dialog3slot.isEmpty())
    {
      dlgresult=result;
      connect (this, SIGNAL (dialog3()), this, dialog3slot.toAscii().constData());
      emit dialog3();
    }
}


void K3Spell::dialog( const QString & word, QStringList & sugg, const char *_slot )
{
  dlgorigword = word;

  dialog3slot = _slot;
  dialogwillprocess = true;
  connect( ksdlg, SIGNAL(command(int)), this, SLOT(dialog2(int)) );
  QString tmpBuf = newbuffer;
  kDebug(750)<<" position = "<<lastpos;

  // extract a context string, replace all characters which might confuse
  // the RichText display and highlight the possibly wrong word
  QString marker( "_MARKER_" );
  tmpBuf.replace( lastpos, word.length(), marker );
  QString context = tmpBuf.mid(qMax(lastpos-18,0), 2*18+marker.length());
  context.replace( '\n',QLatin1Char(' '));
  context.replace( '<', QLatin1String("&lt;") );
  context.replace( '>', QLatin1String("&gt;") );
  context.replace( marker, QString::fromLatin1("<b>%1</b>").arg( word ) );
  context = "<qt>" + context + "</qt>";

  ksdlg->init( word, &sugg, context );
  d->m_bNoMisspellingsEncountered = false;
  emit misspelling( word, sugg, lastpos );

  emitProgress();
  ksdlg->show();
}

QString K3Spell::replacement () const
{
    return dlgreplacement;
}

void K3Spell::dialog2( int result )
{
  QString qs;

  disconnect( ksdlg, SIGNAL(command(int)), this, SLOT(dialog2(int)) );
  dialogwillprocess = false;
  dlgresult = result;
  ksdlg->standby();

  dlgreplacement = ksdlg->replacement();

  //process result here
  switch ( dlgresult )
  {
  case KS_IGNORE:
    emit ignoreword( dlgorigword );
    break;
  case KS_IGNOREALL:
    // would be better to lower case only words with beginning cap
    ignorelist.prepend( dlgorigword.toLower() );
    emit ignoreall( dlgorigword );
    break;
  case KS_ADD:
    addPersonal( dlgorigword );
    personaldict = true;
    emit addword( dlgorigword );
    // adding to pesonal dict takes effect at the next line, not the current
    ignorelist.prepend( dlgorigword.toLower() );
    break;
  case KS_REPLACEALL:
  {
    replacelist.append( dlgorigword );
    QString _replacement = replacement();
    replacelist.append( _replacement );
    emit replaceall( dlgorigword ,  _replacement );
  }
    break;
  case KS_SUGGEST:
    checkWord( ksdlg->replacement(), false, true );
    return;
    break;
  }

  connect( this, SIGNAL(dialog3()), this, dialog3slot.toAscii().constData() );
  emit dialog3();
}


K3Spell::~K3Spell()
{
  delete proc;
  delete ksconfig;
  delete ksdlg;
  delete d->checkNextTimer;
  delete d;
}


K3SpellConfig K3Spell::ksConfig() const
{
  ksconfig->setIgnoreList(ignorelist);
  ksconfig->setReplaceAllList(replacelist);
  return *ksconfig;
}

void K3Spell::cleanUp()
{
  if ( m_status == Cleaning )
    return; // Ignore

  if ( m_status == Running )
  {
    if ( personaldict )
      writePersonalDictionary();
    m_status = Cleaning;
  }
  proc->closeWriteChannel();
}

void K3Spell::setAutoDelete(bool _autoDelete)
{
    autoDelete = _autoDelete;
}

void K3Spell::ispellExit()
{
  kDebug() << "K3Spell::ispellExit() " << m_status;

  if ( (m_status == Starting) && (trystart < maxtrystart) )
  {
    trystart++;
    startIspell();
    return;
  }

  if ( m_status == Starting )
     m_status = Error;
  else if (m_status == Cleaning)
     m_status = d->m_bNoMisspellingsEncountered ? FinishedNoMisspellingsEncountered : Finished;
  else if ( m_status == Running )
     m_status = Crashed;
  else // Error, Finished, Crashed
     return; // Dead already

  kDebug(750) << "Death";
  QTimer::singleShot( 0, this, SLOT(emitDeath()) );
}

// This is always called from the event loop to make
// sure that the receiver can safely delete the
// K3Spell object.
void K3Spell::emitDeath()
{
  bool deleteMe = autoDelete; // Can't access object after next call!
  emit death();
  if ( deleteMe )
    deleteLater();
}

void K3Spell::setProgressResolution (unsigned int res)
{
  progres=res;
}

void K3Spell::emitProgress ()
{
  uint nextprog = (uint) (100.*lastpos/(double)totalpos);

  if ( nextprog >= curprog )
  {
    curprog = nextprog;
    emit progress( curprog );
  }
}

void K3Spell::moveDlg( int x, int y )
{
  QPoint pt( x,y ), pt2;
  pt2 = parent->mapToGlobal( pt );
  ksdlg->move( pt2.x(),pt2.y() );
}

void K3Spell::setIgnoreUpperWords(bool _ignore)
{
  d->m_bIgnoreUpperWords=_ignore;
}

void K3Spell::setIgnoreTitleCase(bool _ignore)
{
  d->m_bIgnoreTitleCase=_ignore;
}
// --------------------------------------------------
// Stuff for modal (blocking) spell checking
//
// Written by Torben Weis <weis@kde.org>. So please
// send bug reports regarding the modal stuff to me.
// --------------------------------------------------

int
K3Spell::modalCheck( QString& text )
{
  return modalCheck( text,0 );
}

int
K3Spell::modalCheck( QString& text, K3SpellConfig* _kcs )
{
  modalreturn = 0;
  modaltext = text;

  K3Spell* spell = new K3Spell( 0L, i18n("Spell Checker"), 0 ,
                              0, _kcs, true, true );

  while (spell->status()!=Finished)
    qApp->processEvents();

  text = modaltext;

  delete spell;
  return modalreturn;
}

void K3Spell::slotSpellCheckerCorrected( const QString & oldText, const QString & newText, unsigned int pos )
{
  modaltext=modaltext.replace(pos,oldText.length(),newText);
}


void K3Spell::slotModalReady()
{
  //kDebug() << qApp->loopLevel();
  //kDebug(750) << "MODAL READY------------------";

  Q_ASSERT( m_status == Running );
  connect( this, SIGNAL( done( const QString & ) ),
           this, SLOT( slotModalDone( const QString & ) ) );
  QObject::connect( this, SIGNAL( corrected( const QString&, const QString&, unsigned int ) ),
                    this, SLOT( slotSpellCheckerCorrected( const QString&, const QString &, unsigned int ) ) );
  QObject::connect( this, SIGNAL( death() ),
                    this, SLOT( slotModalSpellCheckerFinished( ) ) );
  check( modaltext );
}

void K3Spell::slotModalDone( const QString &/*_buffer*/ )
{
  //kDebug(750) << "MODAL DONE " << _buffer;
  //modaltext = _buffer;
  cleanUp();

  //kDebug() << "ABOUT TO EXIT LOOP";
  //qApp->exit_loop();

  //modalWidgetHack->close(true);
  slotModalSpellCheckerFinished();
}

void K3Spell::slotModalSpellCheckerFinished( )
{
  modalreturn=(int)this->status();
}

void K3Spell::initialize( QWidget *_parent, const QString &_caption,
                         QObject *obj, const char *slot, K3SpellConfig *_ksc,
                         bool _progressbar, bool _modal, SpellerType type )
{
  d = new K3SpellPrivate;

  d->m_bIgnoreUpperWords =false;
  d->m_bIgnoreTitleCase =false;
  d->m_bNoMisspellingsEncountered = true;
  d->type = type;
  d->checking = false;
  d->aspellV6 = false;
  d->checkNextTimer = new QTimer( this );
  connect( d->checkNextTimer, SIGNAL( timeout() ),
	   this, SLOT( checkNext() ));
  autoDelete = false;
  modaldlg = _modal;
  progressbar = _progressbar;

  proc     = 0;
  ksconfig = 0;
  ksdlg    = 0;
  lastpos  = 0;

  //won't be using the dialog in ksconfig, just the option values
  if ( _ksc )
    ksconfig = new K3SpellConfig( *_ksc );
  else
    ksconfig = new K3SpellConfig;

  d->m_codec = 0;
  switch ( ksconfig->encoding() )
  {
  case KS_E_LATIN1:
     d->m_codec = QTextCodec::codecForName("ISO 8859-1");
     break;
  case KS_E_LATIN2:
     d->m_codec = QTextCodec::codecForName("ISO 8859-2");
     break;
  case KS_E_LATIN3:
      d->m_codec = QTextCodec::codecForName("ISO 8859-3");
      break;
  case KS_E_LATIN4:
      d->m_codec = QTextCodec::codecForName("ISO 8859-4");
      break;
  case KS_E_LATIN5:
      d->m_codec = QTextCodec::codecForName("ISO 8859-5");
      break;
  case KS_E_LATIN7:
      d->m_codec = QTextCodec::codecForName("ISO 8859-7");
      break;
  case KS_E_LATIN8:
      d->m_codec = QTextCodec::codecForName("ISO 8859-8-i");
      break;
  case KS_E_LATIN9:
      d->m_codec = QTextCodec::codecForName("ISO 8859-9");
      break;
  case KS_E_LATIN13:
      d->m_codec = QTextCodec::codecForName("ISO 8859-13");
      break;
  case KS_E_LATIN15:
      d->m_codec = QTextCodec::codecForName("ISO 8859-15");
      break;
  case KS_E_UTF8:
      d->m_codec = QTextCodec::codecForName("UTF-8");
      break;
  case KS_E_KOI8R:
      d->m_codec = QTextCodec::codecForName("KOI8-R");
      break;
  case KS_E_KOI8U:
      d->m_codec = QTextCodec::codecForName("KOI8-U");
      break;
  case KS_E_CP1251:
      d->m_codec = QTextCodec::codecForName("CP1251");
      break;
  case KS_E_CP1255:
      d->m_codec = QTextCodec::codecForName("CP1255");
      break;
  default:
     break;
  }

  kDebug(750) << __FILE__ << ":" << __LINE__ << " Codec = " << (d->m_codec ? d->m_codec->name() : "<default>");

  // copy ignore list from ksconfig
  ignorelist += ksconfig->ignoreList();

  replacelist += ksconfig->replaceAllList();
  texmode=dlgon=false;
  m_status = Starting;
  dialogsetup = false;
  progres=10;
  curprog=0;

  dialogwillprocess = false;
  dialog3slot.clear();

  personaldict = false;
  dlgresult = -1;

  caption = _caption;

  parent = _parent;

  trystart = 0;
  maxtrystart = 2;

  if ( obj && slot )
      // caller wants to know when k3spell is ready
      connect( this, SIGNAL(ready(K3Spell *)), obj, slot);
  else
      // Hack for modal spell checking
      connect( this, SIGNAL(ready(K3Spell *)), this, SLOT(slotModalReady()) );

  proc = new KProcess();

  startIspell();
}

QString K3Spell::modaltext;
int K3Spell::modalreturn = 0;
QWidget* K3Spell::modalWidgetHack = 0;

#include "k3spell.moc"

