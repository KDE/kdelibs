#include "kshortcut.h"
#include "kkeynative.h"

#include <qevent.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <ksimpleconfig.h>

//----------------------------------------------------
struct ModFlagInfo
{
	KKey::ModFlag flag;
	int flagQt;
	int flagNative;
	const char* psName;
	QString sLabel;
};

static ModFlagInfo g_infoModFlags[KKey::MOD_FLAG_COUNT] =
{
	{ KKey::SHIFT, Qt::SHIFT,  0, I18N_NOOP("Shift"), QString() },
	{ KKey::CTRL,  Qt::CTRL,   0, I18N_NOOP("Ctrl"), QString() },
	{ KKey::ALT,   Qt::ALT,    0, I18N_NOOP("Alt"), QString() },
	{ KKey::WIN,   Qt::ALT<<1, 0, I18N_NOOP("Win"), QString() }
};

static bool g_bInitializedKKey = false;
static KKey* g_pspec = 0;
static KKeySequence* g_pseq = 0;
static KShortcut* g_pcut = 0;

//----------------------------------------------------
// Helper functions for KKey
//----------------------------------------------------

static void _intializeKKeyLabels()
{
	KConfigGroupSaver cgs( KGlobal::config(), "Keyboard" );
	g_infoModFlags[0].sLabel = KGlobal::config()->readEntry( "Label Shift", i18n(g_infoModFlags[0].psName) );
	g_infoModFlags[1].sLabel = KGlobal::config()->readEntry( "Label Ctrl", i18n(g_infoModFlags[1].psName) );
	g_infoModFlags[2].sLabel = KGlobal::config()->readEntry( "Label Alt", i18n(g_infoModFlags[2].psName) );
	g_infoModFlags[3].sLabel = KGlobal::config()->readEntry( "Label Win", i18n(g_infoModFlags[3].psName) );
	g_bInitializedKKey = true;
}

/*static bool modSpecToModQt( int modSpec, int& modQt )
{
	modQt = 0;

	if( modSpec & KKey::SHIFT ) modQt |= Qt::SHIFT;
	if( modSpec & KKey::CTRL )  modQt |= Qt::CTRL;
	if( modSpec & KKey::ALT )   modQt |= Qt::ALT;
	if( modSpec & KKey::WIN )   modQt |= (Qt::ALT<<1);

	return true;
}*/

static bool modQtToModSpec( int modQt, int& modSpec )
{
	modSpec = 0;

	if( modQt & Qt::SHIFT )    modSpec |= KKey::SHIFT;
	if( modQt & Qt::CTRL )     modSpec |= KKey::CTRL;
	if( modQt & Qt::ALT )      modSpec |= KKey::ALT;
	if( modQt & (Qt::ALT<<1) ) modSpec |= KKey::WIN;

	return true;
}

//----------------------------------------------------
// KKey
//----------------------------------------------------

KKey::KKey()                          { clear(); }
KKey::KKey( int key, int modFlags )   { init( key, modFlags ); }
KKey::KKey( int keyQt )               { init( keyQt ); }
KKey::KKey( const QKeySequence& key ) { init( key ); }
KKey::KKey( const QKeyEvent* pEvent ) { init( pEvent ); }
KKey::KKey( const KKey& spec )    { init( spec ); }
KKey::KKey( const QString& sSpec )    { init( sSpec ); }

KKey::~KKey()
{
}

void KKey::clear()
{
	m_key = 0;
	m_mod = 0;
	//m_flags = 0;
}

bool KKey::init( int key, int modFlags )
{
	m_key = key;
	m_mod = modFlags;
	//m_flags = SET | VALID;
	return true;
}

bool KKey::init( int keyQt )
{
	if( KKeyNative::keyQtToSym( keyQt & 0xffff, m_key ) ) {
		modQtToModSpec( keyQt, m_mod );
		//m_flags = SET | VALID;
		return true;
	} else {
		m_key = 0;
		m_mod = 0;
		//m_flags = SET;
		return false;
	}
}

