/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>
    Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef KDECORE_KPLUGINFACTORY_H
#define KDECORE_KPLUGINFACTORY_H

#include "kdecore_export.h"

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include <kcomponentdata.h>

class KPluginFactoryPrivate;
namespace KParts { class Part; }

#define K_PLUGIN_FACTORY_DECLARATION_WITH_BASEFACTORY(name, baseFactory) \
class name : public baseFactory \
{ \
    public: \
        explicit name(const char * = 0, const char * = 0, QObject * = 0); \
        explicit name(const KAboutData *, QObject * = 0); \
        ~name(); \
        static KComponentData componentData(); \
    private: \
        static KPluginFactory *s_instance; \
};

#define K_PLUGIN_FACTORY_DEFINITION_WITH_BASEFACTORY(name, baseFactory, pluginRegistrations) \
KPluginFactory *name::s_instance = 0; \
name::name(const char *componentName, const char *catalogName, QObject *parent) \
    : baseFactory(componentName, catalogName, parent) \
{ \
    Q_ASSERT(s_instance == 0); \
    s_instance = this; \
    pluginRegistrations \
} \
name::name(const KAboutData *aboutData, QObject *parent) \
    : baseFactory(aboutData, parent) \
{ \
    pluginRegistrations \
} \
name::~name() \
{ \
    s_instance = 0; \
} \
KComponentData name::componentData() \
{ \
    Q_ASSERT(s_instance); \
    return static_cast<baseFactory *>(s_instance)->componentData(); \
}

#define K_PLUGIN_FACTORY_WITH_BASEFACTORY(name, baseFactory, pluginRegistrations) \
    K_PLUGIN_FACTORY_DECLARATION_WITH_BASEFACTORY(name, baseFactory) \
    K_PLUGIN_FACTORY_DEFINITION_WITH_BASEFACTORY(name, baseFactory, pluginRegistrations)

/**
 * \relates KPluginFactory
 * Defines a KPluginFactory subclass with two constructors and a static componentData function.
 *
 * The first constructor takes the arguments (const char *componentName, const char *catalogName,
 * QObject *parent).
 * The second constructor takes (const KAboutData *aboutData, QObject *parent).
 *
 * The static componentData method returns the same KComponentData object as the
 * KPluginFactory::componentData function returns. As you normally don't have a pointer to the
 * factory object in the plugin code the static componentData function is a convenient way to access
 * the KComponentData.
 *
 * \param name The name of the KPluginFactory derived class. This is the name you'll need for
 * K_EXPORT_PLUGIN
 *
 * \param pluginRegistrations This is code inserted into the constructors the class. You'll want to
 * call registerPlugin from there.
 *
 * Example:
 * \code
 * #include <KPluginFactory>
 * #include <KPluginLoader>
 * #include <plugininterface.h>
 *
 * class MyPlugin;
 *
 * K_PLUGIN_FACTORY(MyPluginFactory,
 *                  registerPlugin<MyPlugin>();
 *                 )
 * K_EXPORT_PLUGIN(MyPluginFactory("componentName", "catalogName"))
 *
 * // or:
 * static KAboutData *createAboutData()
 * {
 *     KAboutData *aboutData = new KAboutData("myplugin", "myplugin", ki18n("MyPlugin"), "0.1",
 *             ki18n("a description of the plugin"), KAboutData::License_LGPL,
 *             ki18n("Copyright (C) 2007 Your Name"));
 *     aboutData->addAuthor(ki18n("Your Name"), ...);
 *     return aboutData;
 * }
 * K_EXPORT_PLUGIN(MyPluginFactory(createAboutData()))
 *
 * class MyPlugin : public PluginInterface
 * {
 *     ...
 *     KComponentData kcd = MyPluginFactory::componentData();
 *     ...
 * };
 * \endcode
 *
 * \see K_PLUGIN_FACTORY_DECLARATION
 * \see K_PLUGIN_FACTORY_DEFINITION
 */
#define K_PLUGIN_FACTORY(name, pluginRegistrations) K_PLUGIN_FACTORY_WITH_BASEFACTORY(name, KPluginFactory, pluginRegistrations)

