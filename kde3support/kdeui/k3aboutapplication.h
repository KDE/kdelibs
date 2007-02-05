/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Waldo Bastian (bastian@kde.org) and
 * Espen Sand (espen@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef K3ABOUT_APPLICATION_H
#define K3ABOUT_APPLICATION_H

#include <k3aboutdialog.h>

class KAboutData;

/**
 * @short Standard "About Application" dialog box.
 *
 * This class provides the standard "About Application" dialog box
 * that is used by KHelpMenu. It uses the information of the global
 * KAboutData that is specified at the start of your program in
 * main(). Normally you should not use this class directly but rather
 * the KHelpMenu class or even better just subclass your toplevel
 * window from KMainWindow. If you do the latter, the help menu and
 * thereby this dialog box is available through the
 * KMainWindow::helpMenu() function.
 *
 * @author Waldo Bastian (bastian@kde.org) and Espen Sand (espen@kde.org)
 */

class KDE3SUPPORT_EXPORT K3AboutApplication : public K3AboutDialog
{
  public:
    /**
     * Constructor. Creates a fully featured "About Application" dialog box.
     * Note that this dialog is made modeless in the KHelpMenu class so
     * the users may expect a modeless dialog.
     *
     * @param aboutData A pointer to a KAboutData object which data
     *        will be used for filling the dialog.
     * @param parent The parent of the dialog box. You should use the
     *        toplevel window so that the dialog becomes centered.
     * @param modal If false, this widget will be modeless and must be
     *        made visible using QWidget::show(). Otherwise it will be
     *        modal and must be made visible using QWidget::exec().
     */
    explicit K3AboutApplication( const KAboutData *aboutData=0, QWidget *parent=0, bool modal=true );

  private:
    class Private;
    Private* const d;
};

#endif
