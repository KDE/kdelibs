#include <dummyplugin.h>
#include <kpluginfactory.h>

DummyPlugin::DummyPlugin(QObject*, const QVariantList&)
{}

#define IWANTMYJSON(x)

IWANTMYJSON("dummy-service.json")

K_PLUGIN_FACTORY_WITH_JSON(DummyPluginFactory, "dummy-service.json", registerPlugin<DummyPlugin>();)

#include <dummyplugin.moc>
