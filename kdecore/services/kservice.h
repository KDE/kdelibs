/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright     1999-2006  David Faure <faure@kde.org>

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

#ifndef KSERVICE_H
#define KSERVICE_H

#include "kserviceaction.h"
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#ifndef KDE_NO_DEPRECATED
#include <klibloader.h>
#endif
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <ksycocaentry.h>
#include <klocale.h>

class KServiceType;
class QDataStream;
class KDesktopFile;
class QWidget;

class KServicePrivate;

/**
 * Represent a service, like an application or plugin
 * bound to one or several mimetypes (or servicetypes) as written
 * in its desktop entry file.
 *
 * The starting point you need is often the static methods, like createInstance().
 * The types of service a plugin provides is taken from the accompanying desktop file
 * where the 'ServiceTypes=' field is used.
 *
 * For a tutorial on how to build a plugin-loading mechanism and how to write plugins
 * in general, see http://techbase.kde.org/Development/Tutorials#Services:_Applications_and_Plugins
 *
 * @see KServiceType
 * @see KServiceGroup
 * @author Torben Weis
 */
class KDECORE_EXPORT KService : public KSycocaEntry // TODO KDE5: inherit kshared, but move KSycocaEntry to Private
{
public:
    typedef KSharedPtr<KService> Ptr;
    typedef QList<Ptr> List;

    /**
     * Construct a temporary service with a given name, exec-line and icon.
     * @param name the name of the service
     * @param exec the executable
     * @param icon the name of the icon
     */
    KService(const QString & name, const QString &exec, const QString &icon);

    /**
     * Construct a service and take all information from a config file.
     *
     * @param fullpath Full path to the config file.
     */
    explicit KService( const QString & fullpath );

    /**
     * Construct a service and take all information from a desktop file.
     * @param config the desktop file to read
     */
    explicit KService( const KDesktopFile *config );

    /**
     * @internal
     * Construct a service from a stream.
     * The stream must already be positionned at the correct offset.
     */
    KService( QDataStream& str, int offset );

    virtual ~KService();

    /**
     * Services are either applications (executables) or dlopened libraries (plugins).
     * @return true if this service is an application, i.e. it has Type=Application in its
     * .desktop file and exec() will not be empty.
     */
    bool isApplication() const;

    /**
     * Returns the type of the service.
     * @return the type of the service ("Application" or "Service")
     * @deprecated use isApplication()
     */
#ifndef KDE_NO_DEPRECATED
    KDE_DEPRECATED QString type() const;
#endif

    /**
     * Returns the executable.
     * @return the command that the service executes,
     *         or QString() if not set
     */
    QString exec() const;
    /**
     * Returns the name of the service's library.
     * @return the name of the library that contains the service's
     *         implementation, or QString() if not set
     */
    QString library() const;

    /**
     * Returns the name of the icon.
     * @return the icon associated with the service,
     *         or QString() if not set
     */
    QString icon() const;
    /**
     * Checks whethe the service should be run in a terminal.
     * @return true if the service is to be run in a terminal.
     */
    bool terminal() const;

    /**
     * Returns any options associated with the terminal the service
     * runs in, if it requires a terminal.
     *
     * The service must be a tty-oriented program.
     * @return the terminal options,
     *         or QString() if not set
     */
    QString terminalOptions() const;
    /**
     * Checks whether the service runs with a different user id.
     * @return true if the service has to be run under a different uid.
     * @see username()
     */
    bool substituteUid() const;
    /**
     * Returns the user name, if the service runs with a
     * different user id.
     * @return the username under which the service has to be run,
     *         or QString() if not set
     * @see substituteUid()
     */
    QString username() const;

    /**
     * Returns the path to the location where the service desktop entry
     * is stored.
     *
     * This is a relative path if the desktop entry was found in any
     * of the locations pointed to by $KDEDIRS (e.g. "Internet/kppp.desktop")
     * It is a full path if the desktop entry originates from another
     * location.
     *
     * @deprecated use entryPath() instead
     *
     * @return the path of the service's desktop file,
     *         or QString() if not set
     */
#ifndef KDE_NO_DEPRECATED
    KDE_DEPRECATED QString desktopEntryPath() const;
#endif

