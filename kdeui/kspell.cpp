/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h> // atoi

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <qregexp.h>
#include <qtextcodec.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
#include "kspell.h"
#include "kspelldlg.h"
#include <kwin.h>
#include <kprocio.h>

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

class KSpell::KSpellPrivate
{
public:
  bool endOfResponse;
  bool m_bIgnoreUpperWords;
  bool m_bIgnoreTitleCase;
  bool m_bNoMisspellingsEncountered;
  SpellerType type;
  KSpell* suggestSpell;
  bool checking;
  QValueList<BufferedWord> unchecked;
  QTimer *checkNextTimer;
  bool aspellV6;
};

//TODO
//Parse stderr output
//e.g. -- invalid dictionary name

/*
  Things to put in KSpellConfigDlg:
    make root/affix combinations that aren't in the dictionary (-m)
    don't generate any affix/root combinations (-P)
    Report  run-together  words   with   missing blanks as spelling errors.  (-B)
    default dictionary (-d [dictionary])
    personal dictionary (-p [dictionary])
    path to ispell -- NO: ispell should be in $PATH
    */


//  Connects a slot to KProcIO's output signal
#define OUTPUT(x) (connect (proc, SIGNAL (readReady(KProcIO *)), this, SLOT (x(KProcIO *))))

// Disconnect a slot from...
#define NOOUTPUT(x) (disconnect (proc, SIGNAL (readReady(KProcIO *)), this, SLOT (x(KProcIO *))))



KSpell::KSpell( QWidget *_parent, const QString &_caption,
		QObject *obj, const char *slot, KSpellConfig *_ksc,
		bool _progressbar, bool _modal )
{
  initialize( _parent, _caption, obj, slot, _ksc,
              _progressbar, _modal, Text );
}

KSpell::KSpell( QWidget *_parent, const QString &_caption,
		QObject *obj, const char *slot, KSpellConfig *_ksc,
		bool _progressbar, bool _modal, SpellerType type )
{
  initialize( _parent, _caption, obj, slot, _ksc,
              _progressbar, _modal, type );
}

void KSpell::hide() { ksdlg->hide(); }

int KSpell::heightDlg() const { return ksdlg->height(); }
int KSpell::widthDlg() const { return ksdlg->width(); }

// Check if aspell is at least version 0.6
static bool determineASpellV6()
{
  QString result;
  FILE *fs = popen("aspell -v", "r");
  if (fs)
  {
    // Close textstream before we close fs
    {
    QTextStream ts(fs, IO_ReadOnly);
    result = ts.read().stripWhiteSpace();
    }
    pclose(fs);
  }

  QRegExp rx("Aspell (\\d.\\d)");
  if (rx.search(result) != -1)
  {
     float version = rx.cap(1).toFloat();
     return (version >= 0.6);
  }
  return false;
}


void
KSpell::startIspell()
  //trystart = {0,1,2}
{
  if ((trystart == 0) && (ksconfig->client() == KS_CLIENT_ASPELL))
     d->aspellV6 = determineASpellV6();

  kdDebug(750) << "Try #" << trystart << endl;

  if ( trystart > 0 ) {
    proc->resetAll();
  }

  switch ( ksconfig->client() )
  {
  case KS_CLIENT_ISPELL:
    *proc << "ispell";
    kdDebug(750) << "Using ispell" << endl;
    break;
  case KS_CLIENT_ASPELL:
    *proc << "aspell";
    kdDebug(750) << "Using aspell" << endl;
    break;
  case KS_CLIENT_HSPELL:
    *proc << "hspell";
    kdDebug(750) << "Using hspell" << endl;
    break;
  }

  if ( ksconfig->client() == KS_CLIENT_ISPELL || ksconfig->client() == KS_CLIENT_ASPELL )
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
      //only ispell supports
      if ( ksconfig->client() == KS_CLIENT_ISPELL )
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
        kdDebug(750) << "using dictionary [" << ksconfig->dictionary() << "]" << endl;
        *proc << "-d";
        *proc << ksconfig->dictionary();
      }
    }

  //Note to potential debuggers:  -Tlatin2 _is_ being added on the
  //  _first_ try.  But, some versions of ispell will fail with this
  // option, so kspell tries again without it.  That's why as 'ps -ax'
  // shows "ispell -a -S ..." withou the "-Tlatin2" option.

    if ( trystart<1 ) {
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
      case KS_E_LATIN15:
	// will work, if this is the default charset in the dictionary
	kdError(750) << "charsets iso-8859-4 .. iso-8859-15 not supported yet" << endl;
	break;
      case KS_E_UTF8:
        *proc << "-Tutf8";
        if (ksconfig->client() == KS_CLIENT_ASPELL)
          *proc << "--encoding=utf-8";
        else
          *proc << "-Tutf8";

        break;
      case KS_E_KOI8U:
	*proc << "-w'"; // add ' as a word char
	break;
      }
    }

  // -a : pipe mode
  // -S : sort suggestions by probable correctness
  }
  else       // hspell doesn't need all the rest of the options
    *proc << "-a";

  if (trystart == 0) //don't connect these multiple times
  {
    connect( proc, SIGNAL(receivedStderr(KProcess *, char *, int)),
             this, SLOT(ispellErrors(KProcess *, char *, int)) );

    connect( proc, SIGNAL(processExited(KProcess *)),
             this, SLOT(ispellExit (KProcess *)) );

    OUTPUT(KSpell2);
  }

  if ( !proc->start() )
  {
    m_status = Error;
    QTimer::singleShot( 0, this, SLOT(emitDeath()));
  }
}

