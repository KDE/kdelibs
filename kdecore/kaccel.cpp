/*
    Copyright (c) 2001,2002 Ellis Whitehead <ellis@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kaccel.h"

#include <qaccel.h>
#include <qguardedptr.h>
#include <qpopupmenu.h>
#include <qregexp.h>
#include <qstring.h>
#include <qtimer.h>

#include "kaccelbase.h"
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kshortcut.h>

#include "kaccelprivate.h"

#ifdef Q_WS_X11
#	include <X11/Xlib.h>
#	ifdef KeyPress // needed for --enable-final
		// defined by X11 headers
		const int XKeyPress = KeyPress;
#		undef KeyPress
#	endif
#endif

// TODO: Put in kaccelbase.cpp
//---------------------------------------------------------------------
// KAccelEventHandler
//---------------------------------------------------------------------
//
// In KAccelEventHandler::x11Event we do our own X11 keyboard event handling
// This allows us to map the Win key to Qt::MetaButton, Qt does not know about
// the Win key.
//
// KAccelEventHandler::x11Event will generate an AccelOverride event. The
// AccelOverride event is abused a bit to ensure that KAccelPrivate::eventFilter
// (as an event filter on the toplevel widget) will get the key event first
// (in the form of AccelOverride) before any of the intermediate widgets are
// able to process it.
//
// Qt normally sends an AccelOverride, Accel and then a KeyPress event.
// A widget can accept the AccelOverride event in which case the Accel event will be
// skipped and the KeyPress is followed immediately.
// If the Accel event is accepted, no KeyPress event will follow.
//
// KAccelEventHandler::x11Event converts a X11 keyboard event into an AccelOverride
// event, there are now two possibilities:
//
// 1) If KAccel intercepts the AccelOverride we are done and can consider the X11
// keyboard event as handled.
// 2) If another widget accepts the AccelOverride, it will expect to get a normal
// Qt generated KeyPress event afterwards. So we let Qt handle the X11 keyboard event
// again. However, this will first generate an AccelOverride event, and we already
// had send that one. To compnesate for this, the global event filter in KApplication
// is instructed to eat the next AccelOveride event. Qt will then send a normal KeyPress
// event and from then on everything is normal again.
//
// kde_g_bKillAccelOverride is used to tell KApplication::notify to eat the next
// AccelOverride event.

bool kde_g_bKillAccelOverride = false;

class KAccelEventHandler : public QWidget
{
 public:
	static KAccelEventHandler* self()
	{
		if( !g_pSelf )
			g_pSelf = new KAccelEventHandler;
		return g_pSelf;
	}

	static void accelActivated( bool b ) { g_bAccelActivated = b; }

 private:
	KAccelEventHandler();

#	ifdef Q_WS_X11
	bool x11Event( XEvent* pEvent );
#	endif

	static KAccelEventHandler* g_pSelf;
	static bool g_bAccelActivated;
};

KAccelEventHandler* KAccelEventHandler::g_pSelf = 0;
bool KAccelEventHandler::g_bAccelActivated = false;

KAccelEventHandler::KAccelEventHandler()
    : QWidget( 0, "KAccelEventHandler" )
{
#	ifdef Q_WS_X11
	if ( kapp )
		kapp->installX11EventFilter( this );
#	endif
}

#ifdef Q_WS_X11
bool	qt_try_modal( QWidget *, XEvent * );

bool KAccelEventHandler::x11Event( XEvent* pEvent )
{
	if( QWidget::keyboardGrabber() || !kapp->focusWidget() )
		return false;

	if ( !qt_try_modal(kapp->focusWidget(), pEvent) )
	        return false;

	if( pEvent->type == XKeyPress ) {
		KKeyNative keyNative( pEvent );
		KKey key( keyNative );
		key.simplify();
		int keyCodeQt = key.keyCodeQt();
		int state = 0;
		if( key.modFlags() & KKey::SHIFT ) state |= Qt::ShiftButton;
		if( key.modFlags() & KKey::CTRL )  state |= Qt::ControlButton;
		if( key.modFlags() & KKey::ALT )   state |= Qt::AltButton;
		if( key.modFlags() & KKey::WIN )   state |= Qt::MetaButton;

		QKeyEvent ke( QEvent::AccelOverride, keyCodeQt, 0,  state );
		ke.ignore();

		g_bAccelActivated = false;
		kapp->sendEvent( kapp->focusWidget(), &ke );

		// If the Override event was accepted from a non-KAccel widget,
		//  then kill the next AccelOverride in KApplication::notify.
		if( ke.isAccepted() && !g_bAccelActivated )
			kde_g_bKillAccelOverride = true;

		// Stop event processing if a KDE accelerator was activated.
		return g_bAccelActivated;
	}

	return false;
}
#endif // Q_WS_X11

//---------------------------------------------------------------------
// KAccelPrivate
//---------------------------------------------------------------------

KAccelPrivate::KAccelPrivate( KAccel* pParent, QWidget* pWatch )
: KAccelBase( KAccelBase::QT_KEYS )
{
	//kdDebug(125) << "KAccelPrivate::KAccelPrivate( pParent = " << pParent << " ): this = " << this << endl;
	m_pAccel = pParent;
	m_pWatch = pWatch;
	m_bAutoUpdate = true;
	connect( (QAccel*)m_pAccel, SIGNAL(activated(int)), this, SLOT(slotKeyPressed(int)) );

#ifdef Q_WS_X11 //only makes sense if KAccelEventHandler is working
	if( m_pWatch )
		m_pWatch->installEventFilter( this );
#endif
	KAccelEventHandler::self();
}

void KAccelPrivate::setEnabled( bool bEnabled )
{
	m_bEnabled = bEnabled;
	((QAccel*)m_pAccel)->setEnabled( bEnabled );
}

bool KAccelPrivate::setEnabled( const QString& sAction, bool bEnable )
{
	kdDebug(125) << "KAccelPrivate::setEnabled( \"" << sAction << "\", " << bEnable << " ): this = " << this << endl;
	KAccelAction* pAction = actionPtr( sAction );
	if( !pAction )
		return false;
	if( pAction->isEnabled() == bEnable )
		return true;

	pAction->setEnabled( bEnable );

	QMap<int, KAccelAction*>::const_iterator it = m_mapIDToAction.begin();
	for( ; it != m_mapIDToAction.end(); ++it ) {
		if( *it == pAction )
			((QAccel*)m_pAccel)->setItemEnabled( it.key(), bEnable );
	}
	return true;
}

bool KAccelPrivate::removeAction( const QString& sAction )
{
	// FIXME: getID() doesn't contains any useful
	//  information!  Use mapIDToAction. --ellis, 2/May/2002
	//  Or maybe KAccelBase::remove() takes care of QAccel indirectly...
	KAccelAction* pAction = actions().actionPtr( sAction );
	if( pAction ) {
		int nID = pAction->getID();
		//bool b = actions().removeAction( sAction );
		bool b = KAccelBase::remove( sAction );
		((QAccel*)m_pAccel)->removeItem( nID );
		return b;
	} else
		return false;
}

bool KAccelPrivate::emitSignal( KAccelBase::Signal signal )
{
	if( signal == KAccelBase::KEYCODE_CHANGED ) {
		m_pAccel->emitKeycodeChanged();
		return true;
	}
	return false;
}

bool KAccelPrivate::connectKey( KAccelAction& action, const KKeyServer::Key& key )
{
	uint keyQt = key.keyCodeQt();
	int nID = ((QAccel*)m_pAccel)->insertItem( keyQt );
	m_mapIDToAction[nID] = &action;
	m_mapIDToKey[nID] = keyQt;

	if( action.objSlotPtr() && action.methodSlotPtr() ) {
#ifdef Q_WS_WIN /** @todo TEMP: new implementation (commit #424926) didn't work */
		((QAccel*)m_pAccel)->connectItem( nID, action.objSlotPtr(), action.methodSlotPtr() );
