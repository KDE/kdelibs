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

#include "kfinddialog.h"
#include <qcheckbox.h>
#include <qcursor.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <assert.h>

#include <kregexpeditorinterface.h>
#include <kparts/componentfactory.h>

class KFindDialog::KFindDialogPrivate
{
public:
    KFindDialogPrivate() : m_regexpDialog(0),
        m_regexpDialogQueryDone(false), m_hasCursor(true), m_hasSelection(false) {}
    QDialog* m_regexpDialog;
    bool m_regexpDialogQueryDone;
    bool m_hasCursor;
    bool m_hasSelection;
    QStringList findStrings;
    QString pattern;
};

KFindDialog::KFindDialog(QWidget *parent, const char *name, long options, const QStringList &findStrings, bool hasSelection) :
    KDialogBase(parent, name, true, i18n("Find Text"), Ok | Cancel, Ok),
    m_findExtension (0),
    m_replaceExtension (0)
{
    d = new KFindDialogPrivate;
    init(false, findStrings, hasSelection);
    setOptions(options);
}

KFindDialog::KFindDialog(QWidget *parent, const char *name, bool /*forReplace*/) :
    KDialogBase(parent, name, true, i18n("Replace Text"), Ok | Cancel, Ok),
    m_findExtension (0),
    m_replaceExtension (0)
{
    d = new KFindDialogPrivate;
}

KFindDialog::~KFindDialog()
{
    delete d;
}

QWidget *KFindDialog::findExtension()
{
    if (!m_findExtension)
    {
      m_findExtension = new QWidget(m_findGrp);
      m_findLayout->addMultiCellWidget(m_findExtension, 3, 3, 0, 1);
    }

    return m_findExtension;
}

QStringList KFindDialog::findHistory() const
{
    return m_find->historyItems();
}

