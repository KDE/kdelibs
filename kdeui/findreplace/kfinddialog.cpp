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
#include "kfinddialog_p.h"

#include <QtGui/QCheckBox>
#include <QtGui/QCursor>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QMenu>
#include <QtGui/QPushButton>
#include <QtCore/QRegExp>
#include <kcombobox.h>
#include <khistorycombobox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <assert.h>
#include <kfind.h>
#include <kregexpeditorinterface.h>
#include <kservicetypetrader.h>

KFindDialog::KFindDialog(QWidget *parent, long options, const QStringList &findStrings, bool hasSelection, bool replaceDialog)
    : KDialog(parent),
      d(new KFindDialogPrivate(this))
{
    setCaption( i18n("Find Text") );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );

    d->init(replaceDialog, findStrings, hasSelection);
    setOptions(options);
    setButtonGuiItem( KDialog::Cancel, KStandardGuiItem::close() );
}

KFindDialog::~KFindDialog()
{
    delete d;
}

QWidget *KFindDialog::findExtension() const
{
    if (!d->findExtension)
    {
      d->findExtension = new QWidget(d->findGrp);
      d->findLayout->addWidget(d->findExtension, 3, 0, 1, 2);
    }

    return d->findExtension;
}

QStringList KFindDialog::findHistory() const
{
    return d->find->historyItems();
}

