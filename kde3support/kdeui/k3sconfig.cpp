/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>
   Copyright (C) 2000-2001 Wolfram Diestel <wolfram@steloj.de>
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

#include "k3sconfig.h"

#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <ktoolinvocation.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kguiitem.h>
#include <klineedit.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstandardguiitem.h>

#include <config.h>
#include <kconfiggroup.h>

class K3SpellConfigPrivate
{
public:
    QStringList replacelist;
};


K3SpellConfig::K3SpellConfig (const K3SpellConfig &_ksc)
  : QWidget(0), nodialog(true)
  , cb0(0)
  , cb1(0)
  , cb2(0)
  , dictlist(0)
  , dictcombo(0)
  , encodingcombo(0)
  , clientcombo(0)
  , d(new K3SpellConfigPrivate)
{
  kDebug(750) << "Entering K3SpellConfig::K3SpellConfig(K3SpellConfig&)";
  setDoSpellChecking( _ksc.doSpellChecking() );
  setReplaceAllList( _ksc.replaceAllList() );
  setNoRootAffix( _ksc.noRootAffix() );
  setRunTogether( _ksc.runTogether() );
  setDictionary( _ksc.dictionary() );
  setDictFromList( _ksc.dictFromList() );
  //  setPersonalDict (_ksc.personalDict());
  setIgnoreList( _ksc.ignoreList() );
  setEncoding( _ksc.encoding() );
  setClient( _ksc.client() );
}


K3SpellConfig::K3SpellConfig( QWidget *parent,
			    K3SpellConfig *_ksc, bool addHelpButton )
  : QWidget (parent), nodialog(false)
  , kc(0)
  , cb0(0)
  , cb1(0)
  , cb2(0)
  , dictlist(0)
  , dictcombo(0)
  , encodingcombo(0)
  , clientcombo(0)
  , d(new K3SpellConfigPrivate)
{
  kc = KGlobal::config();

  if( !_ksc )
  {
    readGlobalSettings();
  }
  else
  {
    setDoSpellChecking( _ksc->doSpellChecking() );
    setNoRootAffix( _ksc->noRootAffix() );
    setRunTogether( _ksc->runTogether() );
    setDictionary( _ksc->dictionary() );
    setDictFromList( _ksc->dictFromList() );
    //setPersonalDict (_ksc->personalDict());
    setIgnoreList( _ksc->ignoreList() );
    setEncoding( _ksc->encoding() );
    setClient( _ksc->client() );
  }

  QGridLayout *glay = new QGridLayout( this );
  glay->setSpacing( KDialog::spacingHint() );
  cb0 = new QCheckBox( i18nc("@option:check", "Do Spellchecking"), this );
  cb0->setObjectName( "DoSpellChecking" );
  connect( cb0, SIGNAL(toggled(bool)), SLOT(sDoSpell()) );
  cb1 = new QCheckBox( i18nc("@option:check",
                             "Create &root/affix combinations not in dictionary"), this );
  cb1->setObjectName( "NoRootAffix" );
  connect( cb1, SIGNAL(toggled(bool)), SLOT(sNoAff(bool)) );
  glay->addWidget( cb0, 0, 0, 1, 3 );
  glay->addWidget( cb1, 1, 0, 1, 3 );

  cb2 = new QCheckBox( i18nc("@option:check",
                             "Consider run-together &words as spelling errors"), this );
  cb2->setObjectName( "RunTogether" );
  connect( cb2, SIGNAL(toggled(bool)), SLOT(sRunTogether(bool)) );
  glay->addWidget( cb2, 2, 0, 1, 3 );

  dictcombo = new QComboBox( this );
  dictcombo->setObjectName( "DictFromList" );
  dictcombo->setInsertPolicy( QComboBox::NoInsert );
  connect( dictcombo, SIGNAL (activated(int)),
	   this, SLOT (sSetDictionary(int)) );
  glay->addWidget( dictcombo, 3, 1, 1, 2 );

  dictlist = new QLabel( i18nc("@label:listbox", "&Dictionary:"), this );
  dictlist->setBuddy( dictcombo );
  glay->addWidget( dictlist, 3 ,0 );

  encodingcombo = new QComboBox( this );
  encodingcombo->setObjectName( "Encoding" );
  encodingcombo->addItem( "US-ASCII" );
  encodingcombo->addItem( "ISO 8859-1" );
  encodingcombo->addItem( "ISO 8859-2" );
  encodingcombo->addItem( "ISO 8859-3" );
  encodingcombo->addItem( "ISO 8859-4" );
  encodingcombo->addItem( "ISO 8859-5" );
  encodingcombo->addItem( "ISO 8859-7" );
  encodingcombo->addItem( "ISO 8859-8" );
  encodingcombo->addItem( "ISO 8859-9" );
  encodingcombo->addItem( "ISO 8859-13" );
  encodingcombo->addItem( "ISO 8859-15" );
  encodingcombo->addItem( "UTF-8" );
  encodingcombo->addItem( "KOI8-R" );
  encodingcombo->addItem( "KOI8-U" );
  encodingcombo->addItem( "CP1251" );
  encodingcombo->addItem( "CP1255" );

  connect( encodingcombo, SIGNAL(activated(int)), this,
	   SLOT(sChangeEncoding(int)) );
  glay->addWidget( encodingcombo, 4, 1, 1, 2 );

  QLabel *tmpQLabel = new QLabel( i18nc("@label:listbox", "&Encoding:"), this);
  tmpQLabel->setBuddy( encodingcombo );
  glay->addWidget( tmpQLabel, 4, 0 );


  clientcombo = new QComboBox( this );
  clientcombo->setObjectName( "Client" );
  clientcombo->addItem( i18nc("@item:inlistbox Spell checker", "International <application>Ispell</application>") );
  clientcombo->addItem( i18nc("@item:inlistbox Spell checker", "<application>Aspell</application>") );
  clientcombo->addItem( i18nc("@item:inlistbox Spell checker", "<application>Hspell</application>") );
  clientcombo->addItem( i18nc("@item:inlistbox Spell checker", "<application>Zemberek</application>") );
  clientcombo->addItem( i18nc("@item:inlistbox Spell checker", "<application>Hunspell</application>") );
  connect( clientcombo, SIGNAL (activated(int)), this,
	   SLOT (sChangeClient(int)) );
  glay->addWidget( clientcombo, 5, 1, 1, 2 );

  tmpQLabel = new QLabel( i18nc("@label:listbox", "&Client:"), this );
  tmpQLabel->setBuddy( clientcombo );
  glay->addWidget( tmpQLabel, 5, 0 );

  if( addHelpButton )
  {
    QPushButton *pushButton = new KPushButton( KStandardGuiItem::help(), this );
    connect( pushButton, SIGNAL(clicked()), this, SLOT(sHelp()) );
    glay->addWidget(pushButton, 6, 2);
  }

  fillInDialog();
}

