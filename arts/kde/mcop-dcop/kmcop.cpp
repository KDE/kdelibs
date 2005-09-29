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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <kdebug.h>
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <dcopclient.h>

#include <qvaluelist.h>
#include <qcstring.h>

#include <kartsdispatcher.h>
#include <soundserver.h>
#include <dispatcher.h>
#include <object.h>
#include <core.h>

#include "mcopdcopobject.h"

#include "kmcop.moc"

using namespace Arts;
using namespace std;

class KMCOPPrivate
{
public:
	MCOPInfo mcopInfo; 
	QPtrList<MCOPDCOPObject> list;
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
	d->mcopInfo = Reference("global:Arts_MCOPInfo");
	d->list.setAutoDelete(true);
}

KMCOP::~KMCOP()
{
    delete d;
}

int KMCOP::objectCount()
{
	return d->mcopInfo.objectCount();
}

QCString KMCOP::correctType(const QCString &str)
{
	if(str == "string")
		return "QCString";
	return str;
}

void KMCOP::addInterfacesHackHackHack()
{
	for(int i = 0; i <= objectCount(); i++)
	{
		Arts::Object obj = d->mcopInfo.objectForNumber(i);
		
		if(!obj.isNull())		
		{
			QCString interfaceName = obj._interfaceName().c_str();

			if(interfaceName != "Arts::TraderOffer")
			{
				Arts::InterfaceRepo ifaceRepo = Dispatcher::the()->interfaceRepo();

				MCOPDCOPObject *interface = new MCOPDCOPObject(interfaceName);
				d->list.append(interface);
			
				InterfaceDef ifaceDef = ifaceRepo.queryInterface(string(interfaceName));
				vector<MethodDef> ifaceMethods = ifaceDef.methods;

				vector<MethodDef>::iterator ifaceMethodsIterator;
				for(ifaceMethodsIterator = ifaceMethods.begin(); ifaceMethodsIterator != ifaceMethods.end(); ifaceMethodsIterator++)
				{
					QCString function, signature;

					MCOPEntryInfo *entry = new MCOPEntryInfo();
					
					MethodDef currentMethod = *ifaceMethodsIterator;
					vector<ParamDef> currentParameters = currentMethod.signature;

					QCString newType = correctType(QCString(currentMethod.type.c_str()));
					
					entry->setFunctionType(newType);
					entry->setFunctionName(QCString(currentMethod.name.c_str()));
					
					function = entry->functionType() + QCString(" ") + entry->functionName() + QCString("(");
					
					signature = QCString("(");

					QCStringList signatureList;
					
					vector<ParamDef>::iterator methodParametersIterator;
					for(methodParametersIterator = currentParameters.begin(); methodParametersIterator != currentParameters.end(); methodParametersIterator++)
					{
						ParamDef parameter = *methodParametersIterator;
						if(methodParametersIterator != currentParameters.begin())
						{
							function += QCString(", ");						
							signature += QCString(",");
						}
						
						QCString correctParameter = correctType(QCString(parameter.type.c_str()));
						
						function += correctParameter;
						signature += correctParameter;

						signatureList.append(QCString(parameter.type.c_str()));
					}
					
					function += QCString(")");
					signature += QCString(")");

					entry->setSignature(signature);
					entry->setSignatureList(signatureList);

					interface->addDynamicFunction(function, entry);
				}
			}
		}
	}
}