/**
 * \relates KPluginFactory
 * K_PLUGIN_FACTORY_DECLARATION declares the KPluginFactory subclass. This macro can be used in a
 * header file.
 *
 * \param name The name of the KPluginFactory derived class. This is the name you'll need for
 * K_EXPORT_PLUGIN
 *
 * \param pluginRegistrations This is code inserted into the constructors the class. You'll want to
 * call registerPlugin from there.
 *
 * \see K_PLUGIN_FACTORY
 * \see K_PLUGIN_FACTORY_DEFINITION
 */
#define K_PLUGIN_FACTORY_DECLARATION(name) K_PLUGIN_FACTORY_DECLARATION_WITH_BASEFACTORY(name, KPluginFactory)

/**
 * \relates KPluginFactory
 * K_PLUGIN_FACTORY_DECLARATION defines the KPluginFactory subclass. This macro can <b>not</b> be used in a
 * header file.
 *
 * \param name The name of the KPluginFactory derived class. This is the name you'll need for
 * K_EXPORT_PLUGIN
 *
 * \param pluginRegistrations This is code inserted into the constructors the class. You'll want to
 * call registerPlugin from there.
 *
 * \see K_PLUGIN_FACTORY
 * \see K_PLUGIN_FACTORY_DECLARATION
 */
#define K_PLUGIN_FACTORY_DEFINITION(name, pluginRegistrations) K_PLUGIN_FACTORY_DEFINITION_WITH_BASEFACTORY(name, KPluginFactory, pluginRegistrations)

/**
 * If you develop a library that is to be loaded dynamically at runtime, then
 * you should return a pointer to a KPluginFactory.
 *
 * For most cases it is enough to use the K_PLUGIN_FACTORY macro to create the factory.
 *
 * Example:
 * \code
 * #include <KPluginFactory>
 * #include <KPluginLoader>
 * #include <plugininterface.h>
 *
 * class MyPlugin;
 *
 * K_PLUGIN_FACTORY(MyPluginFactory,
 *                  registerPlugin<MyPlugin>();
 *                 )
 * K_EXPORT_PLUGIN(MyPluginFactory("componentName"))
 *
 * class MyPlugin : public PluginInterface
 * {
 *     ...
 *     KComponentData kcd = MyPluginFactory::componentData();
 *     ...
 * };
 * \endcode
 *
 * K_PLUGIN_FACTORY is a convenient macro that expands to a class derived from KPluginFactory
 * providing two constructors and a static componentData() function. The second argument to
 * K_PLUGIN_FACTORY is code that is called from the constructors. There you can use registerPlugin
 * to register as many plugins for the factory as you want to.
 *
 * If you want to load a library use KPluginLoader.
 * The application that wants to instantiate plugin classes later on can do the following:
 * \code
 * KPluginFactory *factory = KPluginLoader("libname").factory();
 * if (factory) {
 *     PluginInterface *p1 = factory->create<PluginInterface>(parent);
 *     OtherInterface *p2  = factory->create<OtherInterface>(parent);
 *     NextInterface *p3   = factory->create<NextInterface>("keyword1", parent);
 *     NextInterface *p3   = factory->create<NextInterface>("keyword2", parent);
 * }
 * \endcode
 *
 * \author Matthias Kretz <kretz@kde.org>
 * \author Bernhard Loos <nhuh.put@web.de>
 */
class KDECORE_EXPORT KPluginFactory : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KPluginFactory)
public:
    explicit KPluginFactory(const char *componentName = 0, const char *catalogName = 0, QObject *parent = 0);
    explicit KPluginFactory(const KAboutData *aboutData, QObject *parent = 0);
    explicit KDE_CONSTRUCTOR_DEPRECATED KPluginFactory(QObject *parent);
    virtual ~KPluginFactory();

    KComponentData componentData() const;

    template<typename T>
    T *create(QObject *parent = 0, const QVariantList &args = QVariantList());

    template<typename T>
    T *create(const QString &keyword, QObject *parent = 0, const QVariantList &args = QVariantList());

    template<typename T>
    KDE_DEPRECATED
    T *create(QObject *parent, const QStringList &args)
    {
        return create<T>(parent, stringListToVariantList(args));
    }

    KDE_DEPRECATED QObject *create(QObject *parent = 0, const char *classname = "QObject", const QStringList &args = QStringList())
    {
        return create(classname, 0, parent, stringListToVariantList(args), QString());
    }

Q_SIGNALS:
    void objectCreated(QObject *object);

