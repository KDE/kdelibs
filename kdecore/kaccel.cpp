// Make insertAction return KAccelAction*
// remove 'virtual's
// remove m_bAutoUpdate

#include "kaccel.h"

#include <qaccel.h>
#include <qstring.h>
#include <qwidget.h>
#include <kaccelbase.h>
#include <kdebug.h>
#include <kkeysequence.h>
#include <klocale.h>

//----------------------------------------------------

class KAccelPrivate : public KAccelBase
{
public:
	QAccel* m_pAccel;
	int m_nIDAccelNext;

	KAccelPrivate( QWidget* pWatch, QObject* pParent )
	{
		m_pAccel = new QAccel( pWatch, pParent, 0 );
		m_nIDAccelNext = 1;
		m_bAutoUpdate = false;
	}

	virtual void setEnabled( bool );

	virtual bool removeAction( const QString& sAction );
	// BCI: make virtual when KAccelBase::setActionSlot has been made virtual
	//bool setActionSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot );

	virtual bool connectKey( KAccelAction&, KKeySequence );
	virtual bool disconnectKey( KAccelAction&, KKeySequence );

	void setAutoUpdateTemp( bool b ) { m_bAutoUpdate = b; }
};

void KAccelPrivate::setEnabled( bool bEnabled )
{
	m_bEnabled = bEnabled;
	m_pAccel->setEnabled( bEnabled );
}

bool KAccelPrivate::removeAction( const QString& sAction )
{
	KAccelAction* pAction = actions().actionPtr( sAction );
	if( pAction ) {
		int nID = pAction->getID();
		bool b = actions().removeAction( sAction );
		m_pAccel->removeItem( nID );
		return b;
	} else
		return false;
}

/*bool KAccelPrivate::setActionSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot )
{
	KAccelAction* pAction = m_rgActions.actionPtr( sAction );
	if( pAction ) {
		// Disconnect if connection might exist,
		if( pAction->getID() && pAction->m_pObjSlot )
			m_pAccel->disconnectItem( pAction->getID(), pAction->m_pObjSlot, pAction->m_psMethodSlot );

		pAction->m_pObjSlot = pObjSlot;
		pAction->m_psMethodSlot = psMethodSlot;

		// Reconnect if ID has already be set.
		if( pAction->getID() && pAction->m_pObjSlot )
			m_pAccel->connectItem( pAction->getID(), pAction->m_pObjSlot, pAction->m_psMethodSlot );
		return true;
	} else
		return false;
}*/

bool KAccelPrivate::connectKey( KAccelAction& action, KKeySequence key )
{
	//if( action.m_sName == "file_new" ) { char* c = 0; *c = 0; }
	if( !action.getID() )
		action.setID( m_nIDAccelNext++ );

	uint keyCombQt = (QKeySequence) key;
	int nID = m_pAccel->insertItem( key, action.getID() );
	if( nID != action.getID() )
		action.setID( nID );
	if( nID && action.m_pObjSlot )
		m_pAccel->connectItem( nID, action.m_pObjSlot, action.m_psMethodSlot );

	kdDebug(125) << "KAccelPrivate::connectKey( " << action.m_sName << ", " << key.toString() << " = 0x" << QString::number(keyCombQt,16) << " ) = " << nID << endl;
	return nID != 0;
}

bool KAccelPrivate::disconnectKey( KAccelAction& action, KKeySequence key )
{
	kdDebug(125) << "disconnectKey( " << action.m_sName << ", " << key.toString() << " )" << endl;
	if( action.getID() && action.m_pObjSlot )
		return m_pAccel->disconnectItem( action.getID(), action.m_pObjSlot, action.m_psMethodSlot );
	return true;
}

//----------------------------------------------------

KAccel::KAccel( QWidget* pParent, const char* psName )
: QObject( pParent, psName )
{
	d = new KAccelPrivate( pParent, this );
}

KAccel::~KAccel()
{
	// KAccelPrivate is not a QObject subclass, but it contains
	//  a QAccel object.  We delete d here, but KAccelPrivate
	//  must not delete m_pAccel, because it was made a child of
	//  this KAccel object and will be automatically deleted.
	delete d;
}

KAccelBase* KAccel::basePtr()             { return d; }
KAccelActions& KAccel::actions()          { return d->actions(); }
bool KAccel::isEnabled()                  { return d->isEnabled(); }
void KAccel::setEnabled( bool bEnabled )  { d->setEnabled( bEnabled ); }
bool KAccel::setAutoUpdate( bool bAuto )  { return d->setAutoUpdate( bAuto ); }

bool KAccel::insertAction( const QString& sAction, const QString& sDesc,
		const KShortcuts& cutsDef3, const KShortcuts& cutsDef4,
		const QObject* pObjSlot, const char* psMethodSlot,
		int nIDMenu, QPopupMenu *pMenu,
		bool bConfigurable, bool bEnabled )
{
	KAccelAction* pAction = d->insertAction( sAction, sDesc,
		cutsDef3, cutsDef4,
		pObjSlot, psMethodSlot,
		nIDMenu, pMenu, bConfigurable, bEnabled );

	return pAction != 0;
}

