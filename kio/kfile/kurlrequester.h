/* This file is part of the KDE libraries
    Copyright (C) 1999,2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

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

#include <keditlistbox.h>
#include <kfile.h>
#include <kpushbutton.h>
#include <kurl.h>

class KComboBox;
class KFileDialog;
class KLineEdit;
class KURLCompletion;
class KURLDragPushButton;

class QString;
class QTimer;

/**
 * This class is a widget showing a lineedit and a button, which invokes a
 * filedialog. File name completion is available in the lineedit.
 *
 * The defaults for the filedialog are to ask for one existing local file, i.e.
 * KFileDialog::setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly )
 * The default filter is "*", i.e. show all files, and the start directory is
 * the current working directory, or the last directory where a file has been
 * selected.
 *
 * You can change this behavior by using @ref setMode() or @ref setFilter().
 *
 * @short A widget to request a filename/url from the user
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KURLRequester : public QHBox
{
    Q_OBJECT

public:
    /**
     * Constructs a KURLRequester widget.
     */
    KURLRequester( QWidget *parent=0, const char *name=0 );

    /**
     * Constructs a KURLRequester widget with the initial URL @p url.
     */
    KURLRequester( const QString& url, QWidget *parent=0, const char *name=0 );
    /**
     * Special constructor, which creates a KURLRequester widget with a custom
     * edit-widget. The edit-widget can be either a KComboBox or a KLineEdit
     * (or inherited thereof). Note: for geometry management reasons, the
     * edit-widget is reparented to have the KURLRequester as parent.
     * @param modal specifies whether the filedialog should be opened as modal
     * or not.
     */
    KURLRequester( QWidget *editWidget, QWidget *parent, const char *name=0 );
    /**
     * Destructs the KURLRequester.
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
     * Sets the mode of the file dialog.
     * Note: you can only select one file with the filedialog,
     * so KFile::Files doesn't make much sense.
     * @see KFileDialog::setMode()
     */
    void setMode( unsigned int m );

    /**
     * Sets the filter for the file dialog.
     * @see KFileDialog::setFilter()
     */
    void setFilter( const QString& filter );

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
    KPushButton * button() const;

    /**
     * @returns the KURLCompletion object used in the lineedit/combobox.
     */
    KURLCompletion *completionObject() const { return myCompletion; }

    /**
     * @returns an object, suitable for use with KEditListBox. It allows you
     * to put this KURLRequester into a KEditListBox.
     * Basically, do it like this:
     * <pre>
     * KURLRequester *req = new KURLRequester( someWidget );
     * [...]
     * KEditListBox *editListBox = new KEditListBox( i18n("Some Title"), req->customEditor(), someWidget );
     * </pre>
     * @since 3.1
     */
    KEditListBox::CustomEditor customEditor();

public slots:
    /**
     * Sets the url in the lineedit to @p url. Depending on the state of
     * @ref showLocalProtocol(), file:/ on local files will be shown or not.
     * @since 3.1
     */
    void setURL( const QString& url );

    /**
     * @reimp
     * Sets the caption of the file dialog.
     * @since 3.1
     */
    virtual void setCaption( const QString& caption );

    /**
     * Clears the lineedit/combobox.
     */
    void clear();

signals:
    // forwards from LineEdit
    /**
     * Emitted when the text in the lineedit changes.
     * The parameter contains the contents of the lineedit.
     * @since 3.1
     */
    void textChanged( const QString& );

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

    KURLCompletion *    myCompletion;


private:
    KURLDragPushButton * myButton;
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

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KURLRequesterPrivate;
    KURLRequesterPrivate *d;
};

#endif // KURLREQUESTER_H