#else
		((QAccel*)m_pAccel)->connectItem( nID, this, SLOT(slotKeyPressed(int)));
#endif
		if( !action.isEnabled() )
			((QAccel*)m_pAccel)->setItemEnabled( nID, false );
	}

	kdDebug(125) << "KAccelPrivate::connectKey( \"" << action.name() << "\", " << key.key().toStringInternal() << " = 0x" << QString::number(keyQt,16) << " ): id = " << nID << " m_pObjSlot = " << action.objSlotPtr() << endl;
	//kdDebug(125) << "m_pAccel = " << m_pAccel << endl;
	return nID != 0;
}

bool KAccelPrivate::connectKey( const KKeyServer::Key& key )
{
	uint keyQt = key.keyCodeQt();
	int nID = ((QAccel*)m_pAccel)->insertItem( keyQt );

	m_mapIDToKey[nID] = keyQt;

	kdDebug(125) << "KAccelPrivate::connectKey( " << key.key().toStringInternal() << " = 0x" << QString::number(keyQt,16) << " ): id = " << nID << endl;
	return nID != 0;
}

bool KAccelPrivate::disconnectKey( KAccelAction& action, const KKeyServer::Key& key )
{
	int keyQt = key.keyCodeQt();
	QMap<int, int>::iterator it = m_mapIDToKey.begin();
	for( ; it != m_mapIDToKey.end(); ++it ) {
		//kdDebug(125) << "m_mapIDToKey[" << it.key() << "] = " << QString::number(*it,16) << " == " << QString::number(keyQt,16) << endl;
		if( *it == keyQt ) {
			int nID = it.key();
			kdDebug(125) << "KAccelPrivate::disconnectKey( \"" << action.name() << "\", 0x" << QString::number(keyQt,16) << " ) : id = " << nID << " m_pObjSlot = " << action.objSlotPtr() << endl;
			((QAccel*)m_pAccel)->removeItem( nID );
			m_mapIDToAction.remove( nID );
			m_mapIDToKey.remove( it );
			return true;
		}
	}
	//kdWarning(125) << kdBacktrace() << endl;
	kdWarning(125) << "Didn't find key in m_mapIDToKey." << endl;
	return false;
}

