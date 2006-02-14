/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef KPLUGINSELECTOR_P_H
#define KPLUGINSELECTOR_P_H

#include <qwidget.h>

#include <kdelibs_export.h>
#include <QList>

class KConfigGroup;
class QTreeWidgetItem;
class KPluginInfo;
class KCModuleInfo;

/**
 * This is a widget to configure what Plugins should be loaded. This widget is
 * used by KPluginSelector and has no direct use.
 *
 * @internal
 * @see KPluginSelector
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @since 3.2
 */
class KPluginSelectionWidget : public QWidget
{
    Q_OBJECT
    public:
        /**
         * Create a new Plugin Selector widget.
         *
         * @param plugininfos  A list of KPluginInfo objects containing the
         *                     necessary information for the plugins you want to
         *                     add the selector's list.
         * @param kps          A KPluginSelector object.
         * @param parent       The parent widget.
         * @param catname      The translated name of the category.
         * @param category     The unstranslated category key name.
         * @param config       Set the KConfigGroup object that holds the
         *                     state of the plugins being enabled or not.
         * @param name         The name of the widget (passed to QWidget)
         *
         * @internal
         */
        KPluginSelectionWidget( const QList<KPluginInfo*> & plugininfos,
                                KPluginSelector * kps, QWidget * parent, const QString & catname,
                                const QString & category, KConfigGroup * config = 0 );

        virtual ~KPluginSelectionWidget();


        /**
         * Returns the translated category name
         *
         * @internal
         */
        QString catName() const;

        /**
         * Tell the KPluginInfo objects to load their state (enabled/disabled).
         */
        virtual void load();

        /**
         * It tells the KPluginInfo objects to save their current state
         * (enabled/disabled).
         */
        virtual void save();

        /**
         * @return whether the plugin is enabled in the ListView or not.
         */
        bool pluginChecked( const QString & pluginName ) const;

    Q_SIGNALS:
        /**
         * Emits true when at least one embedded KCM is changed, or the plugin
         * selection was changed.
         * Emits false when the configuration is back to what it was.
         */
        void changed( bool );

        /**
         * Emitted after the config of an embedded KCM has been saved. The
         * argument is the name of the parent component that needs to reload
         * its config
         */
        void configCommitted( const QByteArray & instanceName );

    protected:
        /**
         * Reimplement in your subclass if you have special needs: The standard
         * implementation looks at the KPluginInfo objects to find the
         * needed information. But if, for some reason, your program doesn't
         * work with that here's your chance to get it working.
         *
         * @return Whether the plugin is loaded.
         */
        virtual bool pluginIsLoaded( const QString & pluginName ) const;

    private Q_SLOTS:
        /**
         * Called when a QCheckListItem is checked or unchecked. It calls
         * checkDependencies on the Plugin and then updateConfigPage.
         *
         * @internal
         */
        void executed( QTreeWidgetItem *, int );

        /**
         * Called whenever the visible config page should change (plugin
         * selection changed, plugin checked changed)
         *
         * First it checks for a widget for the plugin - if there is one, great:
         * show it.
         * If there is none, check whether there should be one and try to load
         * the KCM. If there are more than one KCM create a TabWidget and put
         * all of them inside, else just use the "bare" KCM. If there is no KCM
         * us the infoPage( NoKCM ). If there should be one but it can't be
         * loaded us the infoPage( LoadError ).
         * Depending on whether the currently selected Plugin is checked or not
         * disable or enable the "page" (which is the TabWidget or the KCM).
         *
         * @internal
         */
        void updateConfigPage( KPluginInfo * plugininfo, bool checked );
        void updateConfigPage();

        /**
         * Whenever an embedded KCM emits the changed signal we count the number
         * of changed KCMs. If it becomes one we emit changed( true ), if it
         * becomes zero we emit changed( false ).
         *
         * @internal
         */
        void clientChanged( bool );

        /**
         * Called whenever the tabWidget changes. It checks whether this
         * KPluginSelectionWidget is shown and sets the currentplugininfo
         * accordingly.
         */
        void tabWidgetChanged( QWidget * );

    private:
        /**
         * Load a KCM from a KCModuleInfo. If successfull connect changed
         * signal and return the module. If not, create a label showing "Error",
         * show the loaderError and return the label.
         *
         * @internal
         */
        QWidget * insertKCM( QWidget * parent, const KCModuleInfo & );

        /**
         * Embed the KCMs for the plugin into the widgetstack
         *
         * @internal
         */
        void embeddPluginKCMs( KPluginInfo *, bool );

        void init( const QList<KPluginInfo*> & plugininfos, const QString & );
        void checkDependencies( const KPluginInfo * );

        struct KPluginSelectionWidgetPrivate;
        KPluginSelectionWidgetPrivate* const d;
};

// vim: sw=4 sts=4 et
#endif // KPLUGINSELECTOR_P_H
