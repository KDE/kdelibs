/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>

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

// $Id$

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

#include <qtextcodec.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include "kspell.h"
#include <kwin.h>

#define MAXLINELENGTH 10000

enum {
	GOOD=     0,
	IGNORE=   1,
	REPLACE=  2,
	MISTAKE=  3
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
		


KSpell::KSpell (QWidget *_parent, QString _caption,
		QObject *obj, const char *slot, KSpellConfig *_ksc,
		bool _progressbar, bool _modal)
{
  autoDelete = false;
  modaldlg = _modal;
  progressbar = _progressbar;

  proc=0;
  ksconfig=0;
  ksdlg=0;

  //won't be using the dialog in ksconfig, just the option values
  if (_ksc!=0)
    ksconfig = new KSpellConfig (*_ksc);
  else
    ksconfig = new KSpellConfig;

  codec = 0;
  switch (ksconfig->encoding())
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
      codec = QTextCodec::codecForName("ISO 8859-8");
      break;
  case KS_E_LATIN9:
      codec = QTextCodec::codecForName("ISO 8859-9");
      break;
  case KS_E_LATIN15:
      codec = QTextCodec::codecForName("ISO 8859-15");
      break;
  case KS_E_UTF8:
      codec = QTextCodec::codecForName("UTF-8");
      break;
  default:
     break;
  }

  kdDebug(750) << __FILE__ << ":" << __LINE__ << " Codec = " << (codec ? codec->name() : "<default>") << endl;

  texmode=dlgon=FALSE;
  m_status = Starting;
  dialogsetup = FALSE;
  progres=10;
  curprog=0;

  dialogwillprocess=FALSE;
  dialog3slot="";

  personaldict=FALSE;
  dlgresult=-1;

  caption=_caption;

  parent=_parent;

  trystart=0;
  maxtrystart=2;

  if ( obj && slot )
      // caller wants to know when kspell is ready
      connect (this, SIGNAL (ready(KSpell *)), obj, slot);
  else
      // Hack for modal spell checking
      connect (this, SIGNAL (ready(KSpell *)), this, SLOT( slotModalReady() ) );
  proc=new KProcIO(codec);

  startIspell();
}


void
KSpell::startIspell()
  //trystart = {0,1,2}
{

  kdDebug(750) << "Try #" << trystart << endl;

  if (trystart>0)
    proc->resetAll();

  switch (ksconfig->client())
    {
    case KS_CLIENT_ISPELL:
      proc->setExecutable("ispell");
      kdDebug(750) << "Using ispell" << endl;
      break;
    case KS_CLIENT_ASPELL:
      proc->setExecutable("aspell");
      kdDebug(750) << "Using aspell" << endl;
      break;
    }

  *proc << "-a" << "-S";
  if (ksconfig->noRootAffix())
    {
      *proc<<"-m";
    }
  if (ksconfig->runTogether())
    {
      *proc << "-B";
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

  if (trystart<1)
    switch (ksconfig->encoding())
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
      case KS_E_LATIN15:
      
	// will work, if this is the default charset in the dictionary
	kdError(750) << "charsets iso-8859-4 .. iso-8859-15 not supported yet" << endl;
	break;

      case KS_E_UTF8:
        *proc << "-Tutf8";
        break;
      }


  /*
  if (ksconfig->personalDict()[0]!='\0')
    {
      kdDebug(750) << "personal dictionary [" << ksconfig->personalDict() << "]" << endl;
      *proc << "-p";
      *proc << ksconfig->personalDict();
    }
    */


  // -a : pipe mode
  // -S : sort suggestions by probable correctness
  if (trystart==0) //don't connect these multiple times
    {
      connect (proc, SIGNAL (  receivedStderr (KProcess *, char *, int)),
	       this, SLOT (ispellErrors (KProcess *, char *, int)));


      connect(proc, SIGNAL(processExited(KProcess *)),
	      this, SLOT (ispellExit (KProcess *)));

      OUTPUT(KSpell2);
    }

  if (proc->start ()==FALSE)
  {
     QTimer::singleShot( 0, this, SLOT(emitDeath()));
  }
}

void
KSpell::ispellErrors (KProcess *, char *buffer, int buflen)
{
  buffer [buflen-1] = '\0';
  //  kdDebug(750) << "ispellErrors [" << buffer << "]\n" << endl;
}

