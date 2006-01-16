//
/* This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
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
#ifndef __open_with_h__
#define __open_with_h__

#include <qdialog.h>

#include <kurl.h>
#include <krun.h>
#include <kservice.h>

class KApplicationTree;
class KURLRequester;

class QWidget;
class QCheckBox;
class QPushButton;
class QLabel;

class KOpenWithDlgPrivate;

/* ------------------------------------------------------------------------- */
/**
 * "Open with" dialog box.
 * Used automatically by KRun, and used by libkonq.
 *
 * @author David Faure <faure@kde.org>
 */
class KIO_EXPORT KOpenWithDlg : public QDialog //#TODO: Use KDialogBase for KDE4
{
    Q_OBJECT
public:

    /**
     * Create a dialog that asks for a application to open a given
     * URL(s) with.
     *
     * @param urls   the URLs that should be opened. The list can be empty,
     * if the dialog is used to choose an application but not for some particular URLs.
     * @param parent parent widget
     */
    KOpenWithDlg( const KURL::List& urls, QWidget *parent = 0L );

    /**
     * Create a dialog that asks for a application to open a given
     * URL(s) with.
     *
     * @param urls   is the URL that should be opened
     * @param text   appears as a label on top of the entry box.
     * @param value  is the initial value of the line
     * @param parent parent widget
     */
    KOpenWithDlg( const KURL::List& urls, const QString& text, const QString& value,
                  QWidget *parent = 0L );

    /**
     * Create a dialog to select a service for a given service type.
     * Note that this dialog doesn't apply to URLs.
     *
     * @param serviceType the service type we want to choose an application for.
     * @param value  is the initial value of the line
     * @param parent parent widget
     */
    KOpenWithDlg( const QString& serviceType, const QString& value,
                  QWidget *parent = 0L );

    /**
     * Create a dialog to select an application
     * Note that this dialog doesn't apply to URLs.
     *
     * @param parent parent widget
     * @since 3.1
     */
    KOpenWithDlg( QWidget *parent = 0L );

    /**
     * Destructor
     */
    ~KOpenWithDlg();

    /**
     * @return the text the user entered
     */
    QString text() const;
    /**
     * Hide the "Do not &close when command exits" Checkbox
     */
    void hideNoCloseOnExit();
    /**
     * Hide the "Run in &terminal" Checkbox
     */
    void hideRunInTerminal();
    /**
     * @return the chosen service in the application tree
     * Can be null, if the user typed some text and didn't select a service.
     */
    KService::Ptr service() const { return m_pService; }
    /**
     * Set whether a new .desktop file should be created if the user selects an
     * application for which no corresponding .desktop file can be found.
     *
     * Regardless of this setting a new .desktop file may still be created if
     * the user has chosen to remember the file association.
     *
     * The default is false: no .desktop files are created.
     * @since 3.2
     */
    void setSaveNewApplications(bool b);

public slots:
    /**
    * The slot for clearing the edit widget
    */
    void slotClear();
    void slotSelected( const QString&_name, const QString& _exec );
    void slotHighlighted( const QString& _name, const QString& _exec );
    void slotTextChanged();
    void slotTerminalToggled(bool);
    void slotDbClick();
    void slotOK();

protected slots:
    /**
     * Reimplemented from QDialog::accept() to save history of the combobox
     */
    virtual void accept();

protected:

    /**
     * Determine service type from URLs
     */
    void setServiceType( const KURL::List& _urls );

    /**
     * Create a dialog that asks for a application to open a given
     * URL(s) with.
     *
     * @param text   appears as a label on top of the entry box.
     * @param value  is the initial value of the line
     */
    void init( const QString& text, const QString& value );

    KURLRequester * edit;
    QString m_command;

    KApplicationTree* m_pTree;
    QLabel *label;

    QString qName, qServiceType;
    bool m_terminaldirty;
    QCheckBox   *terminal, *remember, *nocloseonexit;
    QPushButton *UNUSED;
    QPushButton *UNUSED2;

    KService::Ptr m_pService;

    KOpenWithDlgPrivate* const d;
};

#endif
