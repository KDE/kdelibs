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

#ifndef KSETTINGS_DIALOG_H
#define KSETTINGS_DIALOG_H

#include <qobject.h>
#include <kservice.h>
#include <QList>

template<class T> class QList;
class KPluginInfo;
class KCMultiDialog;
class KCModuleInfo;

namespace KSettings
{

/**
 * @short Generic configuration dialog that even works over component boundaries
 *
 * For more information see \ref KSettings.
 *
 * This class aims to standardize the use of configuration dialogs in KDE
 * applications. Especially when using KParts and/or Plugins you face problems
 * creating a consistent config dialog.
 *
 * To show a configuration dialog you only have to call the show method and be
 * done with it. A code example:
 *
 * You initialize \p m_cfgdlg with
 * \code
 * m_cfgdlg = new Dialog( Dialog::Static, this );
 * \endcode
 * If you use a KPart that was not especially designed for your app you can use
 * the second constructor:
 * \code
 * QStringList kpartslist;
 * for( all my kparts )
 *   kpartslist += m_mypart->instance().instanceName();
 * m_cfgdlg = new Dialog( kpartslist, this );
 * \endcode
 * and the action for the config dialog is connected to the show slot:
 * \code
 * KStdAction::preferences( m_cfgdlg, SLOT( show() ), actionCollection() );
 * \endcode
 *
 * If you need to be informed when the config was changed and applied in the
 * dialog you might want to take a look at Dispatcher.
 *
 * For more information see \ref KSettings.
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @since 3.2
 */
class KUTILS_EXPORT Dialog : public QObject
{
    friend class PageNode;
    Q_OBJECT
    public:
        /**
         * Tells the dialog whether the entries in the listview are all static
         * or whether it should add a Configure... button to select which parts
         * of the optional functionality should be active or not.
         */
        enum ContentInListView
        {
            /**
             * Static listview, while running no entries are added or deleted
             */
            Static,
            /**
             * Configurable listview. The user can select what functionality he
             * wants.
             */
            Configurable
        };

        /**
         * Construct a new Preferences Dialog for the application. It uses all
         * KCMs with X-KDE-ParentApp set to KGlobal::instance()->instanceName().
         *
         * @param content      Select whether you want a static or configurable
         *                     config dialog.
         * @param parent       The parent is only used as the parent for the
         *                     dialog - centering the dialog over the parent
         *                     widget.
         */
        Dialog( ContentInListView content = Static, QWidget * parent = 0 );

        /**
         * Construct a new Preferences Dialog with the pages for the selected
         * instance names. For example if you want to have the configuration
         * pages for the kviewviewer KPart you would pass a
         * QStringList consisting of only the name of the part "kviewviewer".
         *
         * @param components   A list of the names of the components that your
         *                     config dialog should merge the config pages in.
         * @param parent       The parent is only used as the parent for the
         *                     dialog - centering the dialog over the parent
         *                     widget.
         */
         Dialog( const QStringList & components, QWidget * parent = 0 );

        /**
         * Construct a new Preferences Dialog with the pages for the selected
         * instance names. For example if you want to have the configuration
         * pages for the kviewviewer KPart you would pass a
         * QStringList consisting of only the name of the part "kviewviewer".
         *
         * @param components   A list of the names of the components that your
         *                     config dialog should merge the config pages in.
         * @param content      Select whether you want a static or configurable
         *                     config dialog.
         * @param parent       The parent is only used as the parent for the
         *                     dialog - centering the dialog over the parent
         *                     widget.
         */
        Dialog( const QStringList & components, ContentInListView
                content, QWidget * parent = 0 );

        ~Dialog();

        /**
         * If you use a Configurable dialog you need to pass KPluginInfo
         * objects that the dialog should configure.
         */
        void addPluginInfos( const QList<KPluginInfo*> & plugininfos );

        KCMultiDialog * dialog();

    public slots:
        /**
         * Show the config dialog. The slot immediatly returns since the dialog
         * is non-modal.
         */
        void show();

    signals:
        /**
         * If you use the dialog in Configurable mode and want to be notified
         * when the user changes the plugin selections use this signal. It's
         * emitted if the selection has changed and the user pressed Apply or
         * Ok. In the slot you would then load and unload the plugins as
         * requested.
         */
        void pluginSelectionChanged();

    protected slots:
        void configureTree();
        void updateTreeList();

    private:
        /**
         * @internal
         * Check whether the plugin associated with this KCM is enabled.
         */
        bool isPluginForKCMEnabled( KCModuleInfo * ) const;

        QList<KService::Ptr> instanceServices() const;
        QList<KService::Ptr> parentComponentsServices(
                const QStringList & ) const;
        /**
         * @internal
         * Read the .setdlg file and add it to the groupmap
         */
        void parseGroupFile( const QString & );

        /**
         * @internal
         * If this module is put into a TreeList hierarchy this will return a
         * list of the names of the parent modules.
         */
        QStringList parentModuleNames( KCModuleInfo * );

        /**
         * @internal
         * This method is called only once. The KCMultiDialog is not created
         * until it's really needed. So if some method needs to access d->dlg it
         * checks for 0 and if it's not created this method will do it.
         */
        void createDialogFromServices();

        class DialogPrivate;
        DialogPrivate* const d;
};

}

// vim: sw=4 sts=4 et
#endif // KSETTINGS_DIALOG_H
