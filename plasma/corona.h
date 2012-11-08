/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright 2012 Marco MArtin <mart@kde.org>
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

#ifndef PLASMA_CORONABASE_H
#define PLASMA_CORONABASE_H

#include <QGraphicsScene>

#include <plasma/containment.h>
#include <plasma/plasma.h>
#include <plasma/plasma_export.h>

class QGraphicsGridLayout;
class QAction;

class KAction;

namespace Plasma
{

class CoronaPrivate;
class ContainmentActionsPluginsConfig;
class AbstractDialogManager;

/**
 * @class CoronaBase plasma/CoronaBase.h <Plasma/CoronaBase>
 *
 * @short A QGraphicsScene for Plasma::Applets
 */
class PLASMA_EXPORT Corona : public QObject
{
    Q_OBJECT

//typedef QHash<QString, QList<Plasma::Applet*> > layouts;

public:
    explicit Corona(QObject * parent = 0);
    ~Corona();

    /**
     *  Sets the mimetype of Drag/Drop items. Default is
     *  text/x-plasmoidservicename
     */
    void setAppletMimeType(const QString &mimetype);

    /**
     * The current mime type of Drag/Drop items.
     */
    QString appletMimeType();

    /**
     * @return the default containment plugin type
     * @since 4.7
     */
    QString defaultContainmentPlugin() const;

    /**
     * @return all containments on this CoronaBase
     */
    QList<Containment*> containments() const;

    /**
     * Clear the CoronaBase from all applets.
     */
    void clearContainments();

    /**
     * Returns the config file used to store the configuration for this CoronaBase
     */
    KSharedConfig::Ptr config() const;

    /**
     * Adds a Containment to the CoronaBase
     *
     * @param name the plugin name for the containment, as given by
     *        KPluginInfo::pluginName(). If an empty string is passed in, the default
     *        containment plugin will be used (usually DesktopContainment). If the
     *        string literal "null" is passed in, then no plugin will be loaded and
     *        a simple Containment object will be created instead.
     * @param args argument list to pass to the containment
     *
     * @return a pointer to the containment on success, or 0 on failure. Failure can be
     * caused by too restrictive of an Immutability type, as containments cannot be added
     * when widgets are locked, or if the requested containment plugin can not be located
     * or successfully loaded.
     */
    Containment *addContainment(const QString &name, const QVariantList &args = QVariantList());

    /**
     * Loads a containment with delayed initialization, primarily useful
     * for implementations of loadDefaultLayout. The caller is responsible
     * for all initializating, saving and notification of a new containment.
     *
     * @param name the plugin name for the containment, as given by
     *        KPluginInfo::pluginName(). If an empty string is passed in, the defalt
     *        containment plugin will be used (usually DesktopContainment). If the
     *        string literal "null" is passed in, then no plugin will be loaded and
     *        a simple Containment object will be created instead.
     * @param args argument list to pass to the containment
     *
     * @return a pointer to the containment on success, or 0 on failure. Failure can
     * be caused by the Immutability type being too restrictive, as containments can't be added
     * when widgets are locked, or if the requested containment plugin can not be located
     * or successfully loaded.
     * @see addContainment
     **/
    Containment *addContainmentDelayed(const QString &name,
                                       const QVariantList &args = QVariantList());

    /**
     * Returns the Containment, if any, for a given physical screen and desktop
     *
     * @param screen number of the physical screen to locate
     * @param desktop the virtual desktop) to locate; if < 0 then it will
     *        simply return the first Containment associated with screen
     */
    Containment *containmentForScreen(int screen, int desktop = -1) const;

    /**
     * Returns the Containment for a given physical screen and desktop, creating one
     * if none exists
     *
     * @param screen number of the physical screen to locate
     * @param desktop the virtual desktop) to locate; if < 0 then it will
     *        simply return the first Containment associated with screen
     * @param defaultPluginIfNonExistent the plugin to load by default; "null" is an empty
     * Containment and "default" creates the default plugin
     * @param defaultArgs optional arguments to pass in when creating a Containment if needed
     * @since 4.6
     */
    Containment *containmentForScreen(int screen, int desktop,
                                      const QString &defaultPluginIfNonExistent,
                                      const QVariantList &defaultArgs = QVariantList());