bool KAccelPrivate::disconnectKey( const KKeyServer::Key& key )
{
	int keyQt = key.keyCodeQt();
	kdDebug(125) << "KAccelPrivate::disconnectKey( 0x" << QString::number(keyQt,16) << " )" << endl;
	QMap<int, int>::iterator it = m_mapIDToKey.begin();
	for( ; it != m_mapIDToKey.end(); ++it ) {
		if( *it == keyQt ) {
			((QAccel*)m_pAccel)->removeItem( it.key() );
			m_mapIDToKey.remove( it );
			return true;
		}
	}
	//kdWarning(125) << kdBacktrace() << endl;
	kdWarning(125) << "Didn't find key in m_mapIDTokey." << endl;
	return false;
}

void KAccelPrivate::slotKeyPressed( int id )
{
	kdDebug(125) << "KAccelPrivate::slotKeyPressed( " << id << " )" << endl;

	if( m_mapIDToKey.contains( id ) ) {
		KKey key = m_mapIDToKey[id];
		KKeySequence seq( key );
		QPopupMenu* pMenu = createPopupMenu( m_pWatch, seq );

		// If there was only one action mapped to this key,
		//  and that action is not a multi-key shortcut,
		//  then activated it without popping up the menu.
		// This is needed for when there are multiple actions
		//  with the same shortcut where all but one is disabled.
		// pMenu->count() also counts the menu title, so one shortcut will give count = 2.
		if( pMenu->count() == 2 && pMenu->accel(1).isEmpty() ) {
			int iAction = pMenu->idAt(1);
			slotMenuActivated( iAction );
		} else {
			connect( pMenu, SIGNAL(activated(int)), this, SLOT(slotMenuActivated(int)) );
			pMenu->exec( m_pWatch->mapToGlobal( QPoint( 0, 0 ) ) );
			disconnect( pMenu, SIGNAL(activated(int)), this, SLOT(slotMenuActivated(int)) );
		}
		delete pMenu;
	}
}

