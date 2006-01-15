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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

// I (espen) prefer that header files are included alphabetically

#include <QLabel>
#include <QTimer>
#include <QWidget>
#include <QWhatsThis>

#include <kaboutapplication.h>
#include <kaboutdata.h>
#include <kaboutkde.h>
#include <kaction.h>
#include <kapplication.h>
#include <kauthorized.h>
#include <kbugreport.h>
#include <kdialogbase.h>
#include <khbox.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kstdaccel.h>
#include <kstdaction.h>
#include <ktoolinvocation.h>

#include "config.h"
#ifdef Q_WS_X11
#include <QX11EmbedWidget>
#endif

class KHelpMenuPrivate
{
public:
    KHelpMenuPrivate()
    {
        mMenu = 0;
        mAboutApp = 0;
        mAboutKDE = 0;
        mBugReport = 0;
        mHandBookAction = 0;
        mWhatsThisAction = 0;
        mReportBugAction = 0;
        mAboutAppAction = 0;
        mAboutKDEAction = 0;
    }
    ~KHelpMenuPrivate()
    {
        delete mMenu;
        delete mAboutApp;
        delete mAboutKDE;
        delete mBugReport;
    }

    KMenu *mMenu;
    KDialog *mAboutApp;
    KAboutKDE *mAboutKDE;
    KBugReport *mBugReport;

// TODO evaluate if we use static_cast<QWidget*>(parent()) instead of mParent to win that bit of memory
    QWidget *mParent;
    QString mAboutAppText;

    bool mShowWhatsThis;

    QAction *mHandBookAction, *mWhatsThisAction, *mReportBugAction, *mAboutAppAction, *mAboutKDEAction;

    const KAboutData *mAboutData;
};

KHelpMenu::KHelpMenu( QWidget *parent, const QString &aboutAppText,
		      bool showWhatsThis )
  : QObject(parent), d(new KHelpMenuPrivate)
{
  d->mAboutAppText = aboutAppText;
  d->mShowWhatsThis = showWhatsThis;
  d->mParent = parent;
  d->mAboutData = 0;
}

KHelpMenu::KHelpMenu( QWidget *parent, const KAboutData *aboutData,
		      bool showWhatsThis, KActionCollection *actions )
  : QObject(parent), d(new KHelpMenuPrivate)
{
  d->mShowWhatsThis = showWhatsThis;
  d->mParent = parent;
  d->mAboutData = aboutData;

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
  delete d;
}


KMenu* KHelpMenu::menu()
{
  if( !d->mMenu )
  {
    const KAboutData *aboutData = d->mAboutData ? d->mAboutData : KGlobal::instance()->aboutData();
    QString appName = (aboutData)? aboutData->programName() : qApp->applicationName();

    d->mMenu = new KMenu();
    connect( d->mMenu, SIGNAL(destroyed()), this, SLOT(menuDestroyed()));

    d->mMenu->setIcon(SmallIcon("help"));
    d->mMenu->setTitle(i18n("&Help"));

    bool need_separator = false;
    if (KAuthorized::authorizeKAction("help_contents"))
    {
      d->mHandBookAction = d->mMenu->addAction(BarIconSet( "contents", KIcon::SizeSmall),
                     i18n("%1 &Handbook").arg(appName) ,this, SLOT(appHelpActivated()),KStdAccel::shortcut(KStdAccel::Help));
      need_separator = true;
    }

    if( d->mShowWhatsThis && KAuthorized::authorizeKAction("help_whats_this") )
    {
      d->mWhatsThisAction = d->mMenu->addAction( SmallIconSet("contexthelp"),i18n( "What's &This" ),this, SLOT(contextHelpActivated()), Qt::SHIFT + Qt::Key_F1);
      need_separator = true;
    }

    if (KAuthorized::authorizeKAction("help_report_bug") && aboutData && !aboutData->bugAddress().isEmpty() )
    {
      if (need_separator)
        d->mMenu->addSeparator();
      d->mReportBugAction = d->mMenu->addAction( i18n( "&Report Bug..." ), this, SLOT(reportBug()) );
      need_separator = true;
    }

    if (need_separator)
      d->mMenu->addSeparator();

    if (KAuthorized::authorizeKAction("help_about_app"))
    {
      d->mAboutAppAction = d->mMenu->addAction( qApp->windowIcon(),
        i18n( "&About %1" ).arg(appName), this, SLOT( aboutApplication() ) );
    }

    if (KAuthorized::authorizeKAction("help_about_kde"))
    {
      d->mAboutKDEAction = d->mMenu->addAction( SmallIconSet("about_kde"), i18n( "About &KDE" ), this, SLOT( aboutKDE() ) );
    }
  }

  return d->mMenu;
}

