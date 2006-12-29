//QT specific includes
#include <QList>

//solid specific includes
#include <solid/devicemanager.h>
#include <solid/device.h>

//kde specific includes
#include <kinstance.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

#include <iostream>

using namespace std;

int main(int args, char **argv)
{
    KInstance instance("tutorial1");
    
    Solid::DeviceManager &manager = Solid::DeviceManager::self();
    
    foreach(Solid::Device device, manager.allDevices() )
    {
        kDebug() << device.udi().toLatin1().constData() << endl;
    }
    return 0;
}

#include "tutorial1.moc"
