#include <qstring.h>
#include <qvariant.h>

#include <kaccel.h>
#include <kaccelaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalaccel.h>
#include <kinstance.h>
#include <kshortcut.h>
#include "kshortcutlist.h"

//---------------------------------------------------------------------
// KShortcutList
//---------------------------------------------------------------------

KShortcutList::KShortcutList()
{
}

KShortcutList::~KShortcutList()
{
}

bool KShortcutList::isGlobal( uint ) const
{
	return false;
}

// FIXME: implement!
int KShortcutList::index( const QString& /*sName*/ ) const
{
	return -1;
}

int KShortcutList::index( const KKeySequence& seq ) const
{
	if( seq.isNull() )
		return -1;

	uint nSize = count();
	for( uint i = 0; i < nSize; i++ ) {
		if( shortcut(i).contains( seq ) )
			return i;
	}

	return -1;
}

const KInstance* KShortcutList::instance() const
{
	return 0;
}

QVariant KShortcutList::getOther( Other, uint ) const
{
	return QVariant();
}

bool KShortcutList::setOther( Other, uint, QVariant )
{
	return false;
}

bool KShortcutList::readSettings( const QString& sConfigGroup, KConfigBase* pConfig )
{
	kdDebug(125) << "KShortcutList::readSettings( \"" << sConfigGroup << "\", " << pConfig << " ) start" << endl;
	if( !pConfig )
		pConfig = KGlobal::config();
	QString sGroup = (!sConfigGroup.isEmpty()) ? sConfigGroup : QString("Shortcuts");

	// If the config file still has the old group name:
	// FIXME: need to rename instead? -- and don't do this if hasGroup( "Shortcuts" ).
	if( sGroup == "Shortcuts" && pConfig->hasGroup( "Keys" ) ) {
		readSettings( "Keys", pConfig );
	}

	kdDebug(125) << "\treadSettings( \"" << sGroup << "\", " << pConfig << " )" << endl;
	if( !pConfig->hasGroup( sGroup ) )
		return true;
	KConfigGroupSaver cgs( pConfig, sGroup );

	uint nSize = count();
	for( uint i = 0; i < nSize; i++ ) {
		if( isConfigurable(i) ) {
			QString sEntry = pConfig->readEntry( name(i) );
			if( !sEntry.isEmpty() ) {
				if( sEntry == "none" )
					setShortcut( i, KShortcut() );
				else
					setShortcut( i, KShortcut(sEntry) );
			}
			else // default shortcut
				setShortcut( i, shortcutDefault(i) );
			kdDebug(125) << "\t" << name(i) << " = '" << sEntry << "'" << endl;
		}
	}

	kdDebug(125) << "KShortcutList::readSettings done" << endl;
	return true;
}

bool KShortcutList::writeSettings( const QString &sConfigGroup, KConfigBase* pConfig, bool bWriteAll, bool bGlobal ) const
{
	kdDebug(125) << "KShortcutList::writeSettings( " << sConfigGroup << ", " << pConfig << ", " << bWriteAll << ", " << bGlobal << " )" << endl;
	if( !pConfig )
		pConfig = KGlobal::config();

	QString sGroup = (!sConfigGroup.isEmpty()) ? sConfigGroup : QString("Shortcuts");

	// If it has the depricated group [Keys], remove it
	if( pConfig->hasGroup( "Keys" ) )
		pConfig->deleteGroup( "Keys", true );

	KConfigGroupSaver cs( pConfig, sGroup );

	uint nSize = count();
	for( uint i = 0; i < nSize; i++ ) {
		if( isConfigurable(i) ) {
			const QString& sName = name(i);
			bool bConfigHasAction = !pConfig->readEntry( sName ).isEmpty();
			bool bSameAsDefault = (shortcut(i) == shortcutDefault(i));
			// If we're using a global config or this setting
			//  differs from the default, then we want to write.
			if( bWriteAll || !bSameAsDefault ) {
				QString s = shortcut(i).toStringInternal();
				if( s.isEmpty() )
					s = "none";
				kdDebug(125) << "\twriting " << sName << " = " << s << endl;
				pConfig->writeEntry( sName, s, true, bGlobal );
			}
			// Otherwise, this key is the same as default
			//  but exists in config file.  Remove it.
			else if( bConfigHasAction ) {
				kdDebug(125) << "\tremoving " << sName << " because == default" << endl;
				pConfig->deleteEntry( sName, false, bGlobal );
			}
		}
	}

	pConfig->sync();
	return true;
}

//---------------------------------------------------------------------
// KAccelShortcutList
//---------------------------------------------------------------------

KAccelShortcutList::KAccelShortcutList( KAccel* pAccel )
: m_actions( pAccel->actions() )
{
	m_bGlobal = false;
}

KAccelShortcutList::KAccelShortcutList( KGlobalAccel* pAccel )
: m_actions( pAccel->actions() )
{
	m_bGlobal = true;
}

KAccelShortcutList::KAccelShortcutList( KAccelActions& actions, bool bGlobal )
: m_actions( actions )
{
	m_bGlobal = bGlobal;
}


KAccelShortcutList::~KAccelShortcutList()
	{ }
uint KAccelShortcutList::count() const
	{ return m_actions.count(); }
QString KAccelShortcutList::name( uint i ) const
	{ return m_actions.actionPtr(i)->name(); }
QString KAccelShortcutList::label( uint i ) const
	{ return m_actions.actionPtr(i)->label(); }
QString KAccelShortcutList::whatsThis( uint i ) const
	{ return m_actions.actionPtr(i)->whatsThis(); }
const KShortcut& KAccelShortcutList::shortcut( uint i ) const
	{ return m_actions.actionPtr(i)->shortcut(); }
const KShortcut& KAccelShortcutList::shortcutDefault( uint i ) const
	{ return m_actions.actionPtr(i)->shortcutDefault(); }
bool KAccelShortcutList::isConfigurable( uint i ) const
	{ return m_actions.actionPtr(i)->isConfigurable(); }
bool KAccelShortcutList::setShortcut( uint i, const KShortcut& cut )
	{ return m_actions.actionPtr(i)->setShortcut( cut ); }
QVariant KAccelShortcutList::getOther( Other, uint ) const
	{ return QVariant(); }
bool KAccelShortcutList::isGlobal( uint ) const
	{ return m_bGlobal; }
bool KAccelShortcutList::setOther( Other, uint, QVariant )
	{ return false; }
bool KAccelShortcutList::save() const
	{ return writeSettings(); }

void KShortcutList::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KAccelShortcutList::virtual_hook( int id, void* data )
{ KShortcutList::virtual_hook( id, data ); }

void KStdAccel::ShortcutList::virtual_hook( int id, void* data )
{ KShortcutList::virtual_hook( id, data ); }

