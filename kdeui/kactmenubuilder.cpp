/*
* kactmenubuilder.cpp -- Implementation of class KActionMenuBuilder.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Fri May 21 10:19:15 EST 1999
*/

#include"qpopupmenu.h"

#include"kactmenubuilder.h"
#include"kuiactions.h"
#include"kaction.h"

int KActionMenuBuilder::insert( const QString& action, int id )
{
	KAction *a = _act->action( action );
	if( a == 0 ) {
		return -1;
	}

	id = _menu->insertItem( a->localDesc(), id );

	if( (_style & WhatsThis) && !a->whatsThis().isEmpty() ) {
		_menu->setWhatsThis( id, a->whatsThis() );
	}
	
	if( _style & Icon ) {
		_menu->changeItem( a->icon(), a->localDesc(), id );
	}

	a->addMenuItem( _menu, id );

	return id;
}
