#include <qchkbox.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qfiledlg.h>

#include <klined.h>
#include <kapp.h>

#include "ksconfig.moc"

//#define KSDEBUG

#ifdef KSDEBUG
#define dsdebug printf
#else
inline void dsdebug (...)  {}
#endif

#define NLS(s) i18n(s)


KSpellConfig::KSpellConfig (const KSpellConfig &_ksc)
{  
  setNoRootAffix (_ksc.noRootAffix());
  setRunTogether (_ksc.runTogether());
  setDictionary  (_ksc.dictionary());
  setDictFromList (_ksc.dictFromList());
  //  setPersonalDict (_ksc.personalDict());
  setIgnoreList (_ksc.ignoreList());
  setEncoding (_ksc.encoding());

  nodialog=TRUE;
}

KSpellConfig::KSpellConfig (QWidget *parent, char *name,
			    KSpellConfig *_ksc=0) : QWidget (parent, name)
{
  kc=kapp->getConfig();

  if (_ksc==0)
    {
      //I'd like to read these from a kspellrc file so that we could have
      //_adjustable_ system-wide defaults.

      //Here it is.
      readGlobalSettings();
    }
  else
    {
      setNoRootAffix (_ksc->noRootAffix());
      setRunTogether (_ksc->runTogether());
      setDictionary  (_ksc->dictionary());
      setDictFromList (_ksc->dictFromList());
      //      setPersonalDict (_ksc->personalDict());
      setIgnoreList (_ksc->ignoreList());
      setEncoding (_ksc->encoding());
    }

  //  langnames=new QStrList ();
  langfnames=new QStrList ();
  nodialog=FALSE;

  //  if (parent!=0)
    {
      //From dlgedit

      layout = new QGridLayout (this, 9, 7, 10, 1);
      const rdictlist = 3,
	//	    rpersdict = 6,
	    rencoding = 5,
	    rhelp = 6;

      //rdictedit = 4,
	cb1 = new QCheckBox( this, "CheckBox_1" );
	cb1->setGeometry( 30, 40, 320, 30 );
	connect( cb1, SIGNAL(toggled(bool)), SLOT(sNoAff(bool)) );
	cb1->setText( NLS("Create Root/Affix combinations not in the dictionary") );
	cb1->setAutoRepeat( FALSE );
	cb1->setAutoResize( FALSE );
	layout->addMultiCellWidget (cb1, 0, 0, 0, 5);
	cb1->setMinimumSize (cb1->sizeHint());

	cb2 = new QCheckBox( this, "CheckBox_3" );
	cb2->setGeometry( 30, 80, 320, 30 );
	connect( cb2, SIGNAL(toggled(bool)), SLOT(sRunTogether(bool)) );
	cb2->setText( NLS("Consider run-together words as spelling errors") );
	cb2->setAutoRepeat( FALSE );
	cb2->setAutoResize( FALSE );
	layout->addMultiCellWidget (cb2, 1, 1, 0, 5);
	cb2->setMinimumSize (cb2->sizeHint());

	/* not w/o alternate dict.
	  dictgroup=new QButtonGroup ();
	  dictgroup->setFrameStyle (QFrame::NoFrame);
	  //layout->addWidget (dictgroup,rdictgroup,0);
	  
	dictlistbutton=new QRadioButton (NLS("Language:"),this);
	connect (dictlistbutton, SIGNAL (toggled(bool)),
		 this, SLOT (sDictionary(bool)));
	layout->addWidget (dictlistbutton,rdictlist,0);
	*/

	dictlist=new QLabel (NLS("Dictionary"),this);
	dictlist->setMinimumSize (dictlist->sizeHint());
	layout->addWidget (dictlist,rdictlist,0);

	/* No alternate now -- is this needed?
	dicteditbutton=new QRadioButton (NLS("Alternate Dictionary"),this);
	connect (dicteditbutton, SIGNAL (toggled(bool)),
		 this, SLOT (sPathDictionary(bool)));
	layout->addWidget (dicteditbutton,rdictedit,0);
	dicteditbutton->setMinimumSize (dicteditbutton->
					 sizeHint();
					 */

	/*
	dictgroup->insert (dictlistbutton);
	dictgroup->insert (dicteditbutton);
	*/

	dictcombo = new QComboBox (this);
	dictcombo->setInsertionPolicy (QComboBox::NoInsertion);
	layout->addWidget (dictcombo,rdictlist,1);
	connect (dictcombo, SIGNAL (activated (int)),
				    this, SLOT (sSetDictionary (int)));

	QLabel* tmpQLabel;
	/*	
	tmpQLabel = new QLabel( this, "Label_1" );
	tmpQLabel->setGeometry( 20, 120, 120, 30 );
	tmpQLabel->setText( NLS("Language:"));
	tmpQLabel->setAlignment( 290 );
	tmpQLabel->setMargin( -1 );
	layout->addWidget (tmpQLabel, 3, 1);
	*/

	/*  I'll put this back if peple think that it's necessary,
	    but it would need to be supported better. */
	/*
	tmpQLabel = new QLabel( this, "Label_2" );
	//tmpQLabel->setGeometry( 30, 160, 120, 30 );
	tmpQLabel->setText( NLS("Personal dictionary:") );
	//	tmpQLabel->setAlignment( 290 );
	tmpQLabel->setAlignment( AlignLeft );
	tmpQLabel->setMargin( -1 );
	layout->addWidget (tmpQLabel, rpersdict, 0);
	tmpQLabel->setMinimumWidth (tmpQLabel->sizeHint().width());
	*/

	tmpQLabel = new QLabel( this, "Label_2" );
	tmpQLabel->setText( NLS("Encoding:") );
	tmpQLabel->setAlignment( AlignLeft );
	tmpQLabel->setMargin( -1 );
	layout->addWidget (tmpQLabel, rencoding, 0);
	tmpQLabel->setMinimumSize (tmpQLabel->sizeHint());

	encodingcombo = new QComboBox (FALSE, this);
	encodingcombo->insertItem (NLS("Latin1"));
	encodingcombo->insertItem (NLS("7-Bit/ASCII"));
	//encodingcombo->setMinimumSize (encodingcombo->sizeHint());
	connect (encodingcombo, SIGNAL (activated(int)), this,
		 SLOT (sChangeEncoding(int)));

	layout->addWidget (encodingcombo,5,1);
	//	layout->addMultiCellWidget (encodingcombo, 
	//		    rencoding, rencoding, 1,4);

	
	/* for alternate dict

	kle1 = new KLined( this, "LineEdit_1" );
	//	kle1->setGeometry( 150, 120, 290, 30 );
	kle1->setText( "" );
	kle1->setMaxLength( 32767 );
	kle1->setEchoMode( QLineEdit::Normal );
	kle1->setFrame( TRUE );
	connect (kle1, SIGNAL (textChanged (const char*)), this, 
		 SLOT (textChanged1 (const char*)));
	layout->addMultiCellWidget (kle1, rdictedit, rdictedit, 1,4);
	//	kle1->setMinimumSize (290,30);
	*/


	/*
	browsebutton1=new QPushButton;
	browsebutton1 = new QPushButton( this, "PushButton_1" );
	connect( browsebutton1, SIGNAL(clicked()), SLOT(sBrowseDict()) );
	browsebutton1->setText( NLS("Browse...") );
	browsebutton1->setAutoRepeat( FALSE );
	browsebutton1->setAutoResize( FALSE );
	layout->addWidget (browsebutton1, rdictedit, 6);
	browsebutton1->setGeometry( 460, 120, 70, 30 );
	browsebutton1->setMinimumWidth (30);
	*/

	QPushButton* tmpQPushButton;
	/*
	tmpQPushButton = new QPushButton( this, "PushButton_2" );
	tmpQPushButton->setGeometry( 460, 160, 70, 30 );
	tmpQPushButton->setMinimumWidth(tmpQPushButton->sizeHint().width());
	connect( tmpQPushButton, SIGNAL(clicked()), SLOT(sBrowsePDict()) );
	tmpQPushButton->setText( NLS("Browse...") );
	tmpQPushButton->setAutoRepeat( FALSE );
	tmpQPushButton->setAutoResize( FALSE );
	layout->addWidget (tmpQPushButton, rpersdict, 6);
	//	tmpQPushButton->setMinimumSize (tmpQPushButton->sizeHint());
	*/

	/*
	kle2 = new KLined( this, "LineEdit_2" );
	//	kle2->setGeometry( 150, 160, 290, 30 );
	kle2->setText( "" );
	kle2->setMaxLength( 32767 );
	kle2->setEchoMode( QLineEdit::Normal );
	kle2->setFrame( TRUE );
	connect (kle2, SIGNAL (textChanged (const char*)), this, 
		 SLOT (textChanged2 (const char*)));
	layout->addMultiCellWidget (kle2, rpersdict,rpersdict,1,4);
	///	kle2->setMinimumSize (290,30);
	*/
	/*
	
	tmpQPushButton = new QPushButton( this, "PushButton_3" );
	tmpQPushButton->setGeometry( 460, 210, 70, 30 );
	connect( tmpQPushButton, SIGNAL(clicked()), SLOT(sHelp()) );
	tmpQPushButton->setText( NLS("Help") );
	tmpQPushButton->setAutoRepeat( FALSE );
	tmpQPushButton->setAutoResize( FALSE );
	////
	tmpQPushButton->setEnabled (FALSE);
	layout->addWidget (tmpQPushButton, rhelp, 6);
	tmpQPushButton->setMinimumSize (tmpQPushButton->sizeHint());
	*/
	
	layout->activate();
	//	resize (sizeHint().width(),sizeHint().height());
	//setMinimumSize (sizeHint());

	////

	fillInDialog();	
	
    }
}


