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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kstdaction.h"

#include <qstringlist.h>
#include <qtoolbutton.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstdaccel.h>
#include <kmainwindow.h>
#include "kstdaction_p.h"

namespace KStdAction
{

QStringList stdNames()
{
    return internal_stdNames();
}

KAction* create( StdAction id, const char *name, const QObject *recvr, const char *slot, KActionCollection* parent )
{
	KAction* pAction = 0;
	const KStdActionInfo* pInfo = infoPtr( id );
	kdDebug(125) << "KStdAction::create( " << id << "=" << (pInfo ? pInfo->psName : (const char*)0) << ", " << parent << ", " << name << " )" << endl; // ellis
	if( pInfo ) {
		QString sLabel, iconName = pInfo->psIconName;
		switch( id ) {
		 case Back: sLabel = i18n("go back", "&Back");
			if (QApplication::reverseLayout() )
			    iconName = "forward";
			break;

		 case Forward: sLabel = i18n("go forward", "&Forward");
			if (QApplication::reverseLayout() )
			    iconName = "back";
			break;

		 case Home: sLabel = i18n("beginning (of line)", "&Home"); break;
		 case Help: sLabel = i18n("show help", "&Help"); break;
		 case AboutApp: iconName = kapp->miniIconName();
		 case Preferences:
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
			QString appName = (aboutData) ? aboutData->programName() : QLatin1String(qApp->name());
			sLabel = i18n(pInfo->psLabel).arg(appName);
			}
			break;
		 default: sLabel = i18n(pInfo->psLabel);
		}

		if (QApplication::reverseLayout()){
			if (id == Prior) iconName = "next";
			if (id == Next ) iconName = "previous";
		}

		KShortcut cut = KStdAccel::shortcut(pInfo->idAccel);
		switch( id ) {
		 case OpenRecent:
			pAction = new KRecentFilesAction( sLabel, pInfo->psIconName, cut,
					recvr, slot,
					parent, (name) ? name : pInfo->psName );
			break;
		 case ShowMenubar:
		 case ShowToolbar:
		 case ShowStatusbar:
		 {
			KToggleAction *ret;
			ret = new KToggleAction( sLabel, pInfo->psIconName, cut,
					recvr, slot,
					parent, (name) ? name : pInfo->psName );
			ret->setChecked( true );
			pAction = ret;
			break;
		 }
		 case FullScreen:
		 {
			KToggleFullScreenAction *ret;
			ret = new KToggleFullScreenAction( cut, recvr, slot,
					parent, NULL, (name) ? name : pInfo->psName );
			ret->setChecked( false );
			pAction = ret;
			break;
		 }
         case PasteText:
         {
            KPasteTextAction *ret;
            ret = new KPasteTextAction(sLabel, iconName, cut,
					recvr, slot,
					parent, (name) ? name : pInfo->psName );
            pAction = ret;
            break;
         }
		 default:
			pAction = new KAction( sLabel, iconName, cut,
					recvr, slot,
					parent, (name) ? name : pInfo->psName );
			break;
		}
	}
	return pAction;
}

const char* name( StdAction id )
{
	const KStdActionInfo* pInfo = infoPtr( id );
	return (pInfo) ? pInfo->psName : 0;
}

