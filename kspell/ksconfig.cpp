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

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <kapp.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klineedit.h>
#include <klocale.h>

#include "ksconfig.h"

KSpellConfig::KSpellConfig (const KSpellConfig &_ksc)
  : QWidget(0, 0), nodialog(true)
{  
  setNoRootAffix (_ksc.noRootAffix());
  setRunTogether (_ksc.runTogether());
  setDictionary  (_ksc.dictionary());
  setDictFromList (_ksc.dictFromList());
  //  setPersonalDict (_ksc.personalDict());
  setIgnoreList (_ksc.ignoreList());
  setEncoding (_ksc.encoding());
  setClient (_ksc.client());
}


KSpellConfig::KSpellConfig( QWidget *parent, const char *name,
			    KSpellConfig *_ksc, bool addHelpButton ) 
  : QWidget (parent, name), nodialog(false)
{
  kc = KGlobal::config();
  if( _ksc == 0 )
  {
    readGlobalSettings();
  }
  else
  {
    setNoRootAffix (_ksc->noRootAffix());
    setRunTogether (_ksc->runTogether());
    setDictionary  (_ksc->dictionary());
    setDictFromList (_ksc->dictFromList());
    //setPersonalDict (_ksc->personalDict());
    setIgnoreList (_ksc->ignoreList());
    setEncoding (_ksc->encoding());
    setClient (_ksc->client());
  }

  QGridLayout *glay = new QGridLayout (this, 6, 3, 0, KDialog::spacingHint() );
  cb1 = new QCheckBox(i18n("Create root/affix combinations"
			   " not in dictionary"), this );
  connect( cb1, SIGNAL(toggled(bool)), SLOT(sNoAff(bool)) );
  glay->addMultiCellWidget( cb1, 0, 0, 0, 2 );

  cb2 = new QCheckBox( i18n("Consider run-together words"
			    " as spelling errors"), this );
  connect( cb2, SIGNAL(toggled(bool)), SLOT(sRunTogether(bool)) );
  glay->addMultiCellWidget( cb2, 1, 1, 0, 2 );
  
  dictcombo = new QComboBox( this );
  dictcombo->setInsertionPolicy (QComboBox::NoInsertion);
  connect (dictcombo, SIGNAL (activated (int)),
	   this, SLOT (sSetDictionary (int)));
  glay->addMultiCellWidget( dictcombo, 2, 2, 1, 2 );

  dictlist = new QLabel (dictcombo, i18n("Dictionary:"), this);
  glay->addWidget( dictlist, 2 ,0 );

  encodingcombo = new QComboBox( this );
  encodingcombo->insertItem ("US-ASCII");
  encodingcombo->insertItem ("ISO 8859-1");
  encodingcombo->insertItem ("ISO 8859-2");
  encodingcombo->insertItem ("ISO 8859-3");
  encodingcombo->insertItem ("ISO 8859-4");
  encodingcombo->insertItem ("ISO 8859-5");
  encodingcombo->insertItem ("ISO 8859-7");
  encodingcombo->insertItem ("ISO 8859-8");
  encodingcombo->insertItem ("ISO 8859-9");
  encodingcombo->insertItem ("ISO 8859-15");
  encodingcombo->insertItem ("UTF-8");
  connect (encodingcombo, SIGNAL (activated(int)), this,
	   SLOT (sChangeEncoding(int)));
  glay->addMultiCellWidget (encodingcombo, 3, 3, 1, 2);

  QLabel *tmpQLabel = new QLabel( encodingcombo, i18n("Encoding:"), this);
  glay->addWidget( tmpQLabel, 3, 0 );

  
  clientcombo = new QComboBox( this );
  clientcombo->insertItem (i18n("International Ispell"));
  clientcombo->insertItem (i18n("Aspell"));
  connect (clientcombo, SIGNAL (activated(int)), this,
	   SLOT (sChangeClient(int)));
  glay->addMultiCellWidget( clientcombo, 4, 4, 1, 2 );
  
  tmpQLabel = new QLabel( clientcombo, i18n("Client:"), this );
  glay->addWidget( tmpQLabel, 4, 0 );
  
  if( addHelpButton == true )
  {
    QPushButton *pushButton = new QPushButton( i18n("&Help"), this );
    connect( pushButton, SIGNAL(clicked()), this, SLOT(sHelp()) );
    glay->addWidget(pushButton, 5, 2);
  }
  
  fillInDialog();	

  //
  // 2000-02-19 Espen Sand
  // What is this? can it be removed?
  //

  /* not w/o alternate dict.
     dictgroup=new QButtonGroup ();
     dictgroup->setFrameStyle (QFrame::NoFrame);
     //layout->addWidget (dictgroup,rdictgroup,0);
	  
     dictlistbutton=new QRadioButton (i18n("Language:"),this);
     connect (dictlistbutton, SIGNAL (toggled(bool)),
     this, SLOT (sDictionary(bool)));
     layout->addWidget (dictlistbutton,rdictlist,0);
  */

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


	
  /* for alternate dict

     kle1 = new KLineEdit( this, "LineEdit_1" );
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
    kle2 = new KLineEdit( this, "LineEdit_2" );
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

}





















KSpellConfig::~KSpellConfig ()
{
}


bool
KSpellConfig::dictFromList () const
{
  return dictfromlist;
}

bool
KSpellConfig::readGlobalSettings ()
{
  kc->setGroup ("KSpell");

  setNoRootAffix   (kc->readNumEntry ("KSpell_NoRootAffix", 0));
  setRunTogether   (kc->readNumEntry ("KSpell_RunTogether", 0));
  setDictionary    (kc->readEntry ("KSpell_Dictionary", ""));
  setDictFromList  (kc->readNumEntry ("KSpell_DictFromList", FALSE));
  //  setPersonalDict  (kc->readEntry ("KSpell_PersonalDict", ""));
  setEncoding (kc->readNumEntry ("KSpell_Encoding", KS_E_ASCII));
  setClient (kc->readNumEntry ("KSpell_Client", KS_CLIENT_ISPELL));

  return TRUE;
}

bool
KSpellConfig::writeGlobalSettings ()
{
  kc->setGroup ("KSpell");
  kc->writeEntry ("KSpell_NoRootAffix",(int) noRootAffix (), TRUE, TRUE);
  kc->writeEntry ("KSpell_RunTogether", (int) runTogether (), TRUE, TRUE);
  kc->writeEntry ("KSpell_Dictionary", dictionary (), TRUE, TRUE);
  kc->writeEntry ("KSpell_DictFromList",(int) dictFromList(), TRUE, TRUE);
  //  kc->writeEntry ("KSpell_PersonalDict", personalDict (), TRUE,  TRUE);
  kc->writeEntry ("KSpell_Encoding", (int) encoding(),
		  TRUE, TRUE);
  kc->writeEntry ("KSpell_Client", client(),
		  TRUE, TRUE);
  kc->sync();
  return TRUE;
}

void
KSpellConfig::sChangeEncoding(int i)
{
    kdDebug(750) << "KSpellConfig::sChangeEncoding(" << i << ")" << endl;
  setEncoding (i);
}

void
KSpellConfig::sChangeClient (int i)
{
  setClient (i);
}

bool
KSpellConfig::interpret (QString &fname, QString &lname,
			      QString &hname)

{

  //Truncate aff

  if (fname.length()>4)
    if ((signed)fname.find(".aff")==(signed)fname.length()-4)
      fname.remove (fname.length()-4,4);


    kdDebug(750) << "KSpellConfig::interpret [" << fname << "]" << endl;

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

  else if (fname=="portugues")
    {
      lname="pt";
      hname=i18n("Portuguese");
    }

  else if (fname=="esperanto")
    {
      lname="eo";
      hname=i18n("Esperanto");
    }
  
  else
    {
      lname="";
      hname=i18n("Unknown");
    }

  //We have explicitly chosen English as the default here.
  if ( (KGlobal::locale()->language()==QString::fromLatin1("C") && 
	lname==QString::fromLatin1("en")) ||
       KGlobal::locale()->language()==lname)
    return TRUE;
    
  return FALSE;
}

void
KSpellConfig::fillInDialog ()
{
  if (nodialog)
    return;

    kdDebug(750) << "KSpellConfig::fillinDialog" << endl;

  cb1->setChecked (noRootAffix());
  cb2->setChecked (runTogether());
  encodingcombo->setCurrentItem (encoding());
  clientcombo->setCurrentItem (client());

  //  kle1->setText (dictionary());
  //  kle2->setText (personalDict());

  if (langfnames.count()==0) //will only happen once
    {
      
      langfnames.append(""); // Default
      dictcombo->insertItem (i18n("ISpell Default"));

      QFileInfo dir ("/usr/lib/ispell");
      if (!dir.exists() || !dir.isDir())
	dir.setFile ("/usr/local/lib");
      if (!dir.exists() || !dir.isDir())
	return;

            kdDebug(750) << "KSpellConfig::fillInDialog " << dir.filePath() << " " << dir.dirPath() << endl;

      QDir thedir (dir.filePath(),"*.aff");
            kdDebug(750) << "KSpellConfig" << thedir.path() << "\n" << endl;

            kdDebug(750) << "entryList().count()=" << thedir.entryList().count() << endl;

      for (unsigned int i=0;i<thedir.entryList().count();i++)
	{
	  QString fname, lname, hname;

	  //	  kdebug (KDEBUG_INFO, 750, "%s/%d %s", __FILE__, __LINE__, (const char *)thedir [i]);
	  fname = thedir [i];

	  if (interpret (fname, lname, hname))
	    { // This one is the KDE default language
	      // so place it first in the lists (overwrite "Default")

	      //	      kdebug (KDEBUG_INFO, 750, "default is [%s][%s]",hname.data(),fname.data());
	      langfnames.remove ( langfnames.begin() );
	      langfnames.prepend ( fname );

	      hname="Default - "+hname+"("+fname+")";
	      
	      dictcombo->changeItem (hname,0);
	    }
	  else
	    {
	      langfnames.append (fname);
	      hname=hname+" ("+fname+")";
	      
	      dictcombo->insertItem (hname);
	    }
	}

      
    }
  int whichelement=-1;
  //  kdebug (KDEBUG_INFO, 750, "dfl=%d",dictFromList());
  if (dictFromList())
    for (unsigned int i=0;i<langfnames.count();i++)
      {
	//	kdebug (KDEBUG_INFO, 750, "[%s]==[%s]?", langfnames[i], dictionary().data());
	if (langfnames[i] == dictionary())
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

void
KSpellConfig::setClient (int c)
{
  iclient = c;
}

void
KSpellConfig::setNoRootAffix (bool b)
{
  bnorootaffix=b;
}

void
KSpellConfig::setRunTogether(bool b)
{
  bruntogether=b;
}

void
KSpellConfig::setDictionary (const QString s)
{
  qsdict=s; //.copy();

  if (qsdict.length()>4)
    if ((signed)qsdict.find(".aff")==(signed)qsdict.length()-4)
      qsdict.remove (qsdict.length()-4,4);

  //  kdebug (KDEBUG_INFO, 750, "setdictionary: [%s]",qsdict.data());
}

void
KSpellConfig::setDictFromList (bool dfl)
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

void
KSpellConfig::setEncoding (int enctype)
{
  enc=enctype;
}

/*
  Options reading routines.
 */
int
KSpellConfig::client () const
{
  return iclient;
}


bool
KSpellConfig::noRootAffix () const
{
  return bnorootaffix;
}

bool
KSpellConfig::runTogether() const
{
  return bruntogether;
}

const
QString KSpellConfig::dictionary () const
{
  return qsdict;
}

/*
const QString KSpellConfig::personalDict () const
{
  return qspdict;
}
*/

int
KSpellConfig::encoding () const
{
  return enc;
}

void
KSpellConfig::sRunTogether(bool)
{
  setRunTogether (cb2->isChecked());
}

void
KSpellConfig::sNoAff(bool)
{
  setNoRootAffix (cb1->isChecked());
}

/*
void
KSpellConfig::sBrowseDict()
{
  return;

  QString qs( QFileDialog::getOpenFileName ("/usr/local/lib","*.hash") );
  if ( !qs.isNull() )
    kle1->setText (qs);

}
*/

/*
void KSpellConfig::sBrowsePDict()
{
  //how do I find home directory path??
  QString qs( QFileDialog::getOpenFileName ("",".ispell_*") );
  if ( !qs.isNull() )
      kle2->setText (qs);

  
}
*/

void
KSpellConfig::sSetDictionary (int i)
{
  setDictionary (langfnames[i]);
  setDictFromList (TRUE);
}

void
KSpellConfig::sDictionary(bool on)
{
  if (on)
    {
      dictcombo->setEnabled (TRUE);
      setDictionary (langfnames[dictcombo->currentItem()] );
      setDictFromList (TRUE);
    }
  else
    {
      dictcombo->setEnabled (FALSE);
    }
}

void
KSpellConfig::sPathDictionary(bool on)
{
  return; //enough for now


  if (on)
    {
      //kle1->setEnabled (TRUE);
      //      browsebutton1->setEnabled (TRUE);
      //setDictionary (kle1->text());
      setDictFromList (FALSE);
    }
  else
    {
      //kle1->setEnabled (FALSE);
      //browsebutton1->setEnabled (FALSE);
    }
}


void KSpellConfig::activateHelp( void )
{
  sHelp();
}

void KSpellConfig::sHelp( void )
{
  kapp->invokeHelp("configuration", "kspell");
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

void
KSpellConfig::operator= (const KSpellConfig &ksc)
{
  //We want to copy the data members, but not the
  //pointers to the child widgets
  setNoRootAffix (ksc.noRootAffix());
  setRunTogether (ksc.runTogether());
  setDictionary (ksc.dictionary());
  setDictFromList (ksc.dictFromList());
  //  setPersonalDict (ksc.personalDict());
  setEncoding (ksc.encoding());
  setClient (ksc.client());

  fillInDialog();
}

void
KSpellConfig::setIgnoreList (QStringList _ignorelist)
{
  ignorelist=_ignorelist;
}

QStringList
KSpellConfig::ignoreList () const
{
  return ignorelist;
}

#include "ksconfig.moc"



