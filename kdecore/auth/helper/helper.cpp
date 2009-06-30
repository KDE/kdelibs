#include <cstdio>

#include <QCoreApplication>

#include "HelperProxy.h"
#include "BackendsManager.h"

#include "helper.h"

void Class::action()
{
    FILE *file = fopen("/home/nicola/file.txt", "rw");
    
    fprintf(file, "Ciao");
    
    fclose(file);
}

int main(int argc, char **argv)
{
    Class object;
    
    if(!BackendsManager::helperProxy()->initHelper("org.kde.auth"))
    {
        printf("initHelper failed\n");
    }
    BackendsManager::helperProxy()->setHelperResponder(&object);
    
    QCoreApplication app(argc, argv);
    app.exec();
    
    return 0;
}