K3SpellConfig::~K3SpellConfig()
{
  delete d;
}


bool
K3SpellConfig::dictFromList() const
{
  return dictfromlist;
}

bool
K3SpellConfig::readGlobalSettings()
{
  kDebug(750) << "Entering K3SpellConfig::readGlobalSettings (see k3sconfig.cpp)";
  KConfigGroup cg( kc,"K3Spell" );

  setDoSpellChecking ( cg.readEntry("K3Spell_DoSpellChecking", false ) );
  setNoRootAffix   ( cg.readEntry("K3Spell_NoRootAffix", 0) );
  setRunTogether   ( cg.readEntry("K3Spell_RunTogether", 0) );
  setDictionary    ( cg.readEntry("K3Spell_Dictionary") );
  setDictFromList  ( cg.readEntry("K3Spell_DictFromList", 0) );
  setEncoding ( cg.readEntry ("K3Spell_Encoding", int(KS_E_ASCII)) );
  setClient ( cg.readEntry ("K3Spell_Client", int(KS_CLIENT_ISPELL)) );

  return true;
}

bool
K3SpellConfig::writeGlobalSettings ()
{
  KConfigGroup cg( kc,"K3Spell" );

  KConfigBase::WriteConfigFlags flags(KConfigBase::Global|KConfigBase::Persistent);
  cg.writeEntry ("K3Spell_DoSpellChecking", doSpellChecking(), flags);
  cg.writeEntry ("K3Spell_NoRootAffix",(int) noRootAffix(), flags);
  cg.writeEntry ("K3Spell_RunTogether", (int) runTogether(), flags);
  cg.writeEntry ("K3Spell_Dictionary", dictionary(), flags);
  cg.writeEntry ("K3Spell_DictFromList",(int) dictFromList(), flags);
  cg.writeEntry ("K3Spell_Encoding", (int) encoding(), flags);
  cg.writeEntry ("K3Spell_Client", client(), flags);
  kc->sync();

  return true;
}

void
K3SpellConfig::sChangeEncoding( int i )
{
  kDebug(750) << "K3SpellConfig::sChangeEncoding(" << i << ")";
  setEncoding( i );
  emit configChanged();
}

