//----------------------------------------------------------------------------
//    filename             : k3mdilistiterator.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 02/2000       by Massimo Morin
//    changes              : 02/2000       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//
//    copyright            : (C) 1999-2003 by Massimo Morin (mmorin@schedsys.com)
//                                         and
//                                         Falk Brettschneider
//    email                :  falkbr@kdevelop.org (Falk Brettschneider)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#ifndef _K3MDILISTITERATOR_H_
#define _K3MDILISTITERATOR_H_

#include <k3mdiiterator.h>

template <class I>
class Q3PtrList;
template <class I>
class Q3PtrListIterator;

template <class Item>
class K3MdiListIterator : public K3MdiIterator<Item*>
{
public:
	K3MdiListIterator( Q3PtrList<Item>& list )
	{
		m_iterator = new Q3PtrListIterator<Item>( list );
	}

	virtual void first() { m_iterator->toFirst(); }
	virtual void last() { m_iterator->toLast(); }
	virtual void next() { ++( *m_iterator ); }
	virtual void prev() { --( *m_iterator ); }
	virtual bool isDone() const { return m_iterator->current() == 0; }
	virtual Item* currentItem() const { return m_iterator->current(); }

	virtual ~K3MdiListIterator() { delete m_iterator; }

private:
	Q3PtrListIterator<Item> *m_iterator;
};

#endif // _K3MDILISTITERATOR_H_ 
// kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;

