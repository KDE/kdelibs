#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

#include <kapp.h>
#include <kwm.h>

#include "kspell.moc"
 
//#define KSDEBUG

#ifndef __GNUC__
#define inline
#endif

#ifdef KSDEBUG
#define dsdebug printf
#else
inline void dsdebug (const char *, ...)  {}
#endif


#define TEMPsz  3072

#define GOOD      0
#define IGNORE    1
#define REPLACE   2
#define MISTAKE   3

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
		


KSpell::KSpell (QWidget *_parent, const char *_caption,
		QObject *obj, char *slot, KSpellConfig *_ksc)
{
  proc=0;

  //won't be using the dialog in ksconfig, just the option values
  if (_ksc!=0)
    ksconfig = new KSpellConfig (*_ksc);
  else
    ksconfig = new KSpellConfig;

#ifdef KSDEBUG      
  dsdebug ("now here\n");
#endif

  ok=texmode=dlgon=FALSE;
  progres=10;
  curprog=0;


  if ((temp = new char [TEMPsz])==0)
    return;

  personaldict=FALSE;
  dlgresult=-1;

  caption = _caption;

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

  dsdebug ("Try #%d\n",trystart);

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
#ifdef KSDEBUG
	  dsdebug ("using dictionary [%s]\n",ksconfig->dictionary());
#endif
	  *proc << "-d";
	  *proc << ksconfig->dictionary();
	}
    }

  if (trystart<1)
    if (ksconfig->encoding()==KS_E_LATIN1)
      {
	dsdebug ("Using Latin1\n");
	*proc << "-Tlatin1";
      }


  /*
  if (ksconfig->personalDict()[0]!='\0')
    {
#ifdef KSDEBUG
      dsdebug ("personal dictionary [%s]\n",ksconfig->personalDict());
#endif
      *proc << "-p";
      *proc << ksconfig->personalDict();
    }
    */


  // -a : pipe mode
  // -S : sort suggestions by probable correctness
  connect(proc, SIGNAL(processExited(KProcess *)),
	  this, SLOT (ispellExit (KProcess *)));

  OUTPUT(KSpell2);

  if (proc->start ()==FALSE)
    {
      emit ready(this);
    }

}

void KSpell::KSpell2 (KProcIO *)

{
  dsdebug ("KSpell2\n");

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
      dsdebug ("@KDE was FALSE\n");
      emit ready(this);
      return;
    }


  NOOUTPUT (KSpell2);

  ksdlg=new KSpellDlg (parent, "dialog", ispellID.data());
  ksdlg->setCaption (caption.data());
  KWM kwm;
  kwm.setMiniIcon (ksdlg->winId(), kapp->getMiniIcon());

  ok=TRUE;

  emit ready(this);
}

bool KSpell::addPersonal (char *word)
{
  QString qs (word);

  //we'll let ispell do the work here b/c we can
  qs.simplifyWhiteSpace();  
  if (qs.find (' ')!=-1 || qs.isEmpty())    // make sure it's a _word_
    return FALSE;

  qs.prepend ("*");
  personaldict=TRUE;
  //  ignorelist.inSort (origword);

  return proc->fputs (qs.data());
}

bool KSpell::ignore (char *word)
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
KSpell::isBadIspellChar (char c)
{
  char badchars [10] = {'*','&','@','+','-','~','#','!','%','^'};
  int i;

  for (i=0;i<10;i++)
    if (c==badchars [i])
      return TRUE;

  return FALSE;
}

bool KSpell::cleanFputsWord (const char *s, bool appendCR)
{
  QString qs(s);

  for (unsigned i=0;i<qs.length();i++)
  {
    //we need some puctuation for ornaments
    if (qs.at(i)!='\'' && qs.at(i)!='\"')
      if (ispunct (qs.at(i)) || isspace (qs.at(i)))
	  qs.remove(i,1);
  }
  
  return proc->fputs (qs.data(), appendCR);
}

bool
KSpell::cleanFputs (const char *s, bool appendCR)
{
  QString qs(s);
  unsigned l=qs.length();
  
  if (l>1)
    for (unsigned i=1;i<l;i++)
      {
	if (qs.at(i-1)=='\n' && 
	    ispunct (qs.at(i)) &&
	    qs.at(i)!='\'' && qs.at(i)!='\"')
	  qs.replace (i,1," ");
      }
  
  return proc->fputs (qs.data(), appendCR);
}

bool KSpell::checkWord (char *buffer, bool _usedialog)
{
  QString qs (buffer);

  qs.simplifyWhiteSpace();  
  if (qs.find (' ')!=-1 || qs.isEmpty())    // make sure it's a _word_
    return FALSE;

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
  char word [TEMPsz];


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

  emit corrected (cwword.data(), replacement(), 0L);
}

