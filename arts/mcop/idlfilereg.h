    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

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

#ifndef IDLFILEREG_H
#define IDLFILEREG_H

#include <string>
#include "startupmanager.h"

/*
 * BC - Status (2000-09-30): IDLFileReg
 *
 * Will be kept binary compatible by NOT TOUCHING AT ALL. Do not change this.
 * (Interaction with generated code). Add a new class if you need something
 * else than this does.
 */

namespace Arts {

class IDLFileReg : public StartupClass {
	long nr;
	const char *_name, *_contents;
public:
	IDLFileReg(const char *name, const char *contents);
	void startup();
	void shutdown();
};
};
#endif /* IDLFILEREG_H */