bool KKey::init( const QKeySequence& key )
{
	return init( (int) key );
}

bool KKey::init( const QKeyEvent* pEvent )
{
	int keyQt = pEvent->key();
	if( pEvent->state() & Qt::ShiftButton )   keyQt |= Qt::SHIFT;
	if( pEvent->state() & Qt::ControlButton ) keyQt |= Qt::CTRL;
	if( pEvent->state() & Qt::AltButton )     keyQt |= Qt::ALT;
	return init( keyQt );
}

bool KKey::init( const KKey& key )
{
	m_key = key.m_key;
	m_mod = key.m_mod;
	//m_flags = key.m_flags;
	return true;
}

bool KKey::init( const QString& sSpec )
{
	clear();

	QString sKey = sSpec.lower().stripWhiteSpace();
	if( sKey.startsWith( "default(" ) && sKey.endsWith( ")" ) )
		sKey = sKey.mid( 8, sKey.length() - 9 );
	// i.e., "Ctrl++" = "Ctrl+Plus"
	if( sKey.endsWith( "++" ) )
		sKey = sKey.left( sKey.length() - 1 ) + "plus";
	QStringList rgs = QStringList::split( '+', sKey, true );

	uint i;
	// Check for modifier keys first.
	for( i = 0; i < rgs.size(); i++ ) {
		if( rgs[i] == "shift" )     m_mod |= KKey::SHIFT;
		else if( rgs[i] == "ctrl" ) m_mod |= KKey::CTRL;
		else if( rgs[i] == "alt" )  m_mod |= KKey::ALT;
		else if( rgs[i] == "win" )  m_mod |= KKey::WIN;
		else if( rgs[i] == "meta" ) m_mod |= KKey::WIN;
		else break;
	}
	// If there is one non-blank key left:
	if( (i == rgs.size() - 1 && !rgs[i].isEmpty()) ) {
		int modTemp;
		KKeyNative::stringToSym( rgs[i], m_key, modTemp );
		//if( KKeyNative::stringToSym( rgs[i], m_key, modTemp ) )
		//	m_flags = SET | VALID;
	}

	if( m_key == 0 ) {
		m_mod = 0;
		//m_flags = 0;
	}

	kdDebug(125) << "KKey::init( \"" << sSpec << "\" ): this = " << this
		<< " m_key = " << QString::number(m_key, 16)
		<< ", m_mod = " << QString::number(m_mod, 16) << endl;

	return m_key != 0;
}

bool KKey::isNull() const         { return m_key == 0; }
//bool KKey::isSetAndValid() const  { return m_flags == (SET | VALID); }
int KKey::key() const             { return m_key; }
int KKey::modFlags() const        { return m_mod; }

int KKey::compare( const KKey& spec ) const
{
	if( m_key != spec.m_key )
		return m_key - spec.m_key;
	if( m_mod != spec.m_mod )
		return m_mod - spec.m_mod;
	return 0;
}

int KKey::keyCodeQt() const
{
	return KKeyNative( *this ).keyCodeQt();
}

QString KKey::toString() const
{
	QString sMods, sSym;

	if( !g_bInitializedKKey )
		_intializeKKeyLabels();

	for( int i = MOD_FLAG_COUNT-1; i >= 0; i-- ) {
		if( m_mod & g_infoModFlags[i].flag ) {
			sMods += g_infoModFlags[i].sLabel;
			sMods += '+';
		}
	}

	sSym = KKeyNative::symToString( m_key );

	return sMods + sSym;
}

