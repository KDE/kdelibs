#include "artsflow.h"
#include "flowsystem.h"
#include "stdsynthmodule.h"
#include <iostream>

using namespace std;

class StereoEffectStack_impl : public StereoEffectStack_skel,
							   public StdSynthModule
{
	Object_var leftIn,rightIn,leftOut,rightOut;
	string leftInP, rightInP, leftOutP, rightOutP;
	bool haveIn, haveOut;
	long nextID;

	float *inleft, *inright, *outleft, *outright;
	struct EffectEntry {
		StereoEffect_var effect;
		string name;
		long id;
	};
	list<EffectEntry *> fx;

	void xconnect(bool connect, Object *from, string fromP, Object *to, string toP)
	{
		if(connect)
			from->_node()->connect(fromP,to->_node(),toP);
		else
			from->_node()->disconnect(fromP,to->_node(),toP);
	}
	void internalconnect(bool c)
	{
		if(!haveIn || !haveOut) return;
		if(fx.empty())
		{
			xconnect(c,leftIn,leftInP,leftOut,leftOutP);
			xconnect(c,rightIn,rightInP,rightOut,rightOutP);
		}
		else
		{
			list<EffectEntry *>::iterator ei;
			EffectEntry *laste = 0;

			long count = 0;
			for(ei = fx.begin(); ei != fx.end(); ei++, count++)
			{
				EffectEntry *e = *ei;
				if(count == 0)		/* top of chain? connect input nodes to effect */
				{
					xconnect(c,leftIn,leftInP,e->effect,"inleft");
					xconnect(c,rightIn,rightInP,e->effect,"inright");
				}
				else				/* not top? connect last effect to current effect */
				{
					xconnect(c,laste->effect,"outleft",e->effect,"inleft");
					xconnect(c,laste->effect,"outright",e->effect,"inright");
				}
				laste = e;
			}
			/* end: connect effect output to output nodes */
			xconnect(c,laste->effect,"outleft",leftOut,leftOutP);
			xconnect(c,laste->effect,"outright",rightOut,rightOutP);
		}
	}
	void disconnect() { internalconnect(false); }
	void reconnect()  { internalconnect(true); }
public:
	StereoEffectStack_impl() : haveIn(false), haveOut(false), nextID(1)
	{
	}
	void setInputs(Object *leftObj, const string& leftPort,
					Object *rightObj, const string& rightPort)
	{
		disconnect();
		leftIn = leftObj->_copy();
		rightIn = rightObj->_copy();
		leftInP = leftPort;
		rightInP = rightPort;
		haveIn = true;
		reconnect();
	}
	void setOutputs(Object *leftObj, const string& leftPort,
					Object *rightObj, const string& rightPort)
	{
		disconnect();
		leftOut = leftObj->_copy();
		rightOut = rightObj->_copy();
		leftOutP = leftPort;
		rightOutP = rightPort;
		haveOut = true;
		reconnect();
	}

	long insertTop(StereoEffect_base* effect, const string& name)
	{
		disconnect();
		EffectEntry *e = new EffectEntry();
		e->effect = effect->_copy();
		e->name = name;
		e->id = nextID++;
		fx.push_front(e);
		reconnect();
		return e->id;
	}
	long insertBottom(StereoEffect_base* effect, const string& name)
	{
		disconnect();
		EffectEntry *e = new EffectEntry();
		e->effect = effect->_copy();
		e->name = name;
		e->id = nextID++;
		fx.push_back(e);
		reconnect();
		return e->id;
	}

	void remove(long ID)
	{
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
			cout << "StereoEffectStack::remove failed. id "
					<< ID << "not found?" << endl;
		}
		reconnect();
	}
};

REGISTER_IMPLEMENTATION(StereoEffectStack_impl);