void KFindDialog::init(bool forReplace, const QStringList &findStrings, bool hasSelection)
{
    QVBoxLayout *topLayout;
    QGridLayout *optionsLayout;

    // Create common parts of dialog.
    QWidget *page = new QWidget(this);
    setMainWidget(page);

    topLayout = new QVBoxLayout(page);
    topLayout->setSpacing( KDialog::spacingHint() );
    topLayout->setMargin( KDialog::marginHint() );

    m_findGrp = new QGroupBox(0, Qt::Vertical, i18n("Find"), page);
    m_findGrp->layout()->setSpacing( KDialog::spacingHint() );
   // m_findGrp->layout()->setMargin( KDialog::marginHint() );
    m_findLayout = new QGridLayout(m_findGrp->layout());
    m_findLayout->setSpacing( KDialog::spacingHint() );
   // m_findLayout->setMargin( KDialog::marginHint() );

    m_findLabel = new QLabel(i18n("&Text to find:"), m_findGrp);
    m_find = new KHistoryCombo(true, m_findGrp);
    m_find->setMaxCount(10);
    m_find->setDuplicatesEnabled(false);
    m_regExp = new QCheckBox(i18n("&Regular expression"), m_findGrp);
    m_regExpItem = new QPushButton(i18n("&Edit..."), m_findGrp);
    m_regExpItem->setEnabled(false);

    m_findLayout->addWidget(m_findLabel, 0, 0);
    m_findLayout->addMultiCellWidget(m_find, 1, 1, 0, 1);
    m_findLayout->addWidget(m_regExp, 2, 0);
    m_findLayout->addWidget(m_regExpItem, 2, 1);
    topLayout->addWidget(m_findGrp);

    m_replaceGrp = new QGroupBox(0, Qt::Vertical, i18n("Replace With"), page);
    m_replaceGrp->layout()->setSpacing( KDialog::spacingHint() );
  //  m_replaceGrp->layout()->setMargin( KDialog::marginHint() );
    m_replaceLayout = new QGridLayout(m_replaceGrp->layout());
    m_replaceLayout->setSpacing( KDialog::spacingHint() );
//    m_replaceLayout->setMargin( KDialog::marginHint() );

    m_replaceLabel = new QLabel(i18n("&Replacement text:"), m_replaceGrp);
    m_replace = new KHistoryCombo(true, m_replaceGrp);
    m_replace->setMaxCount(10);
    m_replace->setDuplicatesEnabled(false);
    m_backRef = new QCheckBox(i18n("Us&e placeholders"), m_replaceGrp);
    m_backRefItem = new QPushButton(i18n("Insert Place&holder"), m_replaceGrp);
    m_backRefItem->setEnabled(false);

    m_replaceLayout->addWidget(m_replaceLabel, 0, 0);
    m_replaceLayout->addMultiCellWidget(m_replace, 1, 1, 0, 1);
    m_replaceLayout->addWidget(m_backRef, 2, 0);
    m_replaceLayout->addWidget(m_backRefItem, 2, 1);
    topLayout->addWidget(m_replaceGrp);

    m_optionGrp = new QGroupBox(0, Qt::Vertical, i18n("Options"), page);
    m_optionGrp->layout()->setSpacing(KDialog::spacingHint());
  //  m_optionGrp->layout()->setMargin(KDialog::marginHint());
    optionsLayout = new QGridLayout(m_optionGrp->layout());
    optionsLayout->setSpacing( KDialog::spacingHint() );
   // optionsLayout->setMargin( KDialog::marginHint() );

    m_caseSensitive = new QCheckBox(i18n("C&ase sensitive"), m_optionGrp);
    m_wholeWordsOnly = new QCheckBox(i18n("&Whole words only"), m_optionGrp);
    m_fromCursor = new QCheckBox(i18n("&From cursor"), m_optionGrp);
    m_findBackwards = new QCheckBox(i18n("Find &backwards"), m_optionGrp);
    m_selectedText = new QCheckBox(i18n("&Selected text"), m_optionGrp);
    setHasSelection( hasSelection );
    m_promptOnReplace = new QCheckBox(i18n("&Prompt on replace"), m_optionGrp);
    m_promptOnReplace->setChecked( true );

    optionsLayout->addWidget(m_caseSensitive, 0, 0);
    optionsLayout->addWidget(m_wholeWordsOnly, 1, 0);
    optionsLayout->addWidget(m_fromCursor, 2, 0);
    optionsLayout->addWidget(m_findBackwards, 0, 1);
    optionsLayout->addWidget(m_selectedText, 1, 1);
    optionsLayout->addWidget(m_promptOnReplace, 2, 1);
    topLayout->addWidget(m_optionGrp);

    // We delay creation of these until needed.
    m_patterns = 0L;
    m_placeholders = 0L;

    // signals and slots connections
    connect(m_selectedText, SIGNAL(toggled(bool)), this, SLOT(slotSelectedTextToggled(bool)));
    connect(m_regExp, SIGNAL(toggled(bool)), m_regExpItem, SLOT(setEnabled(bool)));
    connect(m_backRef, SIGNAL(toggled(bool)), m_backRefItem, SLOT(setEnabled(bool)));
    connect(m_regExpItem, SIGNAL(pressed()), this, SLOT(showPatterns()));
    connect(m_backRefItem, SIGNAL(pressed()), this, SLOT(showPlaceholders()));

    connect(m_find, SIGNAL(textChanged ( const QString & )),this, SLOT(textSearchChanged( const QString & )));

    // tab order
    setTabOrder(m_find, m_regExp);
    setTabOrder(m_regExp, m_regExpItem);
    setTabOrder(m_regExpItem, m_replace);
    setTabOrder(m_replace, m_backRef);
    setTabOrder(m_backRef, m_backRefItem);
    setTabOrder(m_backRefItem, m_caseSensitive);
    setTabOrder(m_caseSensitive, m_wholeWordsOnly);
    setTabOrder(m_wholeWordsOnly, m_fromCursor);
    setTabOrder(m_fromCursor, m_findBackwards);
    setTabOrder(m_findBackwards, m_selectedText);
    setTabOrder(m_selectedText, m_promptOnReplace);

    // buddies
    m_findLabel->setBuddy(m_find);
    m_replaceLabel->setBuddy(m_replace);

    if (!forReplace)
    {
        m_promptOnReplace->hide();
        m_replaceGrp->hide();
    }

    d->findStrings = findStrings;
    m_find->setFocus();
    enableButtonOK( !pattern().isEmpty() );
}