void KAccelPrivate::slotShowMenu()
{
}

void KAccelPrivate::slotMenuActivated( int iAction )
{
	kdDebug(125) << "KAccelPrivate::slotMenuActivated( " << iAction << " )" << endl;
	KAccelAction* pAction = actions().actionPtr( iAction );
#ifdef Q_WS_WIN /** @todo TEMP: new implementation (commit #424926) didn't work */
	if( pAction ) {
		connect( this, SIGNAL(menuItemActivated()), pAction->objSlotPtr(), pAction->methodSlotPtr() );
		emit menuItemActivated();
		disconnect( this, SIGNAL(menuItemActivated()), pAction->objSlotPtr(), pAction->methodSlotPtr() );
	}
#else
	emitActivatedSignal( pAction );
#endif
}

bool KAccelPrivate::eventFilter( QObject* /*pWatched*/, QEvent* pEvent )
{
	if( pEvent->type() == QEvent::AccelOverride && m_bEnabled ) {
		QKeyEvent* pKeyEvent = (QKeyEvent*) pEvent;
		KKey key( pKeyEvent );
		kdDebug(125) << "KAccelPrivate::eventFilter( AccelOverride ): this = " << this << ", key = " << key.toStringInternal() << endl;
		int keyCodeQt = key.keyCodeQt();
		QMap<int, int>::iterator it = m_mapIDToKey.begin();
		for( ; it != m_mapIDToKey.end(); ++it ) {
			if( (*it) == keyCodeQt ) {
				int nID = it.key();
				kdDebug(125) << "shortcut found!" << endl;
				if( m_mapIDToAction.contains( nID ) ) {
					// TODO: reduce duplication between here and slotMenuActivated
					KAccelAction* pAction = m_mapIDToAction[nID];
					if( !pAction->isEnabled() )
						continue;
#ifdef Q_WS_WIN /** @todo TEMP: new implementation (commit #424926) didn't work */
					QGuardedPtr<KAccelPrivate> me = this;
					connect( this, SIGNAL(menuItemActivated()), pAction->objSlotPtr(), pAction->methodSlotPtr() );
					emit menuItemActivated();
					if (me) {
						disconnect( me, SIGNAL(menuItemActivated()), pAction->objSlotPtr(), pAction->methodSlotPtr() );
					}
#else
					emitActivatedSignal( pAction );
#endif
				} else
					slotKeyPressed( nID );

				pKeyEvent->accept();
				KAccelEventHandler::accelActivated( true );
				return true;
			}
		}
	}
	return false;
}

#ifndef Q_WS_WIN /** @todo TEMP: new implementation (commit #424926) didn't work */
void KAccelPrivate::emitActivatedSignal( KAccelAction* pAction )
{
	if( pAction ) {
		QGuardedPtr<KAccelPrivate> me = this;
		QRegExp reg( "([ ]*KAccelAction.*)" );
		if( reg.search( pAction->methodSlotPtr()) >= 0 ) {
			connect( this, SIGNAL(menuItemActivated(KAccelAction*)),
				pAction->objSlotPtr(), pAction->methodSlotPtr() );
			emit menuItemActivated( pAction );
			if (me)
				disconnect( me, SIGNAL(menuItemActivated(KAccelAction*)),
					pAction->objSlotPtr(), pAction->methodSlotPtr() );
		} else {
			connect( this, SIGNAL(menuItemActivated()),
				pAction->objSlotPtr(), pAction->methodSlotPtr() );
			emit menuItemActivated();
			if (me)
				disconnect( me, SIGNAL(menuItemActivated()),
					pAction->objSlotPtr(), pAction->methodSlotPtr() );

		}
	}
}
#endif

//---------------------------------------------------------------------
// KAccel
//---------------------------------------------------------------------

KAccel::KAccel( QWidget* pParent, const char* psName )
: QAccel( pParent, (psName) ? psName : "KAccel-QAccel" )
{
	kdDebug(125) << "KAccel( pParent = " << pParent << ", psName = " << psName << " ): this = " << this << endl;
	d = new KAccelPrivate( this, pParent );
}