void KFindDialog::KFindDialogPrivate::init(bool forReplace, const QStringList &_findStrings, bool hasSelection)
{
    QVBoxLayout *topLayout;
    QGridLayout *optionsLayout;

    // Create common parts of dialog.
    QWidget *page = new QWidget(q);
    q->setMainWidget(page);

    topLayout = new QVBoxLayout(page);
    topLayout->setMargin( 0 );

    findGrp = new QGroupBox(i18nc("@title:group", "Find"), page);
    findLayout = new QGridLayout(findGrp);

    QLabel *findLabel = new QLabel(i18n("&Text to find:"), findGrp);
    find = new KHistoryComboBox(findGrp);
    find->setMaxCount(10);
    find->setDuplicatesEnabled(false);
    regExp = new QCheckBox(i18n("Regular e&xpression"), findGrp);
    regExpItem = new QPushButton(i18n("&Edit..."), findGrp);
    regExpItem->setEnabled(false);

    findLayout->addWidget(findLabel, 0, 0);
    findLayout->addWidget(find, 1, 0, 1, 2);
    findLayout->addWidget(regExp, 2, 0);
    findLayout->addWidget(regExpItem, 2, 1);
    topLayout->addWidget(findGrp);

    replaceGrp = new QGroupBox( i18n("Replace With"), page);
    replaceLayout = new QGridLayout(replaceGrp);

    QLabel *replaceLabel = new QLabel(i18n("Replace&ment text:"), replaceGrp);
    replace = new KHistoryComboBox(replaceGrp);
    replace->setMaxCount(10);
    replace->setDuplicatesEnabled(false);
    backRef = new QCheckBox(i18n("Use p&laceholders"), replaceGrp);
    backRefItem = new QPushButton(i18n("Insert Place&holder"), replaceGrp);
    backRefItem->setEnabled(false);

    replaceLayout->addWidget(replaceLabel, 0, 0);
    replaceLayout->addWidget(replace, 1, 0, 1, 2);
    replaceLayout->addWidget(backRef, 2, 0);
    replaceLayout->addWidget(backRefItem, 2, 1);
    topLayout->addWidget(replaceGrp);

    QGroupBox *optionGrp = new QGroupBox(i18n("Options"), page);
    optionsLayout = new QGridLayout(optionGrp);

    caseSensitive = new QCheckBox(i18n("C&ase sensitive"), optionGrp);
    wholeWordsOnly = new QCheckBox(i18n("&Whole words only"), optionGrp);
    fromCursor = new QCheckBox(i18n("From c&ursor"), optionGrp);
    findBackwards = new QCheckBox(i18n("Find &backwards"), optionGrp);
    selectedText = new QCheckBox(i18n("&Selected text"), optionGrp);
    q->setHasSelection( hasSelection );
    // If we have a selection, we make 'find in selection' default
    // and if we don't, then the option has to be unchecked, obviously.
    selectedText->setChecked( hasSelection );
    _k_slotSelectedTextToggled( hasSelection );

    promptOnReplace = new QCheckBox(i18n("&Prompt on replace"), optionGrp);
    promptOnReplace->setChecked( true );

    optionsLayout->addWidget(caseSensitive, 0, 0);
    optionsLayout->addWidget(wholeWordsOnly, 1, 0);
    optionsLayout->addWidget(fromCursor, 2, 0);
    optionsLayout->addWidget(findBackwards, 0, 1);
    optionsLayout->addWidget(selectedText, 1, 1);
    optionsLayout->addWidget(promptOnReplace, 2, 1);
    topLayout->addWidget(optionGrp);

    // We delay creation of these until needed.
    patterns = 0;
    placeholders = 0;

    // signals and slots connections
    q->connect(selectedText, SIGNAL(toggled(bool)), q, SLOT(_k_slotSelectedTextToggled(bool)));
    q->connect(regExp, SIGNAL(toggled(bool)), regExpItem, SLOT(setEnabled(bool)));
    q->connect(backRef, SIGNAL(toggled(bool)), backRefItem, SLOT(setEnabled(bool)));
    q->connect(regExpItem, SIGNAL(clicked()), q, SLOT(_k_showPatterns()));
    q->connect(backRefItem, SIGNAL(clicked()), q, SLOT(_k_showPlaceholders()));

    q->connect(find, SIGNAL(editTextChanged(const QString &)), q, SLOT(_k_textSearchChanged(const QString &)));

    q->connect(regExp, SIGNAL(toggled(bool)), q, SIGNAL(optionsChanged()));
    q->connect(backRef, SIGNAL(toggled(bool)), q, SIGNAL(optionsChanged()));
    q->connect(caseSensitive, SIGNAL(toggled(bool)), q, SIGNAL(optionsChanged()));
    q->connect(wholeWordsOnly, SIGNAL(toggled(bool)), q, SIGNAL(optionsChanged()));
    q->connect(fromCursor, SIGNAL(toggled(bool)), q, SIGNAL(optionsChanged()));
    q->connect(findBackwards, SIGNAL(toggled(bool)), q, SIGNAL(optionsChanged()));
    q->connect(selectedText, SIGNAL(toggled(bool)), q, SIGNAL(optionsChanged()));
    q->connect(promptOnReplace, SIGNAL(toggled(bool)), q, SIGNAL(optionsChanged()));

    // tab order
    q->setTabOrder(find, regExp);
    q->setTabOrder(regExp, regExpItem);
    q->setTabOrder(regExpItem, replace); //findExtension widgets are inserted in showEvent()
    q->setTabOrder(replace, backRef);
    q->setTabOrder(backRef, backRefItem);
    q->setTabOrder(backRefItem, caseSensitive);
    q->setTabOrder(caseSensitive, wholeWordsOnly);
    q->setTabOrder(wholeWordsOnly, fromCursor);
    q->setTabOrder(fromCursor, findBackwards);
    q->setTabOrder(findBackwards, selectedText);
    q->setTabOrder(selectedText, promptOnReplace);

    // buddies
    findLabel->setBuddy(find);
    replaceLabel->setBuddy(replace);

    if (!forReplace)
    {
        promptOnReplace->hide();
        replaceGrp->hide();
    }

    findStrings = _findStrings;
    find->setFocus();
    q->enableButtonOk( !q->pattern().isEmpty() );
    if (forReplace)
    {
      q->setButtonGuiItem( KDialog::Ok, KGuiItem( i18n("&Replace"), QString(),
                    i18n("Start replace"),
                    i18n("<qt>If you press the <b>Replace</b> button, the text you entered "
                         "above is searched for within the document and any occurrence is "
                         "replaced with the replacement text.</qt>")));
    }
    else
    {
      q->setButtonGuiItem( KDialog::Ok, KGuiItem( i18n("&Find"), "edit-find",
                    i18n("Start searching"),
                    i18n("<qt>If you press the <b>Find</b> button, the text you entered "
                         "above is searched for within the document.</qt>")));
    }

    // QWhatsthis texts
    find->setWhatsThis(i18n(
            "Enter a pattern to search for, or select a previous pattern from "
            "the list.") );
    regExp->setWhatsThis(i18n(
            "If enabled, search for a regular expression.") );
    regExpItem->setWhatsThis(i18n(
            "Click here to edit your regular expression using a graphical editor.") );
    replace->setWhatsThis(i18n(
            "Enter a replacement string, or select a previous one from the list.") );
    backRef->setWhatsThis(i18n(
            "<qt>If enabled, any occurrence of <code><b>\\N</b></code>, where "
            "<code><b>N</b></code> is an integer number, will be replaced with "
            "the corresponding capture (\"parenthesized substring\") from the "
            "pattern.<p>To include (a literal <code><b>\\N</b></code> in your "
            "replacement, put an extra backslash in front of it, like "
            "<code><b>\\\\N</b></code>.</p></qt>") );
    backRefItem->setWhatsThis(i18n(
            "Click for a menu of available captures.") );
    wholeWordsOnly->setWhatsThis(i18n(
            "Require word boundaries in both ends of a match to succeed.") );
    fromCursor->setWhatsThis(i18n(
            "Start searching at the current cursor location rather than at the top.") );
    selectedText->setWhatsThis(i18n(
            "Only search within the current selection.") );
    caseSensitive->setWhatsThis(i18n(
            "Perform a case sensitive search: entering the pattern "
            "'Joe' will not match 'joe' or 'JOE', only 'Joe'.") );
    findBackwards->setWhatsThis(i18n(
            "Search backwards.") );
    promptOnReplace->setWhatsThis(i18n(
            "Ask before replacing each match found.") );

    q->connect(q, SIGNAL(okClicked()), q, SLOT(_k_slotOk()));
    _k_textSearchChanged(find->lineEdit()->text());
}