char * KSpell::funnyWord (char *word)
{
  QString qs;
  unsigned i;

  for (i=0; word [i]!='\0';i++)
    {
      if (word [i]=='+')
	continue;
      if (word [i]=='-')
	{
	  QString shorty;
	  unsigned j;

	  for (j=i+1;word [j]!='\0' && word [j]!='+' &&
		 word [j]!='-';j++)
	    shorty+=word [j];
	  i=j-1;

	  if ((j=qs.find (shorty.data()))==0 ||
	      j==qs.length()-shorty.length())
	    qs.remove (j,shorty.length());
	}
      else
	qs+=word [i];
    }
  strcpy (word, qs.data());

  return word;
}
	
  
int KSpell::parseOneResponse (char *buffer, char *word, QStrList *sugg)
  // buffer is checked, word and sugg are filled in
  // returns
  //   GOOD    if word is fine
  //   IGNORE  if word is in ignorelist
  //   REPLACE if word is in replacelist
  //   MISTAKE if word is misspelled
{
  char temp [TEMPsz];
  int e;

  word [0]='\0';
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
      strcpy (word,qs.mid (2,qs.find (' ',3)-2));
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
      while ((e=replacelist.findNext (word))!=-1 && e%2!=0)
	replacelist.next();

      if (e!=-1)
	{
	  e++;
	  if (replacelist.count()>(unsigned)e)
	    strcpy (word,replacelist.at(e));
	  return REPLACE;
	}


      /////// Suggestions //////
      if (buffer [0]!='#')
	{
	  qs=(index (buffer, ':')+2);
	  qs+=',';
	  sugg->clear();
	  i=j=0;
	  while ((unsigned)i<qs.length())
	    {
	      strcpy (temp,qs.mid (i,(j=qs.find (',',i))-i).data());
	      sugg->append (funnyWord (temp));
	      
	      i=j+2;
	    }
	}
      return MISTAKE;
    }
      
      
  dsdebug ("HERE?: [%s]\nPlease report this to dsweet@physics.umd.edu\nThank you!",buffer);
  emit done(FALSE);
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
      if ((unsigned)lastpos>=wordlist->count())
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


  char word [TEMPsz];

    do
      {
	tempe=proc->fgets (temp, TEMPsz, TRUE); //get ispell's response      
	if (tempe>0)
	  {
	    lastpos++;
	    dsdebug ("lastpos advance on [%s]\n",
		     temp);
	    if ((e=parseOneResponse (temp, word, &sugg))==MISTAKE ||
		e==REPLACE)
	      {
		dlgresult=-1;

		//orig is set by parseOneResponse()
		//		lastpos=newbuffer.find (orig.data(),lastpos,TRUE);

		if (e==REPLACE)
		  {
		    emit corrected (orig.data(), replacement(), lastpos);
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
      dsdebug ("cklist4: lastpos=(%d)\n",lastpos);
      wordlist->remove (lastpos-1);
      wordlist->insert (lastpos-1, replacement());
      wordlist->next();
      break;
    case KS_CANCEL:
      ksdlg->hide();
      emit done (FALSE);
      return;
    case KS_STOP:
      ksdlg->hide();
      emit done (TRUE);
      break;
    };

  emit eza();
}

/*
void KSpell::checkList3 (KProcIO *)
{
  int e, tempe;
  QStrList sugg;
  char word [TEMPsz];

  do
    {
      tempe=proc->fgets (temp, TEMPsz, TRUE); //get ispell's response      
      if (tempe>0)
	{
	  if ((e=parseOneResponse (temp, word, &sugg))==MISTAKE ||
	      e==REPLACE)
	    {
	      dlgresult=-1;
	      
	      //progress here
	      
	      if (e==REPLACE)
		{
		  emit corrected (orig.data(), replacement(), lastpos);
		  //don't (can't) change the wordlist
		}
	      else 
		dialog (word, &sugg);

	      //dlgresult is class-global and set in dialog()
	      if (dlgresult==KS_CANCEL)
		{
		  ksdlg->hide();
		  emit done (FALSE);
		  return;
		}
	      
	      if (dlgresult==KS_STOP)
		{
		  lastpos++;
		  break;
		}
	    }
	}
      lastpos++;
      emitProgress (); //maybe
    } while (tempe>=0); 

  checkList2();
}
*/
  
bool KSpell::check (char *_buffer)
{
  QString qs;

#ifdef KSDEBUG
  dsdebug ("KS: check\n");
#endif
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
#ifdef KSDEBUG
  dsdebug ("! says %d\n", proc->fputs ("!"));
#else
  proc->fputs ("!");
#endif

  offset=lastlastline=lastpos=lastline=0;

  i=buffer.find('\n', lastline)+1;
  qs=buffer.mid (lastpos, i-lastline);
#ifdef KSDEBUG
  dsdebug ("KS: [%s], %d\n",qs.data(),  cleanFputs (qs,FALSE));
#else
  cleanFputs (qs,FALSE);
#endif
  
  lastline=i;


  return TRUE;
}

void KSpell::check2 (KProcIO *)
{
#ifdef KSDEBUG
  dsdebug ("KS: check2\n");
#endif
  //This is here for historical reasons and I'm not quite ready to
  //remove it.
  check2a();
}

void KSpell::check2a (void)
{
  int e, tempe;
  char word [TEMPsz];

  do
    {
      tempe=proc->fgets (temp, TEMPsz); //get ispell's response      
#ifdef KSDEBUG
	  dsdebug ("2:(%d)\n",tempe);
#endif
      
      if (tempe>0)
	{
#ifdef KSDEBUG
	  dsdebug ("2:[%s]\n",temp);
#endif
	  
	  
	  if ((e=parseOneResponse (temp, word, &sugg))==MISTAKE ||
	      e==REPLACE)
	    {
	      dlgresult=-1;
	      lastpos=posinline+lastlastline+offset;
	      
	      //orig is set by parseOneResponse()
	      
	      if (e==REPLACE)
		{
		  dlgreplacement=word;
		  emit corrected (orig.data(), replacement(), lastpos);
		  offset+=strlen(replacement())-orig.length();
		  newbuffer.replace (lastpos,orig.length(),word);
		}
	      else 
		{
		  cwword=word;
		  //		  NOOUTPUT (check2);
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
  //If not, then hide the dialog box and exit.
  if ((unsigned)lastline<buffer.length())
    {
      int i;
      QString qs;
      
#ifdef KSDEBUG      
      dsdebug ("[EOL](%d)[%s]\n",tempe,temp);
#endif
      //      getchar();
      
      lastpos=(lastlastline=lastline)+offset;
      i=buffer.find('\n', lastline)+1;
      qs=buffer.mid (lastline, i-lastline);
      cleanFputs ((const char*)qs,FALSE);
      lastline=i;
      return;  
    }
  else
    {
      //      dsdebug ("done!\n");
      ksdlg->hide();
      buffer=newbuffer.data();
      emit done (buffer.data());
    }
}

void KSpell::check3 ()
{
  disconnect (this, SIGNAL (dialog3()), this, SLOT (check3()));

#ifdef KSDEBUG
  dsdebug ("check3 %s %s %d\n",cwword.data(),replacement(),
	  dlgresult);
#endif

  //others should have been processed by dialog() already
  switch (dlgresult)
    {
    case KS_REPLACE:
    case KS_REPLACEALL:
      offset+=strlen(replacement())-orig.length();
      newbuffer.replace (lastpos,cwword.length(),replacement());
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


void KSpell::dialog (char *word, QStrList *sugg, char *_slot)
{
  dlgorigword=word;

  dialog3slot=_slot;
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
      qs="*";
      qs+=dlgorigword;
      proc->fputs (qs.lower());
      personaldict=TRUE;
      ignorelist.inSort (dlgorigword.lower());
      break;
    case KS_REPLACEALL:
      replacelist.append (dlgorigword);
      replacelist.append (replacement());
      break;
    }

  emit corrected (dlgorigword.data(), replacement (), lastpos);
  connect (this, SIGNAL (dialog3()), this, dialog3slot.data());
  emit dialog3();
}


KSpell:: ~KSpell ()
{

  if (proc!=0 && personaldict)
    {
      proc->fputs ("#");
    }

  
  if (proc!=0)
    delete proc;
    
}

KSpellConfig KSpell::ksConfig (void) const
{
  ksconfig->setIgnoreList (ignorelist);
  return *ksconfig;
}

void KSpell::ispellExit (KProcess *)
{
  dsdebug ("ISpell died\n");
  if (trystart<maxtrystart)
    {
      trystart++;
      startIspell();
      return;
    }

  if (!ok)
    {
      dsdebug ("NOT OK\n");
      emit ready(this);
    }
  dsdebug ("Death\n");
  emit death(this);
}

void KSpell::setProgressResolution (int res)
{
  progres=res;
}

void KSpell::emitProgress (void)
{
  if ((int)lastpos/totalpos>(int)curprog)
    {
      curprog+=progres;
      emit (progress (curprog-progres));
    }
}

void KSpell::moveDlg (int x, int y)
{
  QPoint pt (x,y), pt2;
  pt2=parent->mapToGlobal (pt);
  ksdlg->move (pt2.x(),pt2.y());
}

