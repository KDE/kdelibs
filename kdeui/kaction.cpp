/*
* kuseraction.cpp -- Implementation of class KAction.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Sat May  1 14:43:58 EST 1999
*/


#include<assert.h>
#include<qiconset.h>
#include<qpopupmenu.h>

#include<kckey.h>
#include<kconfigbase.h>

#include<kaction.h>
#include<kdebug.h>
#include<klocale.h>
//#include<kconfigbase.h>

KAction::KAction( const char *desc, 
			const QString& localDesc,
			QObject *receiver, const char *member )
	: _name( desc ),
	_localName( localDesc ),
	_icon( 0 ),
	_accel ( 0 ),
	_accelId( -1 ),

	_triggers( 0 ),
	_menus( 0 ),
	_receiver( 0 ),
	_member( QString::null )
{
	if( receiver && member ) {
		connect( this, SIGNAL(activate()), receiver, member);
		_receiver = receiver;
		_member = member;
	}

	if( _localName.isNull() ) {
		_localName = _name;
	}

	setEnabled( true );
}

KAction::~KAction()
{
	emit removeAccel( this );

	delete _icon;		_icon = 0;
	delete _triggers;	_triggers = 0;
	delete _menus;		_menus = 0;
}

void KAction::setIcon( const QIconSet& icon )
{
	*_icon = icon;
}

const QIconSet& KAction::icon() const
{
	return *_icon;
}

void KAction::setAccel( int accel )
{
	_accel = accel;

	if( _menus == 0 ) {
		if ( _accel == 0 ) {
			emit removeAccel( this );
		}
		else {
			emit globalAccel( this, _accel );
		}

		return;
	}
 
 	// set accel for all related menu items.

	QListIterator<MenuTrigger> iter( *_menus );
	MenuTrigger *t = iter.toFirst();

	for( ; t != 0; t = ++iter ) {
		t->menu->setAccel( accel, t->id );
	}

	if( t != 0 && _accel != 0 ) {
		removeAccel( this );
	}
}

void KAction::addMenuItem( MenuType *menu, int id )
{
	assert( menu != 0 );

	MenuTrigger *t = new MenuTrigger( menu, id );

	if( _menus == 0 ) {
		_menus = new MenuItems;
		_menus->setAutoDelete( true );
	}

	QListIterator<MenuTrigger> iter( *_menus );

	// connect to menu's "destroyed" signal only once
	for( iter.toFirst(); iter.current() && iter.current()->menu != menu; 
			 ++iter ) {
		// continue till found or end
	}

	if( !iter.current() ) {
		connect( menu, SIGNAL(destroyed()), this, SLOT(menuDead()) );
	}

	_menus->append( t );

	connect( menu, SIGNAL(activated(int)), 
			this, SLOT(menuActivated(int)) );

	menu->setItemEnabled( id, enabled() );

	if( _accel ) {
		menu->setAccel( _accel, id );
	}
	
	debug( "going to set global accel..." );

	removeAccel( this );
}

void KAction::addTrigger( Trigger *uitrigger, const char *member,
		bool allowDisable )
{
	assert( uitrigger != 0 );

	if( _triggers == 0 ) {
		_triggers = new Triggers;
		_triggers->setAutoDelete( false );
	}

	if ( !allowDisable ) {
		// TODO: implement this
		warning( "KAction::addTrigger: Disallowing disable is not"
			" yet implemented." );
	}

	_triggers->append( uitrigger );
	connect( uitrigger, member, this, SIGNAL(activate()) );
	connect( uitrigger, SIGNAL( destroyed() ), this, SLOT(senderDead()) );

	uitrigger->setEnabled( enabled() );
}

void KAction::senderDead()
{
	const QObject * s = sender();

	if( s == 0 || ! s->inherits("QWidget") ) {
		return;
	}

	const Trigger *sdr = (const Trigger *)s;

	for (Trigger *curr = _triggers->first(); curr != 0; ) {
		if ( curr == sdr ) {
			_triggers->remove();
			curr = _triggers->current();
			continue;
		}

		curr = _triggers->next();
	}
}

void KAction::menuDead()
{
	const QObject *s = sender();

	if( s == 0 || !s->inherits( "QPopupMenu") ) {
		return;
	}

	const MenuType *sdr = (MenuType *)s;

	for (MenuTrigger *curr = _menus->first(); curr != 0; ) {
		if ( curr->menu == sdr ) {
			_menus->remove();
			curr = _menus->current();
			continue;
		}

		curr = _menus->next();
	}

	if ( _menus->count() == 0 && _accel != 0 ) {
		emit globalAccel( this, _accel );
	}
}

