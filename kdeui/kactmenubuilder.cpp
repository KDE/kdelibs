/*
* kactmenubuilder.cpp -- Implementation of class KActionMenuBuilder.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Fri May 21 10:19:15 EST 1999
*/

#include<assert.h>
#include<qpopupmenu.h>

#include"kactmenubuilder.h"
#include"kuiactions.h"
#include"kaction.h"

int KActionMenuBuilder::insert( const QString& action, int index )
{
	assert( _menu != 0 );
	
	KAction *a = _act->action( action );
	if( a == 0 ) {
		return -1;
	}

	int id = _menu->insertItem( a->localDesc(), -1, index );

	if( (_style & WhatsThis) && !a->whatsThis().isEmpty() ) {
		_menu->setWhatsThis( id, a->whatsThis() );
	}
	
	if( _style & Icon ) {
		_menu->changeItem( a->icon(), a->localDesc(), id );
	}

	a->addMenuItem( _menu, id );

	return id;
}

void KActionMenuBuilder::insertSeparator(int index )
{
	assert( _menu != 0 );
	_menu->insertSeparator( index );
}

int KActionMenuBuilder::insertPopup( const QString& text, 
		QPopupMenu *menu, int index )
{
	assert( _menu != 0 );
	
	return _menu->insertItem( text, menu, index );
}

int KActionMenuBuilder::insertList( const QStringList& actions, 
	int index )
{
	if( actions.count() == 0 ) {
		return -1;
	}

	QStringList::ConstIterator iter = actions.begin();

	// first item is separate so that we can get its index
	// and id.
	
	int id = insert( *iter, index );
	index = _menu->indexOf( id );

	// now add the rest
	for( ++iter; iter != actions.end(); ++iter, ++index ) {
		insert( *iter, index );
	}

	return id;
}
