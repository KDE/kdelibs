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

#include "kstdaction.h"

#include <qstringlist.h>
#include <qtoolbutton.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <qapplication.h>
#include <kinstance.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kguiitem.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstdaccel.h>
#include <kmainwindow.h>
#include <kicon.h>

#include "krecentfilesaction.h"
#include "ktogglefullscreenaction.h"
#include "kpastetextaction.h"

#include "kstdaction_p.h"
#include "kstdaction_p.moc"

namespace KStdAction
{
    AutomaticAction::AutomaticAction( const KIcon &icon, const QString &text, const QKeySequence &cut, const char *slot,
                     KActionCollection *parent, const char *name )
      : KAction( text, parent, name )
    {
      setIcon( KIcon( icon ) );
      setShortcut( cut );
      connect( this, SIGNAL( activated() ), this, slot );
    }

QStringList stdNames()
{
  return internal_stdNames();
}

KAction* create( StdAction id, const char *name, const QObject *recvr, const char *slot, KActionCollection* parent )
{
  KAction* pAction = 0;
  const KStdActionInfo* pInfo = infoPtr( id );

  kDebug(125) << "KStdAction::create( " << id << "=" << (pInfo ? pInfo->psName : (const char*)0) << ", " << parent << ", " << name << " )" << endl; // ellis

  if ( pInfo ) {
    QString sLabel, iconName = pInfo->psIconName;
    switch( id ) {
      case Back:
        sLabel = i18nc( "go back", "&Back" );
        if ( QApplication::isRightToLeft() )
          iconName = "forward";
        break;

      case Forward:
        sLabel = i18nc( "go forward", "&Forward" );
        if ( QApplication::isRightToLeft() )
          iconName = "back";
        break;

      case Home:
        sLabel = i18nc( "beginning (of line)", "&Home" );
        break;
      case Help:
        sLabel = i18nc( "show help", "&Help" );
        break;
      case Preferences:
      case AboutApp:
      case HelpContents:
      {
        const KAboutData *aboutData = KGlobal::instance()->aboutData();
        /* TODO KDE4
        const KAboutData *aboutData;
        if ( parent )
          aboutData = parent->instance()->aboutData();
        else
          aboutData = KGlobal::instance()->aboutData();
        */
        QString appName = (aboutData) ? aboutData->programName() : qApp->applicationName();
        sLabel = i18n( pInfo->psLabel, appName );
      }
       break;
     default:
       sLabel = i18n( pInfo->psLabel );
    }

    if ( QApplication::isRightToLeft() ) {
      if ( id == Prior )
        iconName = "next";

      if ( id == Next )
        iconName = "previous";
    }

    switch ( id ) {
     case OpenRecent:
      pAction = new KRecentFilesAction( KIcon( pInfo->psIconName ), sLabel, parent, name ? name : pInfo->psName );
      break;
     case ShowMenubar:
     case ShowToolbar:
     case ShowStatusbar:
      pAction = new KAction( KIcon( pInfo->psIconName ), sLabel, parent, name ? name : pInfo->psName );
      pAction->setCheckable( true );
      pAction->setChecked( true );
      break;
     case FullScreen:
      pAction = new KToggleFullScreenAction( parent, name ? name : pInfo->psName );
      pAction->setCheckable( true );
      break;
    case PasteText:
      pAction = new KPasteTextAction( KIcon( iconName ), sLabel, parent, name ? name : pInfo->psName );
      break;
    // Same as default, but with the app icon
    case AboutApp:
      pAction = new KAction( sLabel, parent, name ? name : pInfo->psName );
      pAction->QAction::setIcon( qApp->windowIcon() );
      break;

     default:
      pAction = new KAction( iconName.isEmpty() ? KIcon() : KIcon( iconName ), sLabel, parent, name ? name : pInfo->psName );
      break;
    }

    KShortcut cut = KStdAccel::shortcut( pInfo->idAccel );
    if ( !cut.isNull() )
        pAction->setShortcut( cut );
  }

  if ( recvr && slot )
    if ( id != OpenRecent )
      QObject::connect( pAction, SIGNAL( triggered( bool ) ), recvr, slot );
    else
      // FIXME KAction port: probably a good idea to find a cleaner way to do this
      // Open Recent is a special case - provide the selected URL
      QObject::connect( pAction, SIGNAL( urlSelected( const KUrl& ) ), recvr, slot );

  return pAction;
}

const char* name( StdAction id )
{
  const KStdActionInfo* pInfo = infoPtr( id );
  return (pInfo) ? pInfo->psName : 0;
}

KAction *openNew( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( New, name, recvr, slot, parent );
}

KAction *open( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Open, name, recvr, slot, parent );
}

