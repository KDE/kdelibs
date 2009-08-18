// vim: sw=2 et
/* This file is part of the KDE libraries
   Copyright (C) 1999,2000 Kurt Granroth <granroth@kde.org>

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

#include "kstandardaction.h"
#include "kstandardaction_p.h"
#include "kstandardaction_p.moc"

#include <QtCore/QMutableStringListIterator>
#include <QtGui/QToolButton>

#include <kaboutdata.h>
#include <kaction.h>
#include <QtGui/QApplication>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kguiitem.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandardshortcut.h>
#include <kmainwindow.h>
#include <kicon.h>

#include "krecentfilesaction.h"
#include "ktogglefullscreenaction.h"
#include "kpastetextaction.h"
#include "kactioncollection.h"

namespace KStandardAction
{
AutomaticAction::AutomaticAction(const KIcon &icon, const QString &text, const KShortcut &shortcut, const char *slot,
                                 QObject *parent)
    : KAction(parent)
{
    setText(text);
    setIcon(icon);
    setShortcut(shortcut);
    connect(this, SIGNAL(triggered()), this, slot);
}

QStringList stdNames()
{
  return internal_stdNames();
}

QList<StandardAction> actionIds()
{
  QList<StandardAction> result;

  for ( uint i = 0; g_rgActionInfo[i].id != ActionNone; i++ )
      {
      result.append(g_rgActionInfo[i].id);
      }

  return result;
}

KDEUI_EXPORT KStandardShortcut::StandardShortcut shortcutForActionId(StandardAction id)
{
  const KStandardActionInfo* pInfo = infoPtr( id );
  return (pInfo) ? pInfo->idAccel : KStandardShortcut::AccelNone;
}


KAction *create(StandardAction id, const QObject *recvr, const char *slot, QObject *parent)
{
  KAction *pAction = 0;
  const KStandardActionInfo* pInfo = infoPtr(id);

  // kDebug(125) << "KStandardAction::create( " << id << "=" << (pInfo ? pInfo->psName : (const char*)0) << ", " << parent << " )"; // ellis

  if ( pInfo ) {
    QString sLabel, iconName = pInfo->psIconName;
    switch( id ) {
      case Back:
        sLabel = i18nc( "go back", "&Back");
        if ( QApplication::isRightToLeft() )
          iconName = "go-next";
        break;

      case Forward:
        sLabel = i18nc( "go forward", "&Forward" );
        if ( QApplication::isRightToLeft() )
          iconName = "go-previous";
        break;

      case Home:
        sLabel = i18nc( "home page", "&Home" );
        break;
      case Help:
        sLabel = i18nc( "show help", "&Help" );
        break;
      case Preferences:
      case AboutApp:
      case HelpContents:
      {
        const KAboutData *aboutData = KGlobal::mainComponent().aboutData();
        /* TODO KDE4
        const KAboutData *aboutData;
        if ( parent )
          aboutData = parent->componentData().aboutData();
        else
          aboutData = KGlobal::aboutData();
        */
        QString appName = (aboutData) ? aboutData->programName() : qApp->applicationName();
        sLabel = i18n( pInfo->psLabel, appName );
      }
       break;
     default:
       sLabel = i18n( pInfo->psLabel );
    }

    if ( QApplication::isRightToLeft() ) {
     switch ( id ) {
      case Prior:           iconName = "go-next-view-page"; break;
      case Next:            iconName = "go-previous-view-page"; break;
      case FirstPage:       iconName = "go-last-view-page"; break;
      case LastPage:        iconName = "go-first-view-page"; break;
      case DocumentBack:    iconName = "go-next"; break;
      case DocumentForward: iconName = "go-previous"; break;
      default: break;
     }
    }

    QIcon icon = iconName.isEmpty() ? KIcon() : KIcon(iconName);

    switch ( id ) {
     case OpenRecent:
      pAction = new KRecentFilesAction(parent);
      break;
     case ShowMenubar:
     case ShowToolbar:
     case ShowStatusbar:
      pAction = new KAction(parent);
      pAction->setCheckable(true);
      pAction->setChecked(true);
      break;
     case FullScreen:
      pAction = new KToggleFullScreenAction(parent);
      pAction->setCheckable(true);
      break;
    case PasteText:
      pAction = new KPasteTextAction(parent);
      break;
    // Same as default, but with the app icon
    case AboutApp:
      pAction = new KAction(parent);
      icon = qApp->windowIcon();
      break;

     default:
      pAction = new KAction(parent);
      break;
    }

    switch ( id ) {
    case Quit:
      pAction->setMenuRole(QAction::QuitRole);
      break;

    case Preferences:
      pAction->setMenuRole(QAction::PreferencesRole);
      break;

    case AboutApp:
      pAction->setMenuRole(QAction::AboutRole);
      break;

    default:
      pAction->setMenuRole(QAction::NoRole);
      break;
    }

    pAction->setText(sLabel);
    pAction->setIcon(icon);

    KShortcut cut = KStandardShortcut::shortcut(pInfo->idAccel);
    if (!cut.isEmpty())
        pAction->setShortcut(cut);

    pAction->setObjectName(pInfo->psName);
  }

  if (recvr && slot) {
      if (id == OpenRecent) {
          // FIXME KAction port: probably a good idea to find a cleaner way to do this
          // Open Recent is a special case - provide the selected URL
          QObject::connect(pAction, SIGNAL(urlSelected(const KUrl &)), recvr, slot);
      } else if (id == ConfigureToolbars) { // #200815
          QObject::connect(pAction, SIGNAL(triggered(bool)), recvr, slot, Qt::QueuedConnection);
      } else {
          QObject::connect(pAction, SIGNAL(triggered(bool)), recvr, slot);
      }
  }

  KActionCollection *collection = qobject_cast<KActionCollection *>(parent);
  if (pAction && collection)
      collection->addAction(pAction->objectName(), pAction);

  return pAction;
}

