// $Id$

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

#include <kapp.h>
#include <kwm.h>

#include "kspell.h"
 

#define MAXLINELENGTH 150
#define TEMPsz  3072

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
		QObject *obj, const char *slot, KSpellConfig *_ksc)
{
  proc=0;
  ksconfig=0;
  temp=0;
  ksdlg=0;


  //won't be using the dialog in ksconfig, just the option values
  if (_ksc!=0)
    ksconfig = new KSpellConfig (*_ksc);
  else
    ksconfig = new KSpellConfig;

  kdebug(KDEBUG_INFO, 750, "now here %s/%d", __FILE__, __LINE__);

  ok=texmode=dlgon=FALSE;
  cleaning=FALSE;
  progres=10;
  curprog=0;


  dialogwillprocess=FALSE;
  dialog3slot="";


  if ((temp = new char [TEMPsz])==0)
    return;

  personaldict=FALSE;
  dlgresult=-1;

  caption=_caption;

  parent=_parent;

  trystart=0;
  maxtrystart=2;

  // caller wants to know when kspell is ready
  connect (this, SIGNAL (ready(KSpell *)), obj, slot);

  proc=new KProcIO;

  startIspell();
}


void
KSpell::startIspell(void)
  //trystart = {0,1,2}
{

  kdebug(KDEBUG_INFO, 750, "Try #%d",trystart);

  if (trystart>0)
    proc->resetAll();

  proc->setExecutable("ispell");

  //  *proc << "ispell";

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
      if (ksconfig->dictionary()[0]!='\0')
	{
	  kdebug(KDEBUG_INFO, 750,"using dictionary [%s]",ksconfig->dictionary());
	  *proc << "-d";
	  *proc << ksconfig->dictionary();
	}
    }

  if (trystart<1)
    if (ksconfig->encoding()==KS_E_LATIN1)
      {
	kdebug(KDEBUG_INFO, 750, "Using Latin1\n");
	*proc << "-Tlatin1";
      }


  /*
  if (ksconfig->personalDict()[0]!='\0')
    {
      kdebug(KDEBUG_INFO, 750, "personal dictionary [%s]",ksconfig->personalDict());
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
      emit ready(this);
    }

}

void
KSpell::ispellErrors (KProcess *, char *buffer, int buflen)
{
  buffer [buflen-1] = '\0';
  kdebug (KDEBUG_INFO, 750, "ispellErrors [%s]\n",
	  buffer);
}

void KSpell::KSpell2 (KProcIO *)

{
  kdebug(KDEBUG_INFO, 750, "KSpell::KSpell2");
  trystart=maxtrystart;  //We've officially started ispell and don't want
       //to try again if it dies.
  if (proc->fgets (temp, TEMPsz, TRUE)==-1)
    {
      emit ready(this);
      return;
    }


  if (temp [0]!='@') //@ indicates that ispell is working fine
    {
      emit ready(this);
      return;
    }
    
  ispellID = &temp[5];

  //We want to recognize KDE in any text!
  if (ignore ("kde")==FALSE)
    {
      kdebug(KDEBUG_INFO, 750, "@KDE was FALSE");
      emit ready(this);
      return;
    }
  //We want to recognize linux in any text!
  if (ignore ("linux")==FALSE)
    {
      kdebug(KDEBUG_INFO, 750, "@KDE was FALSE");
      emit ready(this);
      return;
    }


  NOOUTPUT (KSpell2);

  ksdlg=new KSpellDlg (parent, "dialog", ispellID);
  ksdlg->setCaption (caption.data());
  connect (ksdlg, SIGNAL (command (int)), this, 
		SLOT (slotStopCancel (int)) );

  KWM kwm;
  kwm.setMiniIcon (ksdlg->winId(), kapp->getMiniIcon());

  ok=TRUE;

  emit ready(this);
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
  
  return proc->fputs (qs.data());
}

bool KSpell::writePersonalDictionary (void)
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

  return proc->fputs (qs.data());
}

bool
KSpell::cleanFputsWord (QString s, bool appendCR)
{
  QString qs(s);

  for (unsigned int i=0;i<qs.length();i++)
  {
    //we need some puctuation for ornaments
    if (qs.at(i)!='\'' && qs.at(i)!='\"')
      if (ispunct (qs.at(i)) || isspace (qs.at(i)))
	  qs.remove(i,1);
  }
  
  return proc->fputs (qs, appendCR);
}

bool
KSpell::cleanFputs (QString s, bool appendCR)
{
  QString qs(s);
  unsigned int j=0,l=qs.length();
  
  if (l<MAXLINELENGTH)
    {
      for (unsigned int i=0;i<l;i++,j++)
	{
	  if (//qs.at(i-1)=='\n' && 
	      ispunct (qs.at(i)) &&
	      qs.at(i)!='\'' && qs.at(i)!='\"')
	    qs.replace (i,1," ");
	  
	}
      
      if (qs.isEmpty())
	qs="";
      
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
  if (_usedialog)
    ksdlg->show();
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


  proc->fgets (temp, TEMPsz, TRUE); //get ispell's response
  
  NOOUTPUT(checkWord2);

  int e;
  if ((e=parseOneResponse (temp, word, &sugg))==MISTAKE &&
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

void KSpell::checkWord3 (void)
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

	  if ((k=qs.findRev (shorty.data()))==0
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
	
  
int KSpell::parseOneResponse (const QString &buffer, QString &word, QStrList *sugg)
  // buffer is checked, word and sugg are filled in
  // returns
  //   GOOD    if word is fine
  //   IGNORE  if word is in ignorelist
  //   REPLACE if word is in replacelist
  //   MISTAKE if word is misspelled
{
  QString temp;
  int e;

  word = "";
  posinline=0;

  sugg->clear();

  if (buffer [0]=='*')
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
      if (ignorelist.find (qword.lower())!=-1)
	return IGNORE;

      //// Position in line ///
      QString qs2;

      qs=buffer;
      if (qs.find(':')!=-1)
	qs2=qs.left (qs.find (':'));
      else
	qs2=qs;

      posinline = atoi (qs2.right (qs2.length()-
				   qs2.findRev (' ')).data());

      ///// Replace-list stuff ////
      replacelist.first();
      while ((e=replacelist.findNext ((const char *)word))!=-1 && e%2!=0)
	replacelist.next();

      if (e!=-1)
	{
	  e++;
	  if (replacelist.count()>(unsigned int)e)
	    word = replacelist.at(e);
	  return REPLACE;
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
	      temp = qs.mid (i,(j=qs.find (',',i))-i).data();
	      sugg->append (funnyWord (temp));
	      
	      i=j+2;
	    }
	}
      return MISTAKE;
    }
      
      
  kdebug(KDEBUG_ERROR, 750, "HERE?: [%s]", buffer.data());
  kdebug(KDEBUG_ERROR, 750, "Please report this to dsweet@physics.umd.edu");
  kdebug(KDEBUG_ERROR, 750, "Thank you!");
  emit done((bool)FALSE);
  emit done (KSpell::buffer.data());
  return MISTAKE;
}


bool KSpell::checkList (QStrList *_wordlist)
{
  wordlist=_wordlist;
  if ((totalpos=wordlist->count())==0)
    return FALSE;
  wordlist->first();
  //  ksdlg->show(); //only show if we need it

  //set the dialog signal handler
  dialog3slot = SLOT (checkList4 ());

  proc->fputs ("%"); // turn off terse mode & check one word at a time
  lastpos=0; //now counts which *word number* we are at in checkList3()
  connect (this, SIGNAL (eza()), this, SLOT (checkList2()));
  emit eza();
  OUTPUT(checkList3a);

  return TRUE;
}

void KSpell::checkList2 (void)
  //output some words from the list
{
  //  disconnect (this, SIGNAL (eza()), this, SLOT (checkList2()));
  if (wordlist->current()==0)
    {
      if ((unsigned int)lastpos>=wordlist->count())
	{
	  NOOUTPUT(checkList3a);
	  ksdlg->hide();
	  emit done(TRUE);
	}
    }
  else
    {
      if (wordlist->current()!=0)
	{
	  cleanFputsWord (wordlist->current());
	  wordlist->next();
	}
    }
}

void KSpell::checkList3a (KProcIO *)
{
  connect (this, SIGNAL (ez()), this, SLOT (checkList3()));
  emit ez();
}

void KSpell::checkList3 ()
{
  int e, tempe;
  
  disconnect (this, SIGNAL (ez()), this, SLOT (checkList3()));


  QString word;

    do
      {
	tempe=proc->fgets (temp, TEMPsz, TRUE); //get ispell's response      
	if (tempe>0)
	  {
	    lastpos++;
	    kdebug(KDEBUG_INFO, 750, "lastpos advance on [%s]", temp);
	    if ((e=parseOneResponse (temp, word, &sugg))==MISTAKE ||
		e==REPLACE)
	      {
		dlgresult=-1;

		//orig is set by parseOneResponse()
		//		lastpos=newbuffer.find (orig.data(),lastpos,TRUE);

		if (e==REPLACE)
		  {
		    emit corrected (orig, replacement(), lastpos);
		    //  newbuffer.replace (lastpos,orig.length(),word);
		  }
		else 
		  {
		    cwword=word;
		    dlgon=TRUE;
		    dialog (word, &sugg, SLOT (checkList4()));
		    return;
		  }
	      }

	  }
      	emitProgress (); //maybe
      } while (tempe>=0);
    
    if (!dlgon) //is this condition needed?
      emit eza();
}

void KSpell::checkList4 ()
{
  dlgon=FALSE;
    
  disconnect (this, SIGNAL (dialog3()), this, SLOT (checkList4()));

  //others should have been processed by dialog() already
  switch (dlgresult)
    {
    case KS_REPLACE:
    case KS_REPLACEALL:
      kdebug(KDEBUG_INFO, 750, "cklist4: lastpos==(%d)", lastpos);
      wordlist->remove (lastpos-1);
      wordlist->insert (lastpos-1, (const char *)replacement());
      wordlist->next();
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

  emit eza();
}

bool KSpell::check (QString _buffer)
{
  QString qs;

  //set the dialog signal handler
  dialog3slot = SLOT (check3 ());

  kdebug(KDEBUG_INFO, 750, "KS: check");
  buffer=_buffer;
  if ((totalpos=buffer.length())==0)
    {
      emit done(buffer.data());
      return FALSE;
    }

  if (buffer.at(buffer.length()-1)!='\n')
    {
      buffer+='\n';
      buffer+='\n';
    }
  int i;

  newbuffer=buffer.data();

  OUTPUT(check2);
  proc->fputs ("!");

  offset=lastlastline=lastpos=lastline=0;

  emitProgress ();

  i=buffer.find('\n', lastline)+1;
  qs=buffer.mid (lastpos, i-lastline);
  cleanFputs (qs,FALSE);

  lastline=i;

  ksdlg->show();

  return TRUE;
}

void KSpell::check2 (KProcIO *)
{
  int e, tempe;
  QString word;

  do
    {
      tempe=proc->fgets (temp, TEMPsz); //get ispell's response      
	  kdebug(KDEBUG_INFO, 750, "2:(%d)", tempe);
      
      if (tempe>0)
	{
	  kdebug(KDEBUG_INFO, 750, "2:[%s]", temp);
	  
	  if ((e=parseOneResponse (temp, word, &sugg))==MISTAKE ||
	      e==REPLACE)
	    {
	      dlgresult=-1;
	      lastpos=posinline+lastlastline+offset;
	      
	      //orig is set by parseOneResponse()
	      
	      if (e==REPLACE)
		{
		  dlgreplacement=word;
		  emit corrected (orig, replacement(), lastpos);
		  offset+=replacement().length()-orig.length();
		  newbuffer.replace (lastpos,orig.length(),word);
		}
	      else  //MISTAKE
		{
		  cwword=word;
		  kdebug(KDEBUG_INFO, 750, "(Before dialog) word=[%s] cwword =[%s]\n",
			 (const char *)word, (const char *)cwword);

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
  if ((unsigned int)lastline<buffer.length())
    {
      int i;
      QString qs;
      
      kdebug(KDEBUG_INFO, 750, "[EOL](%d)[%s]", tempe, temp);
      
      lastpos=(lastlastline=lastline)+offset;
      i=buffer.find('\n', lastline)+1;
      qs=buffer.mid (lastline, i-lastline);
      cleanFputs ((const char*)qs,FALSE);
      lastline=i;
      return;  
    }
  else
  //This is the end of it all
    //  if (lastline==-1)
    {
      ksdlg->hide();
      buffer=newbuffer.data();
      kdebug (KDEBUG_WARN, 750, "check2() done");
      emit done (buffer.data());
    }

  /*    {
      proc->fputs("");
      lastline=-1;
    }
    */

}