bool KSpellConfig::dictFromList (void) const
{
  return dictfromlist;
}

bool KSpellConfig::readGlobalSettings (void)
{
  kc->setGroup ("KSpell");

  setNoRootAffix   (kc->readNumEntry ("KSpell_NoRootAffix", 0));
  setRunTogether   (kc->readNumEntry ("KSpell_RunTogether", 0));
  setDictionary    (kc->readEntry ("KSpell_Dictionary", ""));
  setDictFromList  (kc->readNumEntry ("KSpell_DictFromList", FALSE));
  //  setPersonalDict  (kc->readEntry ("KSpell_PersonalDict", ""));
  setEncoding (kc->readNumEntry ("KSpell_Encoding", KS_E_ASCII));

  return TRUE;
}

bool KSpellConfig::writeGlobalSettings (void)
{
  kc->setGroup ("KSpell");
  kc->writeEntry ("KSpell_NoRootAffix",(int) noRootAffix (), TRUE, TRUE);
  kc->writeEntry ("KSpell_RunTogether", (int) runTogether (), TRUE, TRUE);
  kc->writeEntry ("KSpell_Dictionary", dictionary (), TRUE, TRUE);
  kc->writeEntry ("KSpell_DictFromList",(int) dictFromList(), TRUE, TRUE);
  //  kc->writeEntry ("KSpell_PersonalDict", personalDict (), TRUE,  TRUE);
  dsdebug ("Encoding=(%d)\n",encoding());
  kc->writeEntry ("KSpell_Encoding", (int) encoding(),
		  TRUE, TRUE);
  kc->sync();
  return TRUE;
}

