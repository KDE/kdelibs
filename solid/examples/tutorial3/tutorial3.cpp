//QT specific includes
#include <QList>

//solid specific includes
#include <solid/devicemanager.h>
#include <solid/device.h>
#include <solid/capability.h>
#include <solid/processor.h>

//kde specific includes
#include <kcomponentdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

#include <iostream>

using namespace std;

int main(int args, char **argv)
{
    KComponentData componentData("tutorial3");
    
    Solid::DeviceManager &manager = Solid::DeviceManager::self();
    
    //get a Processor
    Solid::DeviceList list = manager.findDevicesFromQuery("",Solid::Capability::Processor);

    //take the first processor
    Solid::Device device = list[0];
    if(device.is<Solid::Processor>() ) kDebug() << "We've got a processor!" << endl;
    else kDebug() << "Device is not a processor." << endl;

    Solid::Processor *processor = device.as<Solid::Processor>();
    kDebug() << "This processors maximum speed is: " << processor->maxSpeed() << endl;
    
    return 0;
}

#include "tutorial3.moc"