void
K3SpellConfig::sChangeClient( int i )
{
  setClient( i );

  // read in new dict list
  if ( dictcombo ) {
    if ( iclient == KS_CLIENT_ISPELL )
      getAvailDictsIspell();
    else if ( iclient == KS_CLIENT_HSPELL )
    {
      langfnames.clear();
      dictcombo->clear();
      dictcombo->addItem( i18nc("@item Spelling dictionary", "Hebrew") );
      sChangeEncoding( KS_E_CP1255 );
    } else if ( iclient == KS_CLIENT_ZEMBEREK ) {
      langfnames.clear();
      dictcombo->clear();
      dictcombo->addItem( i18nc("@item Spelling dictionary", "Turkish") );
      sChangeEncoding( KS_E_UTF8 );
    }
    else if ( iclient == KS_CLIENT_ASPELL )
      getAvailDictsAspell();
    else
      getAvailDictsHunspell();
  }
  emit configChanged();
}

bool
K3SpellConfig::interpret( const QString &fname, QString &lname,
                         QString &hname )

{

  kDebug(750) << "K3SpellConfig::interpret [" << fname << "]";

  QString dname( fname );

  if( dname.endsWith( '+' ) )
    dname.remove( dname.length()-1, 1 );

  if( dname.endsWith("sml") || dname.endsWith("med") || dname.endsWith("lrg") ||
      dname.endsWith("xlg"))
     dname.remove(dname.length()-3,3);

  QString extension;

  int i = dname.indexOf('-');
  if ( i != -1 )
  {
    extension = dname.mid(i+1);
    dname.truncate(i);
  }

  // Aspell uses 2 alpha language codes or 2 alpha language + 2 alpha country,
  // but since aspell 0.6 also 3-character ISO-codes can be used
  if ( (dname.length() == 2) || (dname.length() == 3) ) {
    lname = dname;
    hname = KGlobal::locale()->languageCodeToName( lname );
  }
  else if ( (dname.length() == 5) && (dname[2] == '_') ) {
    lname = dname.left(2);
    hname = KGlobal::locale()->languageCodeToName( lname );
    QString country = KGlobal::locale()->countryCodeToName( dname.right(2) );
    if ( extension.isEmpty() )
      extension = country;
    else
      extension = country + " - " + extension;
  }
  //These are mostly the ispell-langpack defaults
  else if ( dname=="english" || dname=="american" ||
            dname=="british" || dname=="canadian" ) {
    lname="en"; hname=i18nc("@item Spelling dictionary", "English");
  }
  else if ( dname == "espa~nol" || dname == "espanol" ) {
    lname="es"; hname=i18nc("@item Spelling dictionary", "Spanish");
  }
  else if (dname=="dansk") {
    lname="da"; hname=i18nc("@item Spelling dictionary", "Danish");
  }
  else if (dname=="deutsch") {
    lname="de"; hname=i18nc("@item Spelling dictionary", "German");
  }
  else if (dname=="german") {
    lname="de"; hname=i18nc("@item Spelling dictionary", "German (new spelling)");
  }
  else if (dname=="portuguesb" || dname=="br") {
    lname="br"; hname=i18nc("@item Spelling dictionary", "Brazilian Portuguese");
  }
  else if (dname=="portugues") {
    lname="pt"; hname=i18nc("@item Spelling dictionary", "Portuguese");
  }
  else if (dname=="esperanto") {
    lname="eo"; hname=i18nc("@item Spelling dictionary", "Esperanto");
  }
  else if (dname=="norsk") {
    lname="no"; hname=i18nc("@item Spelling dictionary", "Norwegian");
  }
  else if (dname=="polish") {
    lname="pl"; hname=i18nc("@item Spelling dictionary", "Polish"); sChangeEncoding(KS_E_LATIN2);
  }
  else if (dname=="russian") {
    lname="ru"; hname=i18nc("@item Spelling dictionary", "Russian");
  }
  else if (dname=="slovensko") {
    lname="si"; hname=i18nc("@item Spelling dictionary", "Slovenian"); sChangeEncoding(KS_E_LATIN2);
  }
  else if (dname=="slovak"){
    lname="sk"; hname=i18nc("@item Spelling dictionary", "Slovak"); sChangeEncoding(KS_E_LATIN2);
  }
  else if (dname=="czech") {
    lname="cs"; hname=i18nc("@item Spelling dictionary", "Czech"); sChangeEncoding(KS_E_LATIN2);
  }
  else if (dname=="svenska") {
    lname="sv"; hname=i18nc("@item Spelling dictionary", "Swedish");
  }
  else if (dname=="swiss") {
    lname="de"; hname=i18nc("@item Spelling dictionary", "Swiss German");
  }
  else if (dname=="ukrainian") {
    lname="uk"; hname=i18nc("@item Spelling dictionary", "Ukrainian");
  }
  else if (dname=="lietuviu" || dname=="lithuanian") {
     lname="lt"; hname=i18nc("@item Spelling dictionary", "Lithuanian");
  }
  else if (dname=="francais" || dname=="french") {
    lname="fr"; hname=i18nc("@item Spelling dictionary", "French");
  }
  else if (dname=="belarusian") {  // waiting for post 2.2 to not dissapoint translators
    lname="be"; hname=i18nc("@item Spelling dictionary", "Belarusian");
  }
  else if( dname == "magyar" ) {
    lname="hu"; hname=i18nc("@item Spelling dictionary", "Hungarian");
    sChangeEncoding(KS_E_LATIN2);
  }
  else {
    lname=""; hname=i18nc("@item Spelling dictionary", "Unknown");
  }
  if (!extension.isEmpty())
  {
    hname = hname + " (" + extension + ')';
  }

  //We have explicitly chosen English as the default here.
  if ( ( KGlobal::locale()->language() == QLatin1String("C") &&
         lname==QLatin1String("en") ) ||
       KGlobal::locale()->language() == lname )
    return true;

  return false;
}