void KFindDialog::KFindDialogPrivate::_k_textSearchChanged( const QString & text)
{
    q->enableButtonOk( !text.isEmpty() );
}

void KFindDialog::showEvent( QShowEvent *e )
{
    if ( !d->initialShowDone )
    {
        d->initialShowDone = true; // only once
        kDebug() << "showEvent\n";
        if (!d->findStrings.isEmpty())
            setFindHistory(d->findStrings);
        d->findStrings = QStringList();
        if (!d->pattern.isEmpty()) {
            d->find->lineEdit()->setText( d->pattern );
            d->find->lineEdit()->selectAll();
            d->pattern.clear();
        }
        //maintain a user-friendly tab order
        if (d->findExtension) {
            QWidget* prev=d->regExpItem;
            foreach(QWidget* child, d->findExtension->findChildren<QWidget*>()) {
                setTabOrder(prev, child);
                prev=child;
            }
            setTabOrder(prev, d->replace);
        }
    }
    KDialog::showEvent(e);
}

long KFindDialog::options() const
{
    long options = 0;

    if (d->caseSensitive->isChecked())
        options |= KFind::CaseSensitive;
    if (d->wholeWordsOnly->isChecked())
        options |= KFind::WholeWordsOnly;
    if (d->fromCursor->isChecked())
        options |= KFind::FromCursor;
    if (d->findBackwards->isChecked())
        options |= KFind::FindBackwards;
    if (d->selectedText->isChecked())
        options |= KFind::SelectedText;
    if (d->regExp->isChecked())
        options |= KFind::RegularExpression;
    return options;
}