protected:
    /**
     * Function pointer type to a function that instantiates a plugin.
     */
    typedef QObject *(*CreateInstanceFunction)(QWidget *, QObject *, const QVariantList &);

    explicit KPluginFactory(KPluginFactoryPrivate &dd, QObject *parent = 0);

    /**
     * Registers a plugin with the factory. Call this function from the constructor of the
     * KPluginFactory subclass to make the create function able to instantiate the plugin when asked
     * for an interface the plugin implements.
     *
     * \param T The name of the plugin class
     * \param keyword An optional keyword as unique identifier for the plugin. This allows you to
     * put more than one plugin with the same interface into the same library using the same
     * factory.
     * \param instanceFunction A function pointer to a function that creates an instance of the
     * plugin.
     */
    template<class T>
    void registerPlugin(const QString &keyword = QString(), CreateInstanceFunction instanceFunction
            = InheritanceChecker<T>::createInstanceFunction(reinterpret_cast<T *>(0)))
    {
        registerPlugin(keyword, &T::staticMetaObject, instanceFunction);
    }

    /**
     * \internal
     * Converts a QStringList to a QVariantList
     */
    QVariantList stringListToVariantList(const QStringList &list);

    /**
     * \internal
     * Converts a QVariantList of strings to a QStringList
     */
    QStringList variantListToStringList(const QVariantList &list);

    virtual void setupTranslations();

    KPluginFactoryPrivate *const d_ptr;

    virtual KDE_DEPRECATED QObject *createObject(QObject *parent, const char *className, const QStringList &args);

    void setComponentData(const KComponentData &);

    /**
     * This function is called when the factory asked to create an Object.
     *
     * You may reimplement it to provide a very flexible factory. This is especially useful to
     * provide generic factories for plugins implemeted using a scripting language.
     *
     * \param iface The staticMetaObject::className() string identifying the plugin interface that
     * was requested. E.g. for KCModule plugins this string will be "KCModule".
     * \param parentWidget Only used if the requested plugin is a KPart.
     * \param parent The parent object for the plugin object.
     * \param args A plugin specific list of arbitrary arguments.
     * \param keyword A string that uniquely identifies the plugin. If a KService is used this
     * keyword is read from the X-KDE-PluginKeyword entry in the .desktop file.
     */
    virtual QObject *create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword);

    template<class impl, class ParentType>
    static QObject *createInstance(QWidget *parentWidget, QObject *parent, const QVariantList &args)
    {
        Q_UNUSED(parentWidget);
        ParentType *p = 0;
        if (parent) {
            p = qobject_cast<ParentType *>(parent);
            Q_ASSERT(p);
        }
        return new impl(p, args);
    }

    template<class impl>
    static QObject *createPartInstance(QWidget *parentWidget, QObject *parent, const QVariantList &args)
    {
        return new impl(parentWidget, parent, args);
    }

    template<class impl>
    struct InheritanceChecker
    {
        static CreateInstanceFunction createInstanceFunction(KParts::Part *) { return &createPartInstance<impl>; }
        static CreateInstanceFunction createInstanceFunction(QWidget *) { return &createInstance<impl, QWidget>; }
        static CreateInstanceFunction createInstanceFunction(...) { return &createInstance<impl, QObject>; }
    };

private:
    void registerPlugin(const QString &keyword, const QMetaObject *metaObject, CreateInstanceFunction instanceFunction);
};

KDE_DEPRECATED typedef KPluginFactory KLibFactory;

template<typename T>
T *KPluginFactory::create(QObject *parent, const QVariantList &args)
{
    QObject *o = create(T::staticMetaObject.className(), parent && parent->isWidgetType() ? reinterpret_cast<QWidget *>(parent): 0, parent, args, QString());

    T *t = qobject_cast<T *>(o);
    if (!t) {
        delete o;
    }
    return t;
}

template<typename T>
T *KPluginFactory::create(const QString &keyword, QObject *parent, const QVariantList &args)
{
    QObject *o = create(T::staticMetaObject.className(), parent && parent->isWidgetType() ? reinterpret_cast<QWidget *>(parent): 0, parent, args, keyword);

    T *t = qobject_cast<T *>(o);
    if (!t) {
        delete o;
    }
    return t;
}


#endif // KDECORE_KPLUGINFACTORY_H