    /**
     * Returns the filename of the service desktop entry without any
     * extension. E.g. "kppp"
     * @return the name of the desktop entry without path or extension,
     *         or QString() if not set
     */
    QString desktopEntryName() const;

    /**
     * Returns the menu ID of the service desktop entry.
     * The menu ID is used to add or remove the entry to a menu.
     * @return the menu ID
     */
    QString menuId() const;

    /**
     * Returns a normalized ID suitable for storing in configuration files.
     * It will be based on the menu-id when available and otherwise falls
     * back to entryPath()
     * @return the storage ID
     */
    QString storageId() const;

    /**
     * Describes the DBUS Startup type of the service.
     * @li None - This service has no DBUS support
     * @li Unique - This service provides a unique DBUS service.
     *              The service name is equal to the desktopEntryName.
     * @li Multi - This service provides a DBUS service which can be run
     *             with multiple instances in parallel. The service name of
     *             an instance is equal to the desktopEntryName + "-" +
     *             the PID of the process.
     * @li Wait - This service has no DBUS support, the launcher will wait
     *            till it is finished.
     */
    enum DBusStartupType { DBusNone = 0, DBusUnique, DBusMulti, DBusWait };

    /**
     * Returns the DBUSStartupType supported by this service.
     * @return the DBUSStartupType supported by this service
     */
    DBusStartupType dbusStartupType() const;

    /**
     * Returns the working directory to run the program in.
     * @return the working directory to run the program in,
     *         or QString() if not set
     */
    QString path() const;

    /**
     * Returns the descriptive comment for the service, if there is one.
     * @return the descriptive comment for the service, or QString()
     *         if not set
     */
    QString comment() const;

    /**
     * Returns the generic name for the service, if there is one
     * (e.g. "Mail Client").
     * @return the generic name,
     *         or QString() if not set
     */
    QString genericName() const;

    /**
     * Returns the untranslated (US English) generic name
     * for the service, if there is one
     * (e.g. "Mail Client").
     * @return the generic name,
     *         or QString() if not set
     */
    QString untranslatedGenericName() const;

    /**
     * Returns a list of descriptive keywords the service, if there are any.
     * @return the list of keywords
     */
    QStringList keywords() const;

    /**
     * Returns a list of VFolder categories.
     * @return the list of VFolder categories
     */
    QStringList categories() const;

    /**
     * Returns the service types that this service supports.
     * @return the list of service types that are supported
     * Note that this doesn't include inherited servicetypes or mimetypes,
     * only the service types listed in the .desktop file.
     */
    QStringList serviceTypes() const;

    /**
     * Checks whether the service supports this service type
     * @param serviceTypePtr The name of the service type you are
     *        interested in determining whether this service supports.
     *
     * @return true if the service type you specified is supported, otherwise false.
     */
    bool hasServiceType( const QString& serviceTypePtr ) const;

    /**
     * Checks whether the service supports this mime type
     * @param mimeTypePtr The name of the mime type you are
     *        interested in determining whether this service supports.
     *
     * Note that if you only have the name of the mime type, you have to look it up
     * with KMimeType::mimeType( mimetype ) and use .data() on the result (this is
     * because KService doesn't know KMimeType for dependency reasons)
     *
     * Warning this method will fail to return true if this KService isn't from ksycoca
     * (i.e. it was created with a full path or a KDesktopFile) *and* the mimetype
     * isn't explicited listed in the .desktop file but a parent mimetype is.
     * For this reason you should generally get KServices with KMimeTypeTrader
     * or one of the KService::serviceBy methods.
     *
     * @return true if the mime type you specified is supported, otherwise false.
     * @deprecated, use hasMimeType(QString)
     */
#ifndef KDE_NO_DEPRECATED
    KDE_DEPRECATED bool hasMimeType( const KServiceType* mimeTypePtr ) const;
#endif

