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

#include "../kutils_export.h"
#include "../kcmultidialog.h"

#include <kservice.h>
#include <kplugininfo.h>

template<class T> class QList;
class KPluginInfo;
class KCModuleInfo;

namespace KSettings
{
    class DialogPrivate;

/**
 * @short Generic configuration dialog that works over component boundaries
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
 * m_cfgdlg = new Dialog( this );
 * \endcode
 * If you use a KPart that was not especially designed for your app you can use
 * the second constructor:
 * \code
 * QStringList kpartslist;
 * for( all my kparts )
 *   kpartslist += m_mypart->componentData().componentName();
 * m_cfgdlg = new Dialog( kpartslist, this );
 * \endcode
 * and the action for the config dialog is connected to the show slot:
 * \code
 * KStandardAction::preferences( m_cfgdlg, SLOT( show() ), actionCollection() );
 * \endcode
 *
 * If you need to be informed when the config was changed and applied in the
 * dialog you might want to take a look at Dispatcher.
 *
 * For more information see \ref KSettings.
 *
 * @author Matthias Kretz <kretz@kde.org>
 */
class KUTILS_EXPORT Dialog : public KCMultiDialog
{
    friend class PageNode;
    Q_DECLARE_PRIVATE(Dialog)
    Q_OBJECT
    public:
        /**
         * Construct a new Preferences Dialog for the application. It uses all
         * KCMs with X-KDE-ParentApp set to KGlobal::mainComponent().componentName().
         *
         * @param content      Select whether you want a static or configurable
         *                     config dialog.
         * @param parent       The parent is only used as the parent for the
         *                     dialog - centering the dialog over the parent
         *                     widget.
         */
        explicit Dialog(QWidget * parent = 0);

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
         explicit Dialog(const QStringList & components, QWidget * parent = 0);

        ~Dialog();

        /**
         * If you use a Configurable dialog you need to pass KPluginInfo
         * objects that the dialog should configure.
         */
        void addPluginInfos(const QList<KPluginInfo> &plugininfos);

        /**
         * Sets the argument list that is given to all the KControlModule's when
         * they are created.
         * Use this if you have KControlModule's that need special arguments to
         * work
         *
         * Note that this function only works before showing the
         * KSettings::Dialog for the first time.
         * @param arguments The list of arguments passed to each KCM
         */
        void setKCMArguments(const QStringList& arguments);

        /**
         * Set the blacklisted component list. Any KCM that lists one
         * of the components in the given blacklist is not loaded even if it
         * would fit otherwise. This is a way to explicitly prevent loading of
         * certain KControlModules.
         *
         * Note that this function only works before showing the
         * KSettings::Dialog for the first time.
         * @param blacklist the list of components that prevent a KCM from being
         * loaded
         */
        void setComponentBlacklist(const QStringList& blacklist);

        /**
         * Tells the dialog whether the entries in the listview are all static
         * or whether it should add checkboxes to select which parts
         * of the optional functionality should be active or not.
         *
         * Note that this function only works before showing the dialog for the first time.
         *
         * Defaults to \p false.
         *
         * @param allowSelection \p true The user can select what functionality he wants.
         * @param allowSelection \p false While running no entries are added or deleted
         */
        void setAllowComponentSelection(bool allowSelection);

        bool allowComponentSelection() const;

        /**
         * Returns a list of all KPluginInfo objects the dialog uses.
         */
        QList<KPluginInfo> pluginInfos() const;

    protected:
        /**
         * Reimplemented to lazy create the dialog on first show.
         */
        void showEvent(QShowEvent *);

    Q_SIGNALS:
        /**
         * If you use the dialog in Configurable mode and want to be notified
         * when the user changes the plugin selections use this signal. It's
         * emitted if the selection has changed and the user pressed Apply or
         * Ok. In the slot you would then load and unload the plugins as
         * requested.
         */
        void pluginSelectionChanged();

    private:
        //Q_PRIVATE_SLOT(d_func(), void _k_configureTree())
        Q_PRIVATE_SLOT(d_func(), void _k_updateEnabledState(bool))
        Q_PRIVATE_SLOT(d_func(), void _k_syncConfiguration())
        Q_PRIVATE_SLOT(d_func(), void _k_reparseConfiguration(const QByteArray &))
};

}

#endif // KSETTINGS_DIALOG_H
