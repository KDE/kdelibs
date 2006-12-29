//QT specific includes
#include <QList>

//solid specific includes
#include <solid/devicemanager.h>
#include <solid/device.h>
#include <solid/capability.h>

//kde specific includes
#include <kinstance.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

#include <iostream>

using namespace std;

int main(int args, char **argv)
{
    KInstance instance("tutorial2");
    
    Solid::DeviceManager &manager = Solid::DeviceManager::self();
    
    //get a list of all devices that are AudioHw
    foreach(Solid::Device device, manager.findDevicesFromQuery("",Solid::Capability::AudioHw) )
    {
        kDebug() << device.udi().toLatin1().constData() << endl;
    }
    return 0;
}

#include "tutorial2.moc"
