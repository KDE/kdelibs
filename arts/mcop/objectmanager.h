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

#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include "object.h"
#include "factory.h"
#include <list>

/*
 * BC - Status (2000-09-30): ObjectManager
 *
 * Keep binary compatible (since accessible via ::the()), use d ptr when
 * changing it.
 */

namespace Arts {

class Object;
class ObjectManagerPrivate;

class ObjectManager {
private:
	ObjectManagerPrivate *d;

protected:
	static ObjectManager *_instance;
	std::list<Factory *> factories;
	std::list<std::string> referenceNames;

public:
	ObjectManager();
	~ObjectManager();

	static ObjectManager *the();
	Object_skel *create(std::string name);

	bool addGlobalReference(Object object, std::string name);
	std::string getGlobalReference(std::string name);
	void removeGlobalReferences();
	void removeExtensions();
	void shutdownExtensions();

	void registerFactory(Factory *factory);
	void removeFactory(Factory *factory);

	void provideCapability(const std::string& capability);
	void removeCapability(const std::string& capability);
};

};

#endif /* OBJECTMANAGER_H */