QAction *KHelpMenu::action( MenuId id ) const
{
  switch (id)
  {
    case menuHelpContents:
      return d->mHandBookAction;
    break;

    case menuWhatsThis:
      return d->mWhatsThisAction;
    break;

    case menuReportBug:
      return d->mReportBugAction;
    break;

    case menuAboutApp:
      return d->mAboutAppAction;
    break;

    case menuAboutKDE:
      return d->mAboutKDEAction;
    break;
  }

  return 0;
}

void KHelpMenu::appHelpActivated()
{
  KToolInvocation::invokeHelp();
}


void KHelpMenu::aboutApplication()
{
  if (receivers(SIGNAL(showAboutApplication())) > 0)
  {
    emit showAboutApplication();
  }
  else if (d->mAboutData)
  {
    if( !d->mAboutApp )
    {
      d->mAboutApp = new KAboutApplication( d->mAboutData, d->mParent, false );
      connect( d->mAboutApp, SIGNAL(finished()), this, SLOT( dialogFinished()) );
    }
    d->mAboutApp->show();
  }
  else
  {
    if( !d->mAboutApp )
    {
      d->mAboutApp = new KDialogBase( QString(), // Caption is defined below
				   KDialogBase::Yes, KDialogBase::Yes,
				   KDialogBase::Yes, d->mParent, "about",
				   false, true, KStdGuiItem::ok() );
      connect( d->mAboutApp, SIGNAL(finished()), this, SLOT( dialogFinished()) );

      KHBox *hbox = new KHBox( d->mAboutApp );
      d->mAboutApp->setMainWidget( hbox );
      hbox->setSpacing(KDialog::spacingHint()*3);
      hbox->setMargin(KDialog::marginHint()*1);

      QLabel *label1 = new QLabel(hbox);

      int size = IconSize(KIcon::Desktop);
      label1->setPixmap( qApp->windowIcon().pixmap(size,size) );
      QLabel *label2 = new QLabel(hbox);
      label2->setText( d->mAboutAppText );

      d->mAboutApp->setPlainCaption( i18n("About %1").arg(kapp->caption()) );
    }
    d->mAboutApp->show();
  }
}


void KHelpMenu::aboutKDE()
{
  if( !d->mAboutKDE )
  {
    d->mAboutKDE = new KAboutKDE( d->mParent, false );
    connect( d->mAboutKDE, SIGNAL(finished()), this, SLOT( dialogFinished()) );
  }
  d->mAboutKDE->show();
}


void KHelpMenu::reportBug()
{
  if( !d->mBugReport )
  {
    d->mBugReport = new KBugReport( d->mParent, false, d->mAboutData );
    connect( d->mBugReport, SIGNAL(finished()),this,SLOT( dialogFinished()) );
  }
  d->mBugReport->show();
}


void KHelpMenu::dialogFinished()
{
  QTimer::singleShot( 0, this, SLOT(timerExpired()) );
}


void KHelpMenu::timerExpired()
{
  if( d->mAboutKDE && !d->mAboutKDE->isVisible() )
  {
    delete d->mAboutKDE; d->mAboutKDE = 0;
  }

  if( d->mBugReport && !d->mBugReport->isVisible() )
  {
    delete d->mBugReport; d->mBugReport = 0;
  }

  if( d->mAboutApp && !d->mAboutApp->isVisible() )
  {
    delete d->mAboutApp; d->mAboutApp = 0;
  }
}


void KHelpMenu::menuDestroyed()
{
  d->mMenu = 0;
}


void KHelpMenu::contextHelpActivated()
{
  QWhatsThis::enterWhatsThisMode();
  QWidget* w = QApplication::widgetAt( QCursor::pos(), true );
#ifdef Q_WS_X11
  while ( w && !w->isTopLevel() && !qobject_cast<QX11EmbedWidget*>(w)  )
      w = w->parentWidget();
#warning how to enter whats this mode for a QX11EmbedWidget?
//   if ( w && qobject_cast<QX11EmbedWidget*>(w) )
//	  (( QX11EmbedWidget*) w )->enterWhatsThisMode();
#endif
}

void KHelpMenu::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


#include "khelpmenu.moc"