void
KSpell::ispellErrors( KProcess *, char *buffer, int buflen )
{
  buffer[buflen-1] = '\0';
  //  kdDebug(750) << "ispellErrors [" << buffer << "]\n" << endl;
}

void KSpell::KSpell2( KProcIO * )

{
  QString line;

  kdDebug(750) << "KSpell::KSpell2" << endl;

  trystart = maxtrystart;  //We've officially started ispell and don't want
                           //to try again if it dies.

  if ( proc->readln( line, true ) == -1 )
  {
     QTimer::singleShot( 0, this, SLOT(emitDeath()) );
     return;
  }


  if ( line[0] != '@' ) //@ indicates that ispell is working fine
  {
     QTimer::singleShot( 0, this, SLOT(emitDeath()) );
     return;
  }

  //We want to recognize KDE in any text!
  if ( !ignore("kde") )
  {
     kdDebug(750) << "@KDE was false" << endl;
     QTimer::singleShot( 0, this, SLOT(emitDeath()) );
     return;
  }

  //We want to recognize linux in any text!
  if ( !ignore("linux") )
  {
     kdDebug(750) << "@Linux was false" << endl;
     QTimer::singleShot( 0, this, SLOT(emitDeath()) );
     return;
  }

  NOOUTPUT( KSpell2 );

  m_status = Running;
  emit ready( this );
}

void
KSpell::setUpDialog( bool reallyuseprogressbar )
{
  if ( dialogsetup )
    return;

  //Set up the dialog box
  ksdlg = new KSpellDlg( parent, "dialog",
                         progressbar && reallyuseprogressbar, modaldlg );
  ksdlg->setCaption( caption );

  connect( ksdlg, SIGNAL(command(int)),
           this, SLOT(slotStopCancel(int)) );
  connect( this, SIGNAL(progress(unsigned int)),
	   ksdlg, SLOT(slotProgress(unsigned int)) );

#ifdef Q_WS_X11 // FIXME(E): Implement for Qt/Embedded
  KWin::setIcons( ksdlg->winId(), kapp->icon(), kapp->miniIcon() );
#endif
  if ( modaldlg )
    ksdlg->setFocus();
  dialogsetup = true;
}

bool KSpell::addPersonal( const QString & word )
{
  QString qs = word.simplifyWhiteSpace();

  //we'll let ispell do the work here b/c we can
  if ( qs.find(' ') != -1 || qs.isEmpty() )    // make sure it's a _word_
    return false;

  qs.prepend( "*" );
  personaldict = true;

  return proc->writeStdin( qs );
}

bool KSpell::writePersonalDictionary()
{
  return proc->writeStdin("#");
}

bool KSpell::ignore( const QString & word )
{
  QString qs = word.simplifyWhiteSpace();

  //we'll let ispell do the work here b/c we can
  if ( qs.find (' ') != -1 || qs.isEmpty() )    // make sure it's a _word_
    return false;

  qs.prepend( "@" );

  return proc->writeStdin( qs );
}