void KFindDialog::textSearchChanged( const QString & text)
{
    enableButtonOK( !text.isEmpty() );
}

void KFindDialog::showEvent( QShowEvent *e )
{
    kdDebug() << "showEvent\n";
    if (!d->findStrings.isEmpty())
        setFindHistory(d->findStrings);
    d->findStrings = QStringList();
    if (!d->pattern.isEmpty()) {
        m_find->lineEdit()->setText( d->pattern );
        m_find->lineEdit()->selectAll();
        d->pattern = QString::null;
    }
    KDialogBase::showEvent(e);
}

long KFindDialog::options() const
{
    long options = 0;

    if (m_caseSensitive->isChecked())
        options |= CaseSensitive;
    if (m_wholeWordsOnly->isChecked())
        options |= WholeWordsOnly;
    if (m_fromCursor->isChecked())
        options |= FromCursor;
    if (m_findBackwards->isChecked())
        options |= FindBackwards;
    if (m_selectedText->isChecked())
        options |= SelectedText;
    if (m_regExp->isChecked())
        options |= RegularExpression;
    return options;
}

QString KFindDialog::pattern() const
{
    return m_find->currentText();
}

void KFindDialog::setPattern (const QString &pattern)
{
    m_find->lineEdit()->setText( pattern );
    m_find->lineEdit()->selectAll();
    d->pattern = pattern;
    kdDebug() << "setPattern " << pattern<<endl;
}

void KFindDialog::setFindHistory(const QStringList &strings)
{
    if (strings.count() > 0)
    {
        m_find->setHistoryItems(strings, true);
        m_find->lineEdit()->setText( strings.first() );
        m_find->lineEdit()->selectAll();
    }
    else
        m_find->clearHistory();
}

void KFindDialog::setHasSelection(bool hasSelection)
{
    d->m_hasSelection = hasSelection;
    m_selectedText->setEnabled( hasSelection );
    // If we have a selection, we make 'find in selection' default
    // and if we don't, then the option has to be unchecked, obviously.
    m_selectedText->setChecked( hasSelection );
    slotSelectedTextToggled( hasSelection );
}

void KFindDialog::slotSelectedTextToggled(bool selec)
{
    // From cursor doesn't make sense if we have a selection
    m_fromCursor->setEnabled( !selec && d->m_hasCursor );
    if ( selec ) // uncheck if disabled
        m_fromCursor->setChecked( false );
}

void KFindDialog::setHasCursor(bool hasCursor)
{
    d->m_hasCursor = hasCursor;
    m_fromCursor->setEnabled( hasCursor );
    m_fromCursor->setChecked( hasCursor && (options() & FromCursor) );
}

void KFindDialog::setOptions(long options)
{
    m_caseSensitive->setChecked(options & CaseSensitive);
    m_wholeWordsOnly->setChecked(options & WholeWordsOnly);
    m_fromCursor->setChecked(d->m_hasCursor && (options & FromCursor));
    m_findBackwards->setChecked(options & FindBackwards);
    m_selectedText->setChecked(d->m_hasSelection && (options & SelectedText));
    m_regExp->setChecked(options & RegularExpression);
}

