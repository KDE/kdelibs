#include "kaccel.h"

#include <qaccel.h>
#include <qpopupmenu.h>
#include <qregexp.h>
#include <qstring.h>
#include <qtimer.h>

#include <kaccelbase.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kshortcut.h>

#include "kaccelprivate.h"

//---------------------------------------------------------------------
// KAccelPrivate
//---------------------------------------------------------------------

KAccelPrivate::KAccelPrivate( KAccel* pParent )
: KAccelBase( KAccelBase::QT_KEYS )
{
	//kdDebug(125) << "KAccelPrivate::KAccelPrivate( pParent = " << pParent << " ): this = " << this << endl;
	m_pAccel = pParent;
	m_bAutoUpdate = true;
	connect( (QAccel*)m_pAccel, SIGNAL(activated(int)), this, SLOT(slotKeyPressed(int)) );
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

	QMap<int, KAccelAction*>::iterator it = m_mapIDToAction.begin();
	for( ; it != m_mapIDToAction.end(); ++it ) {
		if( *it == pAction )
			((QAccel*)m_pAccel)->setItemEnabled( it.key(), bEnable );
	}
	return true;
}

bool KAccelPrivate::removeAction( const QString& sAction )
{
	// FIXME: I don't htink getID() contains any useful 
	//  information.  Use mapIDToAction. --ellis, 2/May/2002
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
		if( QRegExp("\\(\\s*int\\s*\\)").search( action.methodSlotPtr() ) == -1 ) {
			((QAccel*)m_pAccel)->connectItem( nID, action.objSlotPtr(), action.methodSlotPtr() );
			if( !action.isEnabled() )
				((QAccel*)m_pAccel)->setItemEnabled( nID, false );
		}
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
	if( m_mapIDToAction.contains( id ) ) {
		KAccelAction* pAction = m_mapIDToAction[id];
		Q_ASSERT( pAction );
		if ( !pAction )
			return;
		QRegExp rxVal("\\{(.+)\\}");
		rxVal.setMinimal( true );
		kdDebug(125) << "pAction->name() = " << pAction->name() << endl;
		if( rxVal.search( pAction->name() ) >= 0 ) {
			kdDebug(125) << "pAction->methodSlotPtr() = " << pAction->methodSlotPtr() << endl;
			if( QRegExp("\\(\\s*int\\s*\\)").search( pAction->methodSlotPtr() ) >= 0 ) {
				int n = rxVal.cap(1).toInt();
				kdDebug(125) << "rxVal.cap(0) = " << rxVal.cap(0) << ", rxVal.cap(1) = " << rxVal.cap(1) << ", n = " << n << endl;
				connect( this, SIGNAL(activateInt(int)), pAction->objSlotPtr(), pAction->methodSlotPtr() );
				emit activateInt( n );
				disconnect( this, SIGNAL(activateInt(int)), pAction->objSlotPtr(), pAction->methodSlotPtr() );
				return;
			}
		}
	} 
	if( m_mapIDToKey.contains( id ) ) {
		KAccelAction* pAction = m_mapIDToAction[id];
		KKey key = m_mapIDToKey[id];
		KKeySequence seq( key );
		QPopupMenu* pMenu = createPopupMenu( kapp->mainWidget(), seq );

		// If there was only one action mapped to this key,
		//  and that action is not a multi-key shortcut,
		//  then activated it without popping up the menu.
		// This is needed for when there are multiple actions
		//  with the same shortcut where all but one is disabled.
		if( pMenu->count() == 1 && pAction->shortcut().contains( key ) ) {
			int iAction = pMenu->idAt(0);
			slotMenuActivated( iAction );
		} else {
			connect( pMenu, SIGNAL(activated(int)), this, SLOT(slotMenuActivated(int)));
			pMenu->exec();
			disconnect( pMenu, SIGNAL(activated(int)), this, SLOT(slotMenuActivated(int)));
			delete pMenu;
		}
	}
}

void KAccelPrivate::slotShowMenu()
{
}

void KAccelPrivate::slotMenuActivated( int iAction )
{
	// TODO: take care of slot(int)
	kdDebug(125) << "KAccelPrivate::slotMenuActivated( " << iAction << " )" << endl;
	KAccelAction* pAction = actions().actionPtr( iAction );
	if( pAction ) {
		connect( this, SIGNAL(menuItemActivated()), pAction->objSlotPtr(), pAction->methodSlotPtr() );
		emit menuItemActivated();
		disconnect( this, SIGNAL(menuItemActivated()), pAction->objSlotPtr(), pAction->methodSlotPtr() );
	}
}

//---------------------------------------------------------------------
// KAccel
//---------------------------------------------------------------------

KAccel::KAccel( QWidget* pParent, const char* psName )
: QAccel( pParent, (psName) ? psName : "KAccel-QAccel" )
{
	kdDebug(125) << "KAccel( pParent = " << pParent << ", psName = " << psName << " ): this = " << this << endl;
	d = new KAccelPrivate( this );
}

KAccel::KAccel( QWidget* watch, QObject* pParent, const char* psName )
: QAccel( watch, pParent, (psName) ? psName : "KAccel-QAccel" )
{
	kdDebug(125) << "KAccel( watch = " << watch << ", pParent = " << pParent << ", psName = " << psName << " ): this = " << this << endl;
	if( !watch )
		kdDebug(125) << kdBacktrace() << endl;
	d = new KAccelPrivate( this );
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
	QString sAction = KStdAccel::action( id );
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
	KAccelAction* pAction = d->insert( KStdAccel::action( id ), sLabel, QString::null,
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

#include <kaccel.moc>
#include <kaccelprivate.moc>
