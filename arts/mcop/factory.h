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

#ifndef FACTORY_H
#define FACTORY_H

#include <string>
#include "object.h"
#include "startupmanager.h"

/*
 * BC - Status (2000-09-30): Factory
 *
 * Will need to remain binary compatible (REGISTER_IMPLEMENTATION), d ptr
 * provided for convenience, watch out for interactions with generated
 * code.
 */

namespace Arts {

class FactoryPrivate;

class Factory : public StartupClass {
private:
	FactoryPrivate *d;
public:
	void startup();
	void shutdown();
	virtual Object_skel *createInstance() = 0;
	virtual std::string interfaceName() = 0;
};

/*
 * Dont's forget to place a semicolon after the use of this macro,
 * like:
 * REGISTER_IMPLEMENTATION(MyClass);
 */
#define REGISTER_IMPLEMENTATION(impl)                                      \
  class impl ## _Factory : virtual public Arts::Factory {                  \
  public:                                                                  \
    std::string interfaceName() { return impl::_interfaceNameSkel(); } \
	Arts::Object_skel *createInstance() { return new impl ; }              \
} The_ ## impl ## _Factory

};

#endif
