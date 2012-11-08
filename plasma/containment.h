/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Ménard Alexis <darktears31@gmail.com>
 *   Copyright (c) 2009 Chani Armitage <chani@kde.org>
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

#ifndef PLASMA_CONTAINMENT_H
#define PLASMA_CONTAINMENT_H

#include <QObject>
#include <QWidget>

#include <kplugininfo.h>
#include <ksharedconfig.h>

#include <plasma/applet.h>


namespace Plasma
{

class AccessAppletJob;
class AppletHandle;
class DataEngine;
class Package;
class Corona;
class View;
class Wallpaper;
class ContainmentActions;
class ContainmentPrivate;
class AbstractToolBox;

/**
 * @class Containment plasma/containment.h <Plasma/Containment>
 *
 * @short The base class for plugins that provide backgrounds and applet grouping containers
 *
 * Containment objects provide the means to group applets into functional sets.
 * They also provide the following:
 *
 * creation of focussing event
 * - drawing of the background image (which can be interactive)
 * - form factors (e.g. panel, desktop, full screen, etc)
 * - applet layout management
 *
 * Since containment is actually just a Plasma::Applet, all the techniques used
 * for writing the visual presentation of Applets is applicable to Containtments.
 * Containments are differentiated from Applets by being marked with the ServiceType
 * of Plasma/Containment. Plugins registered with both the Applet and the Containment
 * ServiceTypes can be loaded for us in either situation.
 *
 * See techbase.kde.org for a tutorial on writing Containments using this class.
 */
class PLASMA_EXPORT Containment : public Applet
{
    Q_OBJECT

    public:

        enum Type {
            NoContainmentType = -1,  /**< @internal */
            DesktopContainment = 0,  /**< A desktop containment */
            PanelContainment,        /**< A desktop panel */
            CustomContainment = 127, /**< A containment that is neither a desktop nor a panel
                                        but something application specific */
            CustomPanelContainment = 128 /**< A customized desktop panel */
        };

        enum ToolType {
            AddTool = 0,
            ConfigureTool = 100,
            ControlTool = 200,
            MiscTool = 300,
            DestructiveTool = 400,
            UserToolType = DestructiveTool + 1000
        };
        Q_ENUMS(ToolType)

        /**
         * @param parent the QObject this applet is parented to
         * @param serviceId the name of the .desktop file containing the
         *      information about the widget
         * @param containmentId a unique id used to differentiate between multiple
         *      instances of the same Applet type
         */
        explicit Containment(QObject *parent = 0,
                             const QString &serviceId = QString(),
                             uint containmentId = 0);

        /**
         * This constructor is to be used with the plugin loading systems
         * found in KPluginInfo and KService. The argument list is expected
         * to have two elements: the KService service ID for the desktop entry
         * and an applet ID which must be a base 10 number.
         *
         * @param parent a QObject parent; you probably want to pass in 0
         * @param args a list of strings containing two entries: the service id
         *      and the applet id
         */
        Containment(QObject *parent, const QVariantList &args);

        ~Containment();

        /**
         * Reimplemented from Applet
         */
        void init();

        /**
         * Returns the type of containment
         */
        Type containmentType() const;

        /**
         * Returns the Corona (if any) that this Containment is hosted by
         */
        Corona *corona() const;

        /**
         * Returns a list of all known containments.
         *
         * @param category Only containments matching this category will be returned.
         *                 Useful in conjunction with knownCategories.
         *                 If "Miscellaneous" is passed in, then applets without a
         *                 Categories= entry are also returned.
         *                 If an empty string is passed in, all applets are
         *                 returned.
         * @param parentApp the application to filter applets on. Uses the
         *                  X-KDE-ParentApp entry (if any) in the plugin info.
         *                  The default value of QString() will result in a
         *                  list containing only applets not specifically
         *                  registered to an application.
         * @return list of applets
         **/
        static KPluginInfo::List listContainments(const QString &category = QString(),
                                                  const QString &parentApp = QString());

