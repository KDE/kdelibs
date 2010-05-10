/**
 * klinkdialog
 *
 * Copyright 2008  Stephen Kelly <steveire@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#ifndef KLINKDIALOG_H
#define KLINKDIALOG_H

//@cond PRIVATE

#include <kdialog.h>

class KLinkDialogPrivate;
class QString;

/**
    @short Dialog to allow user to configure a hyperlink.
    @author Stephen Kelly
    @since 4.1
    @internal

    This class provides a dialog to ask the user for a link target url and
    text.

    The size of the dialog is automatically saved to and restored from the
    global KDE config file.
 */
class KLinkDialog : public KDialog
{
    Q_OBJECT
public:
    /**
     * Create a link dialog.
     * @param parent  Parent widget.
     */
    KLinkDialog(QWidget *parent = 0);

    /**
     * Destructor
     */
    ~KLinkDialog();


    /**
     * Returns the link text shown in the dialog
     * @param linkText The initial text
     */
    void setLinkText(const QString &linkText);

    /**
     * Sets the target link url shown in the dialog
     * @param linkUrl The initial link target url
     */
    void setLinkUrl(const QString &linkUrl);

    /**
     * Returns the link text entered by the user.
     * @return The link text
     */
    QString linkText() const;

    /**
     * Returns the target link url entered by the user.
     * @return The link url
     */
    QString linkUrl() const;

private slots:
    void slotTextChanged(const QString &);

private:
    //@cond PRIVATE
    KLinkDialogPrivate *const d;
    //@endcond
};

//@endcond

#endif
