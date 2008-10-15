/**
 * This file is part of the KDE libraries
 * Copyright 2008 Rafael Fernández López <ereslibre@kde.org>
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
 */

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

int main (int argc, char **argv)
{
    KAboutData aboutData("kfiledialogtest",
                         0,
                         ki18n("kfiledialogtest"),
                         "0.1",
                         ki18n("kfiledialogtest"),
                         KAboutData::License_LGPL,
                         ki18n("(c) 2008 Rafael Fernández López"),
                         ki18n("kfiledialogtest"),
                         "http://www.kde.org/");

    KCmdLineArgs::init( argc, argv, &aboutData );
    KApplication app;

    // Note: when I talk about 'filename' I mean also with path. For instance, a filename on this
    // context is 'foo.txt', but also '/home/foo/bar/bar.txt'.

    // Test for: getOpenFileName.
    //     - Should return the selected file (if any).
    //     - Should return an empty string if 'Cancel' was pressed.
    //     - Should NOT return a non existing filename. If a non existing filename was given to it,
    //       it should inform the user about it, so we always get an empty string or an existing
    //       filename.
    QString getOpenFileName = KFileDialog::getOpenFileName();
    if (!getOpenFileName.isEmpty()) {
        KMessageBox::information(0, QString("\"%1\" file was opened").arg(getOpenFileName), "Dialog for 'getOpenFileName' accepted");
    } else {
        KMessageBox::information(0, QString("Dialog for 'getOpenFileName' cancelled"));
    }

    // Test for: getOpenFileNames.
    //     - Should return the selected files (if any).
    //     - Should return an empty list of strings if 'Cancel' was pressed.
    //     - Should NOT return a non existing filename in the list. If a non existing filename was
    //       given to it, it should inform the user about it, so we always get an empty string or an
    //       existing list of filenames.
    QStringList getOpenFileNames = KFileDialog::getOpenFileNames();
    if (getOpenFileNames.count()) {
        QString str("The listed files below were asked to be opened:\n");
        foreach (const QString &filename, getOpenFileNames) {
            str += QString("\n%1").arg(filename);
        }
        KMessageBox::information(0, str, "Dialog for 'getOpenFileNames' accepted");
    } else {
        KMessageBox::information(0, QString("Dialog for 'getOpenFileNames' cancelled"));
    }

    // Test for: getOpenUrl.
    //     - Is a convenience method for getOpenFileName, that returns a KUrl object instead of a
    //       QString object.
    //     - From the previous point it is expectable that its behavior should be the same as
    //       getOpenFileName.
    KUrl getOpenUrl = KFileDialog::getOpenUrl();
    if (getOpenUrl.isValid()) {
        KMessageBox::information(0, QString("\"%1\" file was opened").arg(getOpenUrl.url()), "Dialog for 'getOpenUrl' accepted");
    } else {
        KMessageBox::information(0, QString("Dialog for 'getOpenUrl' cancelled"));
    }

    // Test for: getOpenUrls.
    //     - Is a convenience method for getOpenFileNames, that returns a KUrl::List object instead
    //       of a QStringList object.
    //     - From the previous point it is expectable that its behavior should be the same as
    //       getOpenFileNames.
    KUrl::List getOpenUrls = KFileDialog::getOpenUrls();
    if (getOpenUrls.count()) {
        QString str("The listed files below were asked to be opened:\n");
        foreach (const KUrl &filename, getOpenUrls) {
            str += QString("\n%1").arg(filename.url());
        }
        KMessageBox::information(0, str, "Dialog for 'getOpenUrls' accepted");
    } else {
        KMessageBox::information(0, QString("Dialog for 'getOpenUrls' cancelled"));
    }

    return 0;
}
