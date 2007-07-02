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
  KAboutData aboutData("K3AboutDialogTest", 0, ki18n("K3AboutDialogTest"), "version", ki18n("K3AboutDialog Test, Enjoy!"), KAboutData::License_GPL, ki18n("(c) 1997-2006, The KDE Developers"));
  aboutData.addAuthor(ki18n("Bob Dobbs"), ki18n("Current Maintainer"), "bob@subgenius.com");
  aboutData.addAuthor(ki18n("Gregory S. Hayes"), ki18n("Developer"), "syncomm@gmail.com");
  aboutData.addAuthor(ki18n("Random Authors from KFind"));
  aboutData.addAuthor(ki18n("Beppe Grimaldi"), ki18n("UI Design & more search options"), "grimalkin@ciaoweb.it");
  aboutData.addAuthor(ki18n("Martin Hartig"));
  aboutData.addAuthor(ki18n("Stephan Kulow"), KLocalizedString(), "coolo@kde.org");
  aboutData.addAuthor(ki18n("Mario Weilguni"), KLocalizedString(), "mweilguni@sime.com");
  aboutData.addAuthor(ki18n("Alex Zepeda"), KLocalizedString(), "zipzippy@sonic.net");
  aboutData.addAuthor(ki18n("Miroslav Fl�r"), KLocalizedString(), "flidr@kky.zcu.cz");
  aboutData.addAuthor(ki18n("Harri Porten"), KLocalizedString(), "porten@kde.org");
  aboutData.addAuthor(ki18n("Dima Rogozin"), KLocalizedString(), "dima@mercury.co.il");
  aboutData.addAuthor(ki18n("Carsten Pfeiffer"), KLocalizedString(), "pfeiffer@kde.org");
  aboutData.addAuthor(ki18n("Hans Petter Bieker"), KLocalizedString(), "bieker@kde.org");
  aboutData.addAuthor(ki18n("Waldo Bastian"), ki18n("UI Design"), "bastian@kde.org");
  aboutData.addAuthor(ki18n("Alexander Neundorf"), KLocalizedString(), "neundorf@kde.org");
  aboutData.addAuthor(ki18n("Clarence Dang"), KLocalizedString(), "dang@kde.org");

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

