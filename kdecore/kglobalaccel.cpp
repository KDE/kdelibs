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
	d = new KGlobalAccelPrivate();
}

KGlobalAccel::~KGlobalAccel()
{
	delete d;
}

KAccelBase* KGlobalAccel::basePtr()
{
	return d;
}

KAccelActions& KGlobalAccel::actions()	{ return d->actions(); }

bool KGlobalAccel::insertAction( const QString& sAction, const QString& sDesc,
		const KShortcuts& cutsDef3, const KShortcuts& cutsDef4,
		const QObject* pObjSlot, const char* psMethodSlot,
		int nIDMenu, QPopupMenu *pMenu, bool bConfigurable )
{
	return d->insertAction( sAction, sDesc,
		cutsDef3, cutsDef4,
		pObjSlot, psMethodSlot,
		nIDMenu, pMenu, bConfigurable );
}

/*bool KGlobalAccel::insertAction( const QString& sAction, KShortcuts cutsDef,
		const QObject* pObjSlot, const char* psMethodSlot,
		int nIDMenu, QPopupMenu* pMenu, bool bConfigurable )
{
	return d->insertAction( sAction, i18n(sAction.latin1()),
		cutsDef, cutsDef,
		pObjSlot, psMethodSlot,
		nIDMenu, pMenu, bConfigurable );
}*/

bool KGlobalAccel::insertLabel( const QString& sName, const QString& sDesc )
	{ return d->insertLabel( sName, sDesc ); }
bool KGlobalAccel::updateConnections()
	{ return d->updateConnections(); }
bool KGlobalAccel::setShortcuts( const QString& sAction, const KShortcuts& cuts )
	{ return d->setShortcuts( sAction, cuts ); }
void KGlobalAccel::readSettings( KConfig* pConfig )
	{ d->readSettings( pConfig ); }
void KGlobalAccel::writeSettings( KConfig* pConfig ) const
	{ d->writeSettings( pConfig ); }
