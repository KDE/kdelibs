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

#ifndef KFINDDIALOG_H
#define KFINDDIALOG_H

#include <kdialogbase.h>
class KHistoryCombo;
class QPushButton;
class QPopupMenu;
class QGridLayout;
class QLabel;
class QGroupBox;
class QCheckBox;

/**
 * @short A generic "find" dialog.
 *
 * @author S.R.Haque <srhaque@iee.org>
 *
 * @sect Detail
 *
 * This widget inherits from @ref KDialogBase and implements
 * the following additional functionalities:  a find string
 * object and an area for a user-defined widget to extend the dialog.
 *
 * @sect Example
 *
 * To use the basic find dialog:
 *
 * <pre>
 *  KFindDialog dlg(....)
 *  if ( dlg.exec() != QDialog::Accepted )
 *      return;
 *
 *  // proceed with KFind from here
 * </pre>
 *
 * To use your own extensions:
 *
 * <pre>
 * </pre>
 */
class KFindDialog:
    public KDialogBase
{
    Q_OBJECT

public:

    // Options.

    enum Options
    {
        WholeWordsOnly = 1,     // Match whole words only.
        FromCursor = 2,         // Start from current cursor position.
        SelectedText = 4,       // Only search selected area.
        CaseSensitive = 8,      // Consider case when matching.
        FindBackwards = 16,     // Go backwards.
        RegularExpression = 32, // Interpret the pattern as a regular expression.
        // User extensions can use boolean options above this value.
        MinimumUserOption = 65536
    };

    /**
     * Construct a find dialog with a parent object and a name.
     *
     * @param parent The parent object of this widget.
     * @param name The name of this widget.
     * @param options A bitfield of the @ref Options to be enabled.
     * @param findStrings The find history, see @ref findHistory()
     * @param hasSelection Whether a selection exists
     */
    KFindDialog( QWidget *parent = 0, const char *name = 0, long options = 0,
                 const QStringList &findStrings = QStringList(), bool hasSelection = false );

    /**
     * Destructor.
     */
    virtual ~KFindDialog();

    /**
     * Provide the list of @p strings to be displayed as the history
     * of find strings. @p strings might get truncated if it is
     * too long.
     *
     * @param history The find history.
     * @see #findHistory
     */
    void setFindHistory( const QStringList &strings );

    /**
     * Returns the list of history items.
     *
     * @see #setFindHistory
     */
    QStringList findHistory() const;

    /**
     * Enable/disable the 'search in selection' option, depending
     * on whether there actually is a selection.
     *
     * @param hasSelection true if a selection exists
     */
    void setHasSelection( bool hasSelection );

    /**
     * Hide/show the 'from cursor' option, depending
     * on whether the application implements a cursor.
     *
     * @param hasCursor true if the application features a cursor
     * This is assumed to be the case by default.
     */
    void setHasCursor( bool hasCursor );

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
     * Returns the pattern to find.
     */
    QString pattern() const;

    /**
     * Sets the pattern to find
     */
    void setPattern ( const QString &pattern );

    /**
     * Returns an empty widget which the user may fill with additional UI
     * elements as required. The widget occupies the width of the dialog,
     * and is positioned immediately below the regular expression support
     * widgets for the pattern string.
     */
    QWidget *findExtension();

signals:

    /**
     * The OK button was pressed, the pattern was not empty, and if it is
     * supposed to be a regular expression, it is known to be valid.
     */
    void okClicked();

protected slots:

    void slotOk();
    void slotSelectedTextToggled(bool);
    void showPatterns();
    void showPlaceholders();
    void textSearchChanged( const QString &);

protected:
    virtual void showEvent ( QShowEvent * );

private:

    QGroupBox *m_findGrp;
    QLabel *m_findLabel;
    KHistoryCombo *m_find;
    QCheckBox *m_regExp;
    QPushButton *m_regExpItem;
    QGridLayout *m_findLayout;
    QWidget *m_findExtension;

    QGroupBox *m_optionGrp;
    QCheckBox *m_wholeWordsOnly;
    QCheckBox *m_fromCursor;
    QCheckBox *m_selectedText;
    QCheckBox *m_caseSensitive;
    QCheckBox *m_findBackwards;

    QPopupMenu *m_patterns;

    // Our dirty little secret is that we also implement the "replace" dialog. But we
    // keep that fact hidden from all but our friends.

    friend class KReplaceDialog;

    /**
     * Construct a find dialog with a parent object and a name. This version of the
     * constructor is for use by friends only!
     *
     * @param forReplace Is this a replace dialog?
     */
    KFindDialog( QWidget *parent, const char *name, bool forReplace );
    void init( bool forReplace, const QStringList &findStrings, bool hasSelection );

    QGroupBox *m_replaceGrp;
    QLabel *m_replaceLabel;
    KHistoryCombo *m_replace;
    QCheckBox* m_backRef;
    QPushButton* m_backRefItem;
    QGridLayout *m_replaceLayout;
    QWidget *m_replaceExtension;

    QCheckBox* m_promptOnReplace;

    QPopupMenu *m_placeholders;

    // Binary compatible extensibility.
    class KFindDialogPrivate;
    KFindDialogPrivate *d;
};

#endif // KFINDDIALOG_H
