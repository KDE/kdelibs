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
#ifndef KDB_CAPABILITY_H
#define KDB_CAPABILITY_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "object.h"
#include "kdb.h"

namespace KDB {

/**
 * base class for all Capabilities implemented by a plugin
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @version kdbcore 0.0.2
 */

class Capability : public Object {

    Q_OBJECT

 public:
    Capability(const char * name);

    virtual ~Capability();

    virtual capability provides() = 0;
};

}

#endif