void KSpell::check3 ()
{
  disconnect (this, SIGNAL (dialog3()), this, SLOT (check3()));

  kdebug(KDEBUG_INFO, 750, "check3 [%s] [%s] %d", (const char *)cwword, 
	 (const char *)replacement(), dlgresult);

  //others should have been processed by dialog() already
  switch (dlgresult)
    {
    case KS_REPLACE:
    case KS_REPLACEALL:
      offset+=replacement().length()-orig.length();
      newbuffer.replace (lastpos, cwword.length(), replacement());
      break;
    case KS_CANCEL:
      ksdlg->hide();
      emit done (buffer.data());
      return;
    case KS_STOP:
      ksdlg->hide();
      buffer=newbuffer.data();
      emit done (buffer.data());
      return;
    };

  proc->ackRead();
  //  connect (this, SIGNAL (ez()), this, SLOT (check2a()));
  //  emit ez();
}

void
KSpell::slotStopCancel (int result)
{
  if (dialogwillprocess)
    return;

  kdebug(KDEBUG_INFO, 750, "KSpell::slotStopCancel [%d]", result);

  if (result==KS_STOP || result==KS_CANCEL)
    if (!dialog3slot.isEmpty())
      {
	dlgresult=result;
	connect (this, SIGNAL (dialog3()), this, dialog3slot.data());
	emit dialog3();
      }
}


