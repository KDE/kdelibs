/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>
   Copyright (C) 2000-2001 Wolfram Diestel <wolfram@steloj.de>

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

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klineedit.h>
#include <klocale.h>

#include "ksconfig.h"

class KSpellConfigPrivate
{
public:
    QStringList replacelist;
};


KSpellConfig::KSpellConfig (const KSpellConfig &_ksc)
  : QWidget(0, 0), nodialog(true)
  , kc(0)
  , cb1(0)
  , cb2(0)
  , dictlist(0)
  , dictcombo(0)
  , encodingcombo(0)
  , clientcombo(0)
{
    d= new KSpellConfigPrivate;
    setReplaceAllList( _ksc.replaceAllList ());
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
  , kc(0)
  , cb1(0)
  , cb2(0)
  , dictlist(0)
  , dictcombo(0)
  , encodingcombo(0)
  , clientcombo(0)
{
    d= new KSpellConfigPrivate;
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
  encodingcombo->insertItem ("ISO 8859-13");
  encodingcombo->insertItem ("ISO 8859-15");
  encodingcombo->insertItem ("UTF-8");
  encodingcombo->insertItem ("KOI8-R");
  encodingcombo->insertItem ("KOI8-U");
  encodingcombo->insertItem ("CP1251");

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
}

KSpellConfig::~KSpellConfig ()
{
    delete d;
}


bool
KSpellConfig::dictFromList () const
{
  return dictfromlist;
}

bool
KSpellConfig::readGlobalSettings ()
{
  KConfigGroupSaver cs(kc,"KSpell");

  setNoRootAffix   (kc->readNumEntry ("KSpell_NoRootAffix", 0));
  setRunTogether   (kc->readNumEntry ("KSpell_RunTogether", 0));
  setDictionary    (kc->readEntry ("KSpell_Dictionary", ""));
  setDictFromList  (kc->readNumEntry ("KSpell_DictFromList", FALSE));
  setEncoding (kc->readNumEntry ("KSpell_Encoding", KS_E_ASCII));
  setClient (kc->readNumEntry ("KSpell_Client", KS_CLIENT_ISPELL));

  return TRUE;
}

bool
KSpellConfig::writeGlobalSettings ()
{
  KConfigGroupSaver cs(kc,"KSpell");
  kc->writeEntry ("KSpell_NoRootAffix",(int) noRootAffix (), TRUE, TRUE);
  kc->writeEntry ("KSpell_RunTogether", (int) runTogether (), TRUE, TRUE);
  kc->writeEntry ("KSpell_Dictionary", dictionary (), TRUE, TRUE);
  kc->writeEntry ("KSpell_DictFromList",(int) dictFromList(), TRUE, TRUE);
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
  emit configChanged();
}

void
KSpellConfig::sChangeClient (int i)
{
  setClient (i);

  // read in new dict list
  if (dictcombo) {
    if (iclient == KS_CLIENT_ISPELL)
      getAvailDictsIspell();
    else
      getAvailDictsAspell();
  }
  emit configChanged();
}

bool
KSpellConfig::interpret (QString &fname, QString &lname,
			      QString &hname)

{

  kdDebug(750) << "KSpellConfig::interpret [" << fname << "]" << endl;

  QString dname(fname);

  if(dname.right(1)=="+")
    dname.remove(dname.length()-1, 1);

  if(dname.right(3)=="sml" || dname.right(3)=="med" || dname.right(3)=="lrg" || dname.right(3)=="xlg")
     dname.remove(dname.length()-3,3);

  //These are mostly the ispell-langpack defaults
  if (dname=="english" || dname=="american" ||
      dname=="british" || dname=="canadian") {
    lname="en"; hname=i18n("English");
  }
  else if (dname=="espa~nol" || dname=="espanol") {
    lname="es"; hname=i18n("Spanish");
  }
  else if (dname=="dansk") {
    lname="da"; hname=i18n("Danish");
  }
  else if (dname=="deutsch") {
    lname="de"; hname=i18n("German");
  }
  else if (dname=="german") {
    lname="de"; hname=i18n("German (new spelling)");
  }
  else if (dname=="portuguesb" || dname=="br") {
    lname="br"; hname=i18n("Brazilian Portuguese");
  }
  else if (dname=="portugues") {
    lname="pt"; hname=i18n("Portuguese");
  }
  else if (dname=="esperanto") {
    lname="eo"; hname=i18n("Esperanto");
  }
  else if (dname=="norsk") {
    lname="no"; hname=i18n("Norwegian");
  }
  else if (dname=="polish") {
    lname="pl"; hname=i18n("Polish"); sChangeEncoding(KS_E_LATIN2);
  }
  else if (dname=="russian") {
    lname="ru"; hname=i18n("Russian");
  }
  else if (dname=="slovensko") {
    lname="si"; hname=i18n("Slovenian"); sChangeEncoding(KS_E_LATIN2);
  }
  else if (dname=="slovak"){
    lname="sk"; hname=i18n("Slovak"); sChangeEncoding(KS_E_LATIN2);
  }
  else if (dname=="czech") {
    lname="cs"; hname=i18n("Czech"); sChangeEncoding(KS_E_LATIN2);
  }
  else if (dname=="svenska") {
    lname="sv"; hname=i18n("Swedish");
  }
  else if (dname=="swiss") {
    lname="de"; hname=i18n("Swiss German");
  }
  else if (dname=="ukrainian") {
    lname="uk"; hname=i18n("Ukrainian");
  }
  else if (dname=="lietuviu" || dname=="lithuanian") {
     lname="lt"; hname=i18n("Lithuanian");
  }
  else if (dname=="francais" || dname=="french") {
    lname="fr"; hname=i18n("French");
  }
  else if (dname=="belarusian") {  // waiting for post 2.2 to not dissapoint translators
    lname="be"; hname=i18n("Belarusian");
  }
  else if( dname == "magyar" ) {
    lname="hu"; hname=i18n("Hungarian");
    sChangeEncoding(KS_E_LATIN2);
  }
  else {
    lname=""; hname=i18n("Unknown ispell dictionary", "Unknown");
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

  // get list of available dictionaries
  if (iclient == KS_CLIENT_ISPELL)
    getAvailDictsIspell();
  else
    getAvailDictsAspell();

  // select the used dictionary in the list
  int whichelement=-1;

  if (dictFromList())
    for (unsigned int i=0; i<langfnames.count(); i++)
      {
	if (langfnames[i] == dictionary())
	  whichelement=i;
      }

  dictcombo->setMinimumWidth (dictcombo->sizeHint().width());

  if (dictionary().isEmpty() ||  whichelement!=-1)
    {
      setDictFromList (TRUE);
      if (whichelement!=-1)
	dictcombo->setCurrentItem(whichelement);
    }
  else
    setDictFromList (FALSE);

  sDictionary (dictFromList());
  sPathDictionary (!dictFromList());

}


void KSpellConfig::getAvailDictsIspell () {

  langfnames.clear();
  dictcombo->clear();
  langfnames.append(""); // Default
  dictcombo->insertItem (i18n("ISpell Default"));

  // dictionary path
  QFileInfo dir ("/usr/lib/ispell");
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/local/lib/ispell");
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/local/share/ispell");
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/share/ispell");
  /* TODO get them all instead of just one of them.
   * If /usr/local/lib exists, it skips the rest
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/local/lib");
  */
  if (!dir.exists() || !dir.isDir()) return;

  kdDebug(750) << "KSpellConfig::getAvailDictsIspell "
	       << dir.filePath() << " " << dir.dirPath() << endl;

  QDir thedir (dir.filePath(),"*.hash");

  kdDebug(750) << "KSpellConfig" << thedir.path() << "\n" << endl;
  kdDebug(750) << "entryList().count()="
	       << thedir.entryList().count() << endl;

  for (unsigned int i=0;i<thedir.entryList().count();i++)
    {
      QString fname, lname, hname;
      fname = thedir [i];

      // remove .hash
      if (fname.right(5) == ".hash") fname.remove (fname.length()-5,5);

      if (interpret (fname, lname, hname) && langfnames[0].isEmpty())
	{ // This one is the KDE default language
	  // so place it first in the lists (overwrite "Default")

   	  langfnames.remove ( langfnames.begin() );
	  langfnames.prepend ( fname );

	  hname=i18n("default spelling dictionary"
		     ,"Default - %1 [%2]").arg(hname).arg(fname);

	  dictcombo->changeItem (hname,0);
	}
      else
	{
	  langfnames.append (fname);
	  hname=hname+" ["+fname+"]";

	  dictcombo->insertItem (hname);
	}
    }
}

void KSpellConfig::getAvailDictsAspell () {

  langfnames.clear();
  dictcombo->clear();

  langfnames.append(""); // Default
  dictcombo->insertItem (i18n("ASpell Default"));

  // dictionary path
  // FIXME: use "aspell dump config" to find out the dict-dir
  QFileInfo dir ("/usr/lib/aspell");
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/local/lib/aspell");
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/share/aspell");
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/local/share/aspell");
  if (!dir.exists() || !dir.isDir()) return;

  kdDebug(750) << "KSpellConfig::getAvailDictsAspell "
	       << dir.filePath() << " " << dir.dirPath() << endl;

  QDir thedir (dir.filePath(),"*");

  kdDebug(750) << "KSpellConfig" << thedir.path() << "\n" << endl;
  kdDebug(750) << "entryList().count()="
	       << thedir.entryList().count() << endl;

  for (unsigned int i=0; i<thedir.entryList().count(); i++)
    {
      QString fname, lname, hname;
      fname = thedir [i];

      // consider only simple dicts without '-' in the name
      // FIXME: may be this is wrong an the list should contain
      // all *.multi files too, to allow using special dictionaries
      if (fname[0] != '.' &&  fname.find('-') < 0)
	{

	  // remove .multi
	  if (fname.right(6) == ".multi") fname.remove (fname.length()-6,6);

	  if (interpret (fname, lname, hname) && langfnames[0].isEmpty())
	    { // This one is the KDE default language
	      // so place it first in the lists (overwrite "Default")

	      langfnames.remove ( langfnames.begin() );
	      langfnames.prepend ( fname );

	      hname=i18n("default spelling dictionary"
			 ,"Default - %1 [%2]").arg(hname).arg(fname);

	      dictcombo->changeItem (hname,0);
	    }
	  else
	    {
	      langfnames.append (fname);
	      hname=hname+" ["+fname+"]";

	      dictcombo->insertItem (hname);
	    }
	}
    }
}

/*
 * Options setting routines.
 */

void
KSpellConfig::setClient (int c)
{
  iclient = c;

  if (clientcombo)
	  clientcombo->setCurrentItem(c);
}

void
KSpellConfig::setNoRootAffix (bool b)
{
  bnorootaffix=b;

  if(cb1)
	  cb1->setChecked(b);
}

void
KSpellConfig::setRunTogether(bool b)
{
  bruntogether=b;

  if(cb2)
	  cb2->setChecked(b);
}

void
KSpellConfig::setDictionary (const QString s)
{
  qsdict=s; //.copy();

  if (qsdict.length()>5)
    if ((signed)qsdict.find(".hash")==(signed)qsdict.length()-5)
      qsdict.remove (qsdict.length()-5,5);


  if(dictcombo)
  {
    int whichelement=-1;
    if (dictFromList())
    {
      for (unsigned int i=0;i<langfnames.count();i++)
      {
         if (langfnames[i] == s)
           whichelement=i;
      }

      if(whichelement >= 0)
      {
        dictcombo->setCurrentItem(whichelement);
      }
    }
  }


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

  if(encodingcombo)
    encodingcombo->setCurrentItem(enctype);
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
  emit configChanged();
}

void
KSpellConfig::sNoAff(bool)
{
  setNoRootAffix (cb1->isChecked());
  emit configChanged();
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
  emit configChanged();
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
  emit configChanged();
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
  emit configChanged();
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

void
KSpellConfig::setReplaceAllList (QStringList _replacelist)
{
  d->replacelist=_replacelist;
}

QStringList
KSpellConfig::replaceAllList () const
{
  return d->replacelist;
}

#include "ksconfig.moc"



