//----------------------------------------------------------------------------
//    filename             : k3mdinulliterator.h
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

#ifndef _K3MDINULLITERATOR_H_
#define _K3MDINULLITERATOR_H_

#include "k3mdiiterator.h"

template <class Item>
class K3MdiNullIterator : public K3MdiIterator<Item> {
public:
   K3MdiNullIterator() {}
   virtual void first() {}
   virtual void last() {}
   virtual void next() {}
   virtual void prev() {}
   virtual bool isDone() const { return true; }
   virtual Item currentItem() const {
   /* should really never go inside here */
   return 0;
   }
};

#endif // _K3MDINULLITERATOR_H_
// kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;

