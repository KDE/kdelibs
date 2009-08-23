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
#include <QtCore/QStringList>
#include <kcomponentdata.h>
#include <kexportplugin.h>
#include <kglobal.h>

class KPluginFactoryPrivate;
namespace KParts { class Part; }

#define K_PLUGIN_FACTORY_DECLARATION_WITH_BASEFACTORY(name, baseFactory) \
class name : public baseFactory \
{ \
    public: \
        explicit name(const char * = 0, const char * = 0, QObject * = 0); \
        explicit name(const KAboutData &, QObject * = 0); \
        ~name(); \
        static KComponentData componentData(); \
    private: \
        void init(); \
};

#define K_PLUGIN_FACTORY_DEFINITION_WITH_BASEFACTORY(name, baseFactory, pluginRegistrations) \
K_GLOBAL_STATIC(KComponentData, name##factorycomponentdata) \
name::name(const char *componentName, const char *catalogName, QObject *parent) \
    : baseFactory(componentName, catalogName, parent) { init(); } \
name::name(const KAboutData &aboutData, QObject *parent) \
    : baseFactory(aboutData, parent) { init(); } \
void name::init() \
{ \
    if (name##factorycomponentdata->isValid()) \
        setComponentData(*name##factorycomponentdata); \
    else \
        *name##factorycomponentdata = KPluginFactory::componentData(); \
    pluginRegistrations \
} \
name::~name() {} \
KComponentData name::componentData() \
{ \
    return *name##factorycomponentdata; \
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
 * static KAboutData createAboutData()
 * {
 *     KAboutData aboutData("myplugin", "myplugin", ki18n("MyPlugin"), "0.1",
 *             ki18n("a description of the plugin"), KAboutData::License_LGPL,
 *             ki18n("Copyright (C) 2007 Your Name"));
 *     aboutData.addAuthor(ki18n("Your Name"), ...);
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
 * \see K_PLUGIN_FACTORY
 * \see K_PLUGIN_FACTORY_DEFINITION
 */
#define K_PLUGIN_FACTORY_DECLARATION(name) K_PLUGIN_FACTORY_DECLARATION_WITH_BASEFACTORY(name, KPluginFactory)

/**
 * \relates KPluginFactory
 * K_PLUGIN_FACTORY_DEFINITION defines the KPluginFactory subclass. This macro can <b>not</b> be used in a
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
 * \class KPluginFactory kpluginfactory.h <KPluginFactory>
 * 
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
 * If you want to write a custom KPluginFactory not using the standard macro(s) you can reimplement
 * the create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword)
 * function.
 *
 * Example:
 * \code
 * class SomeScriptLanguageFactory : public KPluginFactory
 * {
 *     Q_OBJECT
 * public:
 *     SomeScriptLanguageFactory()
 *         : KPluginFactory("SomeScriptLanguageComponent")
 *     {}
 *
 * protected:
 *     virtual QObject *create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword)
 *     {
 *         const QString identifier = QLatin1String(iface) + QLatin1Char('_') + keyword;
 *         // load scripting language module from the information in identifier
 *         // and return it:
 *         return object;
 *     }
 * };
 * \endcode
 *
 * If you want to load a library use KPluginLoader.
 * The application that wants to instantiate plugin classes can do the following:
 * \code
 * KPluginFactory *factory = KPluginLoader("libraryname").factory();
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
    /**
     * This constructor creates a factory for a plugin with the given \p componentName and
     * \p catalogName. Those values are used to initialize a KComponentData object for the plugin.
     * You can later access it with componentData(). If \p componentName is 0, an invalid KComponentData
     * object will be created.
     *
     * \param componentName the component name of the plugin
     * \param catalogName the translation catalog to use
     * \param parent a parent object
     */
    explicit KPluginFactory(const char *componentName = 0, const char *catalogName = 0, QObject *parent = 0);

    /**
     * This constructor creates a factory for a plugin with the given KAboutData object. This object is
     * used to initialize a KComponentData object for the plugin. You can later access it with
     * componentData().
     * KPluginFactory takes ownership of the \p aboutData object, so don't delete it yourself!
     *
     * \param aboutData the KAboutData for the plugin
     * \param parent a parent object
     */
    explicit KPluginFactory(const KAboutData &aboutData, QObject *parent = 0);
    /**
     * @deprecated
     */
    KDE_CONSTRUCTOR_DEPRECATED explicit KPluginFactory(const KAboutData *aboutData, QObject *parent = 0);

    /**
     * @deprecated
     */
    explicit KDE_CONSTRUCTOR_DEPRECATED KPluginFactory(QObject *parent);

    /**
     * This destroys the PluginFactory. It will remove the translation catalog for the plugin,
     * if it was initialized.
     */
    virtual ~KPluginFactory();

    /**
     * You can use this method to get the component data of the plugin. It is filled with the
     * information given to the constructor of KPluginFactory.
     * The K_PLUGIN_FACTORY macros provide a static version of this method, this can be used from
     * any place within the plugin.
     *
     * \returns The KComponentData for the plugin
     */
    KComponentData componentData() const;

    /**
     * Use this method to create an object. It will try to create an object which inherits
     * \p T. If it has multiple choices, you will get a fatal error (kFatal()), so be creaful
     * to request a unique interface or use keywords.
     *
     * \tparam T The interface for which an object should be created. The object will inherit \p T.
     * \param parent The parent of the object. If \p parent is a widget type, it will also passed
     *               to the parentWidget argument of the CreateInstanceFunction for the object.
     * \param args Additional arguments which will be passed to the object.
     * \returns A pointer to the created object is returned, or 0 if an error occurred.
     */
    template<typename T>
    T *create(QObject *parent = 0, const QVariantList &args = QVariantList());

    /**
     * Use this method to create an object. It will try to create an object which inherits
     * \p T and was registered with \p keyword.
     *
     * \tparam T The interface for which an object should be created. The object will inherit \p T.
     * \param keyword The keyword of the object.
     * \param parent The parent of the object. If \p parent is a widget type, it will also passed
     *               to the parentWidget argument of the CreateInstanceFunction for the object.
     * \param args Additional arguments which will be passed to the object.
     * \returns A pointer to the created object is returned, or 0 if an error occurred.
     */
    template<typename T>
    T *create(const QString &keyword, QObject *parent = 0, const QVariantList &args = QVariantList());

    /**
     * Use this method to create an object. It will try to create an object which inherits
     * \p T and was registered with \p keyword.
     * This overload has an additional \p parentWidget argument, which is used by some plugins (e.g. Parts).

     * \tparam T The interface for which an object should be created. The object will inherit \p T.
     * \param parentWidget An additional parent widget.
     * \param parent The parent of the object. If \p parent is a widget type, it will also passed
     *               to the parentWidget argument of the CreateInstanceFunction for the object.
     * \param keyword The keyword of the object.
     * \param args Additional arguments which will be passed to the object.
     * \returns A pointer to the created object is returned, or 0 if an error occurred.
     */
    template<typename T>
    T *create(QWidget *parentWidget, QObject *parent, const QString &keyword = QString(), const QVariantList &args = QVariantList());

    /**
     * @deprecated
     */
    template<typename T>
    KDE_DEPRECATED
    T *create(QObject *parent, const QStringList &args)
    {
        return create<T>(parent, stringListToVariantList(args));
    }

    /**
     * @deprecated
     */
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
     * You can register as many plugin classes as you want as long as either the plugin interface or
     * the \p keyword makes it unique. E.g. it is possible to register a KCModule and a
     * KParts::Part without having to specify keywords since their interfaces differ.
     *
     * \tparam T the name of the plugin class
     *
     * \param keyword An optional keyword as unique identifier for the plugin. This allows you to
     * put more than one plugin with the same interface into the same library using the same
     * factory. X-KDE-PluginKeyword is a convenient way to specify the keyword in a desktop file.
     *
     * \param instanceFunction A function pointer to a function that creates an instance of the
     * plugin. The default function that will be used depends on the type of interface. If the
     * interface inherits from
     * \li \c KParts::Part the function will call
     * \code
     * new T(QWidget *parentWidget, QObject *parent, const QVariantList &args)
     * \endcode
     * \li \c QWidget the function will call
     * \code
     * new T(QWidget *parent, const QVariantList &args)
     * \endcode
     * \li else the function will call
     * \code
     * new T(QObject *parent, const QVariantList &args)
     * \endcode
     */
    template<class T>
    void registerPlugin(const QString &keyword = QString(), CreateInstanceFunction instanceFunction
            = InheritanceChecker<T>().createInstanceFunction(reinterpret_cast<T *>(0)))
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

    /**
     * @deprecated
     */
    virtual KDE_DEPRECATED QObject *createObject(QObject *parent, const char *className, const QStringList &args);

    /**
     * @deprecated
     */
    virtual KDE_DEPRECATED KParts::Part *createPartObject(QWidget *parentWidget, QObject *parent, const char *classname, const QStringList &args);


    /**
     * This method sets the component data of the plugin. You can access the component data object
     * later with componentData().
     * Normally you don't have to call this, because the factory constructs a component data object
     * from the information given to the constructor.
     * The object is destroyed, when the module containing the plugin is unloaded. Normally this happens
     * only on application shutdown.
     *
     * \param componentData the new KComponentData object
     */

    void setComponentData(const KComponentData &componentData);

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

    /**
     * This is used to detect the arguments need for the constructor of plugin classes.
     * You can inherit it, if you want to add new classes and still keep support for the old ones.
     */
    template<class impl>
    struct InheritanceChecker
    {
        CreateInstanceFunction createInstanceFunction(KParts::Part *) { return &createPartInstance<impl>; }
        CreateInstanceFunction createInstanceFunction(QWidget *) { return &createInstance<impl, QWidget>; }
        CreateInstanceFunction createInstanceFunction(...) { return &createInstance<impl, QObject>; }
    };

private:
    void registerPlugin(const QString &keyword, const QMetaObject *metaObject, CreateInstanceFunction instanceFunction);
};

typedef KPluginFactory KLibFactory;

template<typename T>
inline T *KPluginFactory::create(QObject *parent, const QVariantList &args)
{
    QObject *o = create(T::staticMetaObject.className(), parent && parent->isWidgetType() ? reinterpret_cast<QWidget *>(parent): 0, parent, args, QString());

    T *t = qobject_cast<T *>(o);
    if (!t) {
        delete o;
    }
    return t;
}

template<typename T>
inline T *KPluginFactory::create(const QString &keyword, QObject *parent, const QVariantList &args)
{
    QObject *o = create(T::staticMetaObject.className(), parent && parent->isWidgetType() ? reinterpret_cast<QWidget *>(parent): 0, parent, args, keyword);

    T *t = qobject_cast<T *>(o);
    if (!t) {
        delete o;
    }
    return t;
}

template<typename T>
inline T *KPluginFactory::create(QWidget *parentWidget, QObject *parent, const QString &keyword, const QVariantList &args)
{
    QObject *o = create(T::staticMetaObject.className(), parentWidget, parent, args, keyword);

    T *t = qobject_cast<T *>(o);
    if (!t) {
        delete o;
    }
    return t;
}

#endif // KDECORE_KPLUGINFACTORY_H
