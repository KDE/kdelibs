/* This file is part of the KDE libraries
    Copyright (C) 2000 Wilco Greven <j.w.greven@student.utwente.nl>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/


#ifndef KURLREQUESTERDIALOG_H
#define KURLREQUESTERDIALOG_H

#include <kdialogbase.h>
#include <kurl.h>

class KURLCompletion;
class KURLRequester;

/**
 * Dialog in which a user can enter a filename or url. It is a dialog 
 * encapsulating @ref KURLRequester. The API is derived from 
 * @ref KFileDialog. 
 *
 * @short Simple dialog to enter a filename/url.
 * @author Wilco Greven <j.w.greven@student.utwente.nl>
 */
class KURLRequesterDlg : public KDialogBase
{
    Q_OBJECT

public:
    /**
     * Constructs a KURLRequesterDlg
     * 
     * @param urlName   The url of the directory to start in. Use QString::null
     *                  to start in the current working directory, or the last
     *                  directory where a file has been selected.
     * @param modal     Specifies whether the dialog should be opened as modal
     *                  or not.
     */
    KURLRequesterDlg( const QString& urlName, QWidget *parent, 
            const char *name, bool modal = true );
    
    /**
     * Destructs the dialog
     */
    ~KURLRequesterDlg();

    /**
     * Retrieve the fully qualified filename.
     */
    KURL selectedURL() const;
    
    /**
     * Create a modal dialog and return the selected URL.
     * 
     * @param url This specifies the initial path of the input line.
     * @param parent The widget the dialog will be centered on initially.
     */
    static KURL getURL(const QString& url = QString::null,
            QWidget *parent= 0, const QString& caption = QString::null);

protected slots:
    void slotClear();

protected:

    KURLCompletion *    urlCompletion_;
    KURLRequester *     urlRequester_;

private:
    class KURLRequesterDlgPrivate;
    KURLRequesterDlgPrivate *d;

};

#endif // KURLREQUESTERDIALOG_H

// vim:ts=4:sw=4:tw=78
