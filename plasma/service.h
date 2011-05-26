/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_SERVICE_H
#define PLASMA_SERVICE_H

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QVariant>

#include <kconfiggroup.h>

#include <plasma/plasma_export.h>
#include <plasma/plasma.h>
#include "packagemetadata.h"

class QGraphicsWidget;
class QIODevice;
class QWidget;

namespace Plasma
{

class ServiceJob;
class ServicePrivate;

/**
 * @class Service plasma/service.h <Plasma/Service>
 *
 * @short This class provides a generic API for write access to settings or services.
 *
 * Plasma::Service allows interaction with a "destination", the definition of which
 * depends on the Service itself. For a network settings Service this might be a
 * profile name ("Home", "Office", "Road Warrior") while a web based Service this
 * might be a username ("aseigo", "stranger65").
 *
 * A Service provides one or more operations, each of which provides some sort
 * of interaction with the destination. Operations are described using config
 * XML which is used to create a KConfig object with one group per operation.
 * The group names are used as the operation names, and the defined items in
 * the group are the parameters available to be set when using that operation.
 *
 * A service is started with a KConfigGroup (representing a ready to be serviced
 * operation) and automatically deletes itself after completion and signaling
 * success or failure. See KJob for more information on this part of the process.
 *
 * Services may either be loaded "stand alone" from plugins, or from a DataEngine
 * by passing in a source name to be used as the destination.
 *
 * Sample use might look like:
 *
 * @code
 * Plasma::DataEngine *twitter = dataEngine("twitter");
 * Plasma::Service *service = twitter.serviceForSource("aseigo");
 * KConfigGroup op = service->operationDescription("update");
 * op.writeEntry("tweet", "Hacking on plasma!");
 * Plasma::ServiceJob *job = service->startOperationCall(op);
 * connect(job, SIGNAL(finished(KJob*)), this, SLOT(jobCompeted()));
 * @endcode
 *
 * Please remember, the service needs to be deleted when it will no longer be
 * used. This can be done manually or by these (perhaps easier) alternatives:
 *
 * If it is needed throughout the lifetime of the object:
 * @code
 * service->setParent(this);
 * @endcode
 *
 * If the service will not be used after just one operation call, use:
 * @code
 * connect(job, SIGNAL(finished(KJob*)), service, SLOT(deleteLater()));
 * @endcode
 *
 */
class PLASMA_EXPORT Service : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Service)
    Q_PROPERTY(QString destination READ destination WRITE setDestination)
    Q_PROPERTY(QStringList operationNames READ operationNames)
    Q_PROPERTY(QString name READ name)