// Create a popup menu with a list of regular expression terms, to help the user
// compose a regular expression search pattern.
void KFindDialog::showPatterns()
{
    if ( !d->m_regexpDialogQueryDone )
    {
        d->m_regexpDialog = KParts::ComponentFactory::createInstanceFromQuery<QDialog>( "KRegExpEditor/KRegExpEditor", QString::null, this );
        d->m_regexpDialogQueryDone = true;
    }

    if ( d->m_regexpDialog )
    {
        KRegExpEditorInterface *iface = static_cast<KRegExpEditorInterface *>( d->m_regexpDialog->qt_cast( "KRegExpEditorInterface" ) );
        assert( iface );

        iface->setRegExp( pattern() );
        if ( d->m_regexpDialog->exec() == QDialog::Accepted )
            setPattern( iface->regExp() );
    }
    else // No complete regexp-editor available, bring up the old popupmenu
    {
        typedef struct
        {
            const char *description;
            const char *regExp;
            int cursorAdjustment;
        } term;
        static const term items[] =
            {
                { I18N_NOOP("Any Character"),                 ".",        0 },
                { I18N_NOOP("Start of Line"),                 "^",        0 },
                { I18N_NOOP("End of Line"),                   "$",        0 },
                { I18N_NOOP("Set of Characters"),             "[]",       -1 },
                { I18N_NOOP("Repeats, Zero or More Times"),   "*",        0 },
                { I18N_NOOP("Repeats, One or More Times"),    "+",        0 },
                { I18N_NOOP("Optional"),                      "?",        0 },
                { I18N_NOOP("Escape"),                        "\\",       0 },
                { I18N_NOOP("TAB"),                           "\\t",      0 },
                { I18N_NOOP("Newline"),                       "\\n",      0 },
                { I18N_NOOP("Carriage Return"),               "\\r",      0 },
                { I18N_NOOP("White Space"),                   "\\s",      0 },
                { I18N_NOOP("Digit"),                         "\\d",      0 },
            };
        int i;

        // Populate the popup menu.
        if (!m_patterns)
        {
            m_patterns = new QPopupMenu(this);
            for (i = 0; (unsigned)i < sizeof(items) / sizeof(items[0]); i++)
            {
                m_patterns->insertItem(i18n(items[i].description), i, i);
            }
        }

        // Insert the selection into the edit control.
        i = m_patterns->exec(QCursor::pos());
        if (i != -1)
        {
            QLineEdit *editor = m_find->lineEdit();

            editor->insert(items[i].regExp);
            editor->setCursorPosition(editor->cursorPosition() + items[i].cursorAdjustment);
        }
    }
}

// Create a popup menu with a list of backreference terms, to help the user
// compose a regular expression replacement pattern.
void KFindDialog::showPlaceholders()
{
    typedef struct
    {
        const char *description;
        const char *backRef;
    } term;
    static const term items[] =
    {
        { I18N_NOOP("Complete text found"),             "\\0" },
    };
    int i;

    // Populate the popup menu.
    if (!m_placeholders)
    {
        m_placeholders = new QPopupMenu(this);
        for (i = 0; (unsigned)i < sizeof(items) / sizeof(items[0]); i++)
        {
            m_placeholders->insertItem(i18n(items[i].description), i, i);
        }
    }

    // Insert the selection into the edit control.
    i = m_placeholders->exec(QCursor::pos());
    if (i != -1)
    {
        QLineEdit *editor = m_replace->lineEdit();

        editor->insert(items[i].backRef);
    }
}

void KFindDialog::slotOk()
{
    // Nothing to find?
    if (pattern().isEmpty())
    {
        KMessageBox::error(this, i18n("You must enter some text to search for."));
        return;
    }

    if (m_regExp->isChecked())
    {
        // Check for a valid regular expression.
        QRegExp regExp(pattern());

        if (!regExp.isValid())
        {
            KMessageBox::error(this, i18n("Invalid regular expression."));
            return;
        }
    }
    m_find->addToHistory(pattern());
    emit okClicked();
    accept();
}

#include "kfinddialog.moc"
