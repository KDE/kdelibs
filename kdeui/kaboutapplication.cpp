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
#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kurllabel.h>

KAboutApplication::KAboutApplication( QWidget *parent, const char *name,
				      bool modal )
  :KAboutDialog( AbtTabbed|AbtProduct, 
                 kapp ? kapp->caption() : QString::null, 
                 Close, Close,
		 parent, name, modal )
{
  buildDialog(KGlobal::instance()->aboutData());
}

KAboutApplication::KAboutApplication( const KAboutData *aboutData, QWidget *parent,
                                      const char *name, bool modal )
  :KAboutDialog( AbtTabbed|AbtProduct, aboutData->programName(), Close, Close,
		 parent, name, modal )
{
  buildDialog(aboutData);
}

void KAboutApplication::buildDialog( const KAboutData *aboutData )
{
  if( aboutData == 0 )
  {
    //
    // Recovery
    //
    setProduct( kapp ? kapp->caption() : QString::null, i18n("??"), QString::null, QString::null );
    KAboutContainer *appPage = addContainerPage( i18n("&About"));

    QString appPageText =
      i18n("Sorry, no information available.\n"
	   "The supplied KAboutData object does not exist.");
    QLabel *appPageLabel = new QLabel( "\n\n\n\n"+appPageText+"\n\n\n\n", 0 );
    appPage->addWidget( appPageLabel );
    return;
  }

  setProduct( aboutData->programName(), aboutData->version(),
	      QString::null, QString::null );

  QString appPageText = aboutData->shortDescription() + "\n";

  if (!aboutData->otherText().isEmpty())
    appPageText += "\n" + aboutData->otherText()+"\n";

  if (!aboutData->copyrightStatement().isEmpty())
    appPageText += "\n" + aboutData->copyrightStatement()+"\n";

  KAboutContainer *appPage = addContainerPage( i18n("&About"));

  QLabel *appPageLabel = new QLabel( appPageText, 0 );
  appPage->addWidget( appPageLabel );

  if (!aboutData->homepage().isEmpty())
  {
    KURLLabel *url = new KURLLabel();
    url->setText(aboutData->homepage());
    url->setURL(aboutData->homepage());
    appPage->addWidget( url );
    connect( url, SIGNAL(leftClickedURL(const QString &)),
             this, SLOT(openURLSlot(const QString &)));
  }

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

  const QValueList<KAboutTranslator> translatorList = aboutData->translators();

  if(translatorList.count() > 0)
  {
      KAboutContainer *translatorPage =
          addScrolledContainerPage( i18n("T&ranslation") );

      QValueList<KAboutTranslator>::ConstIterator it;
      for(it = translatorList.begin(); it != translatorList.end(); ++it)
      {
          translatorPage->addPerson((*it).name(), (*it).emailAddress(),
                  0,0);
      }

      QLabel *label = new QLabel(KAboutData::aboutTranslationTeam()
              ,translatorPage);
	  label->adjustSize();
	  label->setMinimumSize(label->sizeHint());
      translatorPage->addWidget(label);
  }

  if (!aboutData->license().isEmpty() )
  {
    addLicensePage( i18n("&License agreement"), aboutData->license() );
  }

  //
  // Make sure the dialog has a reasonable width 
  //
  setInitialSize( QSize(400,1) );
}