bool
KSpell::cleanFputsWord( const QString & s, bool appendCR )
{
  QString qs(s);
  bool empty = true;

  for( unsigned int i = 0; i < qs.length(); i++ )
  {
    //we need some punctuation for ornaments
    if ( qs[i] != '\'' && qs[i] != '\"' && qs[i] != '-'
         && qs[i].isPunct() || qs[i].isSpace() )
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

  return proc->writeStdin( "^"+qs, appendCR );
}

bool
KSpell::cleanFputs( const QString & s, bool appendCR )
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
    return proc->writeStdin( "^"+qs, appendCR );
  }
  else
    return proc->writeStdin( QString::fromAscii( "^\n" ),appendCR );
}

bool KSpell::checkWord( const QString & buffer, bool _usedialog )
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
  QString qs = buffer.simplifyWhiteSpace();

  if ( qs.find (' ') != -1 || qs.isEmpty() ) {   // make sure it's a _word_
    d->checkNextTimer->start( 0, true );
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

  QString blank_line;
  while (proc->readln( blank_line, true ) != -1); // eat spurious blanks

  OUTPUT(checkWord2);
  //  connect (this, SIGNAL (dialog3()), this, SLOT (checkWord3()));

  proc->writeStdin( "%" ); // turn off terse mode
  proc->writeStdin( buffer ); // send the word to ispell

  return true;
}

bool KSpell::checkWord( const QString & buffer, bool _usedialog, bool suggest )
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
  QString qs = buffer.simplifyWhiteSpace();

  if ( qs.find (' ') != -1 || qs.isEmpty() ) {   // make sure it's a _word_
    d->checkNextTimer->start( 0, true );
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
  
  QString blank_line;
  while (proc->readln( blank_line, true ) != -1); // eat spurious blanks

  OUTPUT(checkWord2);
  //  connect (this, SIGNAL (dialog3()), this, SLOT (checkWord3()));

  proc->writeStdin( "%" ); // turn off terse mode
  proc->writeStdin( buffer ); // send the word to ispell

  return true;
}

void KSpell::checkWord2( KProcIO* )
{
  QString word;
  QString line;
  proc->readln( line, true ); //get ispell's response

/* ispell man page: "Each sentence of text input is terminated with an
   additional blank line,  indicating that ispell has completed processing
   the input line."
   <sanders>
   But there can be multiple lines returned in the case of an error,
   in this case we should consume all the output given otherwise spell checking
   can get out of sync.
   </sanders>
*/
  QString blank_line;
  while (proc->readln( blank_line, true ) != -1); // eat the blank line
  NOOUTPUT(checkWord2);
  
  bool mistake = ( parseOneResponse(line, word, sugg) == MISTAKE );
  if ( mistake && usedialog )
  {
    cwword = word;
    dialog( word, sugg, SLOT(checkWord3()) );
    d->checkNextTimer->start( 0, true );
    return;
  }
  else if( mistake )
  {
    emit misspelling( word, sugg, lastpos );
  }

  //emits a "corrected" signal _even_ if no change was made
  //so that the calling program knows when the check is complete
  emit corrected( word, word, 0L );
  d->checkNextTimer->start( 0, true );
}

void KSpell::checkNext()
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

void KSpell::suggestWord( KProcIO * )
{
  QString word;
  QString line;
  proc->readln( line, true ); //get ispell's response

/* ispell man page: "Each sentence of text input is terminated with an
   additional blank line,  indicating that ispell has completed processing
   the input line." */
  QString blank_line;
  proc->readln( blank_line, true ); // eat the blank line

  NOOUTPUT(checkWord2);

  bool mistake = ( parseOneResponse(line, word, sugg) == MISTAKE );
  if ( mistake && usedialog )
  {
    cwword=word;
    dialog( word, sugg, SLOT(checkWord3()) );
    return;
  }
}

void KSpell::checkWord3()
{
  disconnect( this, SIGNAL(dialog3()), this, SLOT(checkWord3()) );

  emit corrected( cwword, replacement(), 0L );
}