KAction *openNew( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( New, name, recvr, slot, parent ); }
KAction *open( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Open, name, recvr, slot, parent ); }
KRecentFilesAction *openRecent( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return (KRecentFilesAction*) KStdAction::create( OpenRecent, name, recvr, slot, parent ); }
KAction *save( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Save, name, recvr, slot, parent ); }
KAction *saveAs( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( SaveAs, name, recvr, slot, parent ); }
KAction *revert( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Revert, name, recvr, slot, parent ); }
KAction *print( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Print, name, recvr, slot, parent ); }
KAction *printPreview( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( PrintPreview, name, recvr, slot, parent ); }
KAction *close( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Close, name, recvr, slot, parent ); }
KAction *mail( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Mail, name, recvr, slot, parent ); }
KAction *quit( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Quit, name, recvr, slot, parent ); }
KAction *undo( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Undo, name, recvr, slot, parent ); }
KAction *redo( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Redo, name, recvr, slot, parent ); }
KAction *cut( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Cut, name, recvr, slot, parent ); }
KAction *copy( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Copy, name, recvr, slot, parent ); }
KAction *paste( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Paste, name, recvr, slot, parent ); }
KAction *pasteText( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( PasteText, name, recvr, slot, parent ); }
KAction *clear( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Clear, name, recvr, slot, parent ); }
KAction *selectAll( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( SelectAll, name, recvr, slot, parent ); }
KAction *deselect( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Deselect, name, recvr, slot, parent ); }
KAction *find( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Find, name, recvr, slot, parent ); }
KAction *findNext( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( FindNext, name, recvr, slot, parent ); }
KAction *findPrev( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( FindPrev, name, recvr, slot, parent ); }
KAction *replace( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Replace, name, recvr, slot, parent ); }
KAction *actualSize( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( ActualSize, name, recvr, slot, parent ); }
KAction *fitToPage( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( FitToPage, name, recvr, slot, parent ); }
KAction *fitToWidth( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( FitToWidth, name, recvr, slot, parent ); }
KAction *fitToHeight( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( FitToHeight, name, recvr, slot, parent ); }
KAction *zoomIn( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( ZoomIn, name, recvr, slot, parent ); }
KAction *zoomOut( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( ZoomOut, name, recvr, slot, parent ); }
KAction *zoom( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Zoom, name, recvr, slot, parent ); }
KAction *redisplay( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Redisplay, name, recvr, slot, parent ); }
KAction *up( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Up, name, recvr, slot, parent ); }
KAction *back( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Back, name, recvr, slot, parent ); }
KAction *forward( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Forward, name, recvr, slot, parent ); }
KAction *home( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Home, name, recvr, slot, parent ); }
KAction *prior( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Prior, name, recvr, slot, parent ); }
KAction *next( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Next, name, recvr, slot, parent ); }
KAction *goTo( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Goto, name, recvr, slot, parent ); }
KAction *gotoPage( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( GotoPage, name, recvr, slot, parent ); }
KAction *gotoLine( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( GotoLine, name, recvr, slot, parent ); }
KAction *firstPage( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( FirstPage, name, recvr, slot, parent ); }
KAction *lastPage( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( LastPage, name, recvr, slot, parent ); }
KAction *addBookmark( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( AddBookmark, name, recvr, slot, parent ); }
KAction *editBookmarks( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( EditBookmarks, name, recvr, slot, parent ); }
KAction *spelling( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Spelling, name, recvr, slot, parent ); }

KToggleAction *showMenubar( const QObject *recvr, const char *slot, KActionCollection* parent, const char *_name )
{
    KToggleAction *ret;
    ret = new KToggleAction(i18n("Show &Menubar"), "showmenu", KStdAccel::shortcut(KStdAccel::ShowMenubar), recvr, slot,
                            parent, _name ? _name : name(ShowMenubar));
    ret->setWhatsThis( i18n( "Show Menubar<p>"
                             "Shows the menubar again after it has been hidden" ) );
    KGuiItem guiItem( i18n("Hide &Menubar"), 0 /*same icon*/, QString::null,
                      i18n( "Hide Menubar<p>"
                            "Hide the menubar. You can usually get it back using the right mouse button inside the window itself." ) );
    ret->setCheckedState( guiItem );
    ret->setChecked(true);
    return ret;
}

KToggleAction *showStatusbar( const QObject *recvr, const char *slot,
                                         KActionCollection* parent, const char *_name )
{
    KToggleAction *ret;
    ret = new KToggleAction(i18n("Show St&atusbar"), 0, recvr, slot, parent,
                            _name ? _name : name(ShowStatusbar));
    ret->setWhatsThis( i18n( "Show Statusbar<p>"
                             "Shows the statusbar, which is the bar at the bottom of the window used for status information." ) );
    KGuiItem guiItem( i18n("Hide St&atusbar"), QString::null, QString::null,
                      i18n( "Hide Statusbar<p>"
                            "Hides the statusbar, which is the bar at the bottom of the window used for status information." ) );
    ret->setCheckedState( guiItem );

    ret->setChecked(true);
    return ret;
}

KToggleFullScreenAction *fullScreen( const QObject *recvr, const char *slot, KActionCollection* parent,
    QWidget* window, const char *name )
{
    KToggleFullScreenAction *ret;
    ret = static_cast< KToggleFullScreenAction* >( KStdAction::create( FullScreen, name, recvr, slot, parent ));
    ret->setWindow( window );
    return ret;
}

KAction *saveOptions( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( SaveOptions, name, recvr, slot, parent ); }
KAction *keyBindings( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( KeyBindings, name, recvr, slot, parent ); }
KAction *preferences( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Preferences, name, recvr, slot, parent ); }
KAction *configureToolbars( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( ConfigureToolbars, name, recvr, slot, parent ); }
KAction *configureNotifications( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( ConfigureNotifications, name, recvr, slot, parent ); }
KAction *help( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( Help, name, recvr, slot, parent ); }
KAction *helpContents( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( HelpContents, name, recvr, slot, parent ); }
KAction *whatsThis( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( WhatsThis, name, recvr, slot, parent ); }
KAction *tipOfDay( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( TipofDay, name, recvr, slot, parent ); }
KAction *reportBug( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( ReportBug, name, recvr, slot, parent ); }
KAction *aboutApp( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( AboutApp, name, recvr, slot, parent ); }
KAction *aboutKDE( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
	{ return KStdAction::create( AboutKDE, name, recvr, slot, parent ); }

}