const char* name( StandardAction id )
{
  const KStandardActionInfo* pInfo = infoPtr( id );
  return (pInfo) ? pInfo->psName : 0;
}

KAction *openNew(const QObject *recvr, const char *slot, QObject *parent)
{
  return KStandardAction::create(New, recvr, slot, parent);
}

KAction *open(const QObject *recvr, const char *slot, QObject *parent)
{
  return KStandardAction::create(Open, recvr, slot, parent);
}

KRecentFilesAction *openRecent(const QObject *recvr, const char *slot, QObject *parent)
{
  return (KRecentFilesAction*) KStandardAction::create( OpenRecent, recvr, slot, parent );
}

KAction *save(const QObject *recvr, const char *slot, QObject *parent)
{
  return KStandardAction::create(Save, recvr, slot, parent);
}

KAction *saveAs(const QObject *recvr, const char *slot, QObject *parent)
{
  return KStandardAction::create(SaveAs, recvr, slot, parent);
}

KAction *revert(const QObject *recvr, const char *slot, QObject *parent)
{
  return KStandardAction::create(Revert, recvr, slot, parent);
}

KAction *print(const QObject *recvr, const char *slot, QObject *parent)
{
  return KStandardAction::create(Print, recvr, slot, parent);
}

KAction *printPreview( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( PrintPreview, recvr, slot, parent );
}

KAction *close( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Close, recvr, slot, parent );
}

KAction *mail( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Mail, recvr, slot, parent );
}

KAction *quit( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Quit, recvr, slot, parent );
}

KAction *undo( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Undo, recvr, slot, parent );
}

KAction *redo( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Redo, recvr, slot, parent );
}

KAction *cut( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Cut, recvr, slot, parent );
}

KAction *copy( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Copy, recvr, slot, parent );
}

KAction *paste( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Paste, recvr, slot, parent );
}

KAction *pasteText( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( PasteText, recvr, slot, parent );
}

KAction *clear( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Clear, recvr, slot, parent );
}

KAction *selectAll( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( SelectAll, recvr, slot, parent );
}

KAction *deselect( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Deselect, recvr, slot, parent );
}

KAction *find( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Find, recvr, slot, parent );
}

KAction *findNext( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( FindNext, recvr, slot, parent );
}

KAction *findPrev( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( FindPrev, recvr, slot, parent );
}

KAction *replace( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Replace, recvr, slot, parent );
}

KAction *actualSize( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( ActualSize, recvr, slot, parent );
}

KAction *fitToPage( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( FitToPage, recvr, slot, parent );
}

KAction *fitToWidth( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( FitToWidth, recvr, slot, parent );
}

KAction *fitToHeight( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( FitToHeight, recvr, slot, parent );
}

KAction *zoomIn( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( ZoomIn, recvr, slot, parent );
}

KAction *zoomOut( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( ZoomOut, recvr, slot, parent );
}

KAction *zoom( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Zoom, recvr, slot, parent );
}

KAction *redisplay( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Redisplay, recvr, slot, parent );
}

KAction *up( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Up, recvr, slot, parent );
}

KAction *back( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Back, recvr, slot, parent );
}

KAction *forward( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Forward, recvr, slot, parent );
}

KAction *home( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Home, recvr, slot, parent );
}

KAction *prior( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Prior, recvr, slot, parent );
}

KAction *next( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Next, recvr, slot, parent );
}

KAction *goTo( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Goto, recvr, slot, parent );
}

KAction *gotoPage( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( GotoPage, recvr, slot, parent );
}

KAction *gotoLine( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( GotoLine, recvr, slot, parent );
}

KAction *firstPage( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( FirstPage, recvr, slot, parent );
}

KAction *lastPage( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( LastPage, recvr, slot, parent );
}