void KSpell::KSpell2 (KProcIO *)

{
  kdDebug(750) << "KSpell::KSpell2" << endl;
  trystart=maxtrystart;  //We've officially started ispell and don't want
       //to try again if it dies.
  QString line;

  if (proc->fgets (line, TRUE)==-1)
  {
     QTimer::singleShot( 0, this, SLOT(emitDeath()));
     return;
  }


  if (line[0]!='@') //@ indicates that ispell is working fine
  {
     QTimer::singleShot( 0, this, SLOT(emitDeath()));
     return;
  }

  //We want to recognize KDE in any text!
  if (ignore ("kde")==FALSE)
  {
     kdDebug(750) << "@KDE was FALSE" << endl;
     QTimer::singleShot( 0, this, SLOT(emitDeath()));
     return;
  }

  //We want to recognize linux in any text!
  if (ignore ("linux")==FALSE)
  {
     kdDebug(750) << "@Linux was FALSE" << endl;
     QTimer::singleShot( 0, this, SLOT(emitDeath()));
     return;
  }

  NOOUTPUT (KSpell2);

  m_status = Running;
  emit ready(this);
}

void
KSpell::setUpDialog (bool reallyuseprogressbar)
{
  if (dialogsetup)
    return;

  //Set up the dialog box
  ksdlg=new KSpellDlg (parent, "dialog",
		       progressbar && reallyuseprogressbar, modaldlg );
  ksdlg->setCaption (caption);
  connect (ksdlg, SIGNAL (command (int)), this,
		SLOT (slotStopCancel (int)) );
  connect (this, SIGNAL ( progress (unsigned int) ),
	   ksdlg, SLOT ( slotProgress (unsigned int) ));
  KWin::setIcons (ksdlg->winId(), kapp->icon(), kapp->miniIcon());

  dialogsetup = TRUE;
}

bool KSpell::addPersonal (QString word)
{
  QString qs (word);

  //we'll let ispell do the work here b/c we can
  qs=qs.simplifyWhiteSpace();
  if (qs.find (' ')!=-1 || qs.isEmpty())    // make sure it's a _word_
    return FALSE;

  qs.prepend ("&");
  personaldict=TRUE;

  return proc->fputs(qs);
}

bool KSpell::writePersonalDictionary ()
{
  return proc->fputs ("#");
}

bool KSpell::ignore (QString word)
{
  QString qs (word);

  //we'll let ispell do the work here b/c we can
  qs.simplifyWhiteSpace();
  if (qs.find (' ')!=-1 || qs.isEmpty())    // make sure it's a _word_
    return FALSE;

  qs.prepend ("@");

  return proc->fputs(qs);
}

bool
KSpell::cleanFputsWord (QString s, bool appendCR)
{
  QString qs(s);
  bool firstchar = TRUE;
  bool empty = TRUE;

  for (unsigned int i=0; i<qs.length(); i++)
  {
    //we need some punctuation for ornaments
    if (qs[i] != '\'' && qs[i] != '\"' && !qs[i].isLetter() &&
    // permit hyphen when it's not at the beginning of the word 
	(firstchar || qs[i] != '-')) {
      qs.remove(i,1);
      i--;
    } else {
      firstchar = FALSE; 
      if (qs[i].isLetter()) empty=FALSE;
    }
  }

  // don't check empty words, otherwise synchronisation fails
  if (empty) return FALSE;

  return proc->fputs(qs, appendCR);
}

bool
KSpell::cleanFputs (QString s, bool appendCR)
{
  QString qs(s);
  unsigned l = qs.length();
  bool firstchar = TRUE;

  //  kdDebug(750) << "KS::cleanFputs (before)" << qs.length() << " [" << qs <<"]" << endl;

  // Why we need this stuff?
  if (l<MAXLINELENGTH)
    {
      for (unsigned int i=0; i<l; i++)
	{
	  if ( !qs[i].isLetter() && qs[i] != '\'' && qs[i] != '\"' 
	       && !qs[i].isSpace() &&
	    // let hyphens pass, but in the beginning, where ispell would
	    // interpret it as a control char
	    (firstchar || qs[i] != '-')) {
	      qs.replace (i,1," ");
	    } else firstchar = FALSE; 
	}

      if (qs.isEmpty())
	qs="";

      // kdDebug(750) << "KS::cleanFputs (after) " << qs.length() << " [" << qs << "]" << endl;

      return proc->fputs (qs, appendCR);
    }
  else
    return proc->fputs ("\n",appendCR);

}