        /**
         * Returns a list of all known Containments that match the parameters.
         *
         * @param type Only Containments with this string in X-Plasma-ContainmentCategories
         *             in their .desktop files will be returned. Common values are panel and
         *             desktop
         * @param category Only applets matchin this category will be returned.
         *                 Useful in conjunction with knownCategories.
         *                 If "Miscellaneous" is passed in, then applets without a
         *                 Categories= entry are also returned.
         *                 If an empty string is passed in, all applets are
         *                 returned.
         * @param parentApp the application to filter applets on. Uses the
         *                  X-KDE-ParentApp entry (if any) in the plugin info.
         *                  The default value of QString() will result in a
         *                  list containing only applets not specifically
         *                  registered to an application.
         * @return list of applets
         **/
        static KPluginInfo::List listContainmentsOfType(const QString &type,
                                                        const QString &category = QString(),
                                                        const QString &parentApp = QString());

        /**
         * @return a list of all known types of Containments on this system
         */
        static QStringList listContainmentTypes();

        /**
         * Returns a list of all known applets associated with a certain MimeType
         *
         * @return list of applets
         **/
        static KPluginInfo::List listContainmentsForMimeType(const QString &mimeType);

        /**
         * Adds an applet to this Containment
         *
         * @param name the plugin name for the applet, as given by
         *        KPluginInfo::pluginName()
         * @param args argument list to pass to the plasmoid
         * @param geometry where to place the applet, or to auto-place it if an invalid
         *                 is provided
         *
         * @return a pointer to the applet on success, or 0 on failure
         */
        Applet *addApplet(const QString &name, const QVariantList &args = QVariantList(),
                                  const QRectF &geometry = QRectF(-1, -1, -1, -1));

        /**
         * Add an existing applet to this Containment
         *
         * If dontInit is true, the pending constraints are not flushed either.
         * So it is your responsibility to call both init() and
         * flushPendingConstraints() on the applet.
         *
         * @param applet the applet that should be added
         * @param pos the containment-relative position
         * @param dontInit if true, init() will not be called on the applet
         */
        void addApplet(Applet *applet, const QPointF &pos = QPointF(-1, -1), bool dontInit = true);

        /**
         * @return the applets currently in this Containment
         */
        Applet::List applets() const;

        /**
         * Removes all applets from this Containment
         */
        void clearApplets();

        /**
         * Sets the physical screen this Containment is associated with.
         *
         * @param screen the screen number this containment is the desktop for, or -1
         *               if it is not serving as the desktop for any screen
         * @param desktop the virtual desktop to also associate this this screen with
         */
        void setScreen(int screen, int desktop = -1);

        /**
         * @return the screen number this containment is serving as the desktop for
         *         or -1 if none
         */
        int screen() const;

        /**
         * @return the last screen number this containment had
         *         only returns -1 if it's never ever been on a screen
         * @since 4.5
         */
        int lastScreen() const;

        /**
         * @return the viewport (e.g. virtual desktop) this Containment is associated with.
         */
        int desktop() const;

        /**
         * @return the viewport (e.g. virtual desktop) this Containment was associated with
         * last time it had a screen
         * @since 4.5
         */
        int lastDesktop() const;

        /**
         * @reimp
         * @sa Applet::save(KConfigGroup &)
         */
        void save(KConfigGroup &group) const;

        /**
         * @reimp
         * @sa Applet::restore(KConfigGroup &)
         */
        void restore(KConfigGroup &group);

        /**
         * convenience function - enables or disables an action by name
         *
         * @param name the name of the action in our collection
         * @param enable true to enable, false to disable
         */
        void enableAction(const QString &name, bool enable);

        /**
         * Add an action to the toolbox
         */
        void addToolBoxAction(QAction *action);

        /**
         * Remove an action from the toolbox
         */
        void removeToolBoxAction(QAction *action);