KAccel::KAccel( QWidget* watch, QObject* pParent, const char* psName )
: QAccel( watch, pParent, (psName) ? psName : "KAccel-QAccel" )
{
	kdDebug(125) << "KAccel( watch = " << watch << ", pParent = " << pParent << ", psName = " << psName << " ): this = " << this << endl;
	if( !watch )
		kdDebug(125) << kdBacktrace() << endl;
	d = new KAccelPrivate( this, watch );
}

KAccel::~KAccel()
{
	kdDebug(125) << "~KAccel(): this = " << this << endl;
	delete d;
}

KAccelActions& KAccel::actions()             { return d->actions(); }
const KAccelActions& KAccel::actions() const { return d->actions(); }
bool KAccel::isEnabled()                     { return d->isEnabled(); }
void KAccel::setEnabled( bool bEnabled )     { d->setEnabled( bEnabled ); }
bool KAccel::setAutoUpdate( bool bAuto )     { return d->setAutoUpdate( bAuto ); }

KAccelAction* KAccel::insert( const QString& sAction, const QString& sLabel, const QString& sWhatsThis,
		const KShortcut& cutDef,
		const QObject* pObjSlot, const char* psMethodSlot,
		bool bConfigurable, bool bEnabled )
{
	return d->insert( sAction, sLabel, sWhatsThis,
		cutDef, cutDef,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
}

KAccelAction* KAccel::insert( const QString& sAction, const QString& sLabel, const QString& sWhatsThis,
		const KShortcut& cutDef3, const KShortcut& cutDef4,
		const QObject* pObjSlot, const char* psMethodSlot,
		bool bConfigurable, bool bEnabled )
{
	return d->insert( sAction, sLabel, sWhatsThis,
		cutDef3, cutDef4,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
}

KAccelAction* KAccel::insert( const char* psAction, const KShortcut& cutDef,
		const QObject* pObjSlot, const char* psMethodSlot,
		bool bConfigurable, bool bEnabled )
{
	return d->insert( psAction, i18n(psAction), QString::null,
		cutDef, cutDef,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
}

KAccelAction* KAccel::insert( KStdAccel::StdAccel id,
		const QObject* pObjSlot, const char* psMethodSlot,
		bool bConfigurable, bool bEnabled )
{
	QString sAction = KStdAccel::name( id );
	if( sAction.isEmpty() )
		return 0;

	KAccelAction* pAction = d->insert( sAction, KStdAccel::label( id ), KStdAccel::whatsThis( id ),
		KStdAccel::shortcutDefault3( id ), KStdAccel::shortcutDefault4( id ),
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
	if( pAction )
		pAction->setShortcut( KStdAccel::shortcut( id ) );

	return pAction;
}

bool KAccel::remove( const QString& sAction )
	{ return d->removeAction( sAction ); }
bool KAccel::updateConnections()
	{ return d->updateConnections(); }

const KShortcut& KAccel::shortcut( const QString& sAction ) const
{
	const KAccelAction* pAction = actions().actionPtr( sAction );
	return (pAction) ? pAction->shortcut() : KShortcut::null();
}

bool KAccel::setSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot )
	{ return d->setActionSlot( sAction, pObjSlot, psMethodSlot ); }

bool KAccel::setEnabled( const QString& sAction, bool bEnable )
	{ return d->setEnabled( sAction, bEnable ); }

bool KAccel::setShortcut( const QString& sAction, const KShortcut& cut )
{
	kdDebug(125) << "KAccel::setShortcut( \"" << sAction << "\", " << cut.toStringInternal() << " )" << endl;
	KAccelAction* pAction = actions().actionPtr( sAction );
	if( pAction ) {
		if( pAction->shortcut() != cut )
			return d->setShortcut( sAction, cut );
		return true;
	}
	return false;
}

const QString& KAccel::configGroup() const
	{ return d->configGroup(); }
// for kdegames/ksirtet
void KAccel::setConfigGroup( const QString& s )
	{ d->setConfigGroup( s ); }

bool KAccel::readSettings( KConfigBase* pConfig )
{
	d->readSettings( pConfig );
	return true;
}

bool KAccel::writeSettings( KConfigBase* pConfig ) const
	{ d->writeSettings( pConfig ); return true; }

void KAccel::emitKeycodeChanged()
{
	kdDebug(125) << "KAccel::emitKeycodeChanged()" << endl;
	emit keycodeChanged();
}

#ifndef KDE_NO_COMPAT
//------------------------------------------------------------
// Obsolete methods -- for backward compatibility
//------------------------------------------------------------

bool KAccel::insertItem( const QString& sLabel, const QString& sAction,
		const char* cutsDef,
		int /*nIDMenu*/, QPopupMenu *, bool bConfigurable )
{
	KShortcut cut( cutsDef );
	bool b = d->insert( sAction, sLabel, QString::null,
		cut, cut,
		0, 0,
		bConfigurable ) != 0;
	return b;
}

bool KAccel::insertItem( const QString& sLabel, const QString& sAction,
		int key,
		int /*nIDMenu*/, QPopupMenu*, bool bConfigurable )
{
	KShortcut cut;
	cut.init( QKeySequence(key) );
	KAccelAction* pAction = d->insert( sAction, sLabel, QString::null,
		cut, cut,
		0, 0,
		bConfigurable );
	return pAction != 0;
}

// Used in kdeutils/kjots
bool KAccel::insertStdItem( KStdAccel::StdAccel id, const QString& sLabel )
{
	KAccelAction* pAction = d->insert( KStdAccel::name( id ), sLabel, QString::null,
		KStdAccel::shortcutDefault3( id ), KStdAccel::shortcutDefault4( id ),
		0, 0 );
	if( pAction )
		pAction->setShortcut( KStdAccel::shortcut( id ) );

	return true;
}

bool KAccel::connectItem( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot, bool bActivate )
{
	kdDebug(125) << "KAccel::connectItem( " << sAction << ", " << pObjSlot << ", " << psMethodSlot << " )" << endl;
	if( bActivate == false )
		d->setActionEnabled( sAction, false );
	bool b = setSlot( sAction, pObjSlot, psMethodSlot );
	if( bActivate == true )
		d->setActionEnabled( sAction, true );
	return b;
}

bool KAccel::removeItem( const QString& sAction )
	{ return d->removeAction( sAction ); }

bool KAccel::setItemEnabled( const QString& sAction, bool bEnable )
	{ return setEnabled( sAction, bEnable ); }

void KAccel::changeMenuAccel( QPopupMenu *menu, int id, const QString& action )
{
	KAccelAction* pAction = actions().actionPtr( action );
	QString s = menu->text( id );
	if( !pAction || s.isEmpty() )
		return;

	int i = s.find( '\t' );

	QString k = pAction->shortcut().seq(0).toString();
	if( k.isEmpty() )
		return;

	if ( i >= 0 )
		s.replace( i+1, s.length()-i, k );
	else {
		s += '\t';
		s += k;
	}

	QPixmap *pp = menu->pixmap(id);
	if( pp && !pp->isNull() )
		menu->changeItem( *pp, s, id );
	else
		menu->changeItem( s, id );
}

void KAccel::changeMenuAccel( QPopupMenu *menu, int id, KStdAccel::StdAccel accel )
{
	changeMenuAccel( menu, id, KStdAccel::name( accel ) );
}

int KAccel::stringToKey( const QString& sKey )
{
	return KKey( sKey ).keyCodeQt();
}

int KAccel::currentKey( const QString& sAction ) const
{
	KAccelAction* pAction = d->actionPtr( sAction );
	if( pAction )
		return pAction->shortcut().keyCodeQt();
	return 0;
}

QString KAccel::findKey( int key ) const
{
	KAccelAction* pAction = d->actionPtr( KKey(key) );
	if( pAction )
		return pAction->name();
	else
		return QString::null;
}
#endif // !KDE_NO_COMPAT

void KAccel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kaccel.moc"
#include "kaccelprivate.moc"
