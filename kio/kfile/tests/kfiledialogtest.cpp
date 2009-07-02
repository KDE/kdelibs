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

    // Test for: saved location keyword.
    //     - Should return to the starting directory last used for this test.
    //     - Should have no entered file name.
    KFileDialog keywordDlg(KUrl("kfiledialog:///testKeyword"), QString("*.*|"), 0);
    keywordDlg.setMode(KFile::Files);
    keywordDlg.setCaption(QString("Test for keyword with no file name"));
    keywordDlg.exec();

    // Test for: saved location keyword with file name.
    //     - Should return to the starting directory last used for this test.
    //     - Should enter the file name 'new.file'.
    KFileDialog keywordDlg2(KUrl("kfiledialog:///testKeyword/new.file"), QString("*.*|"), 0);
    keywordDlg2.setMode(KFile::Files);
    keywordDlg2.setCaption(QString("Test for keyword and file name"));
    keywordDlg2.exec();

    // bug 173137
    KFileDialog dlg(KUrl(QString()), QString("*.*|"), 0);
    dlg.setMode(KFile::Files | KFile::Directory);
    dlg.setCaption(QString("Test for bug 173137"));
    dlg.exec();
    KUrl::List selectedUrls = dlg.selectedUrls();
    if (selectedUrls.count()) {
        QString str("The listed files and folders below were asked to be opened:\n");
        foreach (const KUrl &filename, selectedUrls) {
            str += QString("\n%1").arg(filename.url());
        }
        KMessageBox::information(0, str, "Dialog for bug #173137 accepted");
    } else {
        KMessageBox::information(0, QString("Dialog for bug #173137 cancelled"));
    }
    // end bug 173137

    // Note: when I talk about 'filename' I mean also with path. For instance, a filename on this
    // context is 'foo.txt', but also '/home/foo/bar/bar.txt'.

    // Test for: getOpenFileName.
    //     - Should return the selected file (if any).
    //     - Should return an empty string if 'Cancel' was pressed.
    //     - Should NOT return a non existing filename. If a non existing filename was given to it,
    //       it should inform the user about it, so we always get an empty string or an existing
    //       filename.
    QString getOpenFileName = KFileDialog::getOpenFileName(QString(),QString(),0,
                                                           QString("Test getOpenFileName"));

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
    QStringList getOpenFileNames = KFileDialog::getOpenFileNames(QString(),QString(),0,
                                                                 QString("Test getOpenFileNames"));
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
    KUrl getOpenUrl = KFileDialog::getOpenUrl(QString(),QString(),0,
                                              QString("Test getOpenUrl"));
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
    KUrl::List getOpenUrls = KFileDialog::getOpenUrls(QString(),QString(),0,
                                                      QString("Test getOpenUrls"));
    if (getOpenUrls.count()) {
        QString str("The listed files below were asked to be opened:\n");
        foreach (const KUrl &filename, getOpenUrls) {
            str += QString("\n%1").arg(filename.url());
        }
        KMessageBox::information(0, str, "Dialog for 'getOpenUrls' accepted");
    } else {
        KMessageBox::information(0, QString("Dialog for 'getOpenUrls' cancelled"));
    }

    // Test for: getSaveFileName.
    //     - Should return the selected file (if any).
    //     - Should return an empty string if 'Cancel' was pressed.
    //     - Don't care about existing or non existing filenames.
    QString getSaveFileName = KFileDialog::getSaveFileName(QString(),QString(),0,
                                                           QString("Test getSaveFileName"));
    if (!getSaveFileName.isEmpty()) {
        KMessageBox::information(0, QString("\"%1\" file was asked to be saved").arg(getSaveFileName), "Dialog for 'getSaveFileName' accepted");
    } else {
        KMessageBox::information(0, QString("Dialog for 'getSaveFileName' cancelled"));
    }

    // Tests for bug 194900
    //     - Should enter the specified directory with the file preselected.
    getSaveFileName = KFileDialog::getSaveFileName(KUrl("/usr/share/X11/rgb.txt"),
                                                   QString(),0,
                                                   QString("Test bug 194900 getSaveFileName with file preselected"));
    if (!getSaveFileName.isEmpty()) {
        KMessageBox::information(0, QString("\"%1\" file was asked to be saved").arg(getSaveFileName), "Dialog for 'getSaveFileName' accepted");
    } else {
        KMessageBox::information(0, QString("Dialog for 'getSaveFileName' cancelled"));
    }
    //     - Should enter the specified directory with no file preselected.
    getSaveFileName = KFileDialog::getSaveFileName(KUrl("/usr/share/X11"),
                                                   QString(),0,
                                                   QString("Test bug 194900 getSaveFileName with no file preselected"));
    if (!getSaveFileName.isEmpty()) {
        KMessageBox::information(0, QString("\"%1\" file was asked to be saved").arg(getSaveFileName), "Dialog for 'getSaveFileName' accepted");
    } else {
        KMessageBox::information(0, QString("Dialog for 'getSaveFileName' cancelled"));
    }

    // Test for: getSaveUrl.
    //     - Is a convenience method for getSaveFileName, that returns a KUrl object instead of a
    //       QString object.
    //     - From the previous point it is expectable that its behavior should be the same as
    //       getSaveFileName.
    KUrl getSaveUrl = KFileDialog::getSaveUrl(QString(),QString(),0,
                                              QString("Test getSaveUrl"));
    if (getSaveUrl.isValid()) {
        KMessageBox::information(0, QString("\"%1\" file was asked to be saved").arg(getSaveUrl.url()), "Dialog for 'getSaveUrl' accepted");
    } else {
        KMessageBox::information(0, QString("Dialog for 'getSaveUrl' cancelled"));
    }

    // Tests for bug 194900
    //     - Should enter the specified directory with the file preselected.
    getSaveUrl = KFileDialog::getSaveUrl(KUrl("/usr/share/X11/rgb.txt"),
                                         QString(),0,
                                         QString("Test bug 194900 getSaveUrl with file preselected"));
    if (getSaveUrl.isValid()) {
        KMessageBox::information(0, QString("\"%1\" file was asked to be saved").arg(getSaveUrl.url()), "Dialog for 'getSaveUrl' accepted");
    } else {
        KMessageBox::information(0, QString("Dialog for 'getSaveUrl' cancelled"));
    }
    //     - Should enter the specified directory with no file preselected.
    getSaveUrl = KFileDialog::getSaveUrl(KUrl("/usr/share/X11/"),
                                         QString(),0,
                                         QString("Test bug 194900 getSaveUrl with no file preselected"));
    if (getSaveUrl.isValid()) {
        KMessageBox::information(0, QString("\"%1\" file was asked to be saved").arg(getSaveUrl.url()), "Dialog for 'getSaveUrl' accepted");
    } else {
        KMessageBox::information(0, QString("Dialog for 'getSaveUrl' cancelled"));
    }

    // Test for: getImageOpenUrl.
    //     - Is the same as getOpenUrl but showing inline previews.
    KUrl getImageOpenUrl = KFileDialog::getImageOpenUrl(QString(),0,
                                                        QString("Test getImageOpenUrl"));
    if (getImageOpenUrl.isValid()) {
        KMessageBox::information(0, QString("\"%1\" file was asked to be saved").arg(getImageOpenUrl.url()), "Dialog for 'getImageOpenUrl' accepted");
    } else {
        KMessageBox::information(0, QString("Dialog for 'getImageOpenUrl' cancelled"));
    }

    return 0;
}