bool KSpell::checkWord (QString buffer, bool _usedialog)
{
  QString qs (buffer);

  qs.simplifyWhiteSpace();
  if (qs.find (' ')!=-1 || qs.isEmpty())    // make sure it's a _word_
    return FALSE;

  ///set the dialog signal handler
  dialog3slot = SLOT (checkWord3());

  usedialog=_usedialog;
  setUpDialog(FALSE);
  if (_usedialog)
    {
      emitProgress();
      ksdlg->show();
    }
  else
    ksdlg->hide();

  OUTPUT (checkWord2);
  //  connect (this, SIGNAL (dialog3()), this, SLOT (checkWord3()));

  proc->fputs ("%"); // turn off terse mode
  proc->fputs (buffer); // send the word to ispell

  return TRUE;
}

void KSpell::checkWord2 (KProcIO *)
{
  QString word;

  QString line;

  proc->fgets (line, TRUE); //get ispell's response

  NOOUTPUT(checkWord2);

  int e;
  if ((e=parseOneResponse (line, word, &sugg))==MISTAKE &&
      usedialog)
    {
      cwword=word;
      dialog (word, &sugg, SLOT (checkWord3()));
      return;
    }
      //emits a "corrected" signal _even_ if no change was made
      //so that the calling program knows when the check is complete

  emit corrected (word, word, 0L);
}

void KSpell::checkWord3 ()
{
  disconnect (this, SIGNAL (dialog3()), this, SLOT (checkWord3()));

  emit corrected (cwword, replacement(), 0L);
}

QString KSpell::funnyWord (QString word)
{
  QString qs;
  unsigned int i=0;

  for (i=0; word [i]!='\0';i++)
    {
      if (word [i]=='+')
	continue;
      if (word [i]=='-')
	{
	  QString shorty;
	  unsigned int j;
	  int k;
	
	  for (j=i+1;word [j]!='\0' && word [j]!='+' &&
		 word [j]!='-';j++)
	    shorty+=word [j];
	  i=j-1;

	  if ((k=qs.findRev (shorty))==0
	//	 || k==(signed)(qs.length()-shorty.length())
		|| k!=-1)
	    qs.remove (k,shorty.length());
	  else
	    {
              qs+='-';
              qs+=shorty;  //it was a hyphen, not a '-' from ispell
            }
	}
      else
	qs+=word [i];
    }
  return qs;
}
	

int KSpell::parseOneResponse (const QString &buffer, QString &word, QStringList *sugg)
  // buffer is checked, word and sugg are filled in
  // returns
  //   GOOD    if word is fine
  //   IGNORE  if word is in ignorelist
  //   REPLACE if word is in replacelist
  //   MISTAKE if word is misspelled
{
  word = "";
  posinline=0;

  sugg->clear();

  if (buffer [0]=='*' || buffer[0] == '+' || buffer[0] == '-')
    {
      return GOOD;
    }

  if (buffer [0]=='&' || buffer [0]=='?' || buffer [0]=='#')
    {
      int i,j;


      QString qs (buffer);
      word = qs.mid (2,qs.find (' ',3)-2);
      //check() needs this
      orig=word;

      /////// Ignore-list stuff //////////
      //We don't take advantage of ispell's ignore function because
      //we can't interrupt ispell's output (when checking a large
      //buffer) to add a word to _it's_ ignore-list.
      QString qword (word);
      if (ignorelist.findIndex(qword.lower())!=-1)
	return IGNORE;

      //// Position in line ///
      QString qs2;

      qs=buffer;
      if (qs.find(':')!=-1)
	qs2=qs.left (qs.find (':'));
      else
	qs2=qs;

      posinline = qs2.right( qs2.length()-qs2.findRev(' ') ).toInt();

      ///// Replace-list stuff ////
      QStringList::Iterator it = replacelist.begin();
      for(;it != replacelist.end(); it++, it++) // Skip two entries at a time.
      {
         if (word == *it) // Word matches
         {
            it++;
            word = *it;   // Replace it with the next entry
            return REPLACE;
	 }
      }

      /////// Suggestions //////
      if (buffer [0]!='#')
	{
	  qs = buffer.mid(buffer.find(':')+2, buffer.length());
	  qs+=',';
	  sugg->clear();
	  i=j=0;
	  while ((unsigned int)i<qs.length())
	    {
	      QString temp = qs.mid (i,(j=qs.find (',',i))-i);
	      sugg->append (funnyWord (temp));
	
	      i=j+2;
	    }
	}

      if ((sugg->count()==1) && ((*sugg)[0] == word))
	return GOOD;

      return MISTAKE;
    }


  kdError(750) << "HERE?: [" << buffer << "]" << endl;
  kdError(750) << "Please report this to dsweet@kde.org" << endl;
  kdError(750) << "Thank you!" << endl;
  emit done((bool)FALSE);
  emit done (KSpell::origbuffer);
  return MISTAKE;
}

