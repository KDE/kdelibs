/*
* kuiactions.cpp -- Implementation of class KUIActions.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Thu May  6 01:08:20 EST 1999
*/

#include<assert.h>
#include<qaccel.h>

#include"kuiactions.h"

KUIActions::KUIActions( QWidget *parent )
	: QObject( parent ),
	_actions ( new ActionDict ),
	_accelContext( parent ),
	_accel( new QAccel( parent ) )
{
	assert( parent != 0 );

	_actions->setAutoDelete( true );
}

KUIActions::~KUIActions()
{
	delete _actions; _actions = 0;
	delete _accel;	_accel = 0;
}

KAction *KUIActions::newAction( const char *name, 
			QObject *receiver,
			const char *member,
			int accel)
{
	assert( name != 0 && _actions->find( name ) == 0 );

	KAction *action = new KAction( name, QString::null, receiver, member );

	if( action == 0 ) {
		warning( "newAction:: out of memory" );
		return  0;
	}
	_actions->insert( name, action );

	connect( action, SIGNAL(globalAccel(KAction *, int)),
		this, SLOT(setAccel(KAction *, int)) );
	connect( action, SIGNAL(removeAccel(KAction *)),
		this, SLOT(removeAccel(KAction *)) );

	if( accel ) {
		action->setAccel( accel );
	}
	
	return action;
}

KAction *KUIActions::newAction( const char *name,
			const QString& i18nName,
			QObject *receiver,
			const char *member,
			int accel )
{

	KAction *a = newAction( name, receiver, member, accel );
	assert( a != 0 );

	a->setLocalDesc( i18nName );

	return a;
}

KAction *KUIActions::newAction( KAction *action )
{
	assert( action != 0 && _actions->find( action->desc() ) == 0 );

	connect( action, SIGNAL(accelChanged( const KAction *, int)),
		this, SLOT(updateAccel( const KAction *, int)) );
	
	_actions->insert( action->desc(), action );

	return action;
}


bool KUIActions::deleteAction( const char *action )
{
	KAction *a = _actions->find( action );
	assert( a != 0 );

	return _actions->remove( action );
}

bool KUIActions::enabled( const char *action )
{
	KAction *a = _actions->find( action );
	assert( a != 0 );

	return a->enabled();
}

void KUIActions::setEnabled( const char *action, bool enabled )
{
	KAction *a = _actions->find( action );
	assert( a != 0 );

	a->setEnabled( enabled );
}

void KUIActions::setAccel( KAction *action, int accel )
{
	assert( action != 0 );
	debug( "accel: %s, %d", action->desc().ascii(), accel );

	// enable the accel if it's valid, remove otherwise
	int id = action->accelId();

	if( id != -1 ) {
		_accel->removeItem( id );
		id = -1;
	}

	if( accel != 0 && action->receiver() != 0 
			&& !action->member().isNull() ) {
		id = _accel->insertItem( accel, id );
		_accel->connectItem( id, action->receiver(), 
			action->member().ascii() );
	}
	
	action->setAccelId( id );
}

void KUIActions::removeAccel( KAction *action )
{
	int id = action->accelId();

	if ( id == -1 ) {
		return;
	}

	_accel->removeItem( id );
	action->setAccelId( -1 );
}

#include "kuiactions.moc"