bool KAccel::insertAction( const char* psAction, const char* psShortcuts,
		const QObject* pObjSlot, const char* psMethodSlot,
		int nIDMenu, QPopupMenu* pMenu,
		bool bConfigurable, bool bEnabled )
{
	return insertAction( psAction, i18n(psAction),
		KShortcuts(psShortcuts), KShortcuts(psShortcuts),
		pObjSlot, psMethodSlot,
		nIDMenu, pMenu, bConfigurable, bEnabled );
}

/*bool KAccel::insertAction( const QString& sAction, KShortcuts rgCutDefaults,
		const QObject* pObjSlot, const char* psMethodSlot,
		int nIDMenu, QPopupMenu *pMenu,
		bool bConfigurable, bool bEnabled )
{
	KAccelShortcuts cuts( rgCutDefaults.m_s );
	bool b = d->insertAction( sAction, i18n(sAction.latin1()),
		cuts, cuts,
		pObjSlot, psMethodSlot,
		nIDMenu, pMenu, bConfigurable, bEnabled );
	if( m_bAutoUpdate )
		updateConnections();
	return b;
}*/

bool KAccel::insertAction( KStdAccel::StdAccel id,
		const QObject* pObjSlot, const char* psMethodSlot,
		int nIDMenu, QPopupMenu *pMenu,
		bool bConfigurable, bool bEnabled )
{
	QString sAction = KStdAccel::action( id );
	if( sAction.isEmpty() )
		return false;

	KAccelAction* pAction = d->insertAction( sAction, KStdAccel::description( id ),
		KStdAccel::defaultShortcuts3( id ), KStdAccel::defaultShortcuts4( id ),
		pObjSlot, psMethodSlot,
		nIDMenu, pMenu, bConfigurable, bEnabled );
	if( pAction )
		d->setShortcuts( sAction, KAccelShortcuts( KStdAccel::key( id ) ) );

	return pAction != 0;
}

bool KAccel::removeAction( const QString& sAction )
	{ return d->removeAction( sAction ); }
bool KAccel::setActionSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot )
	{ return d->setActionSlot( sAction, pObjSlot, psMethodSlot ); }

bool KAccel::setActionEnabled( const QString& sAction, bool bEnabled )
{
	KAccelAction* pAction = d->actionPtr( sAction );
	if( pAction ) {
		pAction->m_bEnabled = bEnabled;
		if( d->getAutoUpdate() )
			updateConnections();
		return true;
	} else
		return false;
}

bool KAccel::updateConnections()
	{ return d->updateConnections(); }
bool KAccel::setShortcuts( const QString& sAction, const KShortcuts& rgCuts )
	{ return d->setShortcuts( sAction, rgCuts ); }

void KAccel::readSettings( KConfig* pConfig )
{
	bool bAutoUpdate = d->getAutoUpdate();
	d->setAutoUpdateTemp( true );

	d->readSettings( pConfig );

	d->setAutoUpdateTemp( bAutoUpdate );
}

void KAccel::writeSettings( KConfig* pConfig ) const
	{ d->writeSettings( pConfig ); }

bool KAccel::insertItem( const QString& sDesc, const QString& sAction,
		const char* cutsDef,
		int nIDMenu, QPopupMenu *pMenu, bool bConfigurable )
{
	KShortcuts cuts( cutsDef );
	bool b = d->insertAction( sAction, sDesc,
		cuts, cuts,
		0, 0,
		nIDMenu, pMenu, bConfigurable );
	return b;
}

bool KAccel::insertItem( const QString& sDesc, const QString& sAction,
		int key,
		int nIDMenu, QPopupMenu* pMenu, bool bConfigurable )
{
	KShortcuts cuts( key );
	bool b = d->insertAction( sAction, sDesc,
		cuts, cuts,
		0, 0,
		nIDMenu, pMenu, bConfigurable );
	return b;
}

bool KAccel::connectItem( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot, bool bActivate )
{
	if( bActivate == false )
		d->setActionEnabled( sAction, false );
	bool bAutoUpdate = d->getAutoUpdate();
	d->setAutoUpdateTemp( true );
	bool b = setActionSlot( sAction, pObjSlot, psMethodSlot );
	d->setAutoUpdateTemp( bAutoUpdate );
	if( bActivate == true )
		d->setActionEnabled( sAction, true );
	return b;
}

#include <qpopupmenu.h>
void KAccel::changeMenuAccel( QPopupMenu *menu, int id, const QString& action )
{
	KAccelAction* pAction = actions().actionPtr( action );
	QString s = menu->text( id );
	if( !pAction || s.isEmpty() )
		return;

	int i = s.find( '\t' );

	QString k = pAction->getShortcut(0).toString();
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
	changeMenuAccel( menu, id, KStdAccel::action( accel ) );
}

int KAccel::stringToKey( const QString& sKey )
{
	return (QKeySequence) KKeySequence( sKey );
}

int KAccel::currentKey( const QString& sAction ) const
{
	KAccelAction* pAction = d->actionPtr( sAction );
	if( pAction ) {
		return pAction->getShortcut(0).getSequence(0).getKey(0).keyQt();
	} else
		return 0;
}

QString KAccel::findKey( int key ) const
{
	KAccelAction* pAction = d->actionPtr( KKeySequence(key) );
	if( pAction )
		return pAction->m_sName;
	else
		return QString::null;
}