QString KKey::toStringInternal() const
{
	//kdDebug(125) << "KKey::toStringInternal(): this = " << this
	//	<< " mod = " << QString::number(m_mod, 16)
	//	<< " key = " << QString::number(m_key, 16) << endl;
	QString s;

	for( int i = MOD_FLAG_COUNT-1; i >= 0; i-- ) {
		if( m_mod & g_infoModFlags[i].flag ) {
			s += g_infoModFlags[i].psName;
			s += '+';
		}
	}

	s += KKeyNative::symToStringInternal( m_key );
	return s;
}

KKey& KKey::null()
{
	if( !g_pspec )
		g_pspec = new KKey;
	if( !g_pspec->isNull() )
		g_pspec->clear();
	return *g_pspec;
}

//---------------------------------------------------------------------
// KKeySequence
//---------------------------------------------------------------------

KKeySequence::KKeySequence()                          { clear(); }
KKeySequence::KKeySequence( const QKeySequence& seq ) { init( seq ); }
KKeySequence::KKeySequence( const KKey& spec )    { init( spec ); }
KKeySequence::KKeySequence( const KKeySequence& seq ) { init( seq ); }
KKeySequence::KKeySequence( const QString& s )        { init( s ); }

KKeySequence::~KKeySequence()
{
}

void KKeySequence::clear()
{
	m_nKeys = 0;
	m_bTriggerOnRelease = false;
}

bool KKeySequence::init( const QKeySequence& seq )
{
	if( seq ) {
		m_nKeys = 1;
		m_rgvar[0].init( seq );
		m_bTriggerOnRelease = false;
	} else
		clear();
	return true;
}

bool KKeySequence::init( const KKey& key )
{
	if( !key.isNull() ) {
		m_nKeys = 1;
		m_rgvar[0].init( key );
		m_bTriggerOnRelease = false;
	} else
		clear();
	return true;
}

bool KKeySequence::init( const KKeySequence& seq )
{
	m_nKeys = seq.m_nKeys;
	for( uint i = 0; i < m_nKeys; i++ )
		m_rgvar[i] = seq.m_rgvar[i];
	m_bTriggerOnRelease = false;
	return true;
}

bool KKeySequence::init( const QString& s )
{
	m_bTriggerOnRelease = false;
	//kdDebug(125) << "KKeySequence::init( " << s << " )" << endl;
	QStringList rgs = QStringList::split( ',', s );
	if( s == "none" || rgs.size() == 0 ) {
		clear();
		return true;
	} else if( rgs.size() <= MAX_KEYS ) {
		m_nKeys = rgs.size();
		for( uint i = 0; i < m_nKeys; i++ ) {
			m_rgvar[i].init( KKey(rgs[i]) );
			//kdDebug(125) << "\t'" << rgs[i] << "' => " << m_rgvar[i].toStringInternal() << endl;
		}
		return true;
	} else {
		clear();
		return false;
	}
}

uint KKeySequence::count() const
{
	return m_nKeys;
}

const KKey& KKeySequence::key( uint i ) const
{
	if( i < m_nKeys )
		return m_rgvar[i];
	else
		return KKey::null();
}

bool KKeySequence::isTriggerOnRelease() const 
	{ return m_bTriggerOnRelease; }

bool KKeySequence::setKey( uint iKey, const KKey& key )
{
	if( iKey <= m_nKeys && iKey < MAX_KEYS ) {
		m_rgvar[iKey].init( key );
		if( iKey == m_nKeys )
			m_nKeys++;
		return true;
	} else
		return false;
}

bool KKeySequence::isNull() const
{
	return m_nKeys == 0;
}

bool KKeySequence::startsWith( const KKeySequence& seq ) const
{
	if( m_nKeys < seq.m_nKeys )
		return false;

	for( uint i = 0; i < seq.m_nKeys; i++ ) {
		if( m_rgvar[i] != seq.m_rgvar[i] )
			return false;
	}

	return true;
}

