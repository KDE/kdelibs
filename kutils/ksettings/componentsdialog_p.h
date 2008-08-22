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

#ifndef COMPONENTSDIALOG_P_H
#define COMPONENTSDIALOG_P_H

#include <kcmutils_export.h>
#include <kdialog.h>

#include <QtCore/QList>

class QString;
class KPluginInfo;
class QTreeWidgetItem;

namespace KSettings
{

/**
  Dialog for selecting which plugins should be active for an application. Set
  the list of available plugins with \ref setPluginInfos. The dialog will save the
  configuration on clicking ok or apply to the applications config file. Connect
  to the okClicked() and applyClicked() signals to be notified about
  configuration changes.
*/
class KCMUTILS_EXPORT ComponentsDialog : public KDialog
{
    Q_OBJECT
    public:
        /**
          Create Dialog.

          @param parent parent widget
          @param name   name
        */
        explicit ComponentsDialog( QWidget * parent = 0, const char * name = 0 );
        ~ComponentsDialog();

        /**
          Add a plugin that the dialog offers for selection.
        */
        void addPluginInfo( KPluginInfo * );
        /**
          Set list of plugins the dialog offers for selection. (Overwrites a previous list)
        */
        void setPluginInfos( const QMap<QString, KPluginInfo*> & plugininfos );
        /**
          Set list of plugins the dialog offers for selection. (Overwrites a previous list)
        */
        void setPluginInfos( const QList<KPluginInfo *> &plugins );

        /**
         * reimplemented
         */
        void show();

    protected Q_SLOTS:
        void slotOk();
        void slotApply();

    private Q_SLOTS:
        void executed( QTreeWidgetItem *, int );

    private:
        void savePluginInfos();

        class ComponentsDialogPrivate;
        ComponentsDialogPrivate* const d;
};

}

#endif // COMPONENTSDIALOG_P_H
