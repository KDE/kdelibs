/*
 * This file is part of the KDE Libraries
 * Copyright (C) 1999 Espen Sand (espen@kde.org)
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


#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qwidget.h>

#include <kaboutdialog.h>
#include <kapp.h>
#include <khelpmenu.h>
#include <klocale.h>
#include <kstddirs.h>


KHelpMenu::KHelpMenu( QWidget *parent, const QString &aboutAppText )
  : QObject(parent), mMenu(0), mAboutApp(0), mAboutKDE(0)
{
  mParent = parent;
  mAboutAppText = aboutAppText;
}


KHelpMenu::~KHelpMenu( void )
{
  delete mMenu;
  delete mAboutApp;
  delete mAboutKDE;
}


QPopupMenu* KHelpMenu::menu( void )
{
  if( mMenu == 0 )
  {
    mMenu = new QPopupMenu();
    if( mMenu == 0 ) { return(0); }
    connect( mMenu, SIGNAL(destroyed()), this, SLOT(menuDestroyed()));

    int id = mMenu->insertItem( i18n( "&Contents" ) );
    mMenu->connectItem( id, this, SLOT( appHelpActivated() ) );
    mMenu->setAccel( Key_F1, id );

    mMenu->insertSeparator();

    id = mMenu->insertItem( i18n( "&About" ) + " " + kapp->name() + "..." );
    if( mAboutAppText.isNull() == false )
    {
      mMenu->connectItem( id, this, SLOT( aboutApp() ) );
    }

    id = mMenu->insertItem( i18n( "About &KDE..." ) );
    mMenu->connectItem( id, this, SLOT( aboutKDE() ) );
  }

  return( mMenu );
}



void KHelpMenu::appHelpActivated( void )
{
  kapp->invokeHTMLHelp( QString(kapp->name()) + "/" + "index.html", "" );
}


void KHelpMenu::aboutApp( void )
{
  if( mAboutApp == 0 )
  {
    QString caption = i18n("About %1").arg(kapp->caption());
    mAboutApp = new QMessageBox( caption, mAboutAppText, 
      QMessageBox::Information,
      QMessageBox::Ok | QMessageBox::Default | QMessageBox::Escape,
      0, 0, mParent, "about", false );

    mAboutApp->setButtonText(QMessageBox::Ok, i18n("&OK"));
    mAboutApp->setIconPixmap(kapp->icon());
    mAboutApp->adjustSize();
    mAboutApp->setMinimumSize(mAboutApp->size());
  }

  mAboutApp->show();
}


void KHelpMenu::aboutKDE( void )
{
  if( mAboutKDE == 0 )
  {
    mAboutKDE = new KAboutDialog( KAboutDialog::AbtKDEStandard, "KDE",
      KDialogBase::Help|KDialogBase::Close, KDialogBase::Close, mParent, 
      "aboutkde", false );

    const QString text1 = i18n(""
      "The <b>K Desktop Environment</b> is written and maintained by the " 
      "KDE Team, a world-wide network of software engineers committed to "
      "free software development.<br><br>"
      "No single group, company or organization controls the KDE source "
      "code. Everyone is welcome to contribute to KDE.<br><br>"
      "Visit <A HREF=\"http://www.kde.org/\">http://www.kde.org/</A> for "
      "more information on the KDE Project. ");

    const QString text2 = i18n(""
      "Software can always be improved, and the KDE Team is ready to "
      "do so. However, you - the user - must tell us when "
      "something does not work as expected or could be done better.<br><br>"
      "The K Desktop Environment has a bug tracking system. Visit "
      "<A HREF=\"http://bugs.kde.org/\">http://bugs.kde.org/</A> to "
      "report bugs.<br><br>"
      "If you have a suggestion for improvement then you are welcome to visit "
      "<A HREF=\"http://wishlist.kde.org/\">http://wishlist.kde.org/</A> and "
      "register your wish." );

    const QString text3 = i18n(""
      "You don't have to be a software developer to be a member of the "
      "KDE Team. You can join the national teams that translate "
      "program interfaces. You can provide graphics, themes, sounds and "
      "improved documentation. You decide!"
      "<br><br>"
      "Visit "
      "<A HREF=\"http://www.kde.org/jobs/\">http://www.kde.org/jobs/</A> "
      "for information on some projects in which you can participate."
      "<br><br>"
      "If you need more information or documentation, then a visit to "
      "<A HREF=\"http://developer.kde.org/\">http://developer.kde.org/</A> "
      "will provide with what you need.");

    mAboutKDE->setHelp( QString("kdehelp/main.html"), QString::null, 
			QString::null );
    mAboutKDE->setTitle(i18n("K Desktop Environment. Release %1").
			arg(KDE_VERSION_STRING) );
    mAboutKDE->addTextPage( i18n("&About"), text1, true );
    mAboutKDE->addTextPage( i18n("&Report bugs or wishes"), text2, true );
    mAboutKDE->addTextPage( i18n("&Join the KDE team"), text3, true );
    mAboutKDE->setImage( locate( "data", "kdeui/pics/aboutkde.png") );
    mAboutKDE->setImageBackgroundColor( white );
  }

  mAboutKDE->show();
}


void KHelpMenu::menuDestroyed( void )
{
  mMenu = 0;
}




#include "khelpmenu.moc"
