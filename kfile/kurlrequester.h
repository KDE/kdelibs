/* This file is part of the KDE libraries
    Copyright (C) 1999,2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>

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


#ifndef KURLREQUESTER_H
#define KURLREQUESTER_H

#include <qhbox.h>

#include <kfile.h>
#include <kpushbutton.h>
#include <kurl.h>

class KComboBox;
class KFileDialog;
class KLineEdit;
class KURLCompletion;

class QString;
class QTimer;

/**
 * This class is a widget showing a lineedit and a button, which invokes a
 * filedialog. File completion is available in the lineedit.
 *
 * The defaults for the filedialog are to ask for one existing local file, i.e.
 * KFileDialog::setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly )
 * The default filter is "*", i.e. show all files, and the start directory is
 * the current working directory, or the last directory where a file has been
 * selected.
 *
 * You can change this behavior by obtaining a pointer to the dialog
 * (@ref fileDialog()) and calling the appropriate methods.
 *
 * @short A widget to request a filename/url from the user
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KURLRequester : public QHBox
{
    Q_OBJECT

public:
    /**
     * Constructs a KURLRequester widget
     * @param modal specifies whether the filedialog should be opened as modal
     * or not.
     */
    KURLRequester( QWidget *parent=0, const char *name=0, bool modal = true );

    /**
     * Constructs a KURLRequester widget with the default URL @p url
     * @param modal specifies whether the filedialog should be opened as modal
     * or not.
     */
    KURLRequester( const QString& url, QWidget *parent=0, const char *name=0,
		   bool modal = true );

    /**
     * Special constructor, which creates a KURLRequester widget with a custom
     * edit-widget. The edit-widget can be either a KComboBox or a KLineEdit
     * (or inherited thereof). Note: for geometry management reasons, the
     * edit-widget is reparented to have the KURLRequester as parent.
     * @param modal specifies whether the filedialog should be opened as modal
     * or not.
     */
    KURLRequester( QWidget *editWidget, QWidget *parent, const char *name=0,
		   bool modal = true );

    /**
     * Destructs the KURLRequester
     */
    ~KURLRequester();

    /**
     * @returns the current url in the lineedit. May be malformed, if the user
     * entered something weird. ~user or environment variables are substituted
     * for local files.
     */
    QString url() const;

    /**
     * Enables/disables showing file:/ in the lineedit, when a local file has
     * been selected in the filedialog or was set via @ref setURL().
     * Default is false, not showing file:/
     * @see #showLocalProtocol
     */
    void setShowLocalProtocol( bool b );

    /**
     * @returns whether local files will be prefixed with file:/ in the
     * lineedit
     * @see #setShowLocalProtocol
     */
    bool showLocalProtocol() const { return myShowLocalProt; }

    /**
     * @returns a pointer to the filedialog
     * You can use this to customize the dialog, e.g. to specify a filter.
     * Never returns 0L.
     */
    virtual KFileDialog * fileDialog() const;

    /**
     * @returns a pointer to the lineedit, either the default one, or the
     * special one, if you used the special constructor.
     *
     * It is provided so that you can e.g. set an own completion object
     * (e.g. @ref KShellCompletion) into it.
     */
    KLineEdit * lineEdit() const;

    /**
     * @returns a pointer to the combobox, in case you have set one using the
     * special constructor. Returns 0L otherwise.
     */
    KComboBox * comboBox() const;

    /**
     * @returns a pointer to the pushbutton. It is provided so that you can
     * specify an own pixmap or a text, if you really need to.
     */
    QPushButton * button() const { return myButton; }

    /**
     * @returns the KURLCompletion object used in the lineedit/combobox.
     */
    KURLCompletion *completionObject() const { return myCompletion; }

public slots:
    /**
     * Sets the url in the lineedit to @p url. Depending on the state of
     * @ref showLocalProtocol(), file:/ on local files will be shown or not.
     */
    void setURL(const QString& url);

    /**
     * Clears the lineedit/combobox.
     */
    void clear();

signals:
    // forwards from LineEdit
    /**
     * Emitted when the text in the lineedit changes.
     * The parameter contains the contents of the lineedit.
     */
    void textChanged(const QString&);

    /**
     * Emitted when return or enter was pressed in the lineedit.
     */
    void returnPressed();

    /**
     * Emitted when return or enter was pressed in the lineedit.
     * The parameter contains the contents of the lineedit.
     */
    void returnPressed( const QString& );

    /**
     * Emitted before the filedialog is going to open. Connect
     * to this signal to "configure" the filedialog, e.g. set the
     * filefilter, the mode, a preview-widget, etc. It's usually
     * not necessary to set a URL for the filedialog, as it will
     * get set properly from the editfield contents.
     *
     * If you use multiple KURLRequesters, you can connect all of them
     * to the same slot and use the given KURLRequester pointer to know
     * which one is going to open.
     */
    void openFileDialog( KURLRequester * );

    /**
     * Emitted when the user changed the URL via the file dialog.
     * The parameter contains the contents of the lineedit.
     */
    void urlSelected( const QString& );

protected:
    void		init();

    QPushButton *	myButton; // FIXME: make it private KURLDragPushButton
    KURLCompletion *    myCompletion;
    bool 		myModal;


private:
    bool 		myShowLocalProt;
    mutable KFileDialog * myFileDialog;


protected slots:
    /**
     * Called when the button is pressed to open the filedialog.
     * Also called when @ref KStdAccel::Open (default is Ctrl-O) is pressed.
     */
    void slotOpenDialog();

private slots:
    void slotUpdateURL();

private:
    class KURLRequesterPrivate;
    KURLRequesterPrivate *d;

};

#endif // KURLREQUESTER_H