void
K3SpellConfig::fillInDialog ()
{
  if ( nodialog )
    return;

  kDebug(750) << "K3SpellConfig::fillinDialog";

  cb0->setChecked( doSpellChecking() );
  cb1->setChecked( noRootAffix() );
  cb2->setChecked( runTogether() );
  encodingcombo->setCurrentIndex( encoding() );
  clientcombo->setCurrentIndex( client() );

  // get list of available dictionaries
  if ( iclient == KS_CLIENT_ISPELL )
    getAvailDictsIspell();
  else if ( iclient == KS_CLIENT_HSPELL )
  {
    langfnames.clear();
    dictcombo->clear();
    langfnames.append(""); // Default
    dictcombo->addItem( i18nc("@item Spelling dictionary", "Hebrew") );
  } else if ( iclient == KS_CLIENT_ZEMBEREK ) {
    langfnames.clear();
    dictcombo->clear();
    langfnames.append("");
    dictcombo->addItem( i18nc("@item Spelling dictionary", "Turkish") );
  }
  else if ( iclient == KS_CLIENT_ASPELL )
    getAvailDictsAspell();
  else
    getAvailDictsHunspell();

  // select the used dictionary in the list
  int whichelement=-1;

  if ( dictFromList() )
    whichelement = langfnames.indexOf(dictionary());

  dictcombo->setMinimumWidth (dictcombo->sizeHint().width());

  if (dictionary().isEmpty() ||  whichelement!=-1)
  {
    setDictFromList (true);
    if (whichelement!=-1)
      dictcombo->setCurrentIndex(whichelement);
  }
  else
    // Current dictionary vanished, present the user with a default if possible.
    if ( !langfnames.empty() )
    {
      setDictFromList( true );
      dictcombo->setCurrentIndex(0);
    }
    else
      setDictFromList( false );

  sDictionary( dictFromList() );
  sPathDictionary( !dictFromList() );

}


void K3SpellConfig::getAvailDictsIspell () {

  langfnames.clear();
  dictcombo->clear();
  langfnames.append(""); // Default
  dictcombo->addItem( i18nc("@item Spelling dictionary",
                            "<application>ISpell</application> Default") );

  // dictionary path
  QFileInfo dir ("/usr/lib/ispell");
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/local/lib/ispell");
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/local/share/ispell");
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/share/ispell");
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/pkg/lib");
  /* TODO get them all instead of just one of them.
   * If /usr/local/lib exists, it skips the rest
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/local/lib");
  */
  if (!dir.exists() || !dir.isDir()) return;

  kDebug(750) << "K3SpellConfig::getAvailDictsIspell "
	       << dir.filePath() << " " << dir.path() << endl;

  const QDir thedir (dir.filePath(),"*.hash");
  const QStringList entryList = thedir.entryList();

  kDebug(750) << "K3SpellConfig" << thedir.path() << "\n";
  kDebug(750) << "entryList().count()="
	       << entryList.count() << endl;

  QStringList::const_iterator entryListItr = entryList.constBegin();
  const QStringList::const_iterator entryListEnd = entryList.constEnd();

  for ( ; entryListItr != entryListEnd; ++entryListItr)
  {
    QString fname, lname, hname;
    fname = *entryListItr;

    // remove .hash
    if (fname.endsWith(".hash")) fname.remove (fname.length()-5,5);

    if (interpret (fname, lname, hname) && langfnames.first().isEmpty())
    { // This one is the KDE default language
      // so place it first in the lists (overwrite "Default")

      langfnames.removeFirst();
      langfnames.prepend ( fname );

      hname=i18nc("@item Spelling dictionary: %1 dictionary name, %2 file name",
                  "Default - %1 [%2]", hname, fname);

      dictcombo->setItemText (0,hname);
    }
    else
    {
      langfnames.append (fname);
      hname=hname+" ["+fname+']';

      dictcombo->addItem (hname);
    }
  }
}

