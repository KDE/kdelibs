// $Id$

#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <kapp.h>
#include <klocale.h>
#include <kdebug.h>
#include <klined.h>

#include "ksconfig.h"

KSpellConfig::KSpellConfig (const KSpellConfig &_ksc)
	: QWidget(0, 0)
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
			    KSpellConfig *_ksc) : QWidget (parent, name)
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
      //	    rpersdict = 6,

        //rdictedit = 4,
	cb1 = new QCheckBox( this, "CheckBox_1" );
	cb1->setGeometry( 30, 40, 320, 30 );
	connect( cb1, SIGNAL(toggled(bool)), SLOT(sNoAff(bool)) );
	cb1->setText( i18n("Create Root/Affix combinations not in the dictionary") );
	cb1->setAutoRepeat( FALSE );
	cb1->setAutoResize( FALSE );
	layout->addMultiCellWidget (cb1, 0, 0, 0, 5);
	cb1->setMinimumSize (cb1->sizeHint());

	cb2 = new QCheckBox( this, "CheckBox_3" );
	cb2->setGeometry( 30, 80, 320, 30 );
	connect( cb2, SIGNAL(toggled(bool)), SLOT(sRunTogether(bool)) );
	cb2->setText( i18n("Consider run-together words as spelling errors") );
	cb2->setAutoRepeat( FALSE );
	cb2->setAutoResize( FALSE );
	layout->addMultiCellWidget (cb2, 1, 1, 0, 5);
	cb2->setMinimumSize (cb2->sizeHint());

	/* not w/o alternate dict.
	  dictgroup=new QButtonGroup ();
	  dictgroup->setFrameStyle (QFrame::NoFrame);
	  //layout->addWidget (dictgroup,rdictgroup,0);
	  
	dictlistbutton=new QRadioButton (i18n("Language:"),this);
	connect (dictlistbutton, SIGNAL (toggled(bool)),
		 this, SLOT (sDictionary(bool)));
	layout->addWidget (dictlistbutton,rdictlist,0);
	*/

	dictlist=new QLabel (i18n("Dictionary"),this);
	dictlist->setMinimumSize (dictlist->sizeHint());
	layout->addWidget (dictlist,rdictlist,0);

	/* No alternate now -- is this needed?
	dicteditbutton=new QRadioButton (i18n("Alternate Dictionary"),this);
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
	tmpQLabel->setText( i18n("Language:"));
	tmpQLabel->setAlignment( 290 );
	tmpQLabel->setMargin( -1 );
	layout->addWidget (tmpQLabel, 3, 1);
	*/

	/*  I'll put this back if peple think that it's necessary,
	    but it would need to be supported better. */
	/*
	tmpQLabel = new QLabel( this, "Label_2" );
	//tmpQLabel->setGeometry( 30, 160, 120, 30 );
	tmpQLabel->setText( i18n("Personal dictionary:") );
	//	tmpQLabel->setAlignment( 290 );
	tmpQLabel->setAlignment( AlignLeft );
	tmpQLabel->setMargin( -1 );
	layout->addWidget (tmpQLabel, rpersdict, 0);
	tmpQLabel->setMinimumWidth (tmpQLabel->sizeHint().width());
	*/

	tmpQLabel = new QLabel( this, "Label_2" );
	tmpQLabel->setText( i18n("Encoding:") );
	tmpQLabel->setAlignment( AlignLeft );
	tmpQLabel->setMargin( -1 );
	layout->addWidget (tmpQLabel, rencoding, 0);
	tmpQLabel->setMinimumSize (tmpQLabel->sizeHint());

	encodingcombo = new QComboBox (FALSE, this);
	encodingcombo->insertItem (i18n("Latin1"));
	encodingcombo->insertItem (i18n("7-Bit/ASCII"));
	encodingcombo->setMinimumSize (encodingcombo->sizeHint());
	connect (encodingcombo, SIGNAL (activated(int)), this,
		 SLOT (sChangeEncoding(int)));
	layout->addMultiCellWidget (encodingcombo, 
				    rencoding, rencoding, 1,4);

	
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
	browsebutton1->setText( i18n("Browse...") );
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
	tmpQPushButton->setText( i18n("Browse...") );
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

	tmpQPushButton = new QPushButton( this, "PushButton_3" );
	tmpQPushButton->setGeometry( 460, 210, 70, 30 );
	connect( tmpQPushButton, SIGNAL(clicked()), SLOT(sHelp()) );
	tmpQPushButton->setText( i18n("Help") );
	tmpQPushButton->setAutoRepeat( FALSE );
	tmpQPushButton->setAutoResize( FALSE );
	////
	  //tmpQPushButton->setEnabled (FALSE);
	layout->addWidget (tmpQPushButton, rhelp, 6);
	tmpQPushButton->setMinimumSize (tmpQPushButton->sizeHint());

	
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
  kc->writeEntry ("KSpell_Encoding", (int) encoding(),
		  TRUE, TRUE);
  kc->sync();
  return TRUE;
}

void
KSpellConfig::sChangeEncoding(int i)
{
  //  kdebug(KDEBUG_INFO, 750, "KSpellConfig::sChangeEncoding(%d)", i);
  setEncoding (i);
}

bool KSpellConfig::interpret (QString &fname, QString &lname,
			      QString &hname)