    /**
     * Checks whether the service supports this mime type
     * @param mimeTypePtr The name of the mime type you are
     *        interested in determining whether this service supports.
     * @since 4.6
     */
    bool hasMimeType( const QString& mimeType ) const;

    /**
     * Set to true if it is allowed to use this service as the default (main)
     * action for the files it supports (e.g. Left Click in a file manager, or KRun in general).
     *
     * If not, then this service is only available in RMB popups, so it must
     * be selected explicitly by the user in order to be used.
     * Note that servicemenus supersede this functionality though, at least in konqueror.
     *
     * @return true if the service may be used as the default (main) handler
     */
    bool allowAsDefault() const;

    /**
     * Returns the actions defined in this desktop file
     */
    QList<KServiceAction> actions() const;

    /**
     * Checks whether this service can handle several files as
     * startup arguments.
     * @return true if multiple files may be passed to this service at
     * startup. False if only one file at a time may be passed.
     */
    bool allowMultipleFiles() const;

    /**
     * What preference to associate with this service initially (before
     * the user has had any chance to define a profile for it).
     * The bigger the value, the most preferred the service is.
     * @return the service preference level of the service
     */
    int initialPreference() const;

    /**
     * Whether the entry should be suppressed in the K menu.
     * @return true to suppress this service
     *
     * Such services still appear in trader queries, i.e. in
     * "Open With" popup menus for instance.
     */
    bool noDisplay() const;

    /**
     * Whether the service should be shown in KDE at all
     * (including in context menus).
     * @return true if the service should be shown.
     *
     * KMimeTypeTrader honors this and removes such services
     * from its results.
     *
     * @since 4.5
     */
    bool showInKDE() const;

    /**
     * Name of the application this service belongs to.
     * (Useful for e.g. plugins)
     * @return the parent application, or QString() if not set
     */
    QString parentApp() const;

    /**
     * The keyword to be used when constructing the plugin using KPluginFactory. The keyword is
     * defined with X-KDE-PluginKeyword in the .desktop file and with K_REGISTER_PLUGIN_WITH_KEYWORD
     * when implementing the plugin.
     */
    QString pluginKeyword() const;

    /**
     * The path to the documentation for this service.
     * @since 4.2
     * @return the documentation path, or QString() if not set
     */
    QString docPath() const;

    /**
     * Returns the requested property.
     *
     * @param _name the name of the property
     * @param t the assumed type of the property
     * @return the property, or invalid if not found
     * @see KServiceType
     */
    QVariant property( const QString& _name, QVariant::Type t ) const;

    using KSycocaEntry::property;

    /**
     * Returns a path that can be used for saving changes to this
     * service
     * @return path that can be used for saving changes to this service
     */
    QString locateLocal() const;

    /**
     * @internal
     * Set the menu id
     */
    void setMenuId(const QString &menuId);
    /**
     * @internal
     * Sets whether to use a terminal or not
     */
    void setTerminal(bool b);
    /**
     * @internal
     * Sets the terminal options to use
     */
    void setTerminalOptions(const QString &options);

    /**
     * Find a service by name, i.e. the translated Name field. Don't use this.
     * Use serviceByDesktopPath or serviceByStorageId instead.
     *
     * @param _name the name to search
     * @return a pointer to the requested service or 0 if the service is
     *         unknown.
     * @em Very @em important: Don't store the result in a KService* !
     * @deprecated there is never a good reason to use this method.
     */
#ifndef KDE_NO_DEPRECATED
    static Ptr serviceByName( const QString& _name ); // KDE5: remove
#endif

    /**
     * Find a service based on its path as returned by entryPath().
     * It's usually better to use serviceByStorageId() instead.
     *
     * @param _path the path of the configuration file
     * @return a pointer to the requested service or 0 if the service is
     *         unknown.
     * @em Very @em important: Don't store the result in a KService* !
     */
    static Ptr serviceByDesktopPath( const QString& _path );