void K3SpellConfig::getAvailDictsAspell () {

  langfnames.clear();
  dictcombo->clear();

  langfnames.append(""); // Default
  dictcombo->addItem (i18nc("@item Spelling dictionary",
                            "<application>ASpell</application> Default"));

  // dictionary path
  // FIXME: use "aspell dump config" to find out the dict-dir
  QFileInfo dir ("/usr/lib" KDELIBSUFF "/aspell");
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/lib" KDELIBSUFF "/aspell-0.60");
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/local/lib" KDELIBSUFF "/aspell");
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/share/aspell");
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/local/share/aspell");
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/pkg/lib/aspell");
  if (!dir.exists() || !dir.isDir()) return;

  kDebug(750) << "K3SpellConfig::getAvailDictsAspell "
	       << dir.filePath() << " " << dir.path() << endl;

  const QDir thedir (dir.filePath(),"*");
  const QStringList entryList = thedir.entryList();

  kDebug(750) << "K3SpellConfig" << thedir.path() << "\n";
  kDebug(750) << "entryList().count()="
	       << entryList.count() << endl;

  QStringList::const_iterator entryListItr = entryList.constBegin();
  const QStringList::const_iterator entryListEnd = entryList.constEnd();

  for ( ; entryListItr != entryListEnd; ++entryListItr)
  {
    QString fname, lname, hname;
    fname = *entryListItr;

    // consider only simple dicts without '-' in the name
    // FIXME: may be this is wrong an the list should contain
    // all *.multi files too, to allow using special dictionaries

    // Well, K3Spell2 has a better way to do this, but this code has to be
    // cleaned up somehow: since aspell 0.6 we have quite a lot of files in the
    // aspell dictionary that are not dictionaries. These must not be presented as "languages"
    // We only keep
    // *.rws: dictionary
    // *.multi: definition file to load several subdictionaries
    if ( !( fname.endsWith(".rws") || fname.endsWith(".multi") ) ) {
        // remove noise from the language list
      continue;
    }
    if (fname[0] != '.')
    {

      // remove .multi
      if (fname.endsWith(".multi")) fname.remove (fname.length()-6,6);
      // remove .rws
      if (fname.endsWith(".rws")) fname.remove (fname.length()-4,4);

      if (interpret (fname, lname, hname) && langfnames.first().isEmpty())
      { // This one is the KDE default language
        // so place it first in the lists (overwrite "Default")

        langfnames.erase ( langfnames.begin() );
        langfnames.prepend ( fname );

        hname=i18nc("@item Spelling dictionary: %1 dictionary name",
                    "Default - %1", hname);

        dictcombo->setItemText (0,hname);
      }
      else
      {
        langfnames.append (fname);
        dictcombo->addItem (hname);
      }
    }
  }
}

void K3SpellConfig::getAvailDictsHunspell () {

  langfnames.clear();
  dictcombo->clear();
  langfnames.append(""); // Default
  dictcombo->addItem( i18nc("@item Spelling dictionary",
                            "<application>Hunspell</application> Default") );

  // dictionary path
  QFileInfo dir ("/usr/share/myspell");
  if (!dir.exists() || !dir.isDir())
    dir.setFile ("/usr/share/hunspell");
  if (!dir.exists() || !dir.isDir()) return;

  kDebug(750) << "K3SpellConfig::getAvailDictsHunspell "
	       << dir.filePath() << " " << dir.path() << endl;

  const QDir thedir (dir.filePath(),"*.dic");
  const QStringList entryList = thedir.entryList();

  kDebug(750) << "K3SpellConfig" << thedir.path() << "\n";
  kDebug(750) << "entryList().count()="
	       << entryList.count() << endl;

  QStringList::const_iterator entryListItr = entryList.constBegin();
  const QStringList::const_iterator entryListEnd = entryList.constEnd();

  for ( ; entryListItr != entryListEnd; ++entryListItr)
  {
    QString fname, lname, hname;
    fname = *entryListItr;

    // remove .dic
    if (fname.endsWith(".dic")) fname.remove (fname.length()-4,4);

    if (interpret (fname, lname, hname) && langfnames.first().isEmpty())
    { // This one is the KDE default language
      // so place it first in the lists (overwrite "Default")

      langfnames.removeFirst();
      langfnames.prepend ( fname );

      hname=i18nc("@item Spelling dictionary: %1 dictionary name, %2 file name",
                  "Default - %1 [%2]", hname, fname);

      dictcombo->setItemText (0,hname);
    }
    else
    {
      langfnames.append (fname);
      hname=hname+" ["+fname+']';

      dictcombo->addItem (hname);
    }
  }
}