void KSpell::dialog (QString word, QStrList *sugg, const char *_slot)
{
  dlgorigword=word;

  dialog3slot=_slot;
  dialogwillprocess=TRUE;
  connect (ksdlg, SIGNAL (command (int)), this, SLOT (dialog2(int)));
  ksdlg->init (word, sugg);
  emit misspelling (word, sugg, lastpos);
  ksdlg->show();
}

void KSpell::dialog2 (int result)
{
  QString qs;

  //  dsdebug ("received command %d\n",result);

  disconnect (ksdlg, SIGNAL (command (int)), this, SLOT (dialog2(int)));
  dialogwillprocess=FALSE;
  dlgresult=result;
  ksdlg->standby();

  dlgreplacement=ksdlg->replacement();

  //process result here
  switch (dlgresult)
    {
    case KS_IGNOREALL:
      ignorelist.inSort (dlgorigword.lower());
      break;
    case KS_ADD:
      addPersonal ((const char*)dlgorigword);
      personaldict=TRUE;
      ignorelist.inSort (dlgorigword.lower());
      break;
    case KS_REPLACEALL:
      replacelist.append (dlgorigword);
      replacelist.append ((const char *)replacement());
      break;
    }

  emit corrected (dlgorigword, replacement(), lastpos);
  connect (this, SIGNAL (dialog3()), this, dialog3slot.data());
  emit dialog3();
}