bool KSpell::checkList (QStringList *_wordlist)
  // prepare check or string list
{
  wordlist=_wordlist;
  if ((totalpos=wordlist->count())==0)
    return FALSE;
  wlIt = wordlist->begin();

  // prepare the dialog
  setUpDialog();

  //set the dialog signal handler
  dialog3slot = SLOT (checkList4 ());

  proc->fputs ("%"); // turn off terse mode & check one word at a time
  lastpos=0; //now counts which *word number* we are at in checkList3()

  //  connect (this, SIGNAL (eza()), this, SLOT (checkList2()));
  //emit eza();

  lastpos = -1;
  checkList2();

  // when checked, KProcIO calls checkList3a
  OUTPUT(checkList3a);

  return TRUE;
}

void KSpell::checkList2 ()
  // send one word from the list to KProcIO
  // invoked first time by checkList, later by checkList3 and checkList4
{
  // send next word
  if (wlIt != wordlist->end())
    {
      kdDebug(750) << "KS::cklist2 " << lastpos << ": " << *wlIt << endl;

      bool put;
      lastpos++; offset=0;
      put = cleanFputsWord (*wlIt);
      wlIt++;

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
      emit done(TRUE);
    }
}

void KSpell::checkList3a (KProcIO *)
  // invoked by KProcIO, when data from ispell are read
{
  int e, tempe;

  QString word;
  QString line;

    do
      {
	tempe=proc->fgets (line, TRUE); //get ispell's response
	if (tempe>0)
	  {
	    //kdDebug(750) << "lastpos advance on [" << temp << "]" << endl;
	    if ((e=parseOneResponse (line, word, &sugg))==MISTAKE ||
		e==REPLACE)
	      {
		dlgresult=-1;

		if (e==REPLACE)
		  {
		    QString old = *(--wlIt); wlIt++;
		    dlgreplacement=word;   
		    checkList3();
		    // inform application  
		    emit corrected (old, *(--wlIt), lastpos); wlIt++;
		  }
		else
		  {
		    cwword=word;
		    dlgon=TRUE;
		    // show the dialog
		    dialog (word, &sugg, SLOT (checkList4()));
		    return;
		  }
	      }

	  }
      	emitProgress (); //maybe
      } while (tempe>=0);

    if (!dlgon) //is this condition needed?
      // send next word
      checkList2();
}

// rename to "checkListReplaceCurrent", when binary compatibility isn't needed
void KSpell::checkList3 () {

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
  dlgon=FALSE;
  QString old;

  disconnect (this, SIGNAL (dialog3()), this, SLOT (checkList4()));

  //others should have been processed by dialog() already
  switch (dlgresult)
    {
    case KS_REPLACE:
    case KS_REPLACEALL:
      kdDebug(750) << "KS: cklist4: lastpos: " << lastpos << endl;
      old = *(--wlIt); wlIt++;
      // replace word
      checkList3();
      emit corrected (old, *(--wlIt), lastpos); wlIt++;
      break;
    case KS_CANCEL:
      ksdlg->hide();
      emit done ((bool)FALSE);
      return;
    case KS_STOP:
      ksdlg->hide();
      emit done (TRUE);
      break;
    };

  // read more if there is more
  checkList3a(NULL);
}

