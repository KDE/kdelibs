#include <string>
#include <list>
#include <vector>
#include "stdio.h"

namespace Arts {

/**
 * -- internal class --
 * 
 * this stores key-value pairs, where key is a string which is kept unique
 */
template<class T>
class NamedStore
{
private:
	class Element {
	public:
		T t;
		std::string name;

		Element(const T& t, const std::string& name) :t(t), name(name) { }
	};
	typedef std::list<Element> Element_list;
	Element_list elements;

public:
	bool get(std::string name, T& result)
	{
		typename Element_list::iterator i;
		for(i = elements.begin(); i != elements.end(); i++)
		{
			if(i->name == name)
			{
				result = i->t;
				return true;
			}
		}

		return false;
	}
	bool remove(std::string name)
	{
		typename Element_list::iterator i;
		for(i = elements.begin(); i != elements.end(); i++)
		{
			if(i->name == name)
			{
				elements.erase(i);
				return true;
			}
		}
		return false;
	}
	std::vector<std::string> *contents()
	{
		std::vector<std::string> *result = new std::vector<std::string>;

		typename Element_list::iterator i;
		for(i = elements.begin(); i != elements.end(); i++)
			result->push_back(i->name);

		return result;
	}
	std::string put(const std::string& name, const T& t)
	{
		std::string xname = name;
		int append = 1;

		for(;;)
		{
			typename Element_list::iterator i;
		
			i = elements.begin();
			while(i != elements.end() && i->name != xname)
				i++;

			if(i == elements.end())
			{
				elements.push_back(Element(t,xname));
				return xname;
			}

			char buffer[4096];
			sprintf(buffer,"%s%d",name.c_str(),append++);
			xname = buffer;
		}
	}
};

};
