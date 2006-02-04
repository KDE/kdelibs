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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kfinddialog.h"
#include <qcheckbox.h>
#include <qcursor.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmenu.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <assert.h>
#include <kfind.h>
#include <kregexpeditorinterface.h>
#include <kparts/componentfactory.h>

class KFindDialog::KFindDialogPrivate
{
public:
    KFindDialogPrivate() : m_regexpDialog(0),
        m_regexpDialogQueryDone(false),
        m_enabled(KFind::WholeWordsOnly | KFind::FromCursor |  KFind::SelectedText | Qt::CaseSensitive | KFind::FindBackwards | KFind::RegularExpression), m_initialShowDone(false) {}
    QDialog* m_regexpDialog;
    bool m_regexpDialogQueryDone;
    long m_enabled; // uses Options to define which search options are enabled
    bool m_initialShowDone;
    QStringList findStrings;
    QString pattern;
};

KFindDialog::KFindDialog(QWidget *parent, const char *name, long options, const QStringList &findStrings, bool hasSelection) :
    KDialogBase(parent, name, true, i18n("Find Text"), Ok | Cancel, Ok),
    m_findExtension (0),
    m_replaceExtension (0),
	d(new KFindDialogPrivate)
{
    init(false, findStrings, hasSelection);
    setOptions(options);
    setButtonGuiItem( KDialogBase::Cancel, KStdGuiItem::close() );
}

KFindDialog::KFindDialog(bool modal, QWidget *parent, const char *name, long options, const QStringList &findStrings, bool hasSelection) :
    KDialogBase(parent, name, modal, i18n("Find Text"), Ok | Cancel, Ok),
    m_findExtension (0),
    m_replaceExtension (0),
	d(new KFindDialogPrivate)
{
    init(false, findStrings, hasSelection);
    setOptions(options);
    setButtonGuiItem( KDialogBase::Cancel, KStdGuiItem::close() );
}

