/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Espen Sand (espen@kde.org)
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

#ifndef _KABOUT_KDE_H_
#define _KABOUT_KDE_H_

#include <kaboutdialog.h>

/**
 * @short Standard "About KDE" dialog box,
 *
 * This class provides the standard "About KDE" dialog box that is used
 * KHelpMenu Normally you should not use this class directly but
 * rather the KHelpMenu class or even better just subclass your
 * toplevel window from KMainWindow. If you do the latter, the help
 * menu and thereby this dialog box is available through the
 * KMainWindow::helpMenu() function.
 *
 * @author Espen Sand (espen@kde.org)
 */

class KDEUI_EXPORT KAboutKDE : public KAboutDialog
{
  public:
    /**
     * Constructor. Creates a fully featured "About KDE" dialog box.
     * Note that this dialog is made modeless in the KHelpMenu class so
     * the users may expect a modeless dialog.
     *
     * @param parent The parent of the dialog box. You should use the
     *        toplevel window so that the dialog becomes centered.
     * @param name Internal name of the widget. This name in not used in the
     *        caption.
     * @param modal If false, this widget will be modeless and must be
     *        made visible using QWidget::show(). Otherwise it will be
     *        modal and must be made visible using QWidget::exec()
     */
    KAboutKDE( QWidget *parent=0, const char *name=0, bool modal=true );
};


#endif