QString KSpell::funnyWord( const QString & word )
  // composes a guess from ispell to a readable word
  // e.g. "re+fry-y+ies" -> "refries"
{
  QString qs;
  unsigned int i=0;

  for( i=0; word [i]!='\0';i++ )
  {
    if (word [i]=='+')
      continue;
    if (word [i]=='-')
    {
      QString shorty;
      unsigned int j;
      int k;

      for( j = i+1; word[j] != '\0' && word[j] != '+' && word[j] != '-'; j++ )
        shorty += word[j];

      i = j-1;

      if ( !( k = qs.findRev(shorty) ) || k != -1 )
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


int KSpell::parseOneResponse( const QString &buffer, QString &word, QStringList & sugg )
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


    word = buffer.mid( 2, buffer.find( ' ', 3 ) -2 );
    //check() needs this
    orig=word;

    if( d->m_bIgnoreTitleCase && word == word.upper() )
      return IGNORE;

    if( d->m_bIgnoreUpperWords && word[0] == word[0].upper() )
    {
      QString text = word[0] + word.right( word.length()-1 ).lower();
      if( text == word )
        return IGNORE;
    }

    /////// Ignore-list stuff //////////
    //We don't take advantage of ispell's ignore function because
    //we can't interrupt ispell's output (when checking a large
    //buffer) to add a word to _it's_ ignore-list.
    if ( ignorelist.findIndex( word.lower() ) != -1 )
      return IGNORE;

    //// Position in line ///
    QString qs2;

    if ( buffer.find( ':' ) != -1 )
      qs2 = buffer.left( buffer.find(':') );
    else
      qs2 = buffer;

    posinline = qs2.right( qs2.length()-qs2.findRev(' ') ).toInt()-1;

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
      QString qs = buffer.mid( buffer.find(':')+2, buffer.length() );
      qs += ',';
      sugg.clear();
      i = j = 0;

      while( (unsigned int)i < qs.length() )
      {
        QString temp = qs.mid( i, (j=qs.find (',',i)) - i );
        sugg.append( funnyWord(temp) );

        i=j+2;
      }
    }

    if ( (sugg.count()==1) && (sugg.first() == word) )
      return GOOD;

    return MISTAKE;
  }

  if ( buffer.isEmpty() ) {
      kdDebug(750) << "Got an empty response: ignoring"<<endl;
      return GOOD;
  }

  kdError(750) << "HERE?: [" << buffer << "]" << endl;
  kdError(750) << "Please report this to zack@kde.org" << endl;
  kdError(750) << "Thank you!" << endl;

  emit done( false );
  emit done( KSpell::origbuffer );
  return MISTAKE;
}

bool KSpell::checkList (QStringList *_wordlist, bool _usedialog)
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

  proc->writeStdin ("%"); // turn off terse mode & check one word at a time

  //lastpos now counts which *word number* we are at in checkListReplaceCurrent()
  lastpos = -1;
  checkList2();

  // when checked, KProcIO calls checkList3a
  OUTPUT(checkList3a);

  return true;
}

