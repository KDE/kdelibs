/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Waldo Bastian (bastian@kde.org) and 
 * Espen Sand (espen@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

// I (espen) prefer that header files are included alphabetically

#include <qlabel.h>
#include <kaboutapplication.h>
#include <kaboutdialog_private.h>
#include <kaboutdata.h>
#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>

KAboutApplication::KAboutApplication( QWidget *parent, const char *name, 
				      bool modal )
  :KAboutDialog( AbtTabbed|AbtProduct, kapp->caption(), Ok, Ok, 
		 parent, name, modal )
{
  const KAboutData *aboutData = KGlobal::instance()->aboutData();
  if( aboutData == 0 )
  {
    //
    // Recovery
    //
    setProduct( kapp->caption(), i18n("??"), QString::null, QString::null );
    KAboutContainer *appPage = addContainerPage( i18n("&About"));

    QString appPageText = 
      i18n("Sorry, no information available.\n"
	   "The global KAboutData object does not exist.");
    QLabel *appPageLabel = new QLabel( "\n\n\n\n"+appPageText+"\n\n\n\n", 0 );
    appPage->addWidget( appPageLabel );
    return;
  }

  setProduct( aboutData->programName(), aboutData->version(),
	      QString::null, QString::null );

  QString appPageText = aboutData->shortDescription() + "\n";

  if (!aboutData->homepage().isEmpty())
    appPageText += "\n" + aboutData->homepage()+"\n";

  if (!aboutData->otherText().isEmpty())
    appPageText += "\n" + aboutData->otherText()+"\n";

  if (!aboutData->copyrightStatement().isEmpty())
    appPageText += "\n" + aboutData->copyrightStatement()+"\n";

  KAboutContainer *appPage = addContainerPage( i18n("&About"));
 
  QLabel *appPageLabel = new QLabel( appPageText, 0 );
  appPage->addWidget( appPageLabel );

  int authorCount = aboutData->authors().count();
  if (authorCount)
  {
    QString authorPageTitle = authorCount == 1 ? 
      i18n("A&uthor") : i18n("A&uthors");
    KAboutContainer *authorPage = addScrolledContainerPage( authorPageTitle );
    QValueList<KAboutPerson>::ConstIterator it;
    for (it = aboutData->authors().begin(); 
	 it != aboutData->authors().end(); ++it)
    {
      authorPage->addPerson( (*it).name(), (*it).emailAddress(),
			     (*it).webAddress(), (*it).task() );
    }
  }

  int creditsCount = aboutData->credits().count();
  if (creditsCount)
  {
    KAboutContainer *creditsPage = 
      addScrolledContainerPage( i18n("&Thanks to") );
    QValueList<KAboutPerson>::ConstIterator it;
    for (it = aboutData->credits().begin();
	 it != aboutData->credits().end(); ++it)
    {
      creditsPage->addPerson( (*it).name(), (*it).emailAddress(),
			      (*it).webAddress(), (*it).task() );
    }
  }

  if (!aboutData->license().isEmpty() )
  {
    addTextPage( i18n("&License agreement"), aboutData->license() );
  }

  //
  // Make sure the dialog has a reasonable width 
  //
  setInitialSize( QSize(400,1) );
}


