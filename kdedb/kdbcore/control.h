/*
   This file is part of the KDB libraries
   Copyright (c) 2000 Praduroux Alessandro <pradu@thekompany.com>
 
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/     

#ifndef KDB_CONTROL_H
#define KDB_CONTROL_H "$Id$"

#include <kdb/capability.h>

class KConfigBase;
class QWidget;

namespace KDB {

/**
 * Base class to extend the control dialog. All plugins that need a specific
 * configuration dialog in the KControl module must provide a subclass of
 * this class.
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @version kdbcore 0.0.2
 */

class Control : public Capability {

    Q_OBJECT

 public:
    Control(const char * name);

    virtual ~Control();

    virtual capability provides() { return CONFIG; };

    /**
     * show a modal configuration dialog for this plugin. The dialog must
     * save its data on the provided config object
     */
    virtual bool showDialog(KConfigBase *conf = 0L, QWidget *parent = 0L, const char * name = 0L) = 0;
};

}

#endif
