#include "core.h"
#include "debug.h"
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

using namespace Arts;
using namespace std;

namespace Arts {

// TraderRestriction
struct TraderRestriction {
	TraderRestriction(const string& key, const string& value)
		:key(key), value(value) { }

	string key;
	string value;
};

// TraderHelper (internal, no public interface)
class TraderHelper {
private:
	vector<class TraderOffer_impl *> allOffers;
	static TraderHelper *_instance;

protected:
	TraderHelper();
	~TraderHelper();
	void addDirectory(const string& directory, const string& iface = "",
						map< pair<dev_t,ino_t>, bool > *dirsDone = 0);

public:
	vector<TraderOffer> *doQuery(const vector<TraderRestriction>& query);
	void load();
	void unload();
	static TraderHelper *the();
	static void shutdown();
};

// TraderQuery
class TraderQuery_impl : virtual public TraderQuery_skel {
private:
	vector<TraderRestriction> restrictions;
public:
	void supports(const string& property, const string& value);
	vector<TraderOffer> *query();
};

REGISTER_IMPLEMENTATION(TraderQuery_impl);

// TraderOffer
class TraderOffer_impl : virtual public TraderOffer_skel {
private:
	string _interfaceName;
	map<string, vector<string> > property;

public:
	TraderOffer_impl(const string& interfaceName, const string& filename);

	bool match(const vector<TraderRestriction>& restrictions);

	// IDL
	string interfaceName();
	vector<string>* getProperty(const string& name);
};

};

// TraderQuery
//----------------------------------------------------------------------------
void TraderQuery_impl::supports(const string& property, const string& value)
{
	restrictions.push_back(TraderRestriction(property,value));
}

vector<TraderOffer> *TraderQuery_impl::query()
{
	return TraderHelper::the()->doQuery(restrictions);
}

// TraderOffer
//----------------------------------------------------------------------------
TraderOffer_impl::TraderOffer_impl(const string& interfaceName,
								   const string& filename)
								   :_interfaceName(interfaceName)
{
	ifstream file(filename.c_str());
	string line;

	while(getline(file,line))
	{
		string key;
		vector<string> values;

		if(MCOPUtils::tokenize(line,key,values))
			property[key] = values;
	}

	vector<string>& iname = property["InterfaceName"];
	if(iname.empty())
		iname.push_back(interfaceName);
}

bool TraderOffer_impl::match(const vector<TraderRestriction>& restrictions)
{
	vector<TraderRestriction>::const_iterator i;

	for(i = restrictions.begin(); i != restrictions.end();i++)
	{
		const TraderRestriction& res = *i;
		const vector<string>& myvalues = property[res.key];
		bool okay = false;

		vector<string>::const_iterator offerIt = myvalues.begin();
		while(!okay && offerIt != myvalues.end())
		{
			if(res.value == *offerIt)
				okay = true;
			else
				offerIt++;
		}

		if(!okay) return false;
	}
	return true;
}

string TraderOffer_impl::interfaceName()
{
	return _interfaceName;
}

vector<string>* TraderOffer_impl::getProperty(const string& name)
{
	return new vector<string>(property[name]);
}

// TraderHelper
//----------------------------------------------------------------------------

TraderHelper::TraderHelper()
{
	load();
}

TraderHelper::~TraderHelper()
{
	unload();
}

void TraderHelper::load()
{
	const vector<string> *path = MCOPUtils::traderPath();

	vector<string>::const_iterator pi;
	for(pi = path->begin(); pi != path->end(); pi++) addDirectory(*pi);
}

void TraderHelper::unload()
{
	vector<TraderOffer_impl *>::iterator i;

	for(i = allOffers.begin(); i != allOffers.end(); i++)
	{
		TraderOffer_impl *offer = *i;
		offer->_release();
	}
	allOffers.clear();
}

void TraderHelper::addDirectory(const string& directory, const string& iface,
								map< pair<dev_t,ino_t>, bool > *dirsDone)
{
	arts_debug("addDirectory(%s,%s)", directory.c_str(), iface.c_str());

	if(!dirsDone)
	{
		/* map to prevent doing directories again due to symlinks */
		dirsDone = new map< pair<dev_t,ino_t>, bool >;

		struct stat st;
		stat(directory.c_str(), &st);
		(*dirsDone)[make_pair(st.st_dev, st.st_ino)] = true;
	}

	DIR *dir = opendir(directory.c_str());
	if(!dir) return;

	struct dirent *de;
	while((de = readdir(dir)) != 0)
	{
		string currentEntry = directory + "/" + de->d_name;

		string currentIface = iface;
		if(iface != "") currentIface += "::";
		currentIface += de->d_name;

		struct stat st;
		stat(currentEntry.c_str(),&st);

		// recurse into subdirectories
		if(S_ISDIR(st.st_mode))
		{
			bool& done = (*dirsDone)[make_pair(st.st_dev, st.st_ino)];
			if(strcmp(de->d_name,".") && strcmp(de->d_name,"..") && !done)
			{
				done = true;
				addDirectory(currentEntry,currentIface,dirsDone);
			}
		}
		else if(S_ISREG(st.st_mode))
		{
			if(strlen(de->d_name) > 10 &&
			   strncmp(&de->d_name[strlen(de->d_name)-10],".mcopclass",10) == 0)
			{
				// cut .mcopclass for currentIface;
      			currentIface = currentIface.substr(0, currentIface.size()-10);
				allOffers.push_back(
						new TraderOffer_impl(currentIface,currentEntry) );
			}
		}
    }
	closedir(dir);
}

vector<TraderOffer> *TraderHelper::doQuery(const
											vector<TraderRestriction>& query)
{
	vector<TraderOffer> *result = new vector<TraderOffer>;

	vector<TraderOffer_impl *>::iterator i;
	for(i = allOffers.begin(); i != allOffers.end(); i++)
	{
		TraderOffer_impl *offer = *i;

		if(offer->match(query))
		{
			result->push_back(TraderOffer::_from_base(offer->_copy()));
		}
	}
	return result;
}

TraderHelper *TraderHelper::_instance = 0;
TraderHelper *TraderHelper::the()
{
	if(!_instance) _instance = new TraderHelper();
	return _instance;
}

void TraderHelper::shutdown()
{
	if(_instance)
	{
		delete _instance;
		_instance = 0;
	}
}

namespace Arts {

// trader shutdown
class TraderShutdown :public StartupClass
{
public:
	void startup() { };
	void shutdown() { TraderHelper::shutdown(); };
};

static TraderShutdown traderShutdown;

};

// Dispatcher function for reloading the trader data:
void Dispatcher::reloadTraderData()
{
	TraderHelper::the()->unload();
	TraderHelper::the()->load();
}
