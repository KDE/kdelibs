    /*

    Copyright (C) 2001 Stefan Westerfeld
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

#include "dynamicskeleton.h"
#include "dispatcher.h"
#include "debug.h"
#include "stdio.h"

using namespace Arts;
using namespace std;

/* we could share the "class part" of those implicitely, as
   types never change */

namespace Arts {
class DynamicSkeletonData {
public:
	DynamicSkeletonData(DynamicSkeletonBase *base,
						Object_skel *skel,
						const string& interfaceName,
						const string& interfaceNameParent)
		: base(base), skel(skel), interfaceName(interfaceName),
		  interfaceNameParent(interfaceNameParent)
	{
	}

	enum InterfaceType { itNone, itParent, itDynamic };

	void buildInterfaces();
	void buildInterfaces(const string& s, InterfaceType t);
	void buildMethodTable();

	DynamicSkeletonBase *base;
	Object_skel *skel;
	string interfaceName, interfaceNameParent;
	map<string, InterfaceType> interfaceMap;
	map<string, void**> attrs;
};
};

DynamicSkeletonBase::DynamicSkeletonBase(Object_skel *skel,
	const std::string& interfaceName, const std::string& interfaceNameParent)
{
	d = new DynamicSkeletonData(this,skel,interfaceName,interfaceNameParent);
	d->buildInterfaces();

	/* TODO: optimize me! */
	map<string, DynamicSkeletonData::InterfaceType>::iterator ii;
	for(ii = d->interfaceMap.begin(); ii != d->interfaceMap.end(); ii++)
	{
		if(ii->second == DynamicSkeletonData::itDynamic)
		{
			InterfaceDef id;
			id = Dispatcher::the()->interfaceRepo().queryInterface(ii->first);

			vector<AttributeDef>::iterator ai;
			for(ai = id.attributes.begin(); ai != id.attributes.end(); ai++)
			{
				AttributeDef& ad = *ai;
				if((ad.flags & attributeStream) == attributeStream)
				{
					void**& data = d->attrs[ad.name.c_str()];
					arts_assert(data == 0);
					data = new (void*);
					d->skel->_initStream(ad.name.c_str(),data,ad.flags);
				}
			}
		}
	}
}

DynamicSkeletonBase::~DynamicSkeletonBase()
{
	delete d;
}

std::string DynamicSkeletonBase::_dsInterfaceName()
{
	return d->interfaceName;
}

bool DynamicSkeletonBase::_dsIsCompatibleWith(const std::string& interfacename)
{
	d->buildInterfaces();
	return d->interfaceMap[interfacename] != 0;
}

void DynamicSkeletonBase::_dsBuildMethodTable()
{
	d->buildMethodTable();
}

static void _dispatch_dynamic(void *object, long methodID, Buffer *request, Buffer *result)
{
	((DynamicSkeletonBase *)object)->process(methodID, request, result);
}

void DynamicSkeletonData::buildMethodTable()
{
	buildInterfaces();

	map<string, DynamicSkeletonData::InterfaceType>::iterator ii;
	for(ii = interfaceMap.begin(); ii != interfaceMap.end(); ii++)
	{
		if(ii->second == itDynamic)
		{
			InterfaceDef id;
			id = Dispatcher::the()->interfaceRepo().queryInterface(ii->first);

			/* methods */
			vector<MethodDef>::iterator mi;
			for(mi = id.methods.begin(); mi != id.methods.end(); mi++)
				skel->_addMethod(_dispatch_dynamic, base, *mi);

			/* _get_ and _set_ methods for attributes */
			vector<AttributeDef>::iterator ai;
			for(ai = id.attributes.begin();ai != id.attributes.end();ai++)
			{
				AttributeDef& ad = *ai;

				if(ad.flags & attributeAttribute)
				{
					MethodDef md;
					if(ad.flags & streamOut)  /* readable from outside */
					{
						md.name = "_get_"+ad.name;
						md.type = ad.type;
						md.flags = methodTwoway;
						/* no parameters (don't set md.signature) */

						skel->_addMethod(_dispatch_dynamic, base, md);
					}
					if(ad.flags & streamIn)  /* writeable from outside */
					{
						md.name = "_set_"+ad.name;
						md.type = "void";
						md.flags = methodTwoway;

						ParamDef pd;
						pd.type = ad.type;
						pd.name = "newValue";
						md.signature.push_back(pd);

						skel->_addMethod(_dispatch_dynamic, base, md);
					}
				}
			}
		}
	}
}

void DynamicSkeletonData::buildInterfaces()
{
	if(interfaceMap.size() != 0) return;

	buildInterfaces(interfaceName, itDynamic);
	buildInterfaces(interfaceNameParent, itParent);
}

void DynamicSkeletonData::buildInterfaces(const string& name, DynamicSkeletonData::InterfaceType t)
{
	InterfaceDef id = Dispatcher::the()->interfaceRepo().queryInterface(name);  

	if(interfaceMap[name] == t) return;
	interfaceMap[name] = t;

    vector<string>::iterator ii;
    for(ii = id.inheritedInterfaces.begin();
        ii != id.inheritedInterfaces.end(); ii++)
	{
        buildInterfaces(*ii,t);
	}
	buildInterfaces("Arts::Object",t);
}