KFindDialog::KFindDialog(QWidget *parent, const char *name, bool /*forReplace*/) :
    KDialogBase(parent, name, true, i18n("Replace Text"), Ok | Cancel, Ok),
    m_findExtension (0),
    m_replaceExtension (0),
	d(new KFindDialogPrivate)
{
    setButtonGuiItem( KDialogBase::Cancel,  KStdGuiItem::close() );
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
    topLayout->setMargin( 0 );

    m_findGrp = new QGroupBox(i18n("Find"), page);
   // m_findGrp->layout()->setMargin( KDialog::marginHint() );
    m_findLayout = new QGridLayout(m_findGrp);
    m_findLayout->setSpacing( KDialog::spacingHint() );
   // m_findLayout->setMargin( KDialog::marginHint() );

    m_findLabel = new QLabel(i18n("&Text to find:"), m_findGrp);
    m_find = new KHistoryCombo(true, m_findGrp);
    m_find->setMaxCount(10);
    m_find->setDuplicatesEnabled(false);
    m_regExp = new QCheckBox(i18n("Regular e&xpression"), m_findGrp);
    m_regExpItem = new QPushButton(i18n("&Edit..."), m_findGrp);
    m_regExpItem->setEnabled(false);

    m_findLayout->addWidget(m_findLabel, 0, 0);
    m_findLayout->addMultiCellWidget(m_find, 1, 1, 0, 1);
    m_findLayout->addWidget(m_regExp, 2, 0);
    m_findLayout->addWidget(m_regExpItem, 2, 1);
    topLayout->addWidget(m_findGrp);

    m_replaceGrp = new QGroupBox( i18n("Replace With"), page);
  //  m_replaceGrp->layout()->setMargin( KDialog::marginHint() );
    m_replaceLayout = new QGridLayout(m_replaceGrp);
    m_replaceLayout->setSpacing( KDialog::spacingHint() );
//    m_replaceLayout->setMargin( KDialog::marginHint() );

    m_replaceLabel = new QLabel(i18n("Replace&ment text:"), m_replaceGrp);
    m_replace = new KHistoryCombo(true, m_replaceGrp);
    m_replace->setMaxCount(10);
    m_replace->setDuplicatesEnabled(false);
    m_backRef = new QCheckBox(i18n("Use p&laceholders"), m_replaceGrp);
    m_backRefItem = new QPushButton(i18n("Insert Place&holder"), m_replaceGrp);
    m_backRefItem->setEnabled(false);

    m_replaceLayout->addWidget(m_replaceLabel, 0, 0);
    m_replaceLayout->addMultiCellWidget(m_replace, 1, 1, 0, 1);
    m_replaceLayout->addWidget(m_backRef, 2, 0);
    m_replaceLayout->addWidget(m_backRefItem, 2, 1);
    topLayout->addWidget(m_replaceGrp);

    m_optionGrp = new QGroupBox(i18n("Options"), page);
  //  m_optionGrp->layout()->setMargin(KDialog::marginHint());
    optionsLayout = new QGridLayout(m_optionGrp);
    optionsLayout->setSpacing( KDialog::spacingHint() );
   // optionsLayout->setMargin( KDialog::marginHint() );

    m_caseSensitive = new QCheckBox(i18n("C&ase sensitive"), m_optionGrp);
    m_wholeWordsOnly = new QCheckBox(i18n("&Whole words only"), m_optionGrp);
    m_fromCursor = new QCheckBox(i18n("From c&ursor"), m_optionGrp);
    m_findBackwards = new QCheckBox(i18n("Find &backwards"), m_optionGrp);
    m_selectedText = new QCheckBox(i18n("&Selected text"), m_optionGrp);
    setHasSelection( hasSelection );
    // If we have a selection, we make 'find in selection' default
    // and if we don't, then the option has to be unchecked, obviously.
    m_selectedText->setChecked( hasSelection );
    slotSelectedTextToggled( hasSelection );

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
    m_patterns = 0;
    m_placeholders = 0;

    // signals and slots connections
    connect(m_selectedText, SIGNAL(toggled(bool)), this, SLOT(slotSelectedTextToggled(bool)));
    connect(m_regExp, SIGNAL(toggled(bool)), m_regExpItem, SLOT(setEnabled(bool)));
    connect(m_backRef, SIGNAL(toggled(bool)), m_backRefItem, SLOT(setEnabled(bool)));
    connect(m_regExpItem, SIGNAL(clicked()), this, SLOT(showPatterns()));
    connect(m_backRefItem, SIGNAL(clicked()), this, SLOT(showPlaceholders()));

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
    if (forReplace)
    {
      setButtonGuiItem( KDialogBase::Ok, KGuiItem( i18n("&Replace"), QString(),
                    i18n("Start replace"),
                    i18n("<qt>If you press the <b>Replace</b> button, the text you entered "
                         "above is searched for within the document and any occurrence is "
                         "replaced with the replacement text.</qt>")));
    }
    else
    {
      setButtonGuiItem( KDialogBase::Ok, KGuiItem( i18n("&Find"), "find",
                    i18n("Start searching"),
                    i18n("<qt>If you press the <b>Find</b> button, the text you entered "
                         "above is searched for within the document.</qt>")));
    }

    // QWhatsthis texts
    m_find->setWhatsThis(i18n(
            "Enter a pattern to search for, or select a previous pattern from "
            "the list.") );
    m_regExp->setWhatsThis(i18n(
            "If enabled, search for a regular expression.") );
    m_regExpItem->setWhatsThis(i18n(
            "Click here to edit your regular expression using a graphical editor.") );
    m_replace->setWhatsThis(i18n(
            "Enter a replacement string, or select a previous one from the list.") );
    m_backRef->setWhatsThis(i18n(
            "<qt>If enabled, any occurrence of <code><b>\\N</b></code>, where "
            "<code><b>N</b></code> is a integer number, will be replaced with "
            "the corresponding capture (\"parenthesized substring\") from the "
            "pattern.<p>To include (a literal <code><b>\\N</b></code> in your "
            "replacement, put an extra backslash in front of it, like "
            "<code><b>\\\\N</b></code>.</qt>") );
    m_backRefItem->setWhatsThis(i18n(
            "Click for a menu of available captures.") );
    m_wholeWordsOnly->setWhatsThis(i18n(
            "Require word boundaries in both ends of a match to succeed.") );
    m_fromCursor->setWhatsThis(i18n(
            "Start searching at the current cursor location rather than at the top.") );
    m_selectedText->setWhatsThis(i18n(
            "Only search within the current selection.") );
    m_caseSensitive->setWhatsThis(i18n(
            "Perform a case sensitive search: entering the pattern "
            "'Joe' will not match 'joe' or 'JOE', only 'Joe'.") );
    m_findBackwards->setWhatsThis(i18n(
            "Search backwards.") );
    m_promptOnReplace->setWhatsThis(i18n(
            "Ask before replacing each match found.") );
}

