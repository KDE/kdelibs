#include "kglobalaccel.h"
#ifdef Q_WS_X11
#include "kglobalaccel_x11.h"
#else
#include "kglobalaccel_emb.h"
#endif

#include <qstring.h>
#include <kaccelbase.h>
#include <kdebug.h>
#include <kkeysequence.h>
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

void KGlobalAccel::clearActions()
	{ d->clearActions(); }

KAccelActions& KGlobalAccel::actions()
	{ return d->actions(); }

bool KGlobalAccel::insertAction( const QString& sAction, const QString& sDesc, const QString& sHelp,
		const KShortcuts& cutsDef3, const KShortcuts& cutsDef4,
		const QObject* pObjSlot, const char* psMethodSlot,
		bool bConfigurable, bool bEnabled )
{
	return d->insertAction( sAction, sDesc, sHelp,
		cutsDef3, cutsDef4,
		pObjSlot, psMethodSlot,
		0, bConfigurable, bEnabled );
}

bool KGlobalAccel::insertAction( const QString& sAction, const QString& sDesc, const QString& sHelp,
		const char* cutsDef3, const char* cutsDef4,
		const QObject* pObjSlot, const char* psMethodSlot,
		bool bConfigurable, bool bEnabled )
{
	return d->insertAction( sAction, sDesc, sHelp,
		KAccelShortcuts(cutsDef3), KAccelShortcuts(cutsDef4),
		pObjSlot, psMethodSlot,
		0, bConfigurable, bEnabled );
}

/*bool KGlobalAccel::insertAction( const QString& sAction, KShortcuts cutsDef,
		const QObject* pObjSlot, const char* psMethodSlot,
		int nIDMenu, QPopupMenu*, bool bConfigurable )
{
	return d->insertAction( sAction, i18n(sAction.latin1()),
		cutsDef, cutsDef,
		pObjSlot, psMethodSlot,
		nIDMenu, bConfigurable );
}*/

bool KGlobalAccel::insertLabel( const QString& sName, const QString& sDesc )
	{ return d->insertLabel( sName, sDesc ); }
bool KGlobalAccel::setActionSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot )
	{ return d->setActionSlot( sAction, pObjSlot, psMethodSlot ); }
bool KGlobalAccel::updateConnections()
	{ return d->updateConnections(); }
bool KGlobalAccel::setShortcuts( const QString& sAction, const KShortcuts& cuts )
	{ return d->setShortcuts( sAction, cuts ); }
void KGlobalAccel::readSettings( KConfig* pConfig )
	{ d->readSettings( pConfig ); }
void KGlobalAccel::writeSettings( KConfig* pConfig ) const
	{ d->writeSettings( pConfig ); }

#include <kglobalaccel.moc>