        /**
         * associate actions with this widget, including ones added after this call.
         * needed to make keyboard shortcuts work.
         */
        void addAssociatedWidget(QWidget *widget);

        /**
         * un-associate actions from this widget, including ones added after this call.
         * needed to make keyboard shortcuts work.
         */
        void removeAssociatedWidget(QWidget *widget);

        /**
         * Return whether wallpaper is painted or not.
         */
        bool drawWallpaper();

        /**
         * Sets wallpaper plugin.
         *
         * @param pluginName the name of the wallpaper to attempt to load
         * @param mode optional mode or the wallpaper plugin (e.g. "Slideshow").
         *        These values are pugin specific and enumerated in the plugin's
         *        .desktop file.
         */
        void setWallpaper(const QString &pluginName, const QString &mode = QString());

        /**
         * Return wallpaper plugin.
         */
        Plasma::Wallpaper *wallpaper() const;

        /**
         * Sets the current activity by id
         *
         * @param activity the id of the activity
         */
        void setActivity(const QString &activityId);

        /**
         * @return the current activity id associated with this containment
         */
        QString activity() const;

        /**
         * Shows the context menu for the containment directly, bypassing Applets
         * altogether.
         */
        void showContextMenu(const QPointF &containmentPos, const QPoint &screenPos);

        /**
         * Shows a visual clue for drag and drop
         * The default implementation does nothing,
         * reimplement in containments that need it
         *
         * @param pos point where to show the drop target; if an invalid point is passed in
         *        the drop zone should not be shown
         */
        virtual void showDropZone(const QPoint pos);

        /**
         * Sets a containmentactions plugin.
         *
         * @param trigger the mouse button (and optional modifier) to associate the plugin with
         * @param pluginName the name of the plugin to attempt to load. blank = set no plugin.
         * @since 4.4
         */
        void setContainmentActions(const QString &trigger, const QString &pluginName);

        /**
         * @return a list of all triggers that have a containmentactions plugin associated
         * @since 4.4
         */
        QStringList containmentActionsTriggers();

        /**
         * @return the plugin name for the given trigger
         * @since 4.4
         */
        QString containmentActions(const QString &trigger);

        /**
         * @return the config group that containmentactions plugins go in
         * @since 4.6
         */
        KConfigGroup containmentActionsConfig();

        void setAcceptDrops(bool accept);
        bool acceptDrops() const;

Q_SIGNALS:
        /**
         * This signal is emitted when a new applet is created by the containment
         */
        void appletAdded(Plasma::Applet *applet, const QPointF &pos);

        /**
         * This signal is emitted when an applet is destroyed
         */
        void appletRemoved(Plasma::Applet *applet);

        /**
         * Emitted when the user clicks on the toolbox
         */
        void toolBoxToggled();

        /**
         * Emitted when the toolbox is hidden or shown
         * @since 4.3
         */
        void toolBoxVisibilityChanged(bool);

        /**
         * Emitted when the containment wants a new containment to be created.
         * Usually only used for desktop containments.
         */
        void addSiblingContainment(Plasma::Containment *);

        /**
         * Emitted when the containment requests an add widgets dialog is shown.
         * Usually only used for desktop containments.
         *
         * @param pos where in the containment this request was made from, or
         *            an invalid position (QPointF()) is not location specific
         */
        void showAddWidgetsInterface(const QPointF &pos);

        /**
         * This signal indicates that a containment has been newly
         * associated (or dissociated) with a physical screen.
         *
         * @param wasScreen the screen it was associated with
         * @param isScreen the screen it is now associated with
         * @param containment the containment switching screens
         */
        void screenChanged(int wasScreen, int isScreen, Plasma::Containment *containment);

        /**
         * Emitted when the user wants to configure/change containment.
         */
        void configureRequested(Plasma::Containment *containment);

    public Q_SLOTS:
        /**
         * Informs the Corona as to what position it is in. This is informational
         * only, as the Corona doesn't change its actual location. This is,
         * however, passed on to Applets that may be managed by this Corona.
         *
         * @param location the new location of this Corona
         */
        void setLocation(Plasma::Location location);