public:
    /**
     * Destructor
     */
    ~Service();

    /**
     * Used to load a given service from a plugin.
     *
     * @param name the plugin name of the service to load
     * @param args a list of arguments to supply to the service plugin when loading it
     * @param parent the parent object, if any, for the service
     *
     * @return a Service object, guaranteed to be not null.
     * @since 4.5
     */
    static Service *load(const QString &name, const QVariantList &args, QObject *parent = 0);

    /**
     * Used to load a given service from a plugin.
     *
     * @param name the plugin name of the service to load
     * @param parent the parent object, if any, for the service
     *
     * @return a Service object, guaranteed to be not null.
     */
    static Service *load(const QString &name, QObject *parent = 0);

    /**
     * Used to access a service from an url. Always check for the signal serviceReady() that fires
     * when this service is actually ready for use.
     */
    static Service *access(const KUrl &url, QObject *parent = 0);

    /**
     * Sets the destination for this Service to operate on
     *
     * @arg destination specific to each Service, this sets which
     *                  target or address for ServiceJobs to operate on
     */
    Q_INVOKABLE void setDestination(const QString &destination);

    /**
     * @return the target destination, if any, that this service is associated with
     */
    Q_INVOKABLE QString destination() const;

    /**
     * @return the possible operations for this profile
     */
    Q_INVOKABLE QStringList operationNames() const;

    /**
     * Retrieves the parameters for a given operation
     *
     * @param operationName the operation to retrieve parameters for
     * @return KConfigGroup containing the parameters
     */
    Q_INVOKABLE KConfigGroup operationDescription(const QString &operationName);

    /**
     * Called to create a ServiceJob which is associated with a given
     * operation and parameter set.
     *
     * @return a started ServiceJob; the consumer may connect to relevant
     *         signals before returning to the event loop
     */
    Q_INVOKABLE ServiceJob *startOperationCall(const KConfigGroup &description, QObject *parent = 0);

    /**
     * Query to find if an operation is enabled or not.
     *
     * @param operation the name of the operation to check
     * @return true if the operation is enabled, false otherwise
     */
    Q_INVOKABLE bool isOperationEnabled(const QString &operation) const;

    /**
     * The name of this service
     */
    Q_INVOKABLE QString name() const;

    /**
     * Assoicates a widget with an operation, which allows the service to
     * automatically manage, for example, the enabled state of a widget.
     *
     * This will remove any previous associations the widget had with
     * operations on this engine.
     *
     * @param widget the QWidget to associate with the service
     * @param operation the operation to associate the widget with
     */
    Q_INVOKABLE void associateWidget(QWidget *widget, const QString &operation);

    /**
     * Disassociates a widget if it has been associated with an operation
     * on this service.
     *
     * This will not change the enabled state of the widget.
     *
     * @param widget the QWidget to disassociate.
     */
    Q_INVOKABLE void disassociateWidget(QWidget *widget);

    /**
     * Assoicates a widget with an operation, which allows the service to
     * automatically manage, for example, the enabled state of a widget.
     *
     * This will remove any previous associations the widget had with
     * operations on this engine.
     *
     * @param widget the QGraphicsItem to associate with the service
     * @param operation the operation to associate the widget with
     */
    Q_INVOKABLE void associateWidget(QGraphicsWidget *widget, const QString &operation);

    /**
     * Disassociates a widget if it has been associated with an operation
     * on this service.
     *
     * This will not change the enabled state of the widget.
     *
     * @param widget the QGraphicsWidget to disassociate.
     */
    Q_INVOKABLE void disassociateWidget(QGraphicsWidget *widget);

    /**
     * @return a parameter map for the given description
     * @arg description the configuration values to turn into the parameter map
     * @since 4.4
     */
    Q_INVOKABLE QHash<QString, QVariant> parametersFromDescription(const KConfigGroup &description);

Q_SIGNALS:
    /**
     * Emitted when a job associated with this Service completes its task
     */
    void finished(Plasma::ServiceJob *job);

    /**
     * Emitted when the Service's operations change. For example, a
     * media player service may change what operations are available
     * in response to the state of the player.
     */
    void operationsChanged();

    /**
     * Emitted when this service is ready for use
     */
    void serviceReady(Plasma::Service *service);

protected:
    /**
     * Default constructor
     *
     * @arg parent the parent object for this service
     */
    explicit Service(QObject *parent = 0);

    /**
     * Constructor for plugin loading
     */
    Service(QObject *parent, const QVariantList &args);

    /**
     * Called when a job should be created by the Service.
     *
     * @param operation which operation to work on
     * @param parameters the parameters set by the user for the operation
     * @return a ServiceJob that can be started and monitored by the consumer
     */
    virtual ServiceJob *createJob(const QString &operation,
                                  QHash<QString, QVariant> &parameters) = 0;

    /**
     * By default this is based on the file in plasma/services/name.operations, but can be
     * reimplented to use a different mechanism.
     *
     * It should result in a call to setOperationsScheme(QIODevice *);
     */
    virtual void registerOperationsScheme();

    /**
     * Sets the XML used to define the operation schema for
     * this Service.
     */
    void setOperationsScheme(QIODevice *xml);

    /**
     * Sets the name of the Service; useful for Services not loaded from plugins,
     * which use the plugin name for this.
     *
     * @arg name the name to use for this service
     */
    void setName(const QString &name);

    /**
     * Enables a given service by name
     *
     * @param operation the name of the operation to enable or disable
     * @param enable true if the operation should be enabld, false if disabled
     */
    void setOperationEnabled(const QString &operation, bool enable);

private:
    Q_PRIVATE_SLOT(d, void jobFinished(KJob *))
    Q_PRIVATE_SLOT(d, void associatedWidgetDestroyed(QObject *))
    Q_PRIVATE_SLOT(d, void associatedGraphicsWidgetDestroyed(QObject *))

    ServicePrivate * const d;

    friend class Applet;
    friend class DataEnginePrivate;
    friend class GetSource;
    friend class PackagePrivate;
    friend class ServiceProvider;
    friend class RemoveService;
    friend class PluginLoader;
};

} // namespace Plasma

/**
 * Register a service when it is contained in a loadable module
 */
#define K_EXPORT_PLASMA_SERVICE(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_service_" #libname)) \
K_EXPORT_PLUGIN_VERSION(PLASMA_VERSION)

#endif // multiple inclusion guard

