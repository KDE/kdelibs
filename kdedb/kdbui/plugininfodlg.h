/*
   This file is part of the KDB libraries
   Copyright (c) 2000 Praduroux Alessandro <pradu@thekompany.com>
 
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/     
#ifndef PLUGININFODLG_H
#define PLUGININFODLG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kdialogbase.h>

class QListView;

namespace KDB {

class Plugin;

/**
 * Plugin information dialog.
 *
 * This simple dialog presents all the informations related to the
 * plugin that is given to the constructor.
 * There is not much you can do with this dialog apart looking at the info
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @author Michael Koch <m_kock@bigfoot.de>
 * @version kdbui 0.0.2
 */
class PluginInfoDialog: public KDialogBase {

    Q_OBJECT

public:

    PluginInfoDialog(Plugin *plugin, QWidget *parent = 0L);

    ~PluginInfoDialog();

    /*
     * Sets the plugin which infomrations schould be shown.
     */
    void setPlugin( Plugin *plugin );
    
    /*
     * You only need to use this method. It shows the informations of
     * a plugin in a modal dialog. It does all the work for you.
     */
    static void showInfo( Plugin *plugin, QWidget *parent = 0L);

private:

    QListView *m_listview;
};

}
#endif