void KFindDialog::textSearchChanged( const QString & text)
{
    enableButtonOK( !text.isEmpty() );
}

void KFindDialog::showEvent( QShowEvent *e )
{
    if ( !d->m_initialShowDone )
    {
        d->m_initialShowDone = true; // only once
        kDebug() << "showEvent\n";
        if (!d->findStrings.isEmpty())
            setFindHistory(d->findStrings);
        d->findStrings = QStringList();
        if (!d->pattern.isEmpty()) {
            m_find->lineEdit()->setText( d->pattern );
            m_find->lineEdit()->selectAll();
            d->pattern.clear();
        }
    }
    KDialogBase::showEvent(e);
}

long KFindDialog::options() const
{
    long options = 0;

    if (m_caseSensitive->isChecked())
        options |= Qt::CaseSensitive;
    if (m_wholeWordsOnly->isChecked())
        options |= KFind::WholeWordsOnly;
    if (m_fromCursor->isChecked())
        options |= KFind::FromCursor;
    if (m_findBackwards->isChecked())
        options |= KFind::FindBackwards;
    if (m_selectedText->isChecked())
        options |= KFind::SelectedText;
    if (m_regExp->isChecked())
        options |= KFind::RegularExpression;
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
    kDebug() << "setPattern " << pattern<<endl;
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
    if (hasSelection) d->m_enabled |= KFind::SelectedText;
    else d->m_enabled &= ~KFind::SelectedText;
    m_selectedText->setEnabled( hasSelection );
    if ( !hasSelection )
    {
        m_selectedText->setChecked( false );
        slotSelectedTextToggled( hasSelection );
    }
}

void KFindDialog::slotSelectedTextToggled(bool selec)
{
    // From cursor doesn't make sense if we have a selection
    m_fromCursor->setEnabled( !selec && (d->m_enabled & KFind::FromCursor) );
    if ( selec ) // uncheck if disabled
        m_fromCursor->setChecked( false );
}

void KFindDialog::setHasCursor(bool hasCursor)
{
    if (hasCursor) d->m_enabled |= KFind::FromCursor;
    else d->m_enabled &= ~KFind::FromCursor;
    m_fromCursor->setEnabled( hasCursor );
    m_fromCursor->setChecked( hasCursor && (options() & KFind::FromCursor) );
}

void KFindDialog::setSupportsBackwardsFind( bool supports )
{
    // ########## Shouldn't this hide the checkbox instead?
    if (supports) d->m_enabled |= KFind::FindBackwards;
    else d->m_enabled &= ~KFind::FindBackwards;
    m_findBackwards->setEnabled( supports );
    m_findBackwards->setChecked( supports && (options() & KFind::FindBackwards) );
}

void KFindDialog::setSupportsCaseSensitiveFind( bool supports )
{
    // ########## This should hide the checkbox instead
    if (supports) d->m_enabled |= Qt::CaseSensitive;
    else d->m_enabled &= ~Qt::CaseSensitive;
    m_caseSensitive->setEnabled( supports );
    m_caseSensitive->setChecked( supports && (options() & Qt::CaseSensitive) );
}

void KFindDialog::setSupportsWholeWordsFind( bool supports )
{
    // ########## This should hide the checkbox instead
    if (supports) d->m_enabled |= KFind::WholeWordsOnly;
    else d->m_enabled &= ~KFind::WholeWordsOnly;
    m_wholeWordsOnly->setEnabled( supports );
    m_wholeWordsOnly->setChecked( supports && (options() & KFind::WholeWordsOnly) );
}

void KFindDialog::setSupportsRegularExpressionFind( bool supports )
{
    // ########## This should hide the checkbox instead
    if (supports) d->m_enabled |= KFind::RegularExpression;
    else d->m_enabled &= ~KFind::RegularExpression;
    m_regExp->setEnabled( supports );
    m_regExp->setChecked( supports && (options() & KFind::RegularExpression) );
}

void KFindDialog::setOptions(long options)
{
    m_caseSensitive->setChecked((d->m_enabled & Qt::CaseSensitive) && (options & Qt::CaseSensitive));
    m_wholeWordsOnly->setChecked((d->m_enabled & KFind::WholeWordsOnly) && (options & KFind::WholeWordsOnly));
    m_fromCursor->setChecked((d->m_enabled & KFind::FromCursor) && (options & KFind::FromCursor));
    m_findBackwards->setChecked((d->m_enabled & KFind::FindBackwards) && (options & KFind::FindBackwards));
    m_selectedText->setChecked((d->m_enabled & KFind::SelectedText) && (options & KFind::SelectedText));
    m_regExp->setChecked((d->m_enabled & KFind::RegularExpression) && (options & KFind::RegularExpression));
}