{

  //Truncate aff

  if (fname.length()>4)
    if ((signed)fname.find(".aff")==(signed)fname.length()-4)
      fname.remove (fname.length()-4,4);


  //  kdebug(KDEBUG_INFO, 750, "KSpellConfig::interpret [%s]", (const char *)fname);

  //These are mostly the ispell-langpack defaults
  if (fname=="english")
    {
      lname="en";
      hname=i18n("English");
    }

  else if (fname=="espa~nol")
    {
      lname="sp";
      hname=i18n("Spanish");
    }

  else if (fname=="deutsch")
    {
      lname="de";
      hname=i18n("German");
    }

  else if (fname=="portuguesb" ||
	   fname=="br")
    {
      lname="br";
      hname=i18n("Brazilian Portuguese");
    }
  
  else
    {
      lname="";
      hname=i18n("Unknown");
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

  //  kdebug(KDEBUG_INFO, 750, "KSpellConfig::fillinDialog");

  cb1->setChecked (noRootAffix());
  cb2->setChecked (runTogether());
  encodingcombo->setCurrentItem (encoding());

  //  kle1->setText (dictionary());
  //  kle2->setText (personalDict());

  if (langfnames->count()==0) //will only happen once
    {
      
      langfnames->append("");
      //      langnames->append("ISpell Default");
      dictcombo->insertItem (i18n("ISpell Default"));

      QFileInfo dir ("/usr/lib/ispell");
      if (!dir.exists() || !dir.isDir())
	dir.setFile ("/usr/local/lib");
      if (!dir.exists() || !dir.isDir())
	return;

      //      kdebug(KDEBUG_INFO, 750, "KSpellConfig::fillInDialog %s %s", dir.filePath().ascii(), dir.dirPath().ascii());

      QDir thedir (dir.filePath(),"*.aff");
      //      kdebug(KDEBUG_INFO, 750, "KSpellConfig%s\n",thedir.path().ascii());

      //      kdebug(KDEBUG_INFO, 750, "entryList().count()=%d", thedir.entryList().count());

      for (unsigned i=0;i<thedir.entryList().count();i++)
	{
	  QString fname, lname, hname;

	  //	  kdebug (KDEBUG_INFO, 750, "%s/%d %s", __FILE__, __LINE__, (const char *)thedir [i]);
	  fname = thedir [i];

	  if (interpret (fname, lname, hname))
	    { // This one is the KDE default language
	      // so place it first in the lists (overwrite "Default")

	      //	      kdebug (KDEBUG_INFO, 750, "default is [%s][%s]",hname.data(),fname.data());
	      langfnames->removeFirst();
	      langfnames->insert (0,fname.ascii());

	      hname="Default - "+hname+"("+fname+")";
	      //	      langnames->insert (0,hname);
	      //	      langnames->removeFirst();
	      
	      dictcombo->changeItem (hname,0);
	    }
	  else
	    {
	      langfnames->append (fname.ascii());
	      hname=hname+" ("+fname+")";
	      //	      langnames->append (hname);
	      
	      dictcombo->insertItem (hname.data());
	    }
	}

      
    }
  int whichelement=-1;
  //  kdebug (KDEBUG_INFO, 750, "dfl=%d",dictFromList());
  if (dictFromList())
    for (unsigned i=0;i<langfnames->count();i++)
      {
	//	kdebug (KDEBUG_INFO, 750, "[%s]==[%s]?", langfnames->at(i), dictionary().data());
	if (langfnames->at(i) == dictionary())
	  whichelement=i;
      }

  //  kdebug (KDEBUG_INFO, 750, "whiche=%d", whichelement);
  dictcombo->setMinimumWidth (dictcombo->sizeHint().width());

  if (dictionary().isEmpty() ||  whichelement!=-1)
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
 * Options setting routines.
 */
void KSpellConfig::setNoRootAffix (bool b)
{
  bnorootaffix=b;
}

void KSpellConfig::setRunTogether(bool b)
{
  bruntogether=b;
}

void KSpellConfig::setDictionary (const QString s)
{
  qsdict=s; //.copy();

  if (qsdict.length()>4)
    if ((signed)qsdict.find(".aff")==(signed)qsdict.length()-4)
      qsdict.remove (qsdict.length()-4,4);

  //  kdebug (KDEBUG_INFO, 750, "setdictionary: [%s]",qsdict.data());
}

void KSpellConfig::setDictFromList (bool dfl)
{
  //  kdebug (KDEBUG_INFO, 750, "sdfl = %d", dfl);
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
  enc=enctype;
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

const QString KSpellConfig::dictionary (void) const
{
  return qsdict;
}

/*
const QString KSpellConfig::personalDict (void) const
{
  return qspdict;
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
    kle1->setText (qs);

}
/*
void KSpellConfig::sBrowsePDict()
{
  //how do I find home directory path??
  QString qs( QFileDialog::getOpenFileName ("",".ispell_*") );
  if ( !qs.isNull() )
      kle2->setText (qs);

  
}
*/

void KSpellConfig::sSetDictionary (int i)
{
  setDictionary (langfnames->at(i));
  setDictFromList (TRUE);
}

void KSpellConfig::sDictionary(bool on)
{
  if (on)
    {
      dictcombo->setEnabled (TRUE);
      setDictionary (langfnames->at(dictcombo->currentItem()));
      setDictFromList (TRUE);
    }
  else
    {
      dictcombo->setEnabled (FALSE);
    }
}

void KSpellConfig::sPathDictionary(bool on)
{
  return; //enough for now


  if (on)
    {
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
  QString file ("kspell/ksconfig.html"), label ("");
  kapp->invokeHTMLHelp (file, label);
}
/*
void KSpellConfig::textChanged1 (const char *s)
{
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

#include "ksconfig.moc"