void KSpell::checkList2 ()
  // send one word from the list to KProcIO
  // invoked first time by checkList, later by checkListReplaceCurrent and checkList4
{
  // send next word
  if (wlIt != wordlist->end())
  {
    kdDebug(750) << "KS::cklist2 " << lastpos << ": " << *wlIt << endl;

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

void KSpell::checkList3a (KProcIO *)
  // invoked by KProcIO, when data from ispell are read
{
  //kdDebug(750) << "start of checkList3a" << endl;

  // don't read more data, when dialog is waiting
  // for user interaction
  if ( dlgon ) {
    //kdDebug(750) << "dlgon: don't read more data" << endl;
    return;
  }

  int e, tempe;

  QString word;
  QString line;

  do
  {
    tempe=proc->readln( line, true ); //get ispell's response

    //kdDebug(750) << "checkList3a: read bytes [" << tempe << "]" << endl;


    if ( tempe == 0 ) {
      d->endOfResponse = true;
      //kdDebug(750) << "checkList3a: end of resp" << endl;
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

  //kdDebug(750) << "checkList3a: exit loop with [" << tempe << "]" << endl;

  // if we got an empty line, t.e. end of ispell/aspell response
  // and the dialog isn't waiting for user interaction, send next word
  if (d->endOfResponse && !dlgon) {
    //kdDebug(750) << "checkList3a: send next word" << endl;
    checkList2();
  }
}

void KSpell::checkListReplaceCurrent()
{

  // go back to misspelled word
  wlIt--;

  QString s = *wlIt;
  s.replace(posinline+offset,orig.length(),replacement());
  offset += replacement().length()-orig.length();
  wordlist->insert (wlIt, s);
  wlIt = wordlist->remove (wlIt);
  // wlIt now points to the word after the repalced one

}

void KSpell::checkList4 ()
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
    kdDebug(750) << "KS: cklist4: lastpos: " << lastpos << endl;
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
    //proc = new KProcIO( codec );
    //startIspell();
    return;
  };

  // read more if there is more, otherwise send next word
  if (!d->endOfResponse) {
    //kdDebug(750) << "checkList4: read more from response" << endl;
    checkList3a(NULL);
  }
}

bool KSpell::check( const QString &_buffer, bool _usedialog )
{
  QString qs;

  usedialog = _usedialog;
  setUpDialog();
  //set the dialog signal handler
  dialog3slot = SLOT(check3());

  kdDebug(750) << "KS: check" << endl;
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

  // KProcIO calls check2 when read from ispell
  OUTPUT( check2 );
  proc->writeStdin( "!" );

  //lastpos is a position in newbuffer (it has offset in it)
  offset = lastlastline = lastpos = lastline = 0;

  emitProgress();

  // send first buffer line
  int i = origbuffer.find( '\n', 0 ) + 1;
  qs = origbuffer.mid( 0, i );
  cleanFputs( qs, false );

  lastline=i; //the character position, not a line number

  if ( usedialog )
  {
    emitProgress();
  }
  else
    ksdlg->hide();

  return true;
}


void KSpell::check2( KProcIO * )
  // invoked by KProcIO when read from ispell
{
  int e, tempe;
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
    tempe = proc->readln( line, false ); //get ispell's response
    //kdDebug(750) << "KSpell::check2 (" << tempe << "b)" << endl;

    if ( tempe>0 )
    {
      if ( ( e=parseOneResponse (line, word, sugg) )==MISTAKE ||
           e==REPLACE)
      {
        dlgresult=-1;

        // for multibyte encoding posinline needs correction
        if ((ksconfig->encoding() == KS_E_UTF8) && !d->aspellV6) {
          // kdDebug(750) << "line: " << origbuffer.mid(lastlastline,
          // lastline-lastlastline) << endl;
          // kdDebug(750) << "posinline uncorr: " << posinline << endl;

          // convert line to UTF-8, cut at pos, convert back to UCS-2
          // and get string length
          posinline = (QString::fromUtf8(
                         origbuffer.mid(lastlastline,lastline-lastlastline).utf8(),
                         posinline)).length();
          // kdDebug(750) << "posinline corr: " << posinline << endl;
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
          //kdDebug(750) << "(Before dialog) word=[" << word << "] cwword =[" << cwword << "]\n" << endl;
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

  proc->ackRead();


  if ( tempe == -1 ) { //we were called, but no data seems to be ready...
    recursive = false;
    return;
  }

  //If there is more to check, then send another line to ISpell.
  if ( (unsigned int)lastline < origbuffer.length() )
  {
    int i;
    QString qs;

    //kdDebug(750) << "[EOL](" << tempe << ")[" << temp << "]" << endl;

    lastpos = (lastlastline=lastline) + offset; //do we really want this?
    i = origbuffer.find('\n', lastline) + 1;
    qs = origbuffer.mid( lastline, i-lastline );
    cleanFputs( qs, false );
    lastline = i;
    recursive = false;
    return;
  }
  else
    //This is the end of it all
  {
    ksdlg->hide();
    //      kdDebug(750) << "check2() done" << endl;
    newbuffer.truncate( newbuffer.length()-2 );
    emitProgress();
    emit done( newbuffer );
  }
  recursive = false;
}

void KSpell::check3 ()
  // evaluates the return value of the dialog
{
  disconnect (this, SIGNAL (dialog3()), this, SLOT (check3()));
  kdDebug(750) << "check3 [" << cwword << "] [" << replacement() << "] " << dlgresult << endl;

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
    //      kdDebug(750) << "canceled\n" << endl;
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

  proc->ackRead();
}

void
KSpell::slotStopCancel (int result)
{
  if (dialogwillprocess)
    return;

  kdDebug(750) << "KSpell::slotStopCancel [" << result << "]" << endl;

  if (result==KS_STOP || result==KS_CANCEL)
    if (!dialog3slot.isEmpty())
    {
      dlgresult=result;
      connect (this, SIGNAL (dialog3()), this, dialog3slot.ascii());
      emit dialog3();
    }
}


void KSpell::dialog( const QString & word, QStringList & sugg, const char *_slot )
{
  dlgorigword = word;

  dialog3slot = _slot;
  dialogwillprocess = true;
  connect( ksdlg, SIGNAL(command(int)), this, SLOT(dialog2(int)) );
  QString tmpBuf = newbuffer;
  kdDebug(750)<<" position = "<<lastpos<<endl;

  // extract a context string, replace all characters which might confuse
  // the RichText display and highlight the possibly wrong word
  QString marker( "_MARKER_" );
  tmpBuf.replace( lastpos, word.length(), marker );
  QString context = tmpBuf.mid(QMAX(lastpos-18,0), 2*18+marker.length());
  context.replace( '\n',QString::fromLatin1(" "));
  context.replace( '<', QString::fromLatin1("&lt;") );
  context.replace( '>', QString::fromLatin1("&gt;") );
  context.replace( marker, QString::fromLatin1("<b>%1</b>").arg( word ) );
  context = "<qt>" + context + "</qt>";

  ksdlg->init( word, &sugg, context );
  d->m_bNoMisspellingsEncountered = false;
  emit misspelling( word, sugg, lastpos );

  emitProgress();
  ksdlg->show();
}

void KSpell::dialog2( int result )
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
    ignorelist.prepend( dlgorigword.lower() );
    emit ignoreall( dlgorigword );
    break;
  case KS_ADD:
    addPersonal( dlgorigword );
    personaldict = true;
    emit addword( dlgorigword );
    // adding to pesonal dict takes effect at the next line, not the current
    ignorelist.prepend( dlgorigword.lower() );
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

  connect( this, SIGNAL(dialog3()), this, dialog3slot.ascii() );
  emit dialog3();
}


KSpell::~KSpell()
{
  delete proc;
  delete ksconfig;
  delete ksdlg;
  delete d->checkNextTimer;
  delete d;
}


KSpellConfig KSpell::ksConfig() const
{
  ksconfig->setIgnoreList(ignorelist);
  ksconfig->setReplaceAllList(replacelist);
  return *ksconfig;
}

void KSpell::cleanUp()
{
  if ( m_status == Cleaning )
    return; // Ignore

  if ( m_status == Running )
  {
    if ( personaldict )
      writePersonalDictionary();
    m_status = Cleaning;
  }
  proc->closeStdin();
}

void KSpell::ispellExit( KProcess* )
{
  kdDebug() << "KSpell::ispellExit() " << m_status << endl;

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

  kdDebug(750) << "Death" << endl;
  QTimer::singleShot( 0, this, SLOT(emitDeath()) );
}

// This is always called from the event loop to make
// sure that the receiver can safely delete the
// KSpell object.
void KSpell::emitDeath()
{
  bool deleteMe = autoDelete; // Can't access object after next call!
  emit death();
  if ( deleteMe )
    deleteLater();
}

void KSpell::setProgressResolution (unsigned int res)
{
  progres=res;
}

void KSpell::emitProgress ()
{
  uint nextprog = (uint) (100.*lastpos/(double)totalpos);

  if ( nextprog >= curprog )
  {
    curprog = nextprog;
    emit progress( curprog );
  }
}

void KSpell::moveDlg( int x, int y )
{
  QPoint pt( x,y ), pt2;
  pt2 = parent->mapToGlobal( pt );
  ksdlg->move( pt2.x(),pt2.y() );
}

void KSpell::setIgnoreUpperWords(bool _ignore)
{
  d->m_bIgnoreUpperWords=_ignore;
}

void KSpell::setIgnoreTitleCase(bool _ignore)
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
KSpell::modalCheck( QString& text )
{
  return modalCheck( text,0 );
}

int
KSpell::modalCheck( QString& text, KSpellConfig* _kcs )
{
  modalreturn = 0;
  modaltext = text;

  KSpell* spell = new KSpell( 0L, i18n("Spell Checker"), 0 ,
                              0, _kcs, true, true );

  while (spell->status()!=Finished)
    kapp->processEvents();

  text = modaltext;

  delete spell;
  return modalreturn;
}

void KSpell::slotSpellCheckerCorrected( const QString & oldText, const QString & newText, unsigned int pos )
{
  modaltext=modaltext.replace(pos,oldText.length(),newText);
}


void KSpell::slotModalReady()
{
  //kdDebug() << qApp->loopLevel() << endl;
  //kdDebug(750) << "MODAL READY------------------" << endl;

  Q_ASSERT( m_status == Running );
  connect( this, SIGNAL( done( const QString & ) ),
           this, SLOT( slotModalDone( const QString & ) ) );
  QObject::connect( this, SIGNAL( corrected( const QString&, const QString&, unsigned int ) ),
                    this, SLOT( slotSpellCheckerCorrected( const QString&, const QString &, unsigned int ) ) );
  QObject::connect( this, SIGNAL( death() ),
                    this, SLOT( slotModalSpellCheckerFinished( ) ) );
  check( modaltext );
}

void KSpell::slotModalDone( const QString &/*_buffer*/ )
{
  //kdDebug(750) << "MODAL DONE " << _buffer << endl;
  //modaltext = _buffer;
  cleanUp();

  //kdDebug() << "ABOUT TO EXIT LOOP" << endl;
  //qApp->exit_loop();

  //modalWidgetHack->close(true);
  slotModalSpellCheckerFinished();
}

void KSpell::slotModalSpellCheckerFinished( )
{
  modalreturn=(int)this->status();
}

void KSpell::initialize( QWidget *_parent, const QString &_caption,
                         QObject *obj, const char *slot, KSpellConfig *_ksc,
                         bool _progressbar, bool _modal, SpellerType type )
{
  d = new KSpellPrivate;

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
    ksconfig = new KSpellConfig( *_ksc );
  else
    ksconfig = new KSpellConfig;

  codec = 0;
  switch ( ksconfig->encoding() )
  {
  case KS_E_LATIN1:
     codec = QTextCodec::codecForName("ISO 8859-1");
     break;
  case KS_E_LATIN2:
     codec = QTextCodec::codecForName("ISO 8859-2");
     break;
  case KS_E_LATIN3:
      codec = QTextCodec::codecForName("ISO 8859-3");
      break;
  case KS_E_LATIN4:
      codec = QTextCodec::codecForName("ISO 8859-4");
      break;
  case KS_E_LATIN5:
      codec = QTextCodec::codecForName("ISO 8859-5");
      break;
  case KS_E_LATIN7:
      codec = QTextCodec::codecForName("ISO 8859-7");
      break;
  case KS_E_LATIN8:
      codec = QTextCodec::codecForName("ISO 8859-8-i");
      break;
  case KS_E_LATIN9:
      codec = QTextCodec::codecForName("ISO 8859-9");
      break;
  case KS_E_LATIN13:
      codec = QTextCodec::codecForName("ISO 8859-13");
      break;
  case KS_E_LATIN15:
      codec = QTextCodec::codecForName("ISO 8859-15");
      break;
  case KS_E_UTF8:
      codec = QTextCodec::codecForName("UTF-8");
      break;
  case KS_E_KOI8R:
      codec = QTextCodec::codecForName("KOI8-R");
      break;
  case KS_E_KOI8U:
      codec = QTextCodec::codecForName("KOI8-U");
      break;
  case KS_E_CP1251:
      codec = QTextCodec::codecForName("CP1251");
      break;
  case KS_E_CP1255:
      codec = QTextCodec::codecForName("CP1255");
      break;
  default:
     break;
  }

  kdDebug(750) << __FILE__ << ":" << __LINE__ << " Codec = " << (codec ? codec->name() : "<default>") << endl;

  // copy ignore list from ksconfig
  ignorelist += ksconfig->ignoreList();

  replacelist += ksconfig->replaceAllList();
  texmode=dlgon=false;
  m_status = Starting;
  dialogsetup = false;
  progres=10;
  curprog=0;

  dialogwillprocess = false;
  dialog3slot = QString::null;

  personaldict = false;
  dlgresult = -1;

  caption = _caption;

  parent = _parent;

  trystart = 0;
  maxtrystart = 2;

  if ( obj && slot )
      // caller wants to know when kspell is ready
      connect( this, SIGNAL(ready(KSpell *)), obj, slot);
  else
      // Hack for modal spell checking
      connect( this, SIGNAL(ready(KSpell *)), this, SLOT(slotModalReady()) );

  proc = new KProcIO( codec );

  startIspell();
}

QString KSpell::modaltext;
int KSpell::modalreturn = 0;
QWidget* KSpell::modalWidgetHack = 0;

#include "kspell.moc"