QString KFindDialog::pattern() const
{
    return d->find->currentText();
}

void KFindDialog::setPattern (const QString &pattern)
{
    d->find->lineEdit()->setText( pattern );
    d->find->lineEdit()->selectAll();
    d->pattern = pattern;
    kDebug() << "setPattern " << pattern;
}

void KFindDialog::setFindHistory(const QStringList &strings)
{
    if (strings.count() > 0)
    {
        d->find->setHistoryItems(strings, true);
        d->find->lineEdit()->setText( strings.first() );
        d->find->lineEdit()->selectAll();
    }
    else
        d->find->clearHistory();
}

void KFindDialog::setHasSelection(bool hasSelection)
{
    if (hasSelection) d->enabled |= KFind::SelectedText;
    else d->enabled &= ~KFind::SelectedText;
    d->selectedText->setEnabled( hasSelection );
    if ( !hasSelection )
    {
        d->selectedText->setChecked( false );
        d->_k_slotSelectedTextToggled( hasSelection );
    }
}

void KFindDialog::KFindDialogPrivate::_k_slotSelectedTextToggled(bool selec)
{
    // From cursor doesn't make sense if we have a selection
    fromCursor->setEnabled( !selec && (enabled & KFind::FromCursor) );
    if ( selec ) // uncheck if disabled
        fromCursor->setChecked( false );
}

void KFindDialog::setHasCursor(bool hasCursor)
{
    if (hasCursor) d->enabled |= KFind::FromCursor;
    else d->enabled &= ~KFind::FromCursor;
    d->fromCursor->setEnabled( hasCursor );
    d->fromCursor->setChecked( hasCursor && (options() & KFind::FromCursor) );
}

void KFindDialog::setSupportsBackwardsFind( bool supports )
{
    // ########## Shouldn't this hide the checkbox instead?
    if (supports) d->enabled |= KFind::FindBackwards;
    else d->enabled &= ~KFind::FindBackwards;
    d->findBackwards->setEnabled( supports );
    d->findBackwards->setChecked( supports && (options() & KFind::FindBackwards) );
}

void KFindDialog::setSupportsCaseSensitiveFind( bool supports )
{
    // ########## This should hide the checkbox instead
    if (supports) d->enabled |= KFind::CaseSensitive;
    else d->enabled &= ~KFind::CaseSensitive;
    d->caseSensitive->setEnabled( supports );
    d->caseSensitive->setChecked( supports && (options() & KFind::CaseSensitive) );
}

void KFindDialog::setSupportsWholeWordsFind( bool supports )
{
    // ########## This should hide the checkbox instead
    if (supports) d->enabled |= KFind::WholeWordsOnly;
    else d->enabled &= ~KFind::WholeWordsOnly;
    d->wholeWordsOnly->setEnabled( supports );
    d->wholeWordsOnly->setChecked( supports && (options() & KFind::WholeWordsOnly) );
}

void KFindDialog::setSupportsRegularExpressionFind( bool supports )
{
    if (supports) d->enabled |= KFind::RegularExpression;
    else d->enabled &= ~KFind::RegularExpression;
    d->regExp->setEnabled( supports );
    d->regExp->setChecked( supports && (options() & KFind::RegularExpression) );
    if( !supports)
    {
       d->regExpItem->hide();
       d->regExp->hide();
    }
    else
    {
       d->regExpItem->show();
       d->regExp->show();
    }
}

