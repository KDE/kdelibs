/*
   Copyright (c) 2001 Nikolas Zimmermann <wildfox@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <kdebug.h>
#include <kuniqueapp.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <dcopclient.h>

#include <qvaluelist.h>
#include <qcstring.h>

#include <kartsdispatcher.h>
#include <dispatcher.h>
#include <object.h>
#include <core.h>

#include "mcopdcopobject.h"

#include "kmcop.moc"

using namespace Arts;
using namespace std;

class KMCOPPrivate
{
};

int main(int argc, char **argv)
{
    KAboutData aboutdata("kmcop", I18N_NOOP("KMCOP"),
                         "0.1", I18N_NOOP("KDE MCOP-DCOP Bridge"),
                         KAboutData::License_GPL, "(C) 2001, Nikolas Zimmermann");
    aboutdata.addAuthor("Nikolas Zimmermann", I18N_NOOP("Author"), "wildfox@kde.org");

    KCmdLineArgs::init(argc, argv, &aboutdata);
    KUniqueApplication::addCmdLineOptions();

    if(!KUniqueApplication::start())
	{
		kdDebug() << "Running kmcop found" << endl;
		return 0;
    }

    KUniqueApplication app;
    app.disableSessionManagement();

    KArtsDispatcher dispatcher;

    KMCOP notify;
    app.dcopClient()->setDefaultObject("arts");
    app.dcopClient()->setDaemonMode(true);
	
    return app.exec();
}

KMCOP::KMCOP() : QObject(), DCOPObject("arts")
{
    d = new KMCOPPrivate();
}

KMCOP::~KMCOP()
{
    delete d;
}

int KMCOP::objectCount()
{
	return Object_base::_objectCount();
}

void KMCOP::addInterfacesHackHackHack()
{
	list<Object_skel *> which = Dispatcher::the()->activeObjectPool().enumerate();
	list<Object_skel *>::iterator i;
	for(i = which.begin(); i != which.end(); i++)
	{
		QCString interfaceName = ((*i)->_interfaceName()).c_str();
		Arts::InterfaceRepo ifaceRepo = Dispatcher::the()->interfaceRepo();

		MCOPDCOPObject *interface = new MCOPDCOPObject(interfaceName);
		
		InterfaceDef ifaceDef = ifaceRepo.queryInterface(string(interfaceName));
		vector<MethodDef> ifaceMethods = ifaceDef.methods;
	
		vector<MethodDef>::iterator ifaceMethodsIterator;
		for(ifaceMethodsIterator = ifaceMethods.begin(); ifaceMethodsIterator != ifaceMethods.end(); ifaceMethodsIterator++)
		{
			QCString function;
			MethodDef currentMethod = *ifaceMethodsIterator;
			vector<ParamDef> currentParameters = currentMethod.signature;
			
			function = QCString(currentMethod.type.c_str()) + QCString(" ") + QCString(currentMethod.name.c_str()) + QCString("(");
			
			vector<ParamDef>::iterator methodParametersIterator;
			for(methodParametersIterator = currentParameters.begin(); methodParametersIterator != currentParameters.end(); methodParametersIterator++)
			{
				ParamDef parameter = *methodParametersIterator;
				if(methodParametersIterator != currentParameters.begin())
					function += QCString(", ");
				function += QCString(parameter.type.c_str());
			}

			function += QCString(")");
			
			interface->addDynamicFunction(function);
		}
	}
}
