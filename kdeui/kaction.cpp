/*
* kuseraction.cpp -- Implementation of class KAction.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Sat May  1 14:43:58 EST 1999
*/


#include<assert.h>
#include<qiconset.h>
#include<qmenudata.h>

#include<kconfigbase.h>

#include<kaction.h>
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
	QMenuData *d = 0;

	for( MenuTrigger *t = iter.toFirst(); 
			t != 0; t = ++iter ) {
		d = dynamic_cast<QMenuData *>( t->menu );
		assert( d != 0 );
		d->setAccel( accel, t->id );
	}

	if( d != 0 && _accel != 0 ) {
		removeAccel( this );
	}
}

void KAction::addMenuItem( QObject *menu, int id )
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

	QMenuData *menud = dynamic_cast<QMenuData *>(menu);
	menud->setItemEnabled( id, enabled() );

	if( _accel ) {
		menud->setAccel( _accel, id );
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
	const Trigger *sdr = dynamic_cast<const Trigger *>(sender());

	if( sdr == 0 ) {
		return;
	}

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
	const QObject *sdr = sender();

	if( sdr == 0 || _menus == 0 ) {
		return;
	}

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
	const QObject *sdr = sender();
	
	if( sdr == 0 || _menus == 0 ) {
		return;
	}

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
			QMenuData *menu = dynamic_cast<QMenuData *>(t->menu);
			assert( menu != 0 );
			menu->setItemEnabled( t->id, state );
		}
	}
}


bool KAction::readConfig( const KConfigBase& cfg )
{
	QString pfx = "Action" + desc();
	QString key = pfx + "Accel";

	if( cfg.hasKey( key ) ) {
		setAccel( cfg.readNumEntry( key ) );
	}
	// TODO: read icon
	return true;
	
}

void KAction::writeConfig( KConfigBase& cfg )
{
	QString pfx = "Action" + desc();
	QString key = pfx + "Accel";

	cfg.writeEntry( key, accel() );
}

#include"kaction.moc"
