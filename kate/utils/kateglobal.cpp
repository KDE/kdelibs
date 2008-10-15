/* This file is part of the KDE libraries
   Copyright (C) 2001-2005 Christoph Cullmann <cullmann@kde.org>

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

#include "kateglobal.h"
#include "kateglobal.moc"

#include "katedocument.h"
#include "kateview.h"
#include "katerenderer.h"
#include "katecmds.h"
#include "katemodemanager.h"
#include "kateschema.h"
#include "kateconfig.h"
#include "katescriptmanager.h"
#include "katecmd.h"
#include "katebuffer.h"
#include "katepartpluginmanager.h"

#include <klocale.h>
#include <kservicetypetrader.h>
#include <kdirwatch.h>
#include <kdebug.h>
#include <kpagedialog.h>
#include <kpagewidgetmodel.h>
#include <kiconloader.h>

#include <QtGui/QBoxLayout>

KateGlobal *KateGlobal::s_self = 0;

int KateGlobal::s_ref = 0;

KateGlobal::KateGlobal ()
 : KTextEditor::Editor (0)
 , m_aboutData ("katepart", 0, ki18n("Kate Part"), KATEPART_VERSION,
             ki18n( "Embeddable editor component" ), KAboutData::License_LGPL_V2,
             ki18n( "(c) 2000-2007 The Kate Authors" ), KLocalizedString(), "http://www.kate-editor.org")
 , m_componentData (&m_aboutData)
{
  // set s_self
  s_self = this;

  // load the kate part translation catalog
  KGlobal::locale()->insertCatalog("katepart4");

  //
  // fill about data
  //
  m_aboutData.setProgramIconName("preferences-plugin");
  m_aboutData.addAuthor (ki18n("Christoph Cullmann"), ki18n("Maintainer"), "cullmann@kde.org", "http://www.babylon2k.de");
  m_aboutData.addAuthor (ki18n("Anders Lund"), ki18n("Core Developer"), "anders@alweb.dk", "http://www.alweb.dk");
  m_aboutData.addAuthor (ki18n("Joseph Wenninger"), ki18n("Core Developer"), "jowenn@kde.org","http://stud3.tuwien.ac.at/~e9925371");
  m_aboutData.addAuthor (ki18n("Hamish Rodda"), ki18n("Core Developer"), "rodda@kde.org");
  m_aboutData.addAuthor (ki18n("Dominik Haumann"), ki18n("Developer & Highlight wizard"), "dhdev@gmx.de");
  m_aboutData.addAuthor (ki18n("Waldo Bastian"), ki18n( "The cool buffersystem" ), "bastian@kde.org" );
  m_aboutData.addAuthor (ki18n("Charles Samuels"), ki18n("The Editing Commands"), "charles@kde.org");
  m_aboutData.addAuthor (ki18n("Matt Newell"), ki18n("Testing, ..."), "newellm@proaxis.com");
  m_aboutData.addAuthor (ki18n("Michael Bartl"), ki18n("Former Core Developer"), "michael.bartl1@chello.at");
  m_aboutData.addAuthor (ki18n("Michael McCallum"), ki18n("Core Developer"), "gholam@xtra.co.nz");
  m_aboutData.addAuthor (ki18n("Jochen Wilhemly"), ki18n( "KWrite Author" ), "digisnap@cs.tu-berlin.de" );
  m_aboutData.addAuthor (ki18n("Michael Koch"), ki18n("KWrite port to KParts"), "koch@kde.org");
  m_aboutData.addAuthor (ki18n("Christian Gebauer"), KLocalizedString(), "gebauer@kde.org" );
  m_aboutData.addAuthor (ki18n("Simon Hausmann"), KLocalizedString(), "hausmann@kde.org" );
  m_aboutData.addAuthor (ki18n("Glen Parker"), ki18n("KWrite Undo History, Kspell integration"), "glenebob@nwlink.com");
  m_aboutData.addAuthor (ki18n("Scott Manson"), ki18n("KWrite XML Syntax highlighting support"), "sdmanson@alltel.net");
  m_aboutData.addAuthor (ki18n("John Firebaugh"), ki18n("Patches and more"), "jfirebaugh@kde.org");
  m_aboutData.addAuthor (ki18n("Andreas Kling"), ki18n("Developer"), "kling@impul.se");
  m_aboutData.addAuthor (ki18n("Mirko Stocker"), ki18n("Various bugfixes"), "me@misto.ch", "http://misto.ch/");
  m_aboutData.addAuthor (ki18n("Matthew Woehlke"), ki18n("Selection, KColorScheme integration"), "mw_triad@users.sourceforge.net");
  m_aboutData.addAuthor (ki18n("Sebastian Pipping"), ki18n("Search bar back- and front-end"), "webmaster@hartwork.org", "http://www.hartwork.org/");

  m_aboutData.addCredit (ki18n("Matteo Merli"), ki18n("Highlighting for RPM Spec-Files, Perl, Diff and more"), "merlim@libero.it");
  m_aboutData.addCredit (ki18n("Rocky Scaletta"), ki18n("Highlighting for VHDL"), "rocky@purdue.edu");
  m_aboutData.addCredit (ki18n("Yury Lebedev"), ki18n("Highlighting for SQL"),"");
  m_aboutData.addCredit (ki18n("Chris Ross"), ki18n("Highlighting for Ferite"),"");
  m_aboutData.addCredit (ki18n("Nick Roux"), ki18n("Highlighting for ILERPG"),"");
  m_aboutData.addCredit (ki18n("Carsten Niehaus"), ki18n("Highlighting for LaTeX"),"");
  m_aboutData.addCredit (ki18n("Per Wigren"), ki18n("Highlighting for Makefiles, Python"),"");
  m_aboutData.addCredit (ki18n("Jan Fritz"), ki18n("Highlighting for Python"),"");
  m_aboutData.addCredit (ki18n("Daniel Naber"));
  m_aboutData.addCredit (ki18n("Roland Pabel"), ki18n("Highlighting for Scheme"),"");
  m_aboutData.addCredit (ki18n("Cristi Dumitrescu"), ki18n("PHP Keyword/Datatype list"),"");
  m_aboutData.addCredit (ki18n("Carsten Pfeiffer"), ki18n("Very nice help"), "");
  m_aboutData.addCredit (ki18n("Bruno Massa"), ki18n("Highlighting for Lua"), "brmassa@gmail.com");

  m_aboutData.addCredit (ki18n("All people who have contributed and I have forgotten to mention"));

  m_aboutData.setTranslator(ki18nc("NAME OF TRANSLATORS","Your names"), ki18nc("EMAIL OF TRANSLATORS","Your emails"));

  //
  // dir watch
  //
  m_dirWatch = new KDirWatch ();

  //
  // command manager
  //
  m_cmdManager = new KateCmd ();

  //
  // hl manager
  //
  m_hlManager = new KateHlManager ();

  //
  // mode man
  //
  m_modeManager = new KateModeManager ();

  //
  // schema man
  //
  m_schemaManager = new KateSchemaManager ();

  // config objects
  m_documentConfig = new KateDocumentConfig ();
  m_viewConfig = new KateViewConfig ();
  m_rendererConfig = new KateRendererConfig ();

  // create script manager (search scripts) + register commands
  m_scriptManager = new KateScriptManager ();
  KateCmd::self()->registerCommand (m_scriptManager);

  //
  // plugin manager
  //
  m_pluginManager = new KatePartPluginManager ();

  //
  // init the cmds
  //
  m_cmds.push_back (new KateCommands::CoreCommands());
  m_cmds.push_back (new KateCommands::SedReplace ());
  m_cmds.push_back (new KateCommands::Character ());
  m_cmds.push_back (new KateCommands::Date ());

  for ( QList<KTextEditor::Command *>::iterator it = m_cmds.begin(); it != m_cmds.end(); ++it )
    m_cmdManager->registerCommand (*it);
}

KateGlobal::~KateGlobal()
{
  delete m_pluginManager;

  delete m_documentConfig;
  delete m_viewConfig;
  delete m_rendererConfig;

  delete m_modeManager;
  delete m_schemaManager;

  delete m_dirWatch;

  // you too
  qDeleteAll (m_cmds);

  // cu managers
  delete m_scriptManager;
  delete m_hlManager;
  delete m_cmdManager;

  s_self = 0;
}

KTextEditor::Document *KateGlobal::createDocument ( QObject *parent )
{
  KateDocument *doc = new KateDocument (false, false, false, 0, parent);

  emit documentCreated (this, doc);

  return doc;
}

const QList<KTextEditor::Document*> &KateGlobal::documents ()
{
  return m_docs;
}

//BEGIN KTextEditor::Editor config stuff
void KateGlobal::readConfig(KConfig *config)
{
  if( !config )
    config = KGlobal::config().data();

  KateDocumentConfig::global()->readConfig (KConfigGroup(config, "Kate Document Defaults"));

  KateViewConfig::global()->readConfig (KConfigGroup(config, "Kate View Defaults"));

  KateRendererConfig::global()->readConfig (KConfigGroup(config, "Kate Renderer Defaults"));
}

void KateGlobal::writeConfig(KConfig *config)
{
  if( !config )
    config = KGlobal::config().data();

  KConfigGroup cg(config, "Kate Document Defaults");
  KateDocumentConfig::global()->writeConfig (cg);

  KConfigGroup cgDefault(config, "Kate View Defaults");
  KateViewConfig::global()->writeConfig (cgDefault);

  KConfigGroup cgRenderer(config, "Kate Renderer Defaults");
  KateRendererConfig::global()->writeConfig (cgRenderer);

  config->sync();
}
//END KTextEditor::Editor config stuff

bool KateGlobal::configDialogSupported () const
{
  return true;
}

void KateGlobal::configDialog(QWidget *parent)
{
  KPageDialog *kd = new KPageDialog(parent);
  kd->setCaption( i18n("Configure") );
  kd->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Help );
  kd->setFaceType( KPageDialog::List );

  QList<KTextEditor::ConfigPage*> editorPages;

  for (int i = 0; i < configPages (); ++i)
  {
    const QString name = configPageName (i);

    QFrame *page = new QFrame();

    KPageWidgetItem *item = kd->addPage( page, name );
    item->setHeader( configPageFullName (i) );
    item->setIcon( configPageIcon(i) );

    QVBoxLayout *topLayout = new QVBoxLayout( page );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( KDialog::spacingHint() );

    KTextEditor::ConfigPage *cp = configPage(i, page);
    topLayout->addWidget( cp);
    editorPages.append (cp);
  }

  if (kd->exec())
  {
    KateDocumentConfig::global()->configStart ();
    KateViewConfig::global()->configStart ();
    KateRendererConfig::global()->configStart ();

    for (int i=0; i < editorPages.count(); ++i)
    {
      editorPages.at(i)->apply();
    }

    KateDocumentConfig::global()->configEnd ();
    KateViewConfig::global()->configEnd ();
    KateRendererConfig::global()->configEnd ();

    writeConfig ();
  }

  delete kd;
}

int KateGlobal::configPages () const
{
  return 6;
}

KTextEditor::ConfigPage *KateGlobal::configPage (int number, QWidget *parent)
{
  switch( number )
  {
    case 0:
      return new KateViewDefaultsConfig (parent);

    case 1:
      return new KateSchemaConfigPage (parent, 0);

    case 2:
      return new KateEditConfigTab (parent);

    case 3:
      return new KateSaveConfigTab (parent);

    case 4:
      return new KateEditKeyConfiguration (parent);

    case 5:
      return new KatePartPluginConfigPage (parent);

    default:
      return 0;
  }

  return 0;
}

QString KateGlobal::configPageName (int number) const
{
  switch( number )
  {
    case 0:
      return i18n ("Appearance");

    case 1:
      return i18n ("Fonts & Colors");

    case 2:
      return i18n ("Editing");

    case 3:
      return i18n("Open/Save");

    case 4:
      return i18n ("Shortcuts");

    case 5:
      return i18n ("Extensions");

    default:
      return QString ("");
  }

  return QString ("");
}

QString KateGlobal::configPageFullName (int number) const
{
  switch( number )
  {
    case 0:
      return i18n("Appearance");

    case 1:
      return i18n ("Font & Color Schemas");

    case 2:
      return i18n ("Editing Options");

    case 3:
      return i18n("File Opening & Saving");

    case 4:
      return i18n ("Shortcuts Configuration");

    case 5:
      return i18n ("Extensions Manager");

    default:
      return QString ("");
  }

  return QString ("");
}

KIcon KateGlobal::configPageIcon (int number) const
{
  switch( number )
  {
    case 0:
      return KIcon("preferences-desktop-theme");

    case 1:
      return KIcon("preferences-desktop-color");

    case 2:
      return KIcon("accessories-text-editor");

    case 3:
      return KIcon("document-save");

    case 4:
      return KIcon("configure-shortcuts");

    case 5:
      return KIcon("preferences-plugin");

    default:
      return KIcon("document-properties");
  }

  return KIcon("document-properties");
}

KateGlobal *KateGlobal::self ()
{
  if (!s_self) {
    new KateGlobal ();
  }

  return s_self;
}

void KateGlobal::registerDocument ( KateDocument *doc )
{
  KateGlobal::incRef ();
  m_documents.append( doc );
  m_docs.append (doc);
}

void KateGlobal::deregisterDocument ( KateDocument *doc )
{
  m_docs.removeAll (doc);
  m_documents.removeAll( doc );
  KateGlobal::decRef ();
}

void KateGlobal::registerView ( KateView *view )
{
  KateGlobal::incRef ();
  m_views.append( view );
}

void KateGlobal::deregisterView ( KateView *view )
{
  m_views.removeAll( view );
  KateGlobal::decRef ();
}

//BEGIN command interface
bool KateGlobal::registerCommand (KTextEditor::Command *cmd)
{return m_cmdManager->registerCommand(cmd);}

bool KateGlobal::unregisterCommand (KTextEditor::Command *cmd)
{return m_cmdManager->unregisterCommand(cmd);}

KTextEditor::Command *KateGlobal::queryCommand (const QString &cmd) const
{return m_cmdManager->queryCommand(cmd);}

QList<KTextEditor::Command*> KateGlobal::commands() const
{return m_cmdManager->commands();}

QStringList KateGlobal::commandList() const
{return m_cmdManager->commandList();}
//END command interface


// kate: space-indent on; indent-width 2; replace-tabs on;