    /**
     * Returns the number of screens available to plasma.
     * Subclasses should override this method as the default
     * implementation returns a meaningless value.
     */
    virtual int numScreens() const;

    /**
     * Returns the geometry of a given screen.
     * Valid screen ids are 0 to numScreen()-1, or -1 for the full desktop geometry.
     * Subclasses should override this method as the default
     * implementation returns a meaningless value.
     */
    virtual QRect screenGeometry(int id) const;

    /**
     * Returns the available region for a given screen.
     * The available region excludes panels and similar windows.
     * Valid screen ids are 0 to numScreens()-1.
     * By default this method returns a rectangular region
     * equal to screenGeometry(id); subclasses that need another
     * behavior should override this method.
     */
    virtual QRegion availableScreenRegion(int id) const;

    /**
     * This method is useful in order to retrieve the list of available
     * screen edges for panel type containments.
     * @param screen the id of the screen to look for free edges.
     * @returns a list of free edges not filled with panel type containments.
     */
    QList<Plasma::Location> freeEdges(int screen) const;

    /**
     * Returns the QAction with the given name from our collection
     */
    QAction *action(QString name) const;

    /**
     * Adds the action to our collection under the given name
     */
    void addAction(QString name, QAction *action);

    /**
     * Returns all the actions in our collection
     */
    QList<QAction*> actions() const;

    /**
     * convenience function - enables or disables an action by name
     *
     * @param name the name of the action in our collection
     * @param enable true to enable, false to disable
     */
    void enableAction(const QString &name, bool enable);

    /**
     * @since 4.3
     * Updates keyboard shortcuts for all the CoronaBase's actions.
     * If you've added actions to the CoronaBase you'll need to
     * call this for them to be configurable.
     */
    void updateShortcuts();

    /**
     * @since 4.3
     * Adds a set of actions to the shortcut config dialog.
     * don't use this on actions in the CoronaBase's own actioncollection,
     * those are handled automatically. this is for stuff outside of that.
     */
    void addShortcuts(KActionCollection *newShortcuts);

    /**
     * @since 4.3
     * Creates an action in our collection under the given name
     * @return the new action
     * FIXME I'm wrapping so much of kactioncollection API now, maybe I should just expose the
     * collection itself :P
     */
    KAction* addAction(QString name);

    /**
     * @since 4.4
     * Sets the default containmentactions plugins for the given containment type
     */
    void setContainmentActionsDefaults(Containment::Type containmentType, const ContainmentActionsPluginsConfig &config);

    /**
     * @since 4.4
     * Returns the default containmentactions plugins for the given containment type
     */
    ContainmentActionsPluginsConfig containmentActionsDefaults(Containment::Type containmentType);

    /**
     * @param the AbstractDialogManager implementaion
     *
     * @since 4.5
     */
    void setDialogManager(AbstractDialogManager *manager);

    /**
     * @return the AbstractDialogManager that will show dialogs used by applets, like configuration dialogs
     *
     * @since 4.5
     */
    AbstractDialogManager *dialogManager();

    /**
     * Returns the name of the preferred plugin to be used as containment toolboxes.
     * CustomContainments and CustomPanelContainments can still override it as their liking. It's also not guaranteed that the plugin will actually exist.
     *
     * @param type the containment type of which we want to know the associated toolbox plugin
     * @since 4.6
     */
    QString preferredToolBoxPlugin(const Containment::Type type) const;

    /**
     * Imports an applet layout from a config file. The results will be added to the
     * current set of Containments.
     *
     * @param config the name of the config file to load from,
     *               or the default config file if QString()
     * @return the list of containments that were loaded
     * @since 4.6
     */
    QList<Plasma::Containment *> importLayout(const KConfigGroup &config);

