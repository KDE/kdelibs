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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef KSETTINGS_COMPONENTSDIALOG_H
#define KSETTINGS_COMPONENTSDIALOG_H

#include <kdialogbase.h>

class QString;
class KPluginInfo;

namespace KSettings
{

/**
  Dialog for selecting which plugins should be active for an application. Set
  the list of available plugins with \ref setPluginInfos. The dialog will save the
  configuration on clicking ok or apply to the applications config file. Connect
  to the okClicked() and applyClicked() signals to be notified about
  configuration changes.
*/
class ComponentsDialog : public KDialogBase
{
    Q_OBJECT
    public:
        /**
          Create Dialog.
          
          @param parent parent widget
          @param name   name
        */
        ComponentsDialog( QWidget * parent = 0, const char * name = 0 );
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
        void setPluginInfos( const QValueList<KPluginInfo *> &plugins );

        /**
         * @reimplemented
         */
        void show();

    protected slots:
        void slotOk();
        void slotApply();

    private slots:
        void executed( QListViewItem * );

    private:
        void savePluginInfos();

        class ComponentsDialogPrivate;
        ComponentsDialogPrivate * d;
};

}

// vim: sw=4 sts=4 et
#endif // KSETTINGS_COMPONENTSDIALOG_H