void
K3SpellConfig::fillDicts( QComboBox* box, QStringList* dictionaries )
{
  langfnames.clear();
  if ( box ) {
    if ( iclient == KS_CLIENT_ISPELL ) {
      box->clear();
      langfnames.append(""); // Default
      box->addItem( i18nc("@item Spelling dictionary",
                          "<application>ISpell</application> Default") );

      // dictionary path
      QFileInfo dir ("/usr/lib/ispell");
      if (!dir.exists() || !dir.isDir())
        dir.setFile ("/usr/local/lib/ispell");
      if (!dir.exists() || !dir.isDir())
        dir.setFile ("/usr/local/share/ispell");
      if (!dir.exists() || !dir.isDir())
        dir.setFile ("/usr/share/ispell");
      if (!dir.exists() || !dir.isDir())
        dir.setFile ("/usr/pkg/lib");
      /* TODO get them all instead of just one of them.
       * If /usr/local/lib exists, it skips the rest
       if (!dir.exists() || !dir.isDir())
       dir.setFile ("/usr/local/lib");
      */
      if (!dir.exists() || !dir.isDir()) return;

      kDebug(750) << "K3SpellConfig::getAvailDictsIspell "
                   << dir.filePath() << " " << dir.path() << endl;

      const QDir thedir (dir.filePath(),"*.hash");
      const QStringList entryList = thedir.entryList();

      kDebug(750) << "K3SpellConfig" << thedir.path() << "\n";
      kDebug(750) << "entryList().count()="
                   << entryList.count() << endl;

      QStringList::const_iterator entryListItr = entryList.constBegin();
      const QStringList::const_iterator entryListEnd = entryList.constEnd();

      for ( ; entryListItr != entryListEnd; ++entryListItr)
      {
        QString fname, lname, hname;
        fname = *entryListItr;

        // remove .hash
        if (fname.endsWith(".hash")) fname.remove (fname.length()-5,5);

        if (interpret (fname, lname, hname) && langfnames.first().isEmpty())
        { // This one is the KDE default language
          // so place it first in the lists (overwrite "Default")

          langfnames.erase ( langfnames.begin() );
          langfnames.prepend ( fname );

          hname=i18nc("@item Spelling dictionary: %1 dictionary name, %2 file name",
                      "Default - %1 [%2]", hname, fname);

          box->setItemText (0,hname);
        }
        else
        {
          langfnames.append (fname);
          hname=hname+" ["+fname+']';

          box->addItem (hname);
        }
      }
    } else if ( iclient == KS_CLIENT_HSPELL ) {
      box->clear();
      box->addItem( i18nc("@item Spelling dictionary", "Hebrew") );
      langfnames.append(""); // Default
      sChangeEncoding( KS_E_CP1255 );
    } else if ( iclient == KS_CLIENT_ZEMBEREK ) {
      box->clear();
      box->addItem( i18nc("@item Spelling dictionary", "Turkish") );
      langfnames.append("");
      sChangeEncoding( KS_E_UTF8 );
    } else if ( iclient == KS_CLIENT_ASPELL ) {
      box->clear();
      langfnames.append(""); // Default
      box->addItem (i18nc("@item Spelling dictionary",
                          "<application>ASpell</application> Default"));

      // dictionary path
      // FIXME: use "aspell dump config" to find out the dict-dir
      QFileInfo dir ("/usr/lib" KDELIBSUFF "/aspell");
      if (!dir.exists() || !dir.isDir())
        dir.setFile ("/usr/lib" KDELIBSUFF "/aspell-0.60");
      if (!dir.exists() || !dir.isDir())
        dir.setFile ("/usr/local/lib" KDELIBSUFF "/aspell");
      if (!dir.exists() || !dir.isDir())
        dir.setFile ("/usr/share/aspell");
      if (!dir.exists() || !dir.isDir())
        dir.setFile ("/usr/local/share/aspell");
      if (!dir.exists() || !dir.isDir())
        dir.setFile ("/usr/pkg/lib/aspell");
      if (!dir.exists() || !dir.isDir()) return;

      kDebug(750) << "K3SpellConfig::getAvailDictsAspell "
                   << dir.filePath() << " " << dir.path() << endl;

      const QDir thedir (dir.filePath(),"*");
      const QStringList entryList = thedir.entryList();

      kDebug(750) << "K3SpellConfig" << thedir.path() << "\n";
      kDebug(750) << "entryList().count()="
                   << entryList.count() << endl;

      QStringList::const_iterator entryListItr = entryList.constBegin();
      const QStringList::const_iterator entryListEnd = entryList.constEnd();

      for ( ; entryListItr != entryListEnd; ++entryListItr)
      {
        QString fname, lname, hname;
        fname = *entryListItr;

        // consider only simple dicts without '-' in the name
        // FIXME: may be this is wrong an the list should contain
        // all *.multi files too, to allow using special dictionaries

        // Well, K3Spell2 has a better way to do this, but this code has to be
        // cleaned up somehow: since aspell 0.6 we have quite a lot of files in the
        // aspell dictionary that are not dictionaries. These must not be presented as "languages"
        // We only keep
        // *.rws: dictionary
        // *.multi: definition file to load several subdictionaries
        if ( !( fname.endsWith(".rws") || fname.endsWith(".multi") ) ) {
            // remove noise from the language list
          continue;
        }
        if (fname[0] != '.')
        {

          // remove .multi
          if (fname.endsWith(".multi")) fname.remove (fname.length()-6,6);
          // remove .rws
          if (fname.endsWith(".rws")) fname.remove (fname.length()-4,4);

          if (interpret (fname, lname, hname) && langfnames.first().isEmpty())
          { // This one is the KDE default language
            // so place it first in the lists (overwrite "Default")

            langfnames.erase ( langfnames.begin() );
            langfnames.prepend ( fname );

            hname=i18nc("@item Spelling dictionary: %1 dictionary name",
                        "Default - %1", hname);

            box->setItemText (0,hname);
          }
          else
          {
            langfnames.append (fname);
            box->addItem (hname);
          }
        }
      }
    } else {
      box->clear();
      langfnames.append(""); // Default
      box->addItem( i18nc("@item Spelling dictionary",
                          "<application>Hunspell</application> Default") );

      // dictionary path
      QFileInfo dir ("/usr/share/myspell");
      if (!dir.exists() || !dir.isDir())
        dir.setFile ("/usr/share/hunspell");
      if (!dir.exists() || !dir.isDir()) return;

      kDebug(750) << "K3SpellConfig::getAvailDictsHunspell "
                   << dir.filePath() << " " << dir.path() << endl;

      const QDir thedir (dir.filePath(),"*.dic");
      const QStringList entryList = thedir.entryList();

      kDebug(750) << "K3SpellConfig" << thedir.path() << "\n";
      kDebug(750) << "entryList().count()="
                   << entryList.count() << endl;

      QStringList::const_iterator entryListItr = entryList.constBegin();
      const QStringList::const_iterator entryListEnd = entryList.constEnd();

      for ( ; entryListItr != entryListEnd; ++entryListItr)
      {
        QString fname, lname, hname;
        fname = *entryListItr;

        // remove .dic
        if (fname.endsWith(".dic")) fname.remove (fname.length()-4,4);

        if (interpret (fname, lname, hname) && langfnames.first().isEmpty())
        { // This one is the KDE default language
          // so place it first in the lists (overwrite "Default")

          langfnames.erase ( langfnames.begin() );
          langfnames.prepend ( fname );

          hname=i18nc("@item Spelling dictionary: %1 dictionary name, %2 file name",
                      "Default - %1 [%2]", hname, fname);

          box->setItemText (0,hname);
        }
        else
        {
          langfnames.append (fname);
          hname=hname+" ["+fname+']';

          box->addItem (hname);
        }
      }
    }
    int whichelement = langfnames.indexOf(qsdict);
    if ( whichelement >= 0 ) {
      box->setCurrentIndex( whichelement );
    }
    if ( dictionaries )
      *dictionaries = langfnames;
  }
}

