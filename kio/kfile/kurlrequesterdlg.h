/* This file is part of the KDE libraries
    Copyright (C) 2000 Wilco Greven <greven@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#ifndef KURLREQUESTERDIALOG_H
#define KURLREQUESTERDIALOG_H

#include <kdialogbase.h>
#include <kurl.h>

class KUrlCompletion;
class KUrlRequester;
class KFileDialog;
/**
 * Dialog in which a user can enter a filename or url. It is a dialog
 * encapsulating KUrlRequester. The API is derived from
 * KFileDialog.
 *
 * @short Simple dialog to enter a filename/url.
 * @author Wilco Greven <greven@kde.org>
 */
class KIO_EXPORT KUrlRequesterDlg : public KDialog
{
    Q_OBJECT

public:
    /**
     * Constructs a KUrlRequesterDlg.
     *
     * @param url    The url of the directory to start in. Use QString()
     *               to start in the current working directory, or the last
     *               directory where a file has been selected.
     * @param parent The parent object of this widget.
     */
    KUrlRequesterDlg( const QString& url, QWidget *parent);

    /**
     * Constructs a KUrlRequesterDlg.
     *
     * @param url    The url of the directory to start in. Use QString()
     *               to start in the current working directory, or the last
     *               directory where a file has been selected.
     * @param text   Text of the label
     * @param parent The parent object of this widget.
     */
    KUrlRequesterDlg( const QString& url, const QString& text,
                      QWidget *parent);
    /**
     * Destructs the dialog.
     */
    ~KUrlRequesterDlg();

    /**
     * Returns the fully qualified filename.
     */
    KUrl selectedURL() const;

    /**
     * Creates a modal dialog, executes it and returns the selected URL.
     *
     * @param url This specifies the initial path of the input line.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The caption to use for the dialog.
     */
    static KUrl getURL(const QString& url = QString(),
            QWidget *parent= 0, const QString& caption = QString());

    /**
     * Returns a pointer to the file dialog used by the KUrlRequester.
     */
    KFileDialog * fileDialog();
    /**
     * Returns a pointer to the KUrlRequester.
     */
    KUrlRequester *urlRequester();

private Q_SLOTS:
    void slotClear();
    void slotTextChanged(const QString &);
private:
    void initDialog(const QString &text, const QString &url);
    KUrlRequester *urlRequester_;

    class KUrlRequesterDlgPrivate;
    KUrlRequesterDlgPrivate *d;

};

#endif // KURLREQUESTERDIALOG_H

// vim:ts=4:sw=4:tw=78
