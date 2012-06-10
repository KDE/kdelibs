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

#include "khelpmenu.h"

#include <QtCore/QTimer>
#include <QtGui/QLabel>
#include <QtGui/QWidget>
#include <QtGui/QWhatsThis>

#include <kaboutapplicationdialog.h>
#include <kaboutdata.h>
#include <kaboutkdedialog_p.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kauthorized.h>
#include <kbugreport.h>
#include <kdialog.h>
#include <kguiitem.h>
#include <khbox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kstandardshortcut.h>
#include <kstandardaction.h>
#include <kstandardguiitem.h>
#include <kswitchlanguagedialog_p.h>
#include <ktoolinvocation.h>
#include <kstandarddirs.h>

#include <config.h>
#ifdef Q_WS_X11
#include <QX11EmbedWidget>
#endif

using namespace KDEPrivate;

class KHelpMenuPrivate
{
public:
    KHelpMenuPrivate()
      : mSwitchApplicationLanguage(0),
	mActionsCreated(false),
        mSwitchApplicationLanguageAction(0)
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
        delete mSwitchApplicationLanguage;
    }

    void createActions(KHelpMenu* q);

    KMenu *mMenu;
    KDialog *mAboutApp;
    KAboutKdeDialog *mAboutKDE;
    KBugReport *mBugReport;
    KSwitchLanguageDialog *mSwitchApplicationLanguage;

// TODO evaluate if we use static_cast<QWidget*>(parent()) instead of mParent to win that bit of memory
    QWidget *mParent;
    QString mAboutAppText;

    bool mShowWhatsThis;
    bool mActionsCreated;

    KAction *mHandBookAction, *mWhatsThisAction;
    QAction *mReportBugAction, *mSwitchApplicationLanguageAction, *mAboutAppAction, *mAboutKDEAction;

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

    if (actions) {
        d->createActions(this);
        if (d->mHandBookAction)
            actions->addAction(d->mHandBookAction->objectName(), d->mHandBookAction);
        if (d->mWhatsThisAction)
            actions->addAction(d->mWhatsThisAction->objectName(), d->mWhatsThisAction);
        if (d->mReportBugAction)
            actions->addAction(d->mReportBugAction->objectName(), d->mReportBugAction);
        if (d->mSwitchApplicationLanguageAction)
            actions->addAction(d->mSwitchApplicationLanguageAction->objectName(), d->mSwitchApplicationLanguageAction);
        if (d->mAboutAppAction)
            actions->addAction(d->mAboutAppAction->objectName(), d->mAboutAppAction);
        if (d->mAboutKDEAction)
            actions->addAction(d->mAboutKDEAction->objectName(), d->mAboutKDEAction);
    }
}

KHelpMenu::~KHelpMenu()
{
  delete d;
}

void KHelpMenuPrivate::createActions(KHelpMenu* q)
{
    if (mActionsCreated)
        return;
    mActionsCreated = true;

    if (KAuthorized::authorizeKAction("help_contents")) {
        mHandBookAction = KStandardAction::helpContents(q, SLOT(appHelpActivated()), q);
    }
    if (mShowWhatsThis && KAuthorized::authorizeKAction("help_whats_this")) {
        mWhatsThisAction = KStandardAction::whatsThis(q, SLOT(contextHelpActivated()), q);
    }

    const KAboutData *aboutData = mAboutData ? mAboutData : KGlobal::mainComponent().aboutData();
    if (KAuthorized::authorizeKAction("help_report_bug") && aboutData && !aboutData->bugAddress().isEmpty()) {
        mReportBugAction = KStandardAction::reportBug(q, SLOT(reportBug()), q);
    }

    if (KAuthorized::authorizeKAction("switch_application_language")) {
        if((KGlobal::dirs()->findAllResources("locale", QString::fromLatin1("*/entry.desktop"))).count() > 0) {
            mSwitchApplicationLanguageAction = KStandardAction::create(KStandardAction::SwitchApplicationLanguage, q, SLOT(switchApplicationLanguage()), q);
        }
    }

    if (KAuthorized::authorizeKAction("help_about_app")) {
        mAboutAppAction = KStandardAction::aboutApp(q, SLOT(aboutApplication()), q);
    }

    if (KAuthorized::authorizeKAction("help_about_kde")) {
        mAboutKDEAction = KStandardAction::aboutKDE(q, SLOT(aboutKDE()), q);
    }
}

