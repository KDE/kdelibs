/*
 * This file is part of the KDE Libraries
 * Copyright (C) 1999-2000 Espen Sand (espen@kde.org)
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
#include <qhbox.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qtoolbutton.h>
#include <qwhatsthis.h>
#include <qwidget.h>

#include <kaboutdata.h>
#include <kaboutdialog.h>
#include <kaction.h>
#include <kapp.h>
#include <kbugreport.h>
#include <kdialogbase.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdaccel.h>
#include <kstdaction.h>
#include <kstddirs.h>

class KHelpMenuPrivate
{
public:
    KHelpMenuPrivate()
    {
    }
    ~KHelpMenuPrivate()
    {
    }

    const KAboutData *mAboutData;
};

KHelpMenu::KHelpMenu( QWidget *parent, const QString &aboutAppText,
		      bool showWhatsThis )
  : QObject(parent), mMenu(0), mAboutApp(0), mAboutKDE(0), mBugReport(0),
    d(new KHelpMenuPrivate)
{
  mParent = parent;
  mAboutAppText = aboutAppText;
  mShowWhatsThis = showWhatsThis;
}

KHelpMenu::KHelpMenu( QWidget *parent, const KAboutData *aboutData,
		      bool showWhatsThis, QActionCollection *actions )
  : QObject(parent), mMenu(0), mAboutApp(0), mAboutKDE(0), mBugReport(0),
    d(new KHelpMenuPrivate)
{
  mParent = parent;
  mShowWhatsThis = showWhatsThis;

  d->mAboutData = aboutData;
  if (aboutData)
  {
    mAboutAppText = aboutData->programName() + " " + aboutData->version() +
                    "\n" + aboutData->shortDescription();

    if (!aboutData->homepage().isNull())
      mAboutAppText += "\n" + aboutData->homepage();

    QValueList<KAboutPerson>::ConstIterator it;
    for (it = aboutData->authors().begin();
         it != aboutData->authors().end(); ++it)
    {
      mAboutAppText += "\n" + (*it).name() + " <"+(*it).emailAddress()+">";
    }

    if (!aboutData->copyrightStatement().isNull())
      mAboutAppText += "\n" + aboutData->copyrightStatement();
    mAboutAppText += "\n";
  }
  else
    mAboutAppText = QString::null;

    if (actions)
    {
        KStdAction::helpContents(this, SLOT(appHelpActivated()), actions);
        if (showWhatsThis)
            KStdAction::whatsThis(this, SLOT(contextHelpActivated()), actions);
        KStdAction::reportBug(this, SLOT(reportBug()), actions);
        KStdAction::aboutApp(this, SLOT(aboutApplication()), actions);
        KStdAction::aboutKDE(this, SLOT(aboutKDE()), actions);
    }
}

KHelpMenu::~KHelpMenu()
{
  delete mMenu;
  delete mAboutApp;
  delete mAboutKDE;
  delete mBugReport;
  delete d;
}


QPopupMenu* KHelpMenu::menu()
{
  if( mMenu == 0 )
  {
    //
    // 1999-12-02 Espen Sand:
    // I use hardcoded menu id's here. Reason is to stay backward
    // compatible.
    //
    mMenu = new QPopupMenu();
    connect( mMenu, SIGNAL(destroyed()), this, SLOT(menuDestroyed()));

    mMenu->insertItem( BarIcon( "contents", KIconLoader::Small),
		       i18n( "&Contents" ),menuHelpContents );
    mMenu->connectItem( menuHelpContents, this, SLOT(appHelpActivated()) );
    mMenu->setAccel( KStdAccel::key(KStdAccel::Help), menuHelpContents );

    if( mShowWhatsThis == true )
    {
      QToolButton* wtb = QWhatsThis::whatsThisButton(0);
      mMenu->insertItem( wtb->iconSet(),i18n( "What's &This" ), menuWhatsThis);
      mMenu->connectItem( menuWhatsThis, this, SLOT(contextHelpActivated()) );
      delete wtb;
      mMenu->setAccel( SHIFT + Key_F1, menuWhatsThis );
    }

    mMenu->insertSeparator();

    mMenu->insertItem( i18n( "&Report Bug..." ), menuReportBug );
    mMenu->connectItem( menuReportBug, this, SLOT(reportBug()) );

    mMenu->insertSeparator();

    mMenu->insertItem( kapp->miniIcon(),
      i18n( "&About" ) + ' ' + QString::fromLatin1(kapp->name()) + 
      QString::fromLatin1("..."), menuAboutApp );
    mMenu->connectItem( menuAboutApp, this, SLOT( aboutApplication() ) );

    mMenu->insertItem( i18n( "About &KDE..." ), menuAboutKDE );
    mMenu->connectItem( menuAboutKDE, this, SLOT( aboutKDE() ) );
  }

  return( mMenu );
}



void KHelpMenu::appHelpActivated()
{
  kapp->invokeHTMLHelp( QString::fromLatin1(kapp->name()) + 
			QString::fromLatin1("/index.html"), 
			QString::fromLatin1("") );
}


void KHelpMenu::aboutApplication()
{
  if( mAboutAppText.isNull() == true || mAboutAppText.isEmpty() == true )
  {
    emit showAboutApplication();
  }
  else
  {
    if( mAboutApp == 0 )
    {
      mAboutApp = new KDialogBase( QString::null, // Caption is defined below
				   KDialogBase::Yes, KDialogBase::Yes,
				   KDialogBase::Yes, mParent, "about",
				   false, true, i18n("&OK") );
      connect( mAboutApp, SIGNAL(hidden()), this, SLOT( dialogHidden()) );
      
      QHBox *hbox = new QHBox( mAboutApp );
      mAboutApp->setMainWidget( hbox );
      hbox->setSpacing(KDialog::spacingHint()*3);
      hbox->setMargin(KDialog::marginHint()*1);

      QLabel *label1 = new QLabel(hbox);
      label1->setPixmap( kapp->icon() );
      QLabel *label2 = new QLabel(hbox);
      label2->setText( mAboutAppText );

      mAboutApp->setPlainCaption( i18n("About %1").arg(kapp->caption()) );
      mAboutApp->disableResize();
    }

    mAboutApp->show();
  }
}



void KHelpMenu::aboutKDE()
{
  if( mAboutKDE == 0 )
  {
    mAboutKDE = new KAboutDialog( KAboutDialog::AbtKDEStandard, 
      QString::fromLatin1("KDE"),
      KDialogBase::Help|KDialogBase::Close, KDialogBase::Close, mParent,
      "aboutkde", false );
    connect( mAboutKDE, SIGNAL(hidden()), this, SLOT( dialogHidden()) );

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
      "<A HREF=\"http://bugs.kde.org/\">http://bugs.kde.org/</A> or "
      "use the \"Report Bug\" dialog to report bugs.<br><br>"
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
      "<A HREF=\"http://www.kde.org/jobs.html\">http://www.kde.org/jobs.html</A> "
      "for information on some projects in which you can participate."
      "<br><br>"
      "If you need more information or documentation, then a visit to "
      "<A HREF=\"http://developer.kde.org/\">http://developer.kde.org/</A> "
      "will provide with what you need.");

    mAboutKDE->setHelp( QString::fromLatin1("khelpcenter/main.html"), 
			QString::null );
    mAboutKDE->setTitle(i18n("K Desktop Environment. Release %1").
			arg(QString::fromLatin1(KDE_VERSION_STRING)) );
    mAboutKDE->addTextPage( i18n("&About"), text1, true );
    mAboutKDE->addTextPage( i18n("&Report bugs or wishes"), text2, true );
    mAboutKDE->addTextPage( i18n("&Join the KDE team"), text3, true );
    mAboutKDE->setImage( 
      locate( "data", QString::fromLatin1("kdeui/pics/aboutkde.png")) );
    mAboutKDE->setImageBackgroundColor( white );
  }

  mAboutKDE->show();
}


void KHelpMenu::reportBug()
{
  if( mBugReport == 0 )
  {
    mBugReport = new KBugReport( mParent, false );
    connect( mBugReport, SIGNAL(hidden()),this,SLOT( dialogHidden()) );
  }
  mBugReport->show();
}


void KHelpMenu::dialogHidden()
{
  QTimer::singleShot( 0, this, SLOT(timerExpired()) );
}


void KHelpMenu::timerExpired()
{
  if( mAboutKDE != 0 && mAboutKDE->isVisible() == false )
  {
    delete mAboutKDE; mAboutKDE = 0;
  }

  if( mBugReport != 0 && mBugReport->isVisible() == false )
  {
    delete mBugReport; mBugReport = 0;
  }

  if( mAboutApp != 0 && mAboutApp->isVisible() == false )
  {
    delete mAboutApp; mAboutApp = 0;
  }
}


void KHelpMenu::menuDestroyed()
{
  mMenu = 0;
}


void KHelpMenu::contextHelpActivated()
{
  QWhatsThis::enterWhatsThisMode();
}


#include "khelpmenu.moc"
