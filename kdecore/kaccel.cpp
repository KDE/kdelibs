#include "kaccel.h"

#include <qaccel.h>
#include <qpopupmenu.h>
#include <qstring.h>
#include <qtimer.h>
#include <kaccelbase.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kshortcut.h>
#include <klocale.h>

#include "kaccelprivate.h"

//---------------------------------------------------------------------
// KAccelPrivate
//---------------------------------------------------------------------

KAccelPrivate::KAccelPrivate( KAccel* pParent )
: KAccelBase( KAccelBase::QT_KEYS )
{
	m_pAccel = pParent;
	m_nIDAccelNext = 1;
	m_bAutoUpdate = true;
	connect( (QAccel*)m_pAccel, SIGNAL(activated(int)), this, SLOT(slotKeyPressed(int)) );
}

void KAccelPrivate::setEnabled( bool bEnabled )
{
	m_bEnabled = bEnabled;
	((QAccel*)m_pAccel)->setEnabled( bEnabled );
}

bool KAccelPrivate::removeAction( const QString& sAction )
{
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

bool KAccelPrivate::connectKey( KAccelAction& action, const KKey& spec )
{
	//if( action.m_sName == "file_new" ) { char* c = 0; *c = 0; }
	if( !action.getID() )
		action.setID( m_nIDAccelNext++ );

	uint keyQt = spec.keyCodeQt();
	int nID = ((QAccel*)m_pAccel)->insertItem( keyQt, action.getID() );
	if( nID != action.getID() )
		action.setID( nID );
	if( nID && action.objSlotPtr() ) {
		((QAccel*)m_pAccel)->connectItem( nID, action.objSlotPtr(), action.methodSlotPtr() );
		if( !action.isEnabled() )
			((QAccel*)m_pAccel)->setItemEnabled( nID, false );
	}

	kdDebug(125) << "KAccelPrivate::connectKey( \"" << action.name() << "\", " << spec.toStringInternal() << " = 0x" << QString::number(keyQt,16) << " ): id = " << nID << " m_pObjSlot = " << action.objSlotPtr() << endl;
	return nID != 0;
}

bool KAccelPrivate::connectKey( const KKey& spec )
{
	uint keyQt = spec.keyCodeQt();
	int nID = ((QAccel*)m_pAccel)->insertItem( keyQt, m_nIDAccelNext++ );

	m_mapIDToSpec[nID] = spec;

	kdDebug(125) << "KAccelPrivate::connectKey( " << spec.toStringInternal() << " = 0x" << QString::number(keyQt,16) << " ): id = " << nID << endl;
	return nID != 0;
}

bool KAccelPrivate::disconnectKey( KAccelAction& action, const KKey& spec )
{
	QKeySequence key = spec.keyCodeQt();
	//kdDebug(125) << "KAccelPrivate::disconnectKey( &action = " << &action << " )" << endl;
	kdDebug(125) << "KAccelPrivate::disconnectKey( \"" << action.name() << "\", " << spec.toStringInternal() << " )  m_pObjSlot = " << action.objSlotPtr() << endl;
	if( action.getID() && action.objSlotPtr() )
		return ((QAccel*)m_pAccel)->disconnectItem( action.getID(), action.objSlotPtr(), action.methodSlotPtr() );
	return true;
}

bool KAccelPrivate::disconnectKey( const KKey& spec )
{
	QKeySequence key = spec.keyCodeQt();
	//kdDebug(125) << "KAccelPrivate::disconnectKey( &action = " << &action << " )" << endl;
	kdDebug(125) << "KAccelPrivate::disconnectKey( " << spec.toStringInternal() << " )" << endl;
	int id = ((QAccel*)m_pAccel)->findKey( key );
	return ((QAccel*)m_pAccel)->disconnectItem( id, m_pAccel, SLOT(slotKeyPress(int)) );
}

void KAccelPrivate::slotKeyPressed( int id )
{
	kdDebug(125) << "KAccelPrivate::slotKeyPressed( " << id << " )" << endl;
	if( m_mapIDToSpec.contains( id ) ) {
		KKeySequence seq( m_mapIDToSpec[id] );
		QPopupMenu* pMenu = createPopupMenu( kapp->mainWidget(), seq );
		connect( pMenu, SIGNAL(activated(int)), this, SLOT(slotMenuActivated(int)));
		pMenu->exec();
		disconnect( pMenu, SIGNAL(activated(int)), this, SLOT(slotMenuActivated(int)));
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
	kdDebug(125) << "KAccel(): this = " << this << endl;
	d = new KAccelPrivate( this );
}

KAccel::KAccel( QWidget* watch, QObject* parent, const char* psName )
: QAccel( watch, parent, (psName) ? psName : "KAccel-QAccel" )
{
	kdDebug(125) << "KAccel(): this = " << this << endl;
	d = new KAccelPrivate( this );
}

KAccel::~KAccel()
{
	kdDebug(125) << "~KAccel(): this = " << this << endl;
	delete d;
}

//KAccelBase* KAccel::basePtr()             { return d; }
KAccelActions& KAccel::actions()          { return d->actions(); }
const KAccelActions& KAccel::actions() const { return d->actions(); }
bool KAccel::isEnabled()                  { return d->isEnabled(); }
void KAccel::setEnabled( bool bEnabled )  { d->setEnabled( bEnabled ); }
bool KAccel::setAutoUpdate( bool bAuto )  { return d->setAutoUpdate( bAuto ); }

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
{
	kdDebug(125) << "KAccel::setEnabled( " << sAction << ", " << bEnable << " )" << endl;
	KAccelAction* pAction = d->actionPtr( sAction );
	if( pAction ) {
		if( pAction->isEnabled() != bEnable ) {
			QAccel::setItemEnabled( pAction->getID(), bEnable );
			pAction->setEnabled( bEnable );
		}
		return true;
	} else
		return false;
}

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

bool KAccel::readSettings( KConfigBase* pConfig )
{
	bool bAutoUpdate = d->getAutoUpdate();
	d->setAutoUpdateTemp( true );

	d->readSettings( pConfig );

	d->setAutoUpdateTemp( bAutoUpdate );
	return true;
}

bool KAccel::writeSettings( KConfigBase* pConfig ) const
	{ d->writeSettings( pConfig ); return true; }

// for kdegames/ksirtet
void KAccel::setConfigGroup( const QString& s )
	{ d->setConfigGroup( s ); }

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

#include <kaccel.moc>
#include <kaccelprivate.moc>
