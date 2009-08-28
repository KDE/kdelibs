/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

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

#ifndef KSETTINGS_DIALOG_P_H
#define KSETTINGS_DIALOG_P_H

#include "dialog.h"
#include "../kcmultidialog_p.h"
#include "../kcmoduleinfo.h"

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtGui/QLabel>
#include <QtGui/QWidget>

#include <kpagewidgetmodel.h>
#include <kservice.h>
#include <kplugininfo.h>
#include <kvbox.h>

class QCheckBox;

namespace KSettings
{

class DialogPrivate : public KCMultiDialogPrivate
{
    friend class PageNode;
    Q_DECLARE_PUBLIC(Dialog)
    protected:
        DialogPrivate();

        QHash<QString, KPageWidgetItem *> pageItemForGroupId;
        QHash<KPageWidgetItem *, KPluginInfo> pluginForItem;
        QHash<KPageWidgetItem *, QCheckBox *> checkBoxForItem;
        KPluginInfo::List plugininfos;

        QStringList registeredComponents;
        QSet<KCModuleInfo> kcmInfos;
        QStringList componentBlacklist;
        QStringList arguments;
        QStringList components;

        bool staticlistview : 1;
        bool firstshow : 1;
        quint32 pluginStateDirty : 30;

        //void _k_configureTree();
        void _k_updateEnabledState(bool);
        void _k_syncConfiguration();
        void _k_reparseConfiguration(const QByteArray &a);
        virtual void _k_clientChanged();

        KPageWidgetItem *createPageItem(KPageWidgetItem *parentItem,
                                        const QString &name, const QString &comment,
                                        const QString &iconName, int weight);

        void connectItemCheckBox(KPageWidgetItem *item, const KPluginInfo &pinfo,
                            bool isEnabled);

    private:
        /**
         * @internal
         * Check whether the plugin associated with this KCM is enabled.
         */
        bool isPluginForKCMEnabled(const KCModuleInfo *moduleinfo, KPluginInfo &pinfo) const;
        bool isPluginImmutable(const KPluginInfo &pinfo) const;

        QSet<KCModuleInfo> instanceServices();
        QSet<KCModuleInfo> parentComponentsServices(const QStringList &);

        /**
         * @internal
         * Read the .setdlg file and add it to the groupmap
         */
        void parseGroupFile(const QString &);

        /**
         * @internal
         * If this module is put into a TreeList hierarchy this will return a
         * list of the names of the parent modules.
         */
        //QStringList parentModuleNames(KCModuleInfo *);

        /**
         * @internal
         * This method is called only once. The KCMultiDialog is not created
         * until it's really needed. So if some method needs to access d->dlg it
         * checks for 0 and if it's not created this method will do it.
         */
        void createDialogFromServices();
};

} // namespace KSettings
#endif // KSETTINGS_DIALOG_P_H