KSpell:: ~KSpell ()
{
  kdebug(KDEBUG_INFO, 750, "KSpell:~KSpell");

  if (proc)  
    {
      delete proc;
    }
  if (ksconfig)
    delete ksconfig;

  if (temp)
    delete [] temp;

  if (ksdlg)
    delete  ksdlg;
}


KSpellConfig KSpell::ksConfig (void) const
{
  ksconfig->setIgnoreList (ignorelist);
  return *ksconfig;
}

void KSpell::cleanUp ()
{
  kdebug(KDEBUG_WARN, 750, "KSpell::cleanUp()");
  if (personaldict)
    writePersonalDictionary();
  cleaning=TRUE;
  proc->closeStdin();
}

void KSpell::ispellExit (KProcess *)
{
  kdebug(KDEBUG_WARN, 750, "KSpell::ispellExit()");

  if (cleaning)
    {
      emit cleanDone(); //you can delete me now
      return;
    }

  if (trystart<maxtrystart)
    {
      trystart++;
      startIspell();
      return;
    }

  if (!ok)
    {
      kdebug(KDEBUG_WARN, 750, "NOT OK");
      emit ready(this);
    }
  kdebug(KDEBUG_ERROR, 750, "Death");
  emit death(this);
}

void KSpell::setProgressResolution (unsigned int res)
{
  progres=res;
}

void KSpell::emitProgress (void)
{
  kdebug(KDEBUG_INFO, 750, "KSpell::emitProgress (%f) (%d)",
	   100.*lastpos/totalpos,curprog);
  if (100.*lastpos/totalpos>=curprog)
    {
      curprog+=progres;
      kdebug(KDEBUG_INFO, 750, "KSpell::emitProgress (yes)(%f) (%d)",
	       100.*lastpos/totalpos,curprog);
      emit progress (curprog-progres);
    }
}

void KSpell::moveDlg (int x, int y)
{
  QPoint pt (x,y), pt2;
  pt2=parent->mapToGlobal (pt);
  ksdlg->move (pt2.x(),pt2.y());
}
#include "kspell.moc"
