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

#include "artsflow.h"
#include "flowsystem.h"
#include "stdsynthmodule.h"
#include "debug.h"
#include <iostream>

using namespace std;
using namespace Arts;

namespace Arts {

class StereoEffectStack_impl : public StereoEffectStack_skel,
							   public StdSynthModule
{
	long nextID;

	struct EffectEntry {
		StereoEffect effect;
		string name;
		long id;
	};
	list<EffectEntry *> fx;

	void xconnect(bool connect, Object from, string fromP, Object to, string toP)
	{
		if(connect)
			from._node()->connect(fromP,to._node(),toP);
		else
			from._node()->disconnect(fromP,to._node(),toP);
	}
	void xvirtualize(bool connect, string myPort, Object impl, string implPort)
	{
		if(connect)
			_node()->virtualize(myPort,impl._node(),implPort);
		else
			_node()->devirtualize(myPort,impl._node(),implPort);
	}
	void internalconnect(bool c)
	{
		if(fx.empty())
		{
			/* no effects - forward input through to output */
			xvirtualize(c,"outleft",Object::_from_base(this->_copy()),"inleft");
			xvirtualize(c,"outright",Object::_from_base(this->_copy()),"inright");
		}
		else
		{
			list<EffectEntry *>::iterator ei;
			EffectEntry *laste = 0;

			long count = 0;
			for(ei = fx.begin(); ei != fx.end(); ei++, count++)
			{
				EffectEntry *e = *ei;
				if(count == 0)		/* top of chain? virtualize to effect */
				{
					xvirtualize(c,"inleft",e->effect,"inleft");
					xvirtualize(c,"inright",e->effect,"inright");
				}
				else				/* not top? connect last effect to current effect */
				{
					xconnect(c,laste->effect,"outleft",e->effect,"inleft");
					xconnect(c,laste->effect,"outright",e->effect,"inright");
				}
				laste = e;
			}
			/* end: virtualize effect output to our output */
			xvirtualize(c,"outleft",laste->effect,"outleft");
			xvirtualize(c,"outright",laste->effect,"outright");
		}
	}
	void disconnect() { internalconnect(false); }
	void reconnect()  { internalconnect(true); }
public:
	StereoEffectStack_impl() : nextID(1)
	{
		reconnect();
	}
	~StereoEffectStack_impl()
	{
		// disconnect remaining effects
		EffectEntry *laste = 0;
		list<EffectEntry *>::iterator ei;

		for(ei = fx.begin(); ei != fx.end(); ei++)
		{
			EffectEntry *e = *ei;
			if(laste)
			{
				xconnect(false,laste->effect,"outleft",e->effect,"inleft");
				xconnect(false,laste->effect,"outright",e->effect,"inright");
			}
			laste = e;
		}
		// delete remaining effect entries
		for(ei = fx.begin(); ei != fx.end(); ei++)
			delete *ei;
		fx.clear();
	}
	long insertTop(StereoEffect effect, const string& name)
	{
		arts_return_val_if_fail(!effect.isNull(),0);

		disconnect();
		EffectEntry *e = new EffectEntry();
		e->effect = effect;
		e->name = name;
		e->id = nextID++;
		fx.push_front(e);
		reconnect();
		return e->id;
	}
	long insertBottom(StereoEffect effect, const string& name)
	{
		arts_return_val_if_fail(!effect.isNull(),0);

		disconnect();
		EffectEntry *e = new EffectEntry();
		e->effect = effect;
		e->name = name;
		e->id = nextID++;
		fx.push_back(e);
		reconnect();
		return e->id;
	}

	void remove(long ID)
	{
		arts_return_if_fail(ID != 0);

		bool found = false;
		disconnect();
		list<EffectEntry *>::iterator ei = fx.begin();
		
		while(ei != fx.end())
		{
			if((*ei)->id == ID) {
				found = true;
				delete (*ei);
				fx.erase(ei);
				ei = fx.begin();
			}
			else ei++;
		}
		if(!found) {
			arts_warning("StereoEffectStack::remove failed. id %d not found?",
							ID);
		}
		reconnect();
	}

	AutoSuspendState autoSuspend() { return asSuspend; }
};

REGISTER_IMPLEMENTATION(StereoEffectStack_impl);

};