// Create a popup menu with a list of regular expression terms, to help the user
// compose a regular expression search pattern.
void KFindDialog::showPatterns()
{
    if ( !d->m_regexpDialogQueryDone )
    {
        d->m_regexpDialog = KParts::ComponentFactory::createInstanceFromQuery<QDialog>( "KRegExpEditor/KRegExpEditor", QString(), this );
        d->m_regexpDialogQueryDone = true;
    }

    if ( d->m_regexpDialog )
    {
        KRegExpEditorInterface *iface = dynamic_cast<KRegExpEditorInterface*>( d->m_regexpDialog );
        assert( iface );

        iface->setRegExp( pattern() );
        if ( d->m_regexpDialog->exec() == QDialog::Accepted )
            setPattern( iface->regExp() );
    }
    else // No complete regexp-editor available, bring up the old popupmenu
    {
        typedef struct
        {
            char *description;
            char *regExp;
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


        class RegExpAction : public QAction
        {
          public:
            RegExpAction( QObject *parent, const QString &text, const QString &regExp, int cursor )
              : QAction( text, parent ), mText( text ), mRegExp( regExp ), mCursor( cursor )
            {
            }

            QString text() const { return mText; }
            QString regExp() const { return mRegExp; }
            int cursor() const { return mCursor; }

          private:
            QString mText;
            QString mRegExp;
            int mCursor;
        };

        int i;

        // Populate the popup menu.
        if (!m_patterns)
        {
            m_patterns = new QMenu(this);
            for (i = 0; (unsigned)i < sizeof(items) / sizeof(items[0]); i++)
            {
                m_patterns->addAction(new RegExpAction(m_patterns, i18n(items[i].description),
                                                       items[i].regExp,
                                                       items[i].cursorAdjustment));
            }
        }

        // Insert the selection into the edit control.
        QAction *action = m_patterns->exec(m_regExpItem->mapToGlobal(m_regExpItem->rect().bottomLeft()));
        if (action)
        {
            RegExpAction *regExpAction = static_cast<RegExpAction*>( action );
            if ( regExpAction ) {
              QLineEdit *editor = m_find->lineEdit();

              editor->insert(regExpAction->regExp());
              editor->setCursorPosition(editor->cursorPosition() + regExpAction->cursor());
            }
        }
    }
}

class PlaceHolderAction : public QAction
{
  public:
    PlaceHolderAction( QObject *parent, const QString &text, int id )
      : QAction( text, parent ), mText( text ), mId( id )
    {
    }

    QString text() const { return mText; }
    int id() const { return mId; }

  private:
    QString mText;
    int mId;
};

// Create a popup menu with a list of backreference terms, to help the user
// compose a regular expression replacement pattern.
void KFindDialog::showPlaceholders()
{
    // Populate the popup menu.
    if (!m_placeholders)
    {
        m_placeholders = new QMenu(this);
        connect( m_placeholders, SIGNAL(aboutToShow()), this, SLOT(slotPlaceholdersAboutToShow()) );
    }

    // Insert the selection into the edit control.
    QAction *action = m_placeholders->exec(m_backRefItem->mapToGlobal(m_backRefItem->rect().bottomLeft()));
    if (action)
    {
        PlaceHolderAction *placeHolderAction = static_cast<PlaceHolderAction*>(action);
        if (placeHolderAction) {
          QLineEdit *editor = m_replace->lineEdit();
          editor->insert( QString("\\%1").arg( placeHolderAction->id() ) );
        }
    }
}

void KFindDialog::slotPlaceholdersAboutToShow()
{
    m_placeholders->clear();
    m_placeholders->addAction( new PlaceHolderAction(m_placeholders, i18n("Complete Match"), 0));

    QRegExp r( pattern() );
    uint n = r.numCaptures();
    for ( uint i=0; i < n; i++ )
        m_placeholders->addAction( new PlaceHolderAction(m_placeholders, i18n("Captured Text (%1)").arg( i+1 ), i+1 ) );
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
    if ( windowFlags() & Qt::WShowModal )
        accept();
}
// kate: space-indent on; indent-width 4; replace-tabs on;
#include "kfinddialog.moc"