void
KSpellConfig::sChangeEncoding(int i)
{
  dsdebug ("ChangeEncoding to (%d)\n",i);
  setEncoding (i);
}

bool KSpellConfig::interpret (QString &fname, QString &lname,
			      QString &hname)

{

  //Truncate aff

  if (fname.length()>4)
    if ((signed)fname.find(".aff")==(signed)fname.length()-4)
      fname.remove (fname.length()-4,4);


  dsdebug ("[%s]\n",(const char *)fname);

  //These are mostly the ispell-langpack defaults
  if (fname=="english")
    {
      lname="en";
      hname=NLS("English");
    }

  else if (fname=="espa~nol")
    {
      lname="sp";
      hname=NLS("Spanish");
    }

  else if (fname=="deutsch")
    {
      lname="de";
      hname=NLS("German");
    }

  else if (fname=="portuguesb" ||
	   fname=="br")
    {
      lname="br";
      hname=NLS("Brazilian Portuguese");
    }
  
  else
    {
      lname="";
      hname=NLS("Unknown");
    }

  //We have explicitly chosen English as the default here.
  if ( (klocale->language()=="C" && 
	lname=="en") ||
       klocale->language()==lname)
    return TRUE;
    
  return FALSE;
}

void KSpellConfig::fillInDialog (void)
{
  if (nodialog)
    return;

  dsdebug ("fillinDialog\n");


  cb1->setChecked (noRootAffix());
  cb2->setChecked (runTogether());
  encodingcombo->setCurrentItem (encoding());

  //  kle1->setText (dictionary());
  //  kle2->setText (personalDict());

  if (langfnames->count()==0) //will only happen once
    {
      
      langfnames->append("");
      //      langnames->append("ISpell Default");
      dictcombo->insertItem (NLS("ISpell Default"));

      QFileInfo dir ("/usr/lib/ispell");
      if (!dir.exists() || !dir.isDir())
	dir.setFile ("/usr/local/lib");
      if (!dir.exists() || !dir.isDir())
	return;

      dsdebug ("KS: %s %s\n",dir.filePath(),dir.dirPath().data());

      QDir thedir (dir.filePath(),"*.aff");
      dsdebug ("%s\n",thedir.path());


      dsdebug ("entryList().count()=%d\n",thedir.entryList()->count());

      for (unsigned i=0;i<thedir.entryList()->count();i++)
	{
	  QString fname, lname, hname;

	  dsdebug ("%s\n",(const char *)thedir [i]);
	  fname = (const char *)thedir [i];

	  if (interpret (fname, lname, hname))
	    { // This one is the KDE default language
	      // so place it first in the lists (overwrite "Default")

	      dsdebug ("default is [%s][%s]\n",hname.data(),fname.data());
	      langfnames->removeFirst();
	      langfnames->insert (0,fname);

	      hname="Default - "+hname+"("+fname+")";
	      //	      langnames->insert (0,hname);
	      //	      langnames->removeFirst();
	      
	      dictcombo->changeItem (hname,0);
	    }
	  else
	    {
	      langfnames->append (fname);
	      hname=hname+" ("+fname+")";
	      //	      langnames->append (hname);
	      
	      dictcombo->insertItem (hname.data());
	    }
	}

      
    }
  int whichelement=-1;
  dsdebug ("dfl=%d\n",dictFromList());
  if (dictFromList())
    for (unsigned i=0;i<langfnames->count();i++)
      {
	dsdebug ("[%s]==[%s]?\n",langfnames->at(i),dictionary());
	if (!strcmp(langfnames->at(i),dictionary()))
	  whichelement=i;
      }

  dsdebug ("whiche=%d\n",whichelement);
  dictcombo->setMinimumWidth (dictcombo->sizeHint().width());

  if (dictionary()[0]=='\0' ||  whichelement!=-1)
    {
      setDictFromList (TRUE);
      if (whichelement!=-1)
	dictcombo->setCurrentItem(whichelement);
    }
  else
    setDictFromList (FALSE);

    
  //dictlistbutton->setChecked(dictFromList());
  //  dicteditbutton->setChecked(!dictFromList());
  sDictionary (dictFromList());
  sPathDictionary (!dictFromList());

}