KRecentFilesAction *openRecent( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return (KRecentFilesAction*) KStdAction::create( OpenRecent, name, recvr, slot, parent );
}

KAction *save( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Save, name, recvr, slot, parent );
}

KAction *saveAs( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( SaveAs, name, recvr, slot, parent );
}

KAction *revert( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Revert, name, recvr, slot, parent );
}

KAction *print( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Print, name, recvr, slot, parent );
}

KAction *printPreview( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( PrintPreview, name, recvr, slot, parent );
}

KAction *close( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Close, name, recvr, slot, parent );
}

KAction *mail( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Mail, name, recvr, slot, parent );
}

KAction *quit( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Quit, name, recvr, slot, parent );
}

KAction *undo( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Undo, name, recvr, slot, parent );
}

KAction *redo( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Redo, name, recvr, slot, parent );
}

KAction *cut( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Cut, name, recvr, slot, parent );
}

KAction *copy( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Copy, name, recvr, slot, parent );
}

KAction *paste( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Paste, name, recvr, slot, parent );
}

KAction *pasteText( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( PasteText, name, recvr, slot, parent );
}

KAction *clear( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Clear, name, recvr, slot, parent );
}

KAction *selectAll( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( SelectAll, name, recvr, slot, parent );
}

KAction *deselect( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Deselect, name, recvr, slot, parent );
}

KAction *find( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Find, name, recvr, slot, parent );
}

KAction *findNext( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( FindNext, name, recvr, slot, parent );
}

KAction *findPrev( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( FindPrev, name, recvr, slot, parent );
}

KAction *replace( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Replace, name, recvr, slot, parent );
}

KAction *actualSize( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( ActualSize, name, recvr, slot, parent );
}

KAction *fitToPage( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( FitToPage, name, recvr, slot, parent );
}

KAction *fitToWidth( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( FitToWidth, name, recvr, slot, parent );
}

KAction *fitToHeight( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( FitToHeight, name, recvr, slot, parent );
}

KAction *zoomIn( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( ZoomIn, name, recvr, slot, parent );
}

KAction *zoomOut( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( ZoomOut, name, recvr, slot, parent );
}

KAction *zoom( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Zoom, name, recvr, slot, parent );
}

KAction *redisplay( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Redisplay, name, recvr, slot, parent );
}

KAction *up( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Up, name, recvr, slot, parent );
}

KAction *back( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Back, name, recvr, slot, parent );
}

KAction *forward( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Forward, name, recvr, slot, parent );
}

KAction *home( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Home, name, recvr, slot, parent );
}

KAction *prior( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Prior, name, recvr, slot, parent );
}

KAction *next( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Next, name, recvr, slot, parent );
}

KAction *goTo( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Goto, name, recvr, slot, parent );
}

KAction *gotoPage( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( GotoPage, name, recvr, slot, parent );
}

KAction *gotoLine( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( GotoLine, name, recvr, slot, parent );
}

KAction *firstPage( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( FirstPage, name, recvr, slot, parent );
}

KAction *lastPage( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( LastPage, name, recvr, slot, parent );
}

KAction *addBookmark( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( AddBookmark, name, recvr, slot, parent );
}

KAction *editBookmarks( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( EditBookmarks, name, recvr, slot, parent );
}

KAction *spelling( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Spelling, name, recvr, slot, parent );
}

KAction *buildAutomaticAction( KActionCollection* parent, StdAction id, const char* slot )
{
  const KStdActionInfo* p = infoPtr( id );
  if ( !p )
    return 0;

  AutomaticAction *action = new AutomaticAction( KIcon( p->psIconName ), p->psLabel, KStdAccel::shortcut( p->idAccel ),
                                                 slot, parent, p->psName );
  action->setWhatsThis( p->psWhatsThis );

  return action;
}