bool KSpell::check( const QString &_buffer )
{
  QString qs;

  setUpDialog ();
  //set the dialog signal handler
  dialog3slot = SLOT (check3 ());

  kdDebug(750) << "KS: check" << endl;
  origbuffer = _buffer;
  if ( ( totalpos = origbuffer.length() ) == 0 )
    {
      emit done(origbuffer);
      return FALSE;
    }


  // Torben: I corrected the \n\n problem directly in the
  //         origbuffer since I got errors otherwise
  if ( origbuffer.right(2) != "\n\n" )
    {
      if (origbuffer.at(origbuffer.length()-1)!='\n')
	{
	  origbuffer+='\n';
	  origbuffer+='\n'; //shouldn't these be removed at some point?
	}
      else
	origbuffer+='\n';
    }

  newbuffer=origbuffer;

  // KProcIO calls check2 when read from ispell
  OUTPUT(check2);
  proc->fputs ("!");

  //lastpos is a position in newbuffer (it has offset in it)
  offset=lastlastline=lastpos=lastline=0;

  emitProgress ();

  // send first buffer line
  int i = origbuffer.find('\n', 0)+1;
  qs=origbuffer.mid (0,i);
  cleanFputs (qs,FALSE);

  lastline=i; //the character position, not a line number

  emitProgress();
  ksdlg->show();

  return TRUE;
}

