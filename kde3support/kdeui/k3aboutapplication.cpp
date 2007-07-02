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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "k3aboutapplication.h"

#include <QList>
#include <QPixmap>

#include "k3aboutdialog_p.h"
#include <kaboutdata.h>
#include <kapplication.h>
#include <QtGui/QLabel>
#include <kglobal.h>
#include <klocale.h>
#include <QtGui/QStyle>

K3AboutApplication::K3AboutApplication( const KAboutData *aboutData, QWidget *parent, bool modal )
  :K3AboutDialog( Tabbed|Product, aboutData->programName(), parent ),
   d( 0 )
{
  setButtons( Close );
  setDefaultButton( Close );
  setModal( modal );

  if( aboutData == 0 )
    aboutData = KGlobal::mainComponent().aboutData();

  if( !aboutData )
  {
    // Recovery
    setProduct(KGlobal::caption(), i18n("??"), QString(), QString());
    K3AboutContainer *appPage = addContainerPage( i18n("&About"));

    QString appPageText =
      i18n("No information available.\n"
     "The supplied KAboutData object does not exist.");
    QLabel *appPageLabel = new QLabel( "\n\n\n\n"+appPageText+"\n\n\n\n", 0 );
    appPage->addWidget( appPageLabel );
    return;
  }

  setProduct( aboutData->programName(), aboutData->version(),
        QString(), QString() );

  if ( aboutData->programLogo().canConvert<QPixmap>() )
    setProgramLogo( aboutData->programLogo().value<QPixmap>() );
  else if ( aboutData->programLogo().canConvert<QImage>() )
    setProgramLogo( QPixmap::fromImage(aboutData->programLogo().value<QImage>() ) );

  QString appPageText = aboutData->shortDescription() + '\n';

  if (!aboutData->otherText().isEmpty())
    appPageText += '\n' + aboutData->otherText() + '\n';

  if (!aboutData->copyrightStatement().isEmpty())
    appPageText += '\n' + aboutData->copyrightStatement() + '\n';

  K3AboutContainer *appPage = addContainerPage( i18n("&About"));

  QLabel *appPageLabel = new QLabel( appPageText, 0 );
  appPage->addWidget( appPageLabel );

  if (!aboutData->homepage().isEmpty())
  {
    QLabel *url = new QLabel(appPage);
    url->setOpenExternalLinks(true);
    url->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    url->setText(QString("<a href=\"%1\">%1</a>").arg(aboutData->homepage()));
    appPage->addWidget( url );
  }

  int authorCount = aboutData->authors().count();
  if (authorCount)
  {
    QString authorPageTitle = authorCount == 1 ?
      i18n("A&uthor") : i18n("A&uthors");
    K3AboutContainer *authorPage = addScrolledContainerPage( authorPageTitle, Qt::AlignLeft, Qt::AlignLeft );

    if (!aboutData->customAuthorTextEnabled() || !aboutData->customAuthorRichText().isEmpty ())
    {
      QString text;
      QLabel* activeLabel = new QLabel( authorPage );
      activeLabel->setOpenExternalLinks(true);
      activeLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
      if (!aboutData->customAuthorTextEnabled())
      {
        if ( aboutData->bugAddress().isEmpty() || aboutData->bugAddress() == "submit@bugs.kde.org")
          text = i18n( "Please use <a href=\"http://bugs.kde.org\">http://bugs.kde.org</a> to report bugs.\n" );
        else {
          if( aboutData->authors().count() == 1 && ( aboutData->authors().first().emailAddress() == aboutData->bugAddress() ) )
          {
            text = i18n( "Please report bugs to <a href=\"mailto:%1\">%2</a>.\n" ,  aboutData->authors().first().emailAddress() ,  aboutData->authors().first().emailAddress() );
          }
          else {
            text = i18n( "Please report bugs to <a href=\"mailto:%1\">%2</a>.\n" , aboutData->bugAddress(), aboutData->bugAddress() );
          }
        }
      }
      else
      {
        text = aboutData->customAuthorRichText();
      }
      activeLabel->setText( text );
      authorPage->addWidget( activeLabel );
    }

    QList<KAboutPerson> lst = aboutData->authors();
    for (int i = 0; i < lst.size(); ++i)
    {
      authorPage->addPerson( lst.at(i).name(), lst.at(i).emailAddress(),
      lst.at(i).webAddress(), lst.at(i).task() );
    }
  }

  int creditsCount = aboutData->credits().count();
  if (creditsCount)
  {
    K3AboutContainer *creditsPage = addScrolledContainerPage( i18n("&Thanks To") );

    QList<KAboutPerson> lst = aboutData->credits();
    for (int i = 0; i < lst.size(); ++i)
    {
      creditsPage->addPerson( lst.at(i).name(), lst.at(i).emailAddress(),
           lst.at(i).webAddress(), lst.at(i).task() );
    }
  }

  const QList<KAboutPerson> translatorList = aboutData->translators();

  if(translatorList.count() > 0)
  {
      QString text = "<qt>";

      QList<KAboutPerson>::ConstIterator it;
      for(it = translatorList.begin(); it != translatorList.end(); ++it)
      {
        text += QString("<p>%1<br>&nbsp;&nbsp;&nbsp;"
            "<a href=\"mailto:%2\">%2</a></p>")
            .arg((*it).name())
            .arg((*it).emailAddress())
            .arg((*it).emailAddress());
      }

      text += KAboutData::aboutTranslationTeam() + "</qt>";
      addTextPage( i18n("T&ranslation"), text, true);
  }

  if (!aboutData->license().isEmpty() )
  {
    addLicensePage( i18n("&License Agreement"), aboutData->license() );
  }
  // Make sure the dialog has a reasonable width
  setInitialSize( QSize(400,1) );
}
