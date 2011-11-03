/* -*- C++ -*-
 * This file shows an example for the KDE about dialog.
 *
 * Copyright 1999 Mirko Boehm <mirko@kde.org>
 * Licensed under the GNU General Public License version 2
 * mail to:    Mirko Boehm <mirko@kde.org>
 * requires:   recent C++-compiler, at least Qt 1.4
 */

#include <QtGui/QImage>
#include <QtGui/QPixmap>

#include "k3aboutdialog.h"

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <k3aboutapplication.h>
#include <kapplication.h>
#include <klocale.h>

int main(int argc, char** argv)
{
  KAboutData aboutData("K3AboutDialogTest", 0, qi18n("K3AboutDialogTest"), "version", qi18n("K3AboutDialog Test, Enjoy!"), KAboutData::License_GPL, qi18n("(c) 1997-2006, The KDE Developers"));
  aboutData.addAuthor(qi18n("Bob Dobbs"), qi18n("Current Maintainer"), "bob@subgenius.com");
  aboutData.addAuthor(qi18n("Gregory S. Hayes"), qi18n("Developer"), "syncomm@gmail.com");
  aboutData.addAuthor(qi18n("Random Authors from KFind"));
  aboutData.addAuthor(qi18n("Beppe Grimaldi"), qi18n("UI Design & more search options"), "grimalkin@ciaoweb.it");
  aboutData.addAuthor(qi18n("Martin Hartig"));
  aboutData.addAuthor(qi18n("Stephan Kulow"), QLocalizedString(), "coolo@kde.org");
  aboutData.addAuthor(qi18n("Mario Weilguni"), QLocalizedString(), "mweilguni@sime.com");
  aboutData.addAuthor(qi18n("Alex Zepeda"), QLocalizedString(), "zipzippy@sonic.net");
  aboutData.addAuthor(qi18n("Miroslav Fl�r"), QLocalizedString(), "flidr@kky.zcu.cz");
  aboutData.addAuthor(qi18n("Harri Porten"), QLocalizedString(), "porten@kde.org");
  aboutData.addAuthor(qi18n("Dima Rogozin"), QLocalizedString(), "dima@mercury.co.il");
  aboutData.addAuthor(qi18n("Carsten Pfeiffer"), QLocalizedString(), "pfeiffer@kde.org");
  aboutData.addAuthor(qi18n("Hans Petter Bieker"), QLocalizedString(), "bieker@kde.org");
  aboutData.addAuthor(qi18n("Waldo Bastian"), qi18n("UI Design"), "bastian@kde.org");
  aboutData.addAuthor(qi18n("Alexander Neundorf"), QLocalizedString(), "neundorf@kde.org");
  aboutData.addAuthor(qi18n("Clarence Dang"), QLocalizedString(), "dang@kde.org");

  KCmdLineArgs::init(argc, argv, &aboutData);

  KApplication app;

  QImage logo;
  QImage bg;
  QPixmap pix;
  K3AboutDialog about;
  // -----
  // kimgioRegister();
  if(logo.load("RayTracedGear.png"))
    {
      aboutData.setProgramLogo( logo );
      pix=QPixmap::fromImage(logo);
      about.setLogo(pix);
    } else {
      qDebug("main: No logo loaded.");
    }
  if(bg.load("background_1.png"))
    {
      pix=QPixmap::fromImage(bg);
      //about.setBackgroundTile(&pix);
      //about.showTile( true );
      about.enableLinkedHelp( true );

      //about.showBaseFrameTile(true);
    } else {
      qDebug("main: No tile loaded.");
    }
//    about.setButtonOKText
//      ("Cl&ose", "Close this dialog.",
//       "<b>Close</b> the dialog<br>"
//  	  "<i>(there are no settings to save).</i>");
  about.setCaption("K3AboutDialog example");
  about.setVersion("K3AboutDialog, Draft Study");
  about.setHelp("kdehelp/intro.html", "intro");
  // ----- set the application author:
  about.setAuthor
    ("Mirko Boehm", "mirko@kde.org", "", "Initial developer.");
  // ----- set the application maintainer:
  about.setMaintainer("Any One", // name
		      "anyone@kde.org", // email address
		      "http://www.anyhere.com", // URL
		      "Current maintainer."); // description
  // ----- add some contributors:
  about.addContributor("Some One", "someone@kde.org", "http://www.somehere.com",
		       "Making coffee");
  about.addContributor("Another One",
		       "anotherone@kde.org",
		       "http://www.blablax.com",
		       "Pizza donator");
  // ----- contents of the dialog have changed, adapt sizes:
  about.adjust();
  about.exec();
  // -----
  K3AboutApplication aboutApp(&aboutData, app.activeWindow(), true);
  aboutApp.exec();
  return 0;
}

