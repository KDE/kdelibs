#include "kglobalaccel.h"
#ifdef Q_WS_X11
#include "kglobalaccel_x11.h"
#else
#include "kglobalaccel_emb.h"
#endif

#include <qstring.h>
#include <kaccelbase.h>
#include <kdebug.h>
#include <kshortcut.h>
#include <klocale.h>

//----------------------------------------------------

KGlobalAccel::KGlobalAccel( QObject* pParent, const char* psName )
: QObject( pParent, psName )
{
	kdDebug(125) << "KGlobalAccel(): this = " << this << endl;
	d = new KGlobalAccelPrivate();
}

KGlobalAccel::~KGlobalAccel()
{
	kdDebug(125) << "~KGlobalAccel(): this = " << this << endl;
	delete d;
}

void KGlobalAccel::clear()
	{ d->clearActions(); }

KAccelActions& KGlobalAccel::actions()
	{ return d->actions(); }

const KAccelActions& KGlobalAccel::actions() const
	{ return d->actions(); }

bool KGlobalAccel::isEnabled()
	{ return ((KAccelBase*)d)->isEnabled(); }

void KGlobalAccel::setEnabled( bool bEnabled )
	{ d->setEnabled( bEnabled ); }

KAccelAction* KGlobalAccel::insert( const QString& sAction, const QString& sDesc, const QString& sHelp,
		const KShortcut& cutDef3, const KShortcut& cutDef4,
		const QObject* pObjSlot, const char* psMethodSlot,
		bool bConfigurable, bool bEnabled )
{
	return d->insert( sAction, sDesc, sHelp,
		cutDef3, cutDef4,
		pObjSlot, psMethodSlot,
		bConfigurable, bEnabled );
}

KAccelAction* KGlobalAccel::insert( const QString& sName, const QString& sDesc )
	{ return d->insert( sName, sDesc ); }
bool KGlobalAccel::updateConnections()
	{ return d->updateConnections(); }

const KShortcut& KGlobalAccel::shortcut( const QString& sAction ) const
{
	const KAccelAction* pAction = actions().actionPtr( sAction );
	return (pAction) ? pAction->shortcut() : KShortcut::null();
}

bool KGlobalAccel::setShortcut( const QString& sAction, const KShortcut& cut )
	{ return d->setShortcut( sAction, cut ); }
bool KGlobalAccel::setSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot )
	{ return d->setActionSlot( sAction, pObjSlot, psMethodSlot ); }

void KGlobalAccel::readSettings( KConfigBase* pConfig )
	{ d->readSettings( pConfig ); }
void KGlobalAccel::writeSettings( KConfigBase* pConfig ) const
	{ d->writeSettings( pConfig ); }

bool KGlobalAccel::useFourModifierKeys()
	{ return KAccelAction::useFourModifierKeys(); }

#include <kglobalaccel.moc>