/*
 * Options setting routines.
 */

void
K3SpellConfig::setClient (int c)
{
  iclient = c;

  if (clientcombo)
    clientcombo->setCurrentIndex(c);
}

void
K3SpellConfig::setDoSpellChecking (bool b)
{
  bdospellchecking=b;

  if(cb0)
    cb0->setChecked(b);
}

void
K3SpellConfig::setNoRootAffix (bool b)
{
  bnorootaffix=b;

  if(cb1)
    cb1->setChecked(b);
}

void
K3SpellConfig::setRunTogether(bool b)
{
  bruntogether=b;

  if(cb2)
    cb2->setChecked(b);
}

void
K3SpellConfig::setDictionary (const QString s)
{
  qsdict=s; //.copy();

  if (qsdict.length()>5)
    if ((signed)qsdict.indexOf(".hash")==(signed)qsdict.length()-5)
      qsdict.remove (qsdict.length()-5,5);


  if(dictcombo)
  {
    int whichelement=-1;
    if (dictFromList())
    {
      whichelement = langfnames.indexOf(s);

      if(whichelement >= 0)
      {
        dictcombo->setCurrentIndex(whichelement);
      }
    }
  }


}

void
K3SpellConfig::setDictFromList (bool dfl)
{
  //  kdebug (KDEBUG_INFO, 750, "sdfl = %d", dfl);
  dictfromlist=dfl;
}

