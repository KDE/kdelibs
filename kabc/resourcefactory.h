#ifndef RESOURCEFACTORY_H
#define RESOURCEFACTORY_H

#include <qdict.h>

#include <kconfig.h>
#include <klibloader.h>

#include "resource.h"
#include "resourceconfigwidget.h"

namespace KABC {

struct ResourceInfo {
    QString library;
    QString name;
    QString desc;
};

class ResourceFactory
{
public:
    
    /**
     * Get the global resource factory.
     */
    static ResourceFactory *self();


    /**
     * Return the config widget for the
     * given resource name, or a null pointer
     * if resource doesn't exist.
     */
    ResourceConfigWidget *configWidget( const QString& resName, QWidget *parent = 0 );

    /**
     * Return a pointer to a resource object
     */
    Resource *resource( const QString& resName, AddressBook *ab, const KConfig *config );

    /**
     * Return a list of all available resources.
     */
    QStringList resources();

    /**
     * Return info to a resource.
     */
    ResourceInfo *info( const QString& resName );

protected:
    ResourceFactory();
    ~ResourceFactory();

private:
    KLibrary *openLibrary( const QString& libName );

    static ResourceFactory *mSelf;
    QDict<ResourceInfo> mResourceList;
};

}
#endif