int KKeySequence::compare( const KKeySequence& seq ) const
{
	for( uint i = 0; i < m_nKeys && i < seq.m_nKeys; i++ ) {
		int ret = m_rgvar[i].compare( seq.m_rgvar[i] );
		if( ret != 0 )
			return ret;
	}
	if( m_nKeys != seq.m_nKeys )
		return m_nKeys - seq.m_nKeys;
	else
		return 0;
}

QKeySequence KKeySequence::qt() const
{
	QKeySequence seq;

	// TODO: Change this once QKeySequence can handle multiple keys.
	if( count() == 1 )
		seq = KKeyNative(key(0)).keyCodeQt();

	return seq;
}

int KKeySequence::keyCodeQt() const
{
	return (count() == 1) ? KKeyNative(key(0)).keyCodeQt() : 0;
}

QString KKeySequence::toString() const
{
	if( m_nKeys < 1 ) return QString::null;

	QString s;
	s = m_rgvar[0].toString();
	for( uint i = 1; i < m_nKeys; i++ ) {
		s += ",";
		s += m_rgvar[i].toString();
	}

	return s;
}

QString KKeySequence::toStringInternal() const
{
	if( m_nKeys < 1 ) return QString::null;

	QString s;
	s = m_rgvar[0].toStringInternal();
	for( uint i = 1; i < m_nKeys; i++ ) {
		s += ",";
		s += m_rgvar[i].toStringInternal();
	}

	return s;
}

KKeySequence& KKeySequence::null()
{
	if( !g_pseq )
		g_pseq = new KKeySequence;
	if( !g_pseq->isNull() )
		g_pseq->clear();
	return *g_pseq;
}

//---------------------------------------------------------------------
// KShortcut
//---------------------------------------------------------------------

KShortcut::KShortcut()                            { clear(); }
KShortcut::KShortcut( int keyQt )                 { init( keyQt ); }
KShortcut::KShortcut( const QKeySequence& key )   { init( key ); }
KShortcut::KShortcut( const KKey& key )           { init( key ); }
KShortcut::KShortcut( const KKeySequence& seq )   { init( seq ); }
KShortcut::KShortcut( const KShortcut& cut )      { init( cut ); }
KShortcut::KShortcut( const char* ps )            { init( QString(ps) ); }
KShortcut::KShortcut( const QString& s )          { init( s ); }

KShortcut::~KShortcut()
{
}

void KShortcut::clear()
{
	m_nSeqs = 0;
}

bool KShortcut::init( int keyQt )
{
	if( keyQt ) {
		m_nSeqs = 1;
		m_rgseq[0].init( QKeySequence(keyQt) );
	} else
		clear();
	return true;
}

bool KShortcut::init( const QKeySequence& key )
{
	m_nSeqs = 1;
	m_rgseq[0].init( key );
	return true;
}

bool KShortcut::init( const KKey& spec )
{
	m_nSeqs = 1;
	m_rgseq[0].init( spec );
	return true;
}

bool KShortcut::init( const KKeySequence& seq )
{
	m_nSeqs = 1;
	m_rgseq[0] = seq;
	return true;
}

bool KShortcut::init( const KShortcut& cut )
{
	m_nSeqs = cut.m_nSeqs;
	for( uint i = 0; i < m_nSeqs; i++ )
		m_rgseq[i] = cut.m_rgseq[i];
	return true;
}

bool KShortcut::init( const QString& s )
{
	bool bRet = true;
	QStringList rgs = QStringList::split( ';', s );

	if( s == "none" || rgs.size() == 0 )
		clear();
	else if( rgs.size() <= MAX_SEQUENCES ) {
		m_nSeqs = rgs.size();
		for( uint i = 0; i < m_nSeqs; i++ ) {
			QString& sSeq = rgs[i];
			if( sSeq.startsWith( "default(" ) )
				sSeq = sSeq.mid( 8, sSeq.length() - 9 );
			m_rgseq[i].init( sSeq );
			//kdDebug(125) << "\t'" << sSeq << "' => " << m_rgseq[i].toStringInternal() << endl;
		}
	} else {
		clear();
		bRet = false;
	}

	kdDebug(125) << "KShortcut::init( " << s << " )" << endl;
	for( uint i = 0; i < m_nSeqs; i++ ) {
		kdDebug(125) << "\tm_rgseq[" << i << "]: " << QString::number((int) m_rgseq[i].keyCodeQt(),16) << endl;
		KKeyNative::Variations vars;
		vars.init( m_rgseq[i].key(0), true );
		for( uint j = 0; j < vars.count(); j++ )
			kdDebug(125) << "\t\tvariation = " << QString::number(vars.key(0).keyCodeQt(),16) << endl;
	}
	return bRet;
}