// Used in the non-xml-gui case, like kfind or ksnapshot's help button.
KMenu* KHelpMenu::menu()
{
  if( !d->mMenu )
  {
    d->mMenu = new KMenu();
    connect( d->mMenu, SIGNAL(destroyed()), this, SLOT(menuDestroyed()));

    d->mMenu->setTitle(i18n("&Help"));

    d->createActions(this);

    bool need_separator = false;
    if (d->mHandBookAction) {
      d->mMenu->addAction(d->mHandBookAction);
      need_separator = true;
    }

    if (d->mWhatsThisAction) {
      d->mMenu->addAction(d->mWhatsThisAction);
      need_separator = true;
    }

    if (d->mReportBugAction) {
      if (need_separator)
        d->mMenu->addSeparator();
      d->mMenu->addAction(d->mReportBugAction);
      need_separator = true;
    }

    if (d->mSwitchApplicationLanguageAction) {
        if (need_separator)
          d->mMenu->addSeparator();
        d->mMenu->addAction(d->mSwitchApplicationLanguageAction);
        need_separator = true;
    }

    if (need_separator)
      d->mMenu->addSeparator();

    if (d->mAboutAppAction) {
      d->mMenu->addAction(d->mAboutAppAction);
    }

    if (d->mAboutKDEAction) {
      d->mMenu->addAction(d->mAboutKDEAction);
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

    case menuSwitchLanguage:
      return d->mSwitchApplicationLanguageAction;
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
      d->mAboutApp = new KAboutApplicationDialog( d->mAboutData, d->mParent );
      connect( d->mAboutApp, SIGNAL(finished()), this, SLOT(dialogFinished()) );
    }
    d->mAboutApp->show();
  }
  else
  {
    if( !d->mAboutApp )
    {
      d->mAboutApp = new KDialog( d->mParent, Qt::Dialog );
      d->mAboutApp->setCaption( i18n("About %1", KGlobal::caption() ) );
      d->mAboutApp->setButtons( KDialog::Yes );
      d->mAboutApp->setObjectName( "about" );
      d->mAboutApp->setButtonText( KDialog::Yes, KStandardGuiItem::ok().text() );
      d->mAboutApp->setDefaultButton( KDialog::Yes );
      d->mAboutApp->setEscapeButton( KDialog::Yes );
      connect( d->mAboutApp, SIGNAL(finished()), this, SLOT(dialogFinished()) );

      KHBox *hbox = new KHBox( d->mAboutApp );
      d->mAboutApp->setMainWidget( hbox );
      hbox->setSpacing(KDialog::spacingHint()*3);
      hbox->setMargin(KDialog::marginHint()*1);

      QLabel *label1 = new QLabel(hbox);

      int size = IconSize(KIconLoader::Dialog);
      label1->setPixmap( qApp->windowIcon().pixmap(size,size) );
      QLabel *label2 = new QLabel(hbox);
      label2->setText( d->mAboutAppText );
    }
    d->mAboutApp->show();
  }
}


void KHelpMenu::aboutKDE()
{
  if( !d->mAboutKDE )
  {
    d->mAboutKDE = new KAboutKdeDialog( d->mParent );
    connect( d->mAboutKDE, SIGNAL(finished()), this, SLOT(dialogFinished()) );
  }
  d->mAboutKDE->show();
}


void KHelpMenu::reportBug()
{
  if( !d->mBugReport )
  {
    d->mBugReport = new KBugReport( d->mParent, false, d->mAboutData );
    connect( d->mBugReport, SIGNAL(finished()),this,SLOT(dialogFinished()) );
  }
  d->mBugReport->show();
}


void KHelpMenu::switchApplicationLanguage()
{
  if ( !d->mSwitchApplicationLanguage )
  {
    d->mSwitchApplicationLanguage = new KSwitchLanguageDialog( d->mParent );
    connect( d->mSwitchApplicationLanguage, SIGNAL(finished()), this, SLOT(dialogFinished()) );
  }
  d->mSwitchApplicationLanguage->show();
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

  if ( d->mSwitchApplicationLanguage && !d->mSwitchApplicationLanguage->isVisible() )
  {
    delete d->mSwitchApplicationLanguage; d->mSwitchApplicationLanguage = 0;
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
  QWidget* w = QApplication::widgetAt( QCursor::pos() );
#ifdef Q_WS_X11
  while ( w && !w->isTopLevel() && !qobject_cast<QX11EmbedWidget*>(w)  )
      w = w->parentWidget();
#ifdef __GNUC__
#warning how to enter whats this mode for a QX11EmbedWidget?
#endif
//   if ( w && qobject_cast<QX11EmbedWidget*>(w) )
//	  (( QX11EmbedWidget*) w )->enterWhatsThisMode();
#endif
}


#include "khelpmenu.moc"