    /**
     * Find a service by the name of its desktop file, not depending on
     * its actual location (as long as it's under the applnk or service
     * directories). For instance "konqbrowser" or "kcookiejar". Note that
     * the ".desktop" extension is implicit.
     *
     * This is the recommended method (safe even if the user moves stuff)
     * but note that it assumes that no two entries have the same filename.
     *
     * @param _name the name of the configuration file
     * @return a pointer to the requested service or 0 if the service is
     *         unknown.
     * @em Very @em important: Don't store the result in a KService* !
     */
    static Ptr serviceByDesktopName( const QString& _name );

    /**
     * Find a service by its menu-id
     *
     * @param _menuId the menu id of the service
     * @return a pointer to the requested service or 0 if the service is
     *         unknown.
     * @em Very @em important: Don't store the result in a KService* !
     */
    static Ptr serviceByMenuId( const QString& _menuId );

    /**
     * Find a service by its storage-id or desktop-file path. This
     * function will try very hard to find a matching service.
     *
     * @param _storageId the storage id or desktop-file path of the service
     * @return a pointer to the requested service or 0 if the service is
     *         unknown.
     * @em Very @em important: Don't store the result in a KService* !
     */
    static Ptr serviceByStorageId( const QString& _storageId );

    /**
     * Returns the whole list of services.
     *
     *  Useful for being able to
     * to display them in a list box, for example.
     * More memory consuming than the ones above, don't use unless
     * really necessary.
     * @return the list of all services
     */
    static List allServices();

    /**
     * Returns a path that can be used to create a new KService based
     * on @p suggestedName.
     * @param showInMenu true, if the service should be shown in the KDE menu
     *        false, if the service should be hidden from the menu
     *        This argument isn't used anymore, use NoDisplay=true to hide the service.
     * @param suggestedName name to base the file on, if a service with such
     *        name already exists, a prefix will be added to make it unique.
     * @param menuId If provided, menuId will be set to the menu id to use for
     *        the KService
     * @param reservedMenuIds If provided, the path and menu id will be chosen
     *        in such a way that the new menu id does not conflict with any
     *        of the reservedMenuIds
     * @return The path to use for the new KService.
     */
    static QString newServicePath(bool showInMenu, const QString &suggestedName,
                                  QString *menuId = 0,
                                  const QStringList *reservedMenuIds = 0);

    /**
     * This template allows to load the library for the specified service and ask the
     * factory to create an instance of the given template type.
     *
     * @param parent The parent object (see QObject constructor)
     * @param args A list of arguments, passed to the factory and possibly
     *             to the component (see KPluginFactory)
     * @param error A pointer to QString which should receive the error description or 0
     *
     * @return A pointer to the newly created object or a null pointer if the
     *         factory was unable to create an object of the given type.
     */
    template <class T>
    T *createInstance(QObject *parent = 0,
            const QVariantList &args = QVariantList(), QString *error = 0) const
    {
        return createInstance<T>(0, parent, args, error);
    }

    /**
     * This template allows to load the library for the specified service and ask the
     * factory to create an instance of the given template type.
     *
     * @param parentWidget A parent widget for the service. This is used e. g. for parts
     * @param parent The parent object (see QObject constructor)
     * @param args A list of arguments, passed to the factory and possibly
     *             to the component (see KPluginFactory)
     * @param error A pointer to QString which should receive the error description or 0
     *
     * @return A pointer to the newly created object or a null pointer if the
     *         factory was unable to create an object of the given type.
     */
    template <class T>
    T *createInstance(QWidget *parentWidget, QObject *parent,
            const QVariantList &args = QVariantList(), QString *error = 0) const
    {
        KPluginLoader pluginLoader(*this);
        KPluginFactory *factory = pluginLoader.factory();
        if (factory) {
            T *o = factory->template create<T>(parentWidget, parent, pluginKeyword(), args);
            if (!o && error)
                *error = i18n("The service '%1' does not provide an interface '%2' with keyword '%3'",
                              name(), QString::fromLatin1(T::staticMetaObject.className()), pluginKeyword());
            return o;
        }
        else if (error) {
            *error = pluginLoader.errorString();
            pluginLoader.unload();
        }
        return 0;
    }

