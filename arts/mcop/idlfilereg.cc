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

#include "core.h"

using namespace Arts;

IDLFileReg::IDLFileReg(const char *name, const char *contents)
{
	_name = name;
	_contents = contents;
}

void IDLFileReg::startup()
{
	Buffer b;
	b.fromString(_contents,"IDLFile");

	nr = Dispatcher::the()->interfaceRepo().insertModule(ModuleDef(b));
}

void IDLFileReg::shutdown()
{
	Dispatcher::the()->interfaceRepo().removeModule(nr);
}
