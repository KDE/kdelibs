/* This file is part of the KDE project
   Copyright (C) 2002 Anders Lund <anders@alweb.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
   
   $Id: selectionextdcopinterface.h 465272 2005-09-29 09:47:40Z mueller $
*/

#ifndef _SELECTION_EXT_DCOP_INTERFACE_H_
#define _SELECTION_EXT_DCOP_INTERFACE_H_

#include "selectioninterfaceext.h"

#include <qstring.h>
#include <dcopobject.h>
#include <dcopref.h>

namespace KTextEditor {
/**
    DCOP interface for the SelectionInterfaceExt.
    @author Anders Lund <anders@alweb.dk>
*/
class KTEXTEDITOR_EXPORT SelectionExtDCOPInterface : virtual public DCOPObject
{
  K_DCOP
  public:
    SelectionExtDCOPInterface( SelectionInterfaceExt *parent, const char *name );
    virtual ~SelectionExtDCOPInterface();
  k_dcop:
    /** The selection start line number */
    int selStartLine();
    /** The selection start col */
    int selStartCol();
    /** The selection end line */
    int selEndLine();
    /** The selection end col */
    int selEndCol();
  private:
    SelectionInterfaceExt *m_parent;
};

}  // namespace KTextEditor

#endif // _SELECTION_EXT_DCOP_INTERFACE_H_