    /**
     * @deprecated Use the non-static service->createInstance<T>(parent, args, &error)
     */
#ifndef KDE_NO_DEPRECATED
    template <class T>
    static KDE_DEPRECATED T *createInstance(const KService::Ptr &service, QObject *parent = 0,
            const QVariantList &args = QVariantList(), QString *error = 0)
    {
        return service->createInstance<T>(parent, args, error);
    }
#endif

    /**
     * @deprecated Use the non-static service->createInstance<T>(parent, args, &error)
     * where args is a QVariantList rather than a QStringList
     */
#ifndef KDE_NO_DEPRECATED
    template <class T>
    static KDE_DEPRECATED T *createInstance( const KService::Ptr &service,
                              QObject *parent,
                              const QStringList &args,
                              int *error = 0 )
    {
        const QString library = service->library();
        if ( library.isEmpty() ) {
            if ( error )
                *error = KLibLoader::ErrServiceProvidesNoLibrary;
            return 0;
        }

        return KLibLoader::createInstance<T>( library, parent, args, error );
    }
#endif

    /**
     * This template allows to create a component from a list of services,
     * usually coming from a trader query. You probably want to use KServiceTypeTrader instead.
     *
     * @deprecated Use KServiceTypeTrader::createInstanceFromQuery instead
     *
     * @param begin The start iterator to the service describing the library to open
     * @param end The end iterator to the service describing the library to open
     * @param parent The parent object (see QObject constructor)
     * @param args A list of string arguments, passed to the factory and possibly
     *             to the component (see KLibFactory)
     * @param error see KLibLoader
     * @return A pointer to the newly created object or a null pointer if the
     *         factory was unable to create an object of the given type.
     */
#ifndef KDE_NO_DEPRECATED
    template <class T, class ServiceIterator>
    static KDE_DEPRECATED T *createInstance(ServiceIterator begin, ServiceIterator end, QObject *parent = 0,
            const QVariantList &args = QVariantList(), QString *error = 0)
    {
        for (; begin != end; ++begin) {
            KService::Ptr service = *begin;
            if (error) {
                error->clear();
            }

            T *component = createInstance<T>(service, parent, args, error);
            if (component) {
                return component;
            }
        }
        if (error) {
            *error = KLibLoader::errorString(KLibLoader::ErrNoServiceFound);
        }
        return 0;
    }
#endif

#ifndef KDE_NO_DEPRECATED
    template <class T, class ServiceIterator>
    static KDE_DEPRECATED T *createInstance( ServiceIterator begin, ServiceIterator end,
                              QObject *parent,
                              const QStringList &args,
                              int *error = 0 )
    {
        for (; begin != end; ++begin ) {
            KService::Ptr service = *begin;
            if ( error )
                *error = 0;

            T *component = createInstance<T>( service, parent, args, error );
            if ( component )
                return component;
        }
        if ( error )
            *error = KLibLoader::ErrNoServiceFound;
        return 0;
    }
#endif

protected:
    friend class KMimeAssociations;
    friend class KBuildServiceFactory;

    /// @internal for KBuildSycoca only
    struct ServiceTypeAndPreference
    {
        ServiceTypeAndPreference()
            : preference(-1), serviceType() {}
        ServiceTypeAndPreference(int pref, const QString& servType)
            : preference(pref), serviceType(servType) {}
        int preference;
        QString serviceType; // or mimetype
    };
    /// @internal for KBuildSycoca only
    QVector<ServiceTypeAndPreference>& _k_accessServiceTypes();

    friend QDataStream& operator>>( QDataStream&, ServiceTypeAndPreference& );
    friend QDataStream& operator<<( QDataStream&, const ServiceTypeAndPreference& );
private:
    Q_DECLARE_PRIVATE(KService)
};
#endif