        /**
         * Sets the form factor for this Containment. This may cause changes in both
         * the arrangement of Applets as well as the display choices of individual
         * Applets.
         */
        void setFormFactor(Plasma::FormFactor formFactor);

        /**
         * Tells the corona to create a new desktop containment
         */
        void addSiblingContainment();

        /**
         * switch keyboard focus to the next of our applets
         */
        void focusNextApplet();

        /**
         * switch keyboard focus to the previous one of our applets
         */
        void focusPreviousApplet();

        /**
         * Destroys this containment and all its applets (after a confirmation dialog);
         * it will be removed nicely and deleted.
         * Its configuration will also be deleted.
         */
        void destroy();

        /**
         * Destroys this containment and all its applets (after a confirmation dialog);
         * it will be removed nicely and deleted.
         * Its configuration will also be deleted.
         *
         * @param confirm whether or not confirmation from the user should be requested
         */
        void destroy(bool confirm);

        /**
         * @reimp
         * @sa Applet::showConfigurationInterface()
         */
        void showConfigurationInterface();

        /**
         * Sets the type of this containment.
         */
        void setContainmentType(Containment::Type type);

        /**
         * Sets whether wallpaper is painted or not.
         */
        void setDrawWallpaper(bool drawWallpaper);

    protected:
        /**
         * Called when the contents of the containment should be saved. By default this saves
         * all loaded Applets
         *
         * @param group the KConfigGroup to save settings under
         */
        virtual void saveContents(KConfigGroup &group) const;

        /**
         * Called when the contents of the containment should be loaded. By default this loads
         * all previously saved Applets
         *
         * @param group the KConfigGroup to save settings under
         */
        virtual void restoreContents(KConfigGroup &group);

        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
        void keyPressEvent(QKeyEvent *event);
        void wheelEvent(QGraphicsSceneWheelEvent *event);

        /**
         * @reimp
         * @sa QGraphicsItem::dropEvent()
         */
        void dropEvent(QDropEvent *event);

        /**
         * @reimp
         * @sa QObject::resizeEvent()
         */
        void resizeEvent(QResizeEvent *event);

        //FIXME: kill those
        QSizeF maximumSize() const;
        void setMaximumSize(QSizeF size);
        QSizeF minimumSize() const;
        void setMinimumSize(QSizeF size);

    private:
        /**
         * @internal This constructor is to be used with the Package loading system.
         *
         * @param parent a QObject parent; you probably want to pass in 0
         * @param args a list of strings containing two entries: the service id
         *      and the applet id
         * @since 4.3
         */
        Containment(const QString &packagePath, uint appletId, const QVariantList &args);

        Q_PRIVATE_SLOT(d, void appletDeleted(Plasma::Applet*))
        Q_PRIVATE_SLOT(d, void triggerShowAddWidgets())
        Q_PRIVATE_SLOT(d, void requestConfiguration())
        Q_PRIVATE_SLOT(d, void showDropZoneDelayed())
        Q_PRIVATE_SLOT(d, void checkStatus(Plasma::ItemStatus))
        Q_PRIVATE_SLOT(d, void remoteAppletReady(Plasma::AccessAppletJob *))
        /**
        * This slot is called when the 'stat' after a job event has finished.
        */
        Q_PRIVATE_SLOT(d, void mimeTypeRetrieved(KIO::Job *, const QString &))
        Q_PRIVATE_SLOT(d, void dropJobResult(KJob *))

        friend class Applet;
        friend class AppletPrivate;
        friend class CoronaPrivate;
        friend class CoronaBasePrivate;
        friend class ContainmentPrivate;
        friend class ContainmentActions;
        friend class PopupApplet;
        friend class View;
        ContainmentPrivate *const d;
};

} // Plasma namespace

#endif // multiple inclusion guard