    /**
     * Exports a set of containments to a config file.
     *
     * @param config the config group to save to
     * @param containments the list of containments to save
     * @since 4.6
     */
    void exportLayout(KConfigGroup &config, QList<Containment*> containments);

public Q_SLOTS:
    /**
     * Initializes the layout from a config file. This will first clear any existing
     * Containments, load a layout from the requested configuration file, request the
     * default layout if needed and update immutability.
     *
     * @param config the name of the config file to load from,
     *               or the default config file if QString()
     */
    void initializeLayout(const QString &config = QString());

    /**
     * Load applet layout from a config file. The results will be added to the
     * current set of Containments.
     *
     * @param config the name of the config file to load from,
     *               or the default config file if QString()
     */
    void loadLayout(const QString &config = QString());

    /**
     * Save applets layout to file
     * @param config the file to save to, or the default config file if QString()
     */
    void saveLayout(const QString &config = QString()) const;

    /**
     * @return The type of immutability of this CoronaBase
     */
    ImmutabilityType immutability() const;

    /**
     * Sets the immutability type for this CoronaBase (not immutable,
     * user immutable or system immutable)
     * @param immutable the new immutability type of this applet
     */
    void setImmutability(const ImmutabilityType immutable);

    /**
     * Schedules a flush-to-disk synchronization of the configuration state
     * at the next convenient moment.
     */
    void requestConfigSync();

    /**
     * Schedules a time sensitive flush-to-disk synchronization of the
     * configuration state. Since this method does not provide any sort of
     * event compression, it should only be used when an *immediate* disk
     * sync is *absolutely* required. Otherwise, use @see requestConfigSync()
     * which does do event compression.
     */
    void requireConfigSync();

Q_SIGNALS:
    /**
     * This signal indicates a new containment has been added to
     * the CoronaBase
     */
    void containmentAdded(Plasma::Containment *containment);

    /**
     * This signal indicates that a containment has been newly
     * associated (or dissociated) with a physical screen.
     *
     * @param wasScreen the screen it was associated with
     * @param isScreen the screen it is now associated with
     * @param containment the containment switching screens
     */
    void screenOwnerChanged(int wasScreen, int isScreen, Plasma::Containment *containment);

    /**
     * This signal indicates that an application launch, window
     * creation or window focus event was triggered. This is used, for instance,
     * to ensure that the Dashboard view in Plasma hides when such an event is
     * triggered by an item it is displaying.
     */
    void releaseVisualFocus();

    /**
     * This signal indicates that the configuration file was flushed to disc.
     */
    void configSynced();

    /**
     * This signal inicates that a change in available screen goemetry occurred.
     */
    void availableScreenRegionChanged();

    /**
     * emitted when immutability changes.
     * this is for use by things that don't get contraints events, like plasmaapp.
     * it's NOT for containments or applets or any of the other stuff on the scene.
     * if your code's not in shells/ it probably shouldn't be using it.
     */
    void immutabilityChanged(Plasma::ImmutabilityType immutability);

    /**
     * @since 4.3
     * emitted when the user changes keyboard shortcut settings
     * connect to this if you've put some extra shortcuts in your app
     * that are NOT in CoronaBase's actioncollection.
     * if your code's not in shells/ it probably shouldn't be using this function.
     * @see addShortcuts
     */
    void shortcutsChanged();

protected:
    /**
     * Loads the default (system wide) layout for this user
     **/
    virtual void loadDefaultLayout();

    /**
     * @return The preferred toolbox plugin name for a given containment type.
     * @param type the containment type of which we want to know the preferred toolbox plugin.
     * @param plugin the toolbox plugin name
     * @since 4.6
     */
    void setPreferredToolBoxPlugin(const Containment::Type type, const QString &plugin);

    /**
     * Sets the default containment plugin to try and load
     * @since 4.7
     */
    void setDefaultContainmentPlugin(const QString &name);

private:
    CoronaPrivate *const d;

    Q_PRIVATE_SLOT(d, void containmentDestroyed(QObject*))
    Q_PRIVATE_SLOT(d, void syncConfig())
    Q_PRIVATE_SLOT(d, void toggleImmutability())
    Q_PRIVATE_SLOT(d, void showShortcutConfig())

    friend class CoronaPrivate;
    friend class Corona;
    friend class View;
};

} // namespace Plasma

#endif