/*
void K3SpellConfig::setPersonalDict (const char *s)
{
  qspdict=s;
}
*/

void
K3SpellConfig::setEncoding (int enctype)
{
  enc=enctype;

  if(encodingcombo)
    encodingcombo->setCurrentIndex(enctype);
}

/*
  Options reading routines.
 */
int
K3SpellConfig::client () const
{
  return iclient;
}


bool
K3SpellConfig::doSpellChecking () const
{
  return bdospellchecking;
}

bool
K3SpellConfig::noRootAffix () const
{
  return bnorootaffix;
}

bool
K3SpellConfig::runTogether() const
{
  return bruntogether;
}

const
QString K3SpellConfig::dictionary () const
{
  return qsdict;
}

/*
const QString K3SpellConfig::personalDict () const
{
  return qspdict;
}
*/

int
K3SpellConfig::encoding () const
{
  return enc;
}

void
K3SpellConfig::sRunTogether(bool)
{
  setRunTogether (cb2->isChecked());
  emit configChanged();
}

void
K3SpellConfig::sNoAff(bool)
{
  setNoRootAffix (cb1->isChecked());
  emit configChanged();
}

void
K3SpellConfig::sDoSpell()
{
  setDoSpellChecking (cb0->isChecked());
  emit configChanged();
}

/*
void
K3SpellConfig::sBrowseDict()
{
  return;

  QString qs( KFileDialog::getOpenFileName ("/usr/local/lib","*.hash") );
  if ( !qs.isNull() )
    kle1->setText (qs);

}
*/

/*
void K3SpellConfig::sBrowsePDict()
{
  //how do I find home directory path??
  QString qs( KFileDialog::getOpenFileName ("",".ispell_*") );
  if ( !qs.isNull() )
      kle2->setText (qs);


}
*/

void
K3SpellConfig::sSetDictionary (int i)
{
  setDictionary (langfnames[i]);
  setDictFromList (true);
  emit configChanged();
}

void
K3SpellConfig::sDictionary(bool on)
{
  if (on)
  {
    dictcombo->setEnabled (true);
    setDictionary (langfnames[dictcombo->currentIndex()] );
    setDictFromList (true);
  }
  else
  {
    dictcombo->setEnabled (false);
  }
  emit configChanged();
}

void
K3SpellConfig::sPathDictionary(bool on)
{
  return; //enough for now


  if (on)
  {
    //kle1->setEnabled (true);
    //      browsebutton1->setEnabled (true);
    //setDictionary (kle1->text());
    setDictFromList (false);
  }
  else
  {
    //kle1->setEnabled (false);
    //browsebutton1->setEnabled (false);
  }
  emit configChanged();
}


void K3SpellConfig::activateHelp( void )
{
  sHelp();
}

void K3SpellConfig::sHelp( void )
{
  KToolInvocation::invokeHelp("configuration", "k3spell");
}

/*
void K3SpellConfig::textChanged1 (const char *s)
{
  setDictionary (s);
}

void K3SpellConfig::textChanged2 (const char *)
{
  //  setPersonalDict (s);
}
*/

void
K3SpellConfig::operator= (const K3SpellConfig &ksc)
{
  //We want to copy the data members, but not the
  //pointers to the child widgets
  setDoSpellChecking (ksc.doSpellChecking());
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
K3SpellConfig::setIgnoreList (const QStringList &_ignorelist)
{
  ignorelist=_ignorelist;
}

QStringList
K3SpellConfig::ignoreList () const
{
  return ignorelist;
}

void
K3SpellConfig::setReplaceAllList (const QStringList &_replacelist)
{
  d->replacelist=_replacelist;
}

QStringList
K3SpellConfig::replaceAllList() const
{
  return d->replacelist;
}

#include "k3sconfig.moc"