uint KShortcut::count() const
{
	return m_nSeqs;
}

const KKeySequence& KShortcut::seq( uint i ) const
{
	return m_rgseq[i];
}

int KShortcut::keyCodeQt() const
{
	if( m_nSeqs >= 1 )
		return m_rgseq[0].keyCodeQt();
	return QKeySequence();
}

bool KShortcut::isNull() const
{
	return m_nSeqs == 0;
}

int KShortcut::compare( const KShortcut& cut ) const
{
	for( uint i = 0; i < m_nSeqs && i < cut.m_nSeqs; i++ ) {
		int ret = m_rgseq[i].compare( cut.m_rgseq[i] );
		if( ret != 0 )
			return ret;
	}
	return m_nSeqs - cut.m_nSeqs;
}

bool KShortcut::contains( const KKey& key ) const
{
	return contains( KKeySequence(key) );
}

bool KShortcut::contains( const KKeyNative& key ) const
{
	for( uint i = 0; i < count(); i++ ) {
		if( !m_rgseq[i].isNull()
		    && m_rgseq[i].count() == 1
		    && KKeyNative(m_rgseq[i].key(0)) == key )
			return true;
	}
	return false;
}

bool KShortcut::contains( const KKeySequence& seq ) const
{
	for( uint i = 0; i < count(); i++ ) {
		if( !m_rgseq[i].isNull() && m_rgseq[i] == seq )
			return true;
	}
	return false;
}

bool KShortcut::setSeq( uint iSeq, const KKeySequence& seq )
{
	// TODO: check if seq is null, and act accordingly.
	if( iSeq <= m_nSeqs && iSeq < MAX_SEQUENCES ) {
		m_rgseq[iSeq] = seq;
		if( iSeq == m_nSeqs )
			m_nSeqs++;
		return true;
	} else
		return false;
}

bool KShortcut::append( const KKeySequence& seq )
{
	if( m_nSeqs < MAX_SEQUENCES ) {
		if( !seq.isNull() ) {
			m_rgseq[m_nSeqs] = seq;
			m_nSeqs++;
		}
		return true;
	} else
		return false;
}

KShortcut::operator QKeySequence () const
{
	if( count() >= 1 )
		return m_rgseq[0].qt();
	else
		return QKeySequence();
}

QString KShortcut::toString() const
{
	QString s;

	for( uint i = 0; i < count(); i++ ) {
		s += m_rgseq[i].toString();
		if( i < count() - 1 )
			s += ';';
	}

	return s;
}

QString KShortcut::toStringInternal( const KShortcut* pcutDefault ) const
{
	QString s;

	for( uint i = 0; i < count(); i++ ) {
		const KKeySequence& seq = m_rgseq[i];
		if( pcutDefault && i < pcutDefault->count() && seq == (*pcutDefault).seq(i) ) {
			s += "default(";
			s += seq.toStringInternal();
			s += ")";
		} else
			s += seq.toStringInternal();
		if( i < count() - 1 )
			s += ';';
	}

	return s;
}

KShortcut& KShortcut::null()
{
	if( !g_pcut )
		g_pcut = new KShortcut;
	if( !g_pcut->isNull() )
		g_pcut->clear();
	return *g_pcut;
}
