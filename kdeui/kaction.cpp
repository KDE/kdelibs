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
#include<kglobal.h>

#include<kaction.h>
#include<klocale.h>
#include<kiconloader.h>

KAction::KAction( const char *desc, 
			const QString& localDesc,
			QObject *receiver, const char *member )
	: _name( desc ),
	_localName( localDesc ),
	_icon( 0 ),
	_accel ( 0 ),
	_accelId( -1 ),

	_triggers( 0 ),
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
}

void KAction::setAccel( int accel )
{
	_accel = accel;

 	// set accel for all related menu items.

	if( _triggers == 0 ) {
		if ( _accel == 0 ) {
			emit removeAccel( this );
		}
		else {
			emit globalAccel( this, _accel );
		}
		return;
	}

	QListIterator<TriggerInfo> iter( *_triggers );
	TriggerInfo *t = iter.toFirst();
	int menuCount = 0;

	for( ; t != 0; t = ++iter ) {
		if( t->type != Menu ) {
			continue;
		}
		++menuCount;

		t->item.menu->setAccel( accel, t->id );
	}

	if( menuCount ) {
		if ( _accel == 0 ) {
			emit removeAccel( this );
		}
	}
	else {
		if ( _accel == 0 ) {
			emit removeAccel( this );
		}
		else {
			emit globalAccel( this, _accel );
		}
	}
}

void KAction::addMenuItem( MenuType *menu, int id )
{
	assert( menu != 0 );

	TriggerInfo *t = new TriggerInfo( menu, id );

	if( _triggers == 0 ) {
		_triggers = new Triggers;
		_triggers->setAutoDelete( true );
	}

	QListIterator<TriggerInfo> iter( *_triggers );

	// connect to menu's "destroyed" signal only once
	TriggerInfo *i = iter.toFirst();
	for( ; i  && (i->type != Menu || i->item.menu != menu ); 
			 i = ++iter ) {
		// continue till found or end
	}

	if( i ) {
		connect( menu, SIGNAL(destroyed()), this, SLOT(menuDead()) );
	}

	_triggers->append( t );

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

	_triggers->append( new TriggerInfo( uitrigger ) );

	connect( uitrigger, member, this, SIGNAL(activate()) );
	connect( uitrigger, SIGNAL( destroyed() ), this, SLOT(senderDead()) );

	uitrigger->setEnabled( enabled() );
}

void KAction::senderDead()
{
	assert( _triggers != 0 );

	const QObject * s = sender();

	if( s == 0 || ! s->inherits("QWidget") ) {
		return;
	}

	const Trigger *sdr = (const Trigger *)s;

	for (TriggerInfo *curr = _triggers->first(); curr != 0; ) {
		if( curr->type != Button ) {
			curr = _triggers->next();
			continue;
		}

		if ( curr->item.trigger == sdr ) {
			_triggers->remove();
			curr = _triggers->current();
			continue;
		}

		curr = _triggers->next();
	}
}

void KAction::menuDead()
{
	assert( _triggers != 0 );

	const QObject *s = sender();

	if( s == 0 || !s->inherits( "QPopupMenu") ) {
		return;
	}

	const MenuType *sdr = (MenuType *)s;
	int menuCount = 0;

	for (TriggerInfo *curr = _triggers->first(); curr != 0; ) {
		if( curr->type != Menu ) {
			curr = _triggers->next();
			continue;
		}

		++menuCount;

		if ( curr->item.menu == sdr ) {
			_triggers->remove();
			curr = _triggers->current();
			continue;
		}

		curr = _triggers->next();
	}

	if ( menuCount == 0 && _accel != 0 ) {
		emit globalAccel( this, _accel );
	}
}

void KAction::menuActivated( int id )
{
	assert( _triggers != 0 );

	const QObject *s = sender();
	
	if( s == 0 || !s->inherits( "QPopupMenu" ) ) {
		return;
	}

	const MenuType *sdr = (MenuType *)s;

	QListIterator<TriggerInfo> miter( *_triggers );
	const TriggerInfo *m = miter.toFirst();

	for( ; m != 0; m = ++miter ) {
		if ( m->type != Menu ) {
			continue;
		}

		if ( m->item.menu == sdr && m->id == id ) {
			debug( "%s: match! %d", _name.ascii(), m->id );
			emit activate();
			return;
		}
	}
}

void KAction::setEnabled( bool state )
{
	blockSignals( !state );

	if ( _triggers == 0 ) {
		return;
	}
		
	QListIterator<TriggerInfo> titer ( *_triggers );
	TriggerInfo *t = titer.current();

	for( ; t != 0 ; t = ++titer ) {
		switch ( t->type ) {
			case Button:
				t->item.trigger->setEnabled( state );
				break;
			case Menu:
				t->item.menu->setItemEnabled( t->id, state );
				break;
		}
	}
}


bool KAction::readConfig( const KConfigBase& cfg )
{
	QString pfx = "Action" + desc();

	QString key = pfx + "Key";
	if( cfg.hasKey( key ) ) {
		QString accel = cfg.readEntry( key );
		int ck = stringToKey( accel );
		if( ck != 0 ) {
			setAccel( ck );
		}
	}

	key = pfx + "Icon";
	if( cfg.hasKey( key ) ) {
		setIcon( cfg.readEntry( key ) );
	}
	return true;
}

void KAction::writeConfig( KConfigBase& cfg )
{
	QString pfx = "Action" + desc();

	QString key = pfx + "Key";
	cfg.writeEntry( key, keyToString( accel() ) );

	key = pfx + "Icon";
	cfg.writeEntry( key, _iconPath.isEmpty() ? QString("") : _iconPath );
}

uint KAction::stringToKey( const QString& key )
{
	if ( key.isNull() ) {
		debug( "stringToKey::Null key" );
		return 0;
	} else if ( key.isEmpty() ) {
		debug( "stringToKey::Empty key" );
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
			debug( "stringToKey::Empty token" );
			return 0;
		}

		if ( str=="SHIFT" )     keyCode |= Qt::SHIFT;
		else if ( str=="CTRL" ) keyCode |= Qt::CTRL;
		else if ( str=="ALT" )  keyCode |= Qt::ALT;
		else if (codeFound) {
			debug( "stringToKey::Duplicate keycode" );
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
				debug("stringToKey::Unknown key name %s", 
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

void KAction::setIcon( const QString& iconp )
{
	_iconPath = iconp;
	delete _icon; _icon = 0;
}

const QIconSet& KAction::icon() const
{
	if( _icon == 0 ) {
		KIconLoader *ldr = KGlobal::iconLoader();

		// hackety hack
		const_cast<KAction *>(this)->_icon = 
			new QIconSet( ldr->loadIcon( _iconPath, 0, 0, false ) );
	}

	return *_icon;
}


#include"kaction.moc"