void KFindDialog::setOptions(long options)
{
    d->caseSensitive->setChecked((d->enabled & KFind::CaseSensitive) && (options & KFind::CaseSensitive));
    d->wholeWordsOnly->setChecked((d->enabled & KFind::WholeWordsOnly) && (options & KFind::WholeWordsOnly));
    d->fromCursor->setChecked((d->enabled & KFind::FromCursor) && (options & KFind::FromCursor));
    d->findBackwards->setChecked((d->enabled & KFind::FindBackwards) && (options & KFind::FindBackwards));
    d->selectedText->setChecked((d->enabled & KFind::SelectedText) && (options & KFind::SelectedText));
    d->regExp->setChecked((d->enabled & KFind::RegularExpression) && (options & KFind::RegularExpression));
}

// Create a popup menu with a list of regular expression terms, to help the user
// compose a regular expression search pattern.
void KFindDialog::KFindDialogPrivate::_k_showPatterns()
{
    if ( !regexpDialogQueryDone )
    {
        regexpDialog = KServiceTypeTrader::createInstanceFromQuery<QDialog>( "KRegExpEditor/KRegExpEditor", QString(), q );
        regexpDialogQueryDone = true;
    }

    if ( regexpDialog )
    {
        KRegExpEditorInterface *iface = qobject_cast<KRegExpEditorInterface*>( regexpDialog );
        assert( iface );

        iface->setRegExp( q->pattern() );
        if ( regexpDialog->exec() == QDialog::Accepted )
            q->setPattern( iface->regExp() );
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
        if (!patterns)
        {
            patterns = new QMenu(q);
            for (i = 0; (unsigned)i < sizeof(items) / sizeof(items[0]); i++)
            {
                patterns->addAction(new RegExpAction(patterns, i18n(items[i].description),
                                                     items[i].regExp,
                                                     items[i].cursorAdjustment));
            }
        }

        // Insert the selection into the edit control.
        QAction *action = patterns->exec(regExpItem->mapToGlobal(regExpItem->rect().bottomLeft()));
        if (action)
        {
            RegExpAction *regExpAction = static_cast<RegExpAction*>( action );
            if ( regExpAction ) {
              QLineEdit *editor = find->lineEdit();

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
void KFindDialog::KFindDialogPrivate::_k_showPlaceholders()
{
    // Populate the popup menu.
    if (!placeholders)
    {
        placeholders = new QMenu(q);
        q->connect( placeholders, SIGNAL(aboutToShow()), q, SLOT(_k_slotPlaceholdersAboutToShow()) );
    }

    // Insert the selection into the edit control.
    QAction *action = placeholders->exec(backRefItem->mapToGlobal(backRefItem->rect().bottomLeft()));
    if (action)
    {
        PlaceHolderAction *placeHolderAction = static_cast<PlaceHolderAction*>(action);
        if (placeHolderAction) {
          QLineEdit *editor = replace->lineEdit();
          editor->insert( QString("\\%1").arg( placeHolderAction->id() ) );
        }
    }
}

void KFindDialog::KFindDialogPrivate::_k_slotPlaceholdersAboutToShow()
{
    placeholders->clear();
    placeholders->addAction( new PlaceHolderAction(placeholders, i18n("Complete Match"), 0));

    QRegExp r( q->pattern() );
    uint n = r.numCaptures();
    for ( uint i=0; i < n; i++ )
        placeholders->addAction( new PlaceHolderAction(placeholders, i18n("Captured Text (%1)",  i+1 ), i+1 ) );
}

void KFindDialog::KFindDialogPrivate::_k_slotOk()
{
    // Nothing to find?
    if (q->pattern().isEmpty())
    {
        KMessageBox::error(q, i18n("You must enter some text to search for."));
        return;
    }

    if (regExp->isChecked())
    {
        // Check for a valid regular expression.
        QRegExp _regExp(q->pattern());

        if (!_regExp.isValid())
        {
            KMessageBox::error(q, i18n("Invalid regular expression."));
            return;
        }
    }
    find->addToHistory(q->pattern());
    if ( q->windowModality() != Qt::NonModal )
        q->accept();
}
// kate: space-indent on; indent-width 4; replace-tabs on;
#include "kfinddialog.moc"