void KSpell::check2 (KProcIO *)
  // invoked by KProcIO when read from ispell
{
  int e, tempe;
  QString word;
  QString line;

  do
    {
      tempe=proc->fgets (line); //get ispell's response
      kdDebug(750) << "KSpell::check2 (" << tempe << "b)" << endl;

      if (tempe>0)
	{
	  if ((e=parseOneResponse (line, word, &sugg))==MISTAKE ||
	      e==REPLACE)
	    {
	      dlgresult=-1;

	      // for multibyte encoding posinline needs correction
	      if (ksconfig->encoding() == KS_E_UTF8) {
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
	      
	      lastpos=posinline+lastlastline+offset;
	
	      //orig is set by parseOneResponse()
	
	      if (e==REPLACE)
		{
		  dlgreplacement=word;
		  emit corrected (orig, replacement(), lastpos);
		  offset+=replacement().length()-orig.length();
		  newbuffer.replace (lastpos, orig.length(), word);
		}
	      else  //MISTAKE
		{
		  cwword=word;
		  //kdDebug(750) << "(Before dialog) word=[" << word << "] cwword =[" << cwword << "]\n" << endl;

		  // show the word in the dialog
		  dialog (word, &sugg, SLOT (check3()));
		  return;
		}
	    }
	
	  }

      emitProgress (); //maybe

    } while (tempe>0);

  proc->ackRead();


  if (tempe==-1) //we were called, but no data seems to be ready...
    return;

  //If there is more to check, then send another line to ISpell.
  if ((unsigned int)lastline<origbuffer.length())
    {
      int i;
      QString qs;

      //kdDebug(750) << "[EOL](" << tempe << ")[" << temp << "]" << endl;

      lastpos=(lastlastline=lastline)+offset; //do we really want this?
      i=origbuffer.find('\n', lastline)+1;
      qs=origbuffer.mid (lastline, i-lastline);
      cleanFputs (qs,FALSE);
      lastline=i;
      return;
    }
  else
  //This is the end of it all
    {
      ksdlg->hide();
      //      kdDebug(750) << "check2() done" << endl;
      newbuffer.truncate (newbuffer.length()-2);
      emitProgress();
      emit done (newbuffer);
    }
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
    //      kdDebug(750) << "cancelled\n" << endl;
      ksdlg->hide();
      emit done (origbuffer);
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


void KSpell::dialog(QString word, QStringList *sugg, const char *_slot)
{
  dlgorigword=word;

  dialog3slot=_slot;
  dialogwillprocess=TRUE;
  connect (ksdlg, SIGNAL (command (int)), this, SLOT (dialog2(int)));
  ksdlg->init (word, sugg);
  emit misspelling (word, sugg, lastpos);

  emitProgress();
  ksdlg->show();
}

void KSpell::dialog2 (int result)
{
  QString qs;

  disconnect (ksdlg, SIGNAL (command (int)), this, SLOT (dialog2(int)));
  dialogwillprocess=FALSE;
  dlgresult=result;
  ksdlg->standby();

  dlgreplacement=ksdlg->replacement();

  //process result here
  switch (dlgresult)
    {

    case KS_IGNOREALL:
      ignorelist.prepend(dlgorigword.lower());
      break;
    case KS_ADD:
      addPersonal (dlgorigword);
      personaldict=TRUE;
      ignorelist.prepend(dlgorigword.lower());
      break;
    case KS_REPLACEALL:
      replacelist.append (dlgorigword);
      replacelist.append (replacement());
      /*
    case KS_REPLACE:
    emit corrected (dlgorigword, replacement(), lastpos);
*/
      break;
    }

  
  // emit corrected (dlgorigword, replacement(), lastpos);
  connect (this, SIGNAL (dialog3()), this, dialog3slot.ascii());
  emit dialog3();
}


KSpell:: ~KSpell ()
{

  if (proc)
    {
      delete proc;
    }
  if (ksconfig)
    delete ksconfig;

  if (ksdlg)
    delete  ksdlg;
}


KSpellConfig KSpell::ksConfig () const
{
  ksconfig->setIgnoreList(ignorelist);
  return *ksconfig;
}

void KSpell::cleanUp ()
{
  if (m_status == Cleaning) return; // Ignore
  if (m_status == Running)
  {
    if (personaldict)
       writePersonalDictionary();
    m_status = Cleaning;
  }
  proc->closeStdin();
}

void KSpell::ispellExit (KProcess *)
{
  kdDebug() << "KSpell::ispellExit() " << m_status << endl;

  if ((m_status == Starting) && (trystart<maxtrystart))
  {
    trystart++;
    startIspell();
    return;
  }

  if (m_status == Starting)
     m_status = Error;
  else if (m_status == Cleaning)
     m_status = Finished;
  else if (m_status == Running)
     m_status = Crashed;
  else // Error, Finished, Crashed
     return; // Dead already
 
  kdDebug(750) << "Death" << endl;
  QTimer::singleShot( 0, this, SLOT(emitDeath()));
}

// This is always called from the event loop to make
// sure that the receiver can safely delete the
// KSpell object.
void KSpell::emitDeath()
{
  emit death();
  if (autoDelete)
     delete this;
}

void KSpell::setProgressResolution (unsigned int res)
{
  progres=res;
}

void KSpell::emitProgress ()
{
  uint nextprog = (uint) (100.*lastpos/(double)totalpos);

  if (nextprog>=curprog)
    {
      curprog=nextprog;
      emit progress (curprog);
    }
}

void KSpell::moveDlg (int x, int y)
{
  QPoint pt (x,y), pt2;
  pt2=parent->mapToGlobal (pt);
  ksdlg->move (pt2.x(),pt2.y());
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
    modalreturn = 0;
    modaltext = text;

    /*modalWidgetHack = new QWidget(0,0,WType_Modal);
    modalWidgetHack->setGeometry(-10,-10,2,2);
    */

    // kdDebug() << "KSpell1" << endl;
    KSpell* spell = new KSpell( 0L, i18n("Spell Checker"), 0 ,
				0, 0, true, true );
    //modalWidgetHack->show();
    //qApp->enter_loop();

    while (spell->status()!=Finished)
      kapp->processEvents();

    text = modaltext;

    //delete modalWidgetHack;
    //modalWidgetHack = 0;

    delete spell;
    return modalreturn;
}

void KSpell::slotModalReady()
{
  //kdDebug() << qApp->loopLevel() << endl;
  //kdDebug(750) << "MODAL READY" << endl;
  
  ASSERT( m_status == Running );
  connect( this, SIGNAL( done( const QString & ) ), 
	   this, SLOT( slotModalDone( const QString & ) ) );
  check( modaltext );

}

void KSpell::slotModalDone( const QString &_buffer )
{
  //kdDebug(750) << "MODAL DONE " << _buffer << endl;
    modaltext = _buffer;
    cleanUp();

    //kdDebug() << "ABOUT TO EXIT LOOP" << endl;
    //qApp->exit_loop();

    //modalWidgetHack->close(true);
}

QString KSpell::modaltext;
int KSpell::modalreturn = 0;
QWidget* KSpell::modalWidgetHack = 0;


#include "kspell.moc"

