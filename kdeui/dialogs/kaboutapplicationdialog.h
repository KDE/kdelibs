/* This file is part of the KDE libraries
   Copyright (C) 2007 Urs Wolfer <uwolfer at kde.org>

   Parts of this class have been take from the KAboutApplication class, which was
   Copyright (C) 2000 Waldo Bastian (bastian@kde.org) and Espen Sand (espen@kde.org)

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

#ifndef KABOUT_APPLICATION_DIALOG_H
#define KABOUT_APPLICATION_DIALOG_H

#include <kdialog.h>

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
 * \image html kaboutapplicationdialog.png "KDE About Application Dialog"
 *
 * @author Urs Wolfer uwolfer @ kde.org
 */

class KDEUI_EXPORT KAboutApplicationDialog : public KDialog
{
    Q_OBJECT

    public:
        /**
         * Constructor. Creates a fully featured "About Application" dialog box.
         *
         * @param aboutData A pointer to a KAboutData object which data
         *        will be used for filling the dialog.
         * @param parent The parent of the dialog box. You should use the
         *        toplevel window so that the dialog becomes centered.
         */
        explicit KAboutApplicationDialog(const KAboutData *aboutData, QWidget *parent = 0);

        virtual ~KAboutApplicationDialog();

    private:
        class Private;
        Private* const d;

        Q_PRIVATE_SLOT( d, void _k_showLicense(const QString&) )

        Q_DISABLE_COPY( KAboutApplicationDialog )
};

#endif