KAction *cut( KActionCollection* parent )
{
  return buildAutomaticAction( parent, Cut, SLOT( cut() ) );
}

KAction *copy( KActionCollection* parent )
{
  return buildAutomaticAction( parent, Copy, SLOT( copy() ) );
}

KAction *paste( KActionCollection* parent )
{
  return buildAutomaticAction( parent, Paste, SLOT( paste() ) );
}

KAction *clear( KActionCollection* parent )
{
  return buildAutomaticAction( parent, Clear, SLOT( clear() ) );
}

KAction *selectAll( KActionCollection* parent )
{
  return buildAutomaticAction( parent, SelectAll, SLOT( selectAll() ) );
}

KToggleAction *showMenubar( const QObject *recvr, const char *slot, KActionCollection* parent, const char *_name )
{
  KToggleAction *ret = new KToggleAction( i18n( "Show &Menubar" ), parent, _name ? _name : name( ShowMenubar ) );
  ret->setIcon( KIcon( "showmenu" ) );

  if ( recvr && slot )
    QObject::connect( ret, SIGNAL( triggered( bool ) ), recvr, slot );

  ret->setShortcut( KStdAccel::shortcut( KStdAccel::ShowMenubar ) );

  ret->setWhatsThis( i18n( "Show Menubar<p>"
                           "Shows the menubar again after it has been hidden" ) );
  KGuiItem guiItem( i18n("Hide &Menubar"), 0 /*same icon*/, QString(),
                    i18n( "Hide Menubar<p>"
                    "Hide the menubar. You can usually get it back using the right mouse button inside the window itself." ) );
  ret->setCheckedState( guiItem );

  ret->setChecked( true );

  return ret;
}

KToggleAction *showStatusbar( const QObject *recvr, const char *slot,
                              KActionCollection* parent, const char *_name )
{
  KToggleAction *ret = new KToggleAction( i18n( "Show St&atusbar" ), parent, _name ? _name : name( ShowStatusbar ) );

  if ( recvr && slot )
    QObject::connect( ret, SIGNAL( triggered( bool ) ), recvr, slot );

  ret->setWhatsThis( i18n( "Show Statusbar<p>"
                           "Shows the statusbar, which is the bar at the bottom of the window used for status information." ) );
  KGuiItem guiItem( i18n( "Hide St&atusbar" ), QString(), QString(),
                    i18n( "Hide Statusbar<p>"
                          "Hides the statusbar, which is the bar at the bottom of the window used for status information." ) );
  ret->setCheckedState( guiItem );

  ret->setChecked( true );

  return ret;
}

KToggleFullScreenAction *fullScreen( const QObject *recvr, const char *slot, KActionCollection* parent,
                                     QWidget* window, const char *name )
{
  KToggleFullScreenAction *ret;
  ret = static_cast< KToggleFullScreenAction* >( KStdAction::create( FullScreen, name, recvr, slot, parent ) );
  ret->setWindow( window );

  return ret;
}

KAction *saveOptions( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( SaveOptions, name, recvr, slot, parent );
}

KAction *keyBindings( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( KeyBindings, name, recvr, slot, parent );
}

KAction *preferences( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Preferences, name, recvr, slot, parent );
}

KAction *configureToolbars( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( ConfigureToolbars, name, recvr, slot, parent );
}

KAction *configureNotifications( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( ConfigureNotifications, name, recvr, slot, parent );
}

KAction *help( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( Help, name, recvr, slot, parent );
}

KAction *helpContents( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( HelpContents, name, recvr, slot, parent );
}

KAction *whatsThis( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( WhatsThis, name, recvr, slot, parent );
}

KAction *tipOfDay( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( TipofDay, name, recvr, slot, parent );
}

KAction *reportBug( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( ReportBug, name, recvr, slot, parent );
}

KAction *aboutApp( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( AboutApp, name, recvr, slot, parent );
}

KAction *aboutKDE( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
  return KStdAction::create( AboutKDE, name, recvr, slot, parent );
}

}