void KAction::menuActivated( int id )
{
	const QObject *s = sender();
	
	if( s == 0 || !s->inherits( "QPopupMenu" ) ) {
		return;
	}

	const MenuType *sdr = (MenuType *)s;

	QListIterator<MenuTrigger> miter( *_menus );
	const MenuTrigger *m = miter.toFirst();

	for( ; m != 0; m = ++miter ) {
		if ( m->menu == sdr && m->id == id ) {
			debug( "%s: match! %d", _name.ascii(), m->id );
			emit activate();
			return;
		}
	}
}

void KAction::setEnabled( bool state )
{
	blockSignals( !state );

	if( _triggers != 0 ) {
		QListIterator<Trigger> titer ( *_triggers );

		for( ; titer.current(); ++titer ) {
			titer.current()->setEnabled( state );
		}
	}

	if( _menus != 0 ) {
		QListIterator<MenuTrigger> miter ( *_menus );
		MenuTrigger *t = 0;

		for( ; (t = miter.current()) != 0; ++miter ) {
			t->menu->setItemEnabled( t->id, state );
		}
	}
}


bool KAction::readConfig( const KConfigBase& cfg )
{
#ifndef KAC_NO_CFG
	QString pfx = "Action" + desc();

	QString key = pfx + "Key";
	if( cfg.hasKey( key ) ) {
		QString key = cfg.readEntry( key );
		int ck = stringToKey( key );
		if( ck != 0 ) {
			setAccel( ck );
		}
	}
	// TODO: read icon
	return true;
#endif	
}

void KAction::writeConfig( KConfigBase& cfg )
{
#ifndef KAC_NO_CFG
	QString pfx = "Action" + desc();

	QString key = pfx + "Accel";
	cfg.writeEntry( key, accel() );

	key = pfx + "Key";
	cfg.writeEntry( key, keyToString( accel() ) );
#endif
}

uint KAction::stringToKey( const QString& key )
{
	if ( key.isNull() ) {
		kdebug(KDEBUG_WARN, 125, "stringToKey::Null key");
		return 0;
	} else if ( key.isEmpty() ) {
		kdebug(KDEBUG_WARN, 125, "stringToKey::Empty key");
		return 0;
	}

	// break the string in tokens separated by "+"
	uint k = 0;
	QArray<int> tokens;
	int i = -1;

	do {
		tokens.resize(k+1);
		tokens[k] = i+1;
		i = key.find('+', i+1);
		k++;
	} while ( i!=-1 );
	tokens.resize(k+1);
	tokens[k] = key.length() + 1;
	
	// we have k tokens.
	// find a keycode (only one)
	// the other tokens are accelerators (SHIFT, CTRL & ALT)
	// the order is unimportant
	bool codeFound = FALSE;
	QString str;
	uint keyCode = 0;
	for (uint i=0; i<k; i++) {
		str = key.mid(tokens[i], tokens[i+1]-tokens[i]-1);
		str.stripWhiteSpace();
		if ( str.isEmpty() ) {
			kdebug(KDEBUG_WARN, 125, "stringToKey::Empty token");
			return 0;
		}

		if ( str=="SHIFT" )     keyCode |= Qt::SHIFT;
		else if ( str=="CTRL" ) keyCode |= Qt::CTRL;
		else if ( str=="ALT" )  keyCode |= Qt::ALT;
		else if (codeFound) {
			kdebug(KDEBUG_WARN, 125, 
				"stringToKey::Duplicate keycode");
			return 0;
		} else {
			// search for keycode
			uint j;
			for(j=0; j<NB_KEYS; j++) {
				if ( str==KKEYS[j].name ) {
				    keyCode |= KKEYS[j].code;
					break;
				}
			}
			if ( j==NB_KEYS ) {
				kdebug(KDEBUG_WARN, 125, 
					"stringToKey::Unknown key name %s", 
					str.ascii());
				return 0;
			}
		}
	}
	return keyCode;
}

QString KAction::keyToString( uint keyCode, bool i18_n )
{
	QString res = "";
	
	if ( keyCode == 0 ) return res;
	if ( keyCode & Qt::SHIFT ){
		if (i18_n) res += i18n("SHIFT");
	    else       res += "SHIFT";
	    res += "+";
	}
	if ( keyCode & Qt::CTRL ){
	   if (i18_n) res += i18n("CTRL");
	   else       res += "CTRL";
	    res += "+";
	}
	if ( keyCode & Qt::ALT ){
		if (i18_n) res += i18n("ALT");
		else       res += "ALT";
	    res += "+";
	}

	uint kCode = keyCode & ~(Qt::SHIFT | Qt::CTRL | Qt::ALT);

	for (int i=0; i<NB_KEYS; i++) {
		if ( kCode == (uint)KKEYS[i].code ) {
			res += KKEYS[i].name;
			return res;
		}
	}
	
	return QString::null;
}

#include"kaction.moc"
