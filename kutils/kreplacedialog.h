/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>.
    Copyright (C) 2002, David Faure <david@mandrakesoft.com>
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
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

#ifndef KREPLACEDIALOG_H
#define KREPLACEDIALOG_H

#include "kfinddialog.h"

class KHistoryCombo;
class QCheckBox;
class QGroupBox;
class QLabel;
class QPopupMenu;
class QPushButton;
class QRect;

/**
 * @short A generic "replace" dialog.
 *
 * @author S.R.Haque <srhaque@iee.org>
 *
 * @sect Detail
 *
 * This widget inherits from @ref KFindDialog and implements
 * the following additional functionalities:  a replacement string
 * object and an area for a user-defined widget to extend the dialog.
 *
 * @sect Example
 *
 * To use the basic replace dialog:
 *
 * <pre>
 * </pre>
 *
 * To use your own extensions:
 *
 * <pre>
 * </pre>
 */
class KReplaceDialog:
    public KFindDialog
{
    Q_OBJECT

public:

    // Options.

    enum Options
    {
        // Should the user be prompted before the replace operation?
        PromptOnReplace = 256,
        BackReference = 512
    };

    /**
     * Construct a replace dialog.read-only or rather select-only combo box with a
     * parent object and a name.
     *
     * @param parent The parent object of this widget
     * @param name The name of this widget
     * @param options A bitfield of the @ref Options to be enabled.
     * @param hasSelection Whether a selection exists
     */
    KReplaceDialog( QWidget *parent = 0, const char *name = 0, long options = 0,
                     const QStringList &findStrings = QStringList(), const QStringList &replaceStrings = QStringList(),
                     bool hasSelection = true );

    /**
     * Destructor.
     */
    virtual ~KReplaceDialog();

    /**
     * Provide the list of @p strings to be displayed as the history
     * of replacement strings. @p strings might get truncated if it is
     * too long.
     *
     * @param history The replacement history.
     * @see #replacementHistory
     */
    void setReplacementHistory( const QStringList &strings );

    /**
     * Returns the list of history items.
     *
     * @see #setReplacementHistory
     */
    QStringList replacementHistory() const;

    /**
     * Set the options which are enabled.
     *
     * @param options The setting of the @ref Options.
     */
    void setOptions( long options );

    /**
     * Returns the state of the options. Disabled options may be returned in
     * an indeterminate state.
     *
     * @see #setOptions
     */
    long options() const;

    /**
     * Returns the replacement string.
     */
    QString replacement() const;

    /**
     * Returns an empty widget which the user may fill with additional UI
     * elements as required. The widget occupies the width of the dialog,
     * and is positioned immediately the regular expression support widgets
     * for the replacement string.
     */
    QWidget *replaceExtension();

protected slots:

    void slotOk();
    virtual void showEvent ( QShowEvent * );

private:

    // Binary compatible extensibility.
    class KReplaceDialogPrivate;
    KReplaceDialogPrivate *d;
};


#endif // KREPLACEDIALOG_H