/*
  Options setting routines.
  */
void KSpellConfig::setNoRootAffix (bool b)
{
  bnorootaffix=b;
}

void KSpellConfig::setRunTogether(bool b)
{
  bruntogether=b;
}

void KSpellConfig::setDictionary (const char *s)
{
  qsdict=s;

  if (qsdict.length()>4)
    if ((signed)qsdict.find(".aff")==(signed)qsdict.length()-4)
      qsdict.remove (qsdict.length()-4,4);

  dsdebug ("setdictionary: [%s]\n",qsdict.data());
}

void KSpellConfig::setDictFromList (bool dfl)
{
  dsdebug ("sdfl = %d\n",dfl);
  dictfromlist=dfl;
}

/*
void KSpellConfig::setPersonalDict (const char *s)
{
  qspdict=s;
}
*/

void KSpellConfig::setEncoding (int enctype)
{
  dsdebug ("setEncoding (%d)\n",enctype);
  enc=enctype;
  dsdebug ("encoding=(%d)\n",encoding());
}

/*
  Options reading routines.
  */

bool KSpellConfig::noRootAffix (void) const
{
  return bnorootaffix;
}

bool KSpellConfig::runTogether(void) const
{
  return bruntogether;
}

char *KSpellConfig::dictionary (void) const
{
  return qsdict.data();
}