KAction *documentBack( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( DocumentBack, recvr, slot, parent );
}

KAction *documentForward( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( DocumentForward, recvr, slot, parent );
}

KAction *addBookmark( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( AddBookmark, recvr, slot, parent );
}

KAction *editBookmarks( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( EditBookmarks, recvr, slot, parent );
}

KAction *spelling( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Spelling, recvr, slot, parent );
}

static KAction *buildAutomaticAction( QObject* parent, StandardAction id, const char* slot )
{
  const KStandardActionInfo* p = infoPtr( id );
  if ( !p )
    return 0;

  AutomaticAction *action = new AutomaticAction(
      KIcon( p->psIconName ),
      i18n(p->psLabel),
      KStandardShortcut::shortcut( p->idAccel ),
      slot,
      parent);

  action->setObjectName(p->psName);
  action->setWhatsThis( i18n(p->psWhatsThis) );

  KActionCollection *collection = qobject_cast<KActionCollection *>(parent);
  if (collection)
      collection->addAction(action->objectName(), action);

    return action;
}

KAction *cut( QObject* parent )
{
  return buildAutomaticAction( parent, Cut, SLOT( cut() ) );
}

KAction *copy( QObject* parent )
{
  return buildAutomaticAction( parent, Copy, SLOT( copy() ) );
}

KAction *paste( QObject* parent )
{
  return buildAutomaticAction( parent, Paste, SLOT( paste() ) );
}

KAction *clear( QObject* parent )
{
  return buildAutomaticAction( parent, Clear, SLOT( clear() ) );
}

KAction *selectAll( QObject* parent )
{
  return buildAutomaticAction( parent, SelectAll, SLOT( selectAll() ) );
}

KToggleAction *showMenubar(const QObject *recvr, const char *slot, QObject *parent)
{
  KToggleAction *ret = new KToggleAction(i18n( "Show &Menubar" ), parent);
  ret->setObjectName(name(ShowMenubar));
  ret->setIcon( KIcon( "show-menu" ) );

  ret->setShortcut( KStandardShortcut::shortcut( KStandardShortcut::ShowMenubar ) );

  ret->setWhatsThis( i18n( "Show Menubar<p>"
                           "Shows the menubar again after it has been hidden</p>" ) );

  ret->setChecked( true );

  if ( recvr && slot )
    QObject::connect( ret, SIGNAL( triggered( bool ) ), recvr, slot );

  KActionCollection *collection = qobject_cast<KActionCollection *>(parent);
  if (collection)
    collection->addAction(ret->objectName(), ret);

  return ret;
}

KToggleAction *showStatusbar(const QObject *recvr, const char *slot, QObject *parent)
{
  KToggleAction *ret = new KToggleAction(i18n( "Show St&atusbar" ), parent);
  ret->setObjectName(name(ShowStatusbar));

  ret->setWhatsThis( i18n( "Show Statusbar<br /><br />"
                           "Shows the statusbar, which is the bar at the bottom of the window used for status information." ) );

  ret->setChecked( true );

  if ( recvr && slot )
    QObject::connect( ret, SIGNAL( triggered( bool ) ), recvr, slot );

  KActionCollection *collection = qobject_cast<KActionCollection *>(parent);
  if (collection)
    collection->addAction(ret->objectName(), ret);

  return ret;
}

KToggleFullScreenAction *fullScreen(const QObject *recvr, const char *slot, QWidget* window, QObject *parent)
{
  KToggleFullScreenAction *ret;
  ret = static_cast< KToggleFullScreenAction* >( KStandardAction::create( FullScreen, recvr, slot, parent ) );
  ret->setWindow( window );

  return ret;
}

KAction *saveOptions( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( SaveOptions, recvr, slot, parent );
}

KAction *keyBindings( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( KeyBindings, recvr, slot, parent );
}

KAction *preferences( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Preferences, recvr, slot, parent );
}

KAction *configureToolbars( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( ConfigureToolbars, recvr, slot, parent );
}

KAction *configureNotifications( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( ConfigureNotifications, recvr, slot, parent );
}

KAction *help( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( Help, recvr, slot, parent );
}

KAction *helpContents( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( HelpContents, recvr, slot, parent );
}

KAction *whatsThis( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( WhatsThis, recvr, slot, parent );
}

KAction *tipOfDay( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( TipofDay, recvr, slot, parent );
}

KAction *reportBug( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( ReportBug, recvr, slot, parent );
}

KAction *switchApplicationLanguage( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( SwitchApplicationLanguage, recvr, slot, parent );
}

KAction *aboutApp( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( AboutApp, recvr, slot, parent );
}

KAction *aboutKDE( const QObject *recvr, const char *slot, QObject *parent )
{
  return KStandardAction::create( AboutKDE, recvr, slot, parent );
}

}