/*
char *KSpellConfig::personalDict (void) const
{
  return qspdict.data();
}
*/

int KSpellConfig::encoding (void) const
{
  return enc;
}

void KSpellConfig::sRunTogether(bool)
{
  setRunTogether (cb2->isChecked());
}

void KSpellConfig::sNoAff(bool)
{
  setNoRootAffix (cb1->isChecked());
}

void KSpellConfig::sBrowseDict()
{
  return;

  QString qs( QFileDialog::getOpenFileName ("/usr/local/lib","*.hash") );
  if ( !qs.isNull() )
    kle1->setText (qs.data());

}
/*
void KSpellConfig::sBrowsePDict()
{
  //how do I find home directory path??
  QString qs( QFileDialog::getOpenFileName ("",".ispell_*") );
  if ( !qs.isNull() )
      kle2->setText (qs.data());

  
}
*/

void KSpellConfig::sSetDictionary (int i)
{
  dsdebug ("SSD (%d,%d)\n",i,dictcombo->currentItem());
  setDictionary (langfnames->at(i));
  setDictFromList (TRUE);
}

void KSpellConfig::sDictionary(bool on)
{
  if (on)
    {
      dictcombo->setEnabled (TRUE);
      dsdebug ("SD (%d)\n",dictcombo->currentItem());
      setDictionary (langfnames->at(dictcombo->currentItem()));
      setDictFromList (TRUE);
    }
  else
    {
      dictcombo->setEnabled (FALSE);
      dsdebug ("SDoff\n");
    }
}

void KSpellConfig::sPathDictionary(bool on)
{
  return; //enough for now


  if (on)
    {
      dsdebug ("spd\n");
      //kle1->setEnabled (TRUE);
      //      browsebutton1->setEnabled (TRUE);
      setDictionary (kle1->text());
      setDictFromList (FALSE);
    }
  else
    {
      kle1->setEnabled (FALSE);
      //      browsebutton1->setEnabled (FALSE);
    }
}

void KSpellConfig::sHelp()
{
}
/*
void KSpellConfig::textChanged1 (const char *s)
{
  dsdebug ("tc1\n");
  setDictionary (s);
}

void KSpellConfig::textChanged2 (const char *)
{
  //  setPersonalDict (s);
}
*/

void KSpellConfig::operator= (const KSpellConfig &ksc)
{
  //We want to copy the data members, but not the
  //pointers to the child widgets
  dsdebug ("op=\n");
  setNoRootAffix (ksc.noRootAffix());
  setRunTogether (ksc.runTogether());
  setDictionary (ksc.dictionary());
  setDictFromList (ksc.dictFromList());
  //  setPersonalDict (ksc.personalDict());
  setEncoding (ksc.encoding());

  fillInDialog();
}

void KSpellConfig::setIgnoreList (QStrList _ignorelist)
{
  ignorelist=_ignorelist;
}

QStrList KSpellConfig::ignoreList (void) const
{
  return ignorelist;
}


