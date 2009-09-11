/* This file is part of the KDE libraries
   Copyright (C) 2007 Sebastian Pipping <webmaster@hartwork.org>
   Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
   Copyright (C) 2007 Thomas Friedrichsmeier <thomas.friedrichsmeier@ruhr-uni-bochum.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "katesearchbar.h"
#include "kateview.h"
#include "katedocument.h"
#include "kateglobal.h"

#include "ui_searchbarincremental.h"
#include "ui_searchbarpower.h"

#include <kactioncollection.h>
#include <ktexteditor/rangefeedback.h>

#include <QtGui/QVBoxLayout>
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QKeySequence>
#include <QtGui/QShortcut>
#include <QtGui/QCursor>
#include <QStringListModel>
#include <QCompleter>
#include <QMutexLocker>

using namespace KTextEditor;



// Turn debug messages on/off here
// #define FAST_DEBUG_ENABLE

#ifdef FAST_DEBUG_ENABLE
# define FAST_DEBUG(x) (kDebug() << x)
#else
# define FAST_DEBUG(x)
#endif



namespace {

class AddMenuManager {

private:
    QVector<QString> m_insertBefore;
    QVector<QString> m_insertAfter;
    QSet<QAction *> m_actionPointers;
    uint m_indexWalker;
    QMenu * m_menu;

public:
    AddMenuManager(QMenu * parent, int expectedItemCount)
            : m_insertBefore(QVector<QString>(expectedItemCount)),
            m_insertAfter(QVector<QString>(expectedItemCount)),
            m_indexWalker(0),
            m_menu(NULL) {
        Q_ASSERT(parent != NULL);
        m_menu = parent->addMenu(i18n("Add..."));
        if (m_menu == NULL) {
            return;
        }
        m_menu->setIcon(KIcon("list-add"));
    }

    void enableMenu(bool enabled) {
        if (m_menu == NULL) {
            return;
        }
        m_menu->setEnabled(enabled);
    }

    void addEntry(const QString & before, const QString after,
            const QString description, const QString & realBefore = QString(),
            const QString & realAfter = QString()) {
        if (m_menu == NULL) {
            return;
        }
        QAction * const action = m_menu->addAction(before + after + '\t' + description);
        m_insertBefore[m_indexWalker] = QString(realBefore.isEmpty() ? before : realBefore);
        m_insertAfter[m_indexWalker] = QString(realAfter.isEmpty() ? after : realAfter);
        action->setData(QVariant(m_indexWalker++));
        m_actionPointers.insert(action);
    }

    void addSeparator() {
        if (m_menu == NULL) {
            return;
        }
        m_menu->addSeparator();
    }

    void handle(QAction * action, QLineEdit * lineEdit) {
        if (!m_actionPointers.contains(action)) {
            return;
        }

        const int cursorPos = lineEdit->cursorPosition();
        const int index = action->data().toUInt();
        const QString & before = m_insertBefore[index];
        const QString & after = m_insertAfter[index];
        lineEdit->insert(before + after);
        lineEdit->setCursorPosition(cursorPos + before.count());
        lineEdit->setFocus();
    }
};

} // anon namespace



KateSearchBar::KateSearchBar(bool initAsPower, KateView* kateView, QWidget* parent)
        : KateViewBarWidget(true, kateView, parent),
        m_topRange(NULL),
        m_rangeNotifier(new KTextEditor::SmartRangeNotifier),
        m_layout(new QVBoxLayout()),
        m_widget(NULL),
        m_incUi(NULL),
        m_incMenu(NULL),
        m_incMenuMatchCase(NULL),
        m_incMenuFromCursor(NULL),
        m_incMenuHighlightAll(NULL),
        m_incInitCursor(0, 0),
        m_powerUi(NULL),
        m_powerMenu(NULL),
        m_powerMenuFromCursor(NULL),
        m_powerMenuHighlightAll(NULL),
        m_powerMenuSelectionOnly(NULL),
        m_incHighlightAll(false),
        m_incFromCursor(true),
        m_incMatchCase(false),
        m_powerMatchCase(true),
        m_powerFromCursor(false),
        m_powerHighlightAll(false),
        m_powerMode(0) {

    connect(m_rangeNotifier,SIGNAL(rangeContentsChanged(KTextEditor::SmartRange*)),
      this,SLOT(rangeContentsChanged(KTextEditor::SmartRange*)));

    // Modify parent
    QWidget * const widget = centralWidget();
    widget->setLayout(m_layout);
    m_layout->setMargin(2);

    // Init highlight
    {
      QMutexLocker lock(view()->doc()->smartMutex());
      
      m_topRange = view()->doc()->newSmartRange(view()->doc()->documentRange());
      static_cast<KateSmartRange*>(m_topRange)->setInternal();
      m_topRange->setInsertBehavior(SmartRange::ExpandLeft | SmartRange::ExpandRight);
      enableHighlights(true);
    }


    // Copy global to local config backup
    KateViewConfig * const globalConfig = KateGlobal::self()->viewConfig();
    const long searchFlags = globalConfig->searchFlags();
    m_incHighlightAll = (searchFlags & KateViewConfig::IncHighlightAll) != 0;
    m_incFromCursor = (searchFlags & KateViewConfig::IncFromCursor) != 0;
    m_incMatchCase = (searchFlags & KateViewConfig::IncMatchCase) != 0;
    m_powerMatchCase = (searchFlags & KateViewConfig::PowerMatchCase) != 0;
    m_powerFromCursor = (searchFlags & KateViewConfig::PowerFromCursor) != 0;
    m_powerHighlightAll = (searchFlags & KateViewConfig::PowerHighlightAll) != 0;
    m_powerMode = ((searchFlags & KateViewConfig::PowerModeRegularExpression) != 0)
            ? MODE_REGEX
            : (((searchFlags & KateViewConfig::PowerModeEscapeSequences) != 0)
                ? MODE_ESCAPE_SEQUENCES
                : (((searchFlags & KateViewConfig::PowerModeWholeWords) != 0)
                    ? MODE_WHOLE_WORDS
                    : MODE_PLAIN_TEXT));


    // Load one of either dialogs
    if (initAsPower) {
        onMutatePower();
    } else {
        onMutateIncremental();
    }
}



KateSearchBar::~KateSearchBar() {
//  delete m_topRange; this gets deleted somewhere else (bug #176027)
    delete m_layout;
    delete m_widget;

    delete m_incUi;
    delete m_incMenu;

    delete m_powerUi;
    delete m_powerMenu;
}



void KateSearchBar::findNext() {
    if (m_incUi != NULL) {
        onIncNext();
    } else {
        onPowerFindNext();
    }
}



void KateSearchBar::findPrevious() {
    if (m_incUi != NULL) {
        onIncPrev();
    } else {
        onPowerFindPrev();
    }
}



void KateSearchBar::highlight(const Range & range, const QColor & color) {
    SmartRange * const highlight = view()->doc()->newSmartRange(range, m_topRange);
    highlight->setInsertBehavior(SmartRange::DoNotExpand);
    Attribute::Ptr attribute(new Attribute());
    attribute->setBackground(color);
    highlight->setAttribute(attribute);
    highlight->addNotifier(m_rangeNotifier);
}



void KateSearchBar::highlightMatch(const Range & range) {
    highlight(range, Qt::yellow); // TODO make this part of the color scheme
}



void KateSearchBar::highlightReplacement(const Range & range) {
    highlight(range, Qt::green); // TODO make this part of the color scheme
}



void KateSearchBar::highlightAllMatches(const QString & pattern,
        Search::SearchOptions searchOptions) {
    onForAll(pattern, view()->doc()->documentRange(),
            searchOptions, NULL);
}

void KateSearchBar::rangeContentsChanged(KTextEditor::SmartRange* range) {
  neutralMatch();
  Attribute::Ptr attribute(new Attribute());
  //attribute->setBackground(color);
  range->setAttribute(attribute);

}

void KateSearchBar::neutralMatch() {
    if (m_incUi != NULL) {
        QPalette background(m_incUi->pattern->palette());
        KColorScheme::adjustBackground(background, KColorScheme::NeutralBackground);
        m_incUi->pattern->setPalette(background);
    } else {
        QLineEdit * const lineEdit = m_powerUi->pattern->lineEdit();
        Q_ASSERT(lineEdit != NULL);
        QPalette background(lineEdit->palette());
        KColorScheme::adjustBackground(background, KColorScheme::NeutralBackground);
        lineEdit->setPalette(background);
    }
}

void KateSearchBar::indicateMatch(bool wrapped) {
    if (m_incUi != NULL) {
        // Green background for line edit
        QPalette background(m_incUi->pattern->palette());
        KColorScheme::adjustBackground(background, KColorScheme::PositiveBackground);
        m_incUi->pattern->setPalette(background);

        // Update status label
        m_incUi->status->setText(wrapped
                ? i18n("Reached bottom, continued from top")
                : "");
    } else {
        // Green background for line edit
        QLineEdit * const lineEdit = m_powerUi->pattern->lineEdit();
        Q_ASSERT(lineEdit != NULL);
        QPalette background(lineEdit->palette());
        KColorScheme::adjustBackground(background, KColorScheme::PositiveBackground);
        lineEdit->setPalette(background);
    }
}



void KateSearchBar::indicateMismatch() {
    if (m_incUi != NULL) {
        // Red background for line edit
        QPalette background(m_incUi->pattern->palette());
        KColorScheme::adjustBackground(background, KColorScheme::NegativeBackground);
        m_incUi->pattern->setPalette(background);

        // Update status label
        m_incUi->status->setText(i18n("Not found"));
    } else {
        // Red background for line edit
        QLineEdit * const lineEdit = m_powerUi->pattern->lineEdit();
        Q_ASSERT(lineEdit != NULL);
        QPalette background(lineEdit->palette());
        KColorScheme::adjustBackground(background, KColorScheme::NegativeBackground);
        lineEdit->setPalette(background);
    }
}



void KateSearchBar::indicateNothing() {
    if (m_incUi != NULL) {
        // Reset background of line edit
        m_incUi->pattern->setPalette(QPalette());

        // Update status label
        m_incUi->status->setText("");
    } else {
        // Reset background of line edit
        QLineEdit * const lineEdit = m_powerUi->pattern->lineEdit();
        Q_ASSERT(lineEdit != NULL);
        // ### this is fragile (depends on knowledge of QPalette::ColorGroup)
        // ...would it better to cache the original palette?
        QColor color = QPalette().color(QPalette::Base);
        QPalette background(lineEdit->palette());
        background.setBrush(QPalette::Active, QPalette::Base, QPalette().brush(QPalette::Active, QPalette::Base));
        background.setBrush(QPalette::Inactive, QPalette::Base, QPalette().brush(QPalette::Inactive, QPalette::Base));
        background.setBrush(QPalette::Disabled, QPalette::Base, QPalette().brush(QPalette::Disabled, QPalette::Base));
        lineEdit->setPalette(background);
    }
}



/*static*/ void KateSearchBar::selectRange(KateView * view, const KTextEditor::Range & range) {
    view->setCursorPositionInternal(range.start(), 1);

    // don't make a selection if the vi input mode is used
    if (!view->viInputMode())
        view->setSelection(range);
}



void KateSearchBar::buildReplacement(QString & output, QList<ReplacementPart> & parts,
        const QVector<Range> & details, int replacementCounter) {
    const int MIN_REF_INDEX = 0;
    const int MAX_REF_INDEX = details.count() - 1;

    output.clear();
    ReplacementPart::Type caseConversion = ReplacementPart::KeepCase;
    for (QList<ReplacementPart>::iterator iter = parts.begin(); iter != parts.end(); iter++) {
        ReplacementPart & curPart = *iter;
        switch (curPart.type) {
        case ReplacementPart::Reference:
            if ((curPart.index < MIN_REF_INDEX) || (curPart.index > MAX_REF_INDEX)) {
                // Insert just the number to be consistent with QRegExp ("\c" becomes "c")
                output.append(QString::number(curPart.index));
            } else {
                const Range & captureRange = details[curPart.index];
                if (captureRange.isValid()) {
                    // Copy capture content
                    const bool blockMode = view()->blockSelection();
                    const QString content = view()->doc()->text(captureRange, blockMode);
                    switch (caseConversion) {
                    case ReplacementPart::UpperCase:
                        // Copy as uppercase
                        output.append(content.toUpper());
                        break;

                    case ReplacementPart::LowerCase:
                        // Copy as lowercase
                        output.append(content.toLower());
                        break;

                    case ReplacementPart::KeepCase: // FALLTHROUGH
                    default:
                        // Copy unmodified
                        output.append(content);
                        break;

                    }
                }
            }
            break;

        case ReplacementPart::UpperCase: // FALLTHROUGH
        case ReplacementPart::LowerCase: // FALLTHROUGH
        case ReplacementPart::KeepCase:
            caseConversion = curPart.type;
            break;

        case ReplacementPart::Counter:
            {
                // Zero padded counter value
                const int minWidth = curPart.index;
                const int number = replacementCounter;
                output.append(QString("%1").arg(number, minWidth, 10, QLatin1Char('0')));
            }
            break;

        case ReplacementPart::Text: // FALLTHROUGH
        default:
            switch (caseConversion) {
            case ReplacementPart::UpperCase:
                // Copy as uppercase
                output.append(curPart.text.toUpper());
                break;

            case ReplacementPart::LowerCase:
                // Copy as lowercase
                output.append(curPart.text.toLower());
                break;

            case ReplacementPart::KeepCase: // FALLTHROUGH
            default:
                // Copy unmodified
                output.append(curPart.text);
                break;

            }
            break;

        }
    }
}



void KateSearchBar::replaceMatch(const QVector<Range> & match, const QString & replacement,
        int replacementCounter) {
    // Placeholders depending on search mode
    bool usePlaceholders = false;
    switch (m_powerUi->searchMode->currentIndex()) {
    case MODE_REGEX: // FALLTHROUGH
    case MODE_ESCAPE_SEQUENCES:
        usePlaceholders = true;
        break;

    default:
        break;

    }

    const Range & targetRange = match[0];
    QString finalReplacement;
    if (usePlaceholders) {
        // Resolve references and escape sequences
        QList<ReplacementPart> parts;
        QString writableHack(replacement);
        const bool REPLACEMENT_GOODIES = true;
        KateDocument::escapePlaintext(writableHack, &parts, REPLACEMENT_GOODIES);
        buildReplacement(finalReplacement, parts, match, replacementCounter);
    } else {
        // Plain text replacement
        finalReplacement = replacement;
    }

    const bool blockMode = (view()->blockSelection() && !targetRange.onSingleLine());
    view()->doc()->replaceText(targetRange, finalReplacement, blockMode);
}



void KateSearchBar::onIncPatternChanged(const QString & pattern, bool invokedByUserAction) {
    if (pattern.isEmpty()) {
        if (invokedByUserAction) {
            // Kill selection
            view()->setSelection(Range::invalid());

            // Kill highlight
            resetHighlights();
        }

        // Reset edit color
        indicateNothing();

        // Disable next/prev
        m_incUi->next->setDisabled(true);
        m_incUi->prev->setDisabled(true);
        return;
    }

    // Enable next/prev
    m_incUi->next->setDisabled(false);
    m_incUi->prev->setDisabled(false);

    if (invokedByUserAction) {
        // How to find?
        Search::SearchOptions enabledOptions(KTextEditor::Search::Default);
        const bool matchCase = isChecked(m_incMenuMatchCase);
        if (!matchCase) {
            enabledOptions |= Search::CaseInsensitive;
        }


        // Where to find?
        Range inputRange;
        const bool fromCursor = isChecked(m_incMenuFromCursor);
        if (fromCursor) {
            inputRange.setRange(m_incInitCursor, view()->doc()->documentEnd());
        } else {
            inputRange = view()->doc()->documentRange();
        }

        // Find, first try
        const QVector<Range> resultRanges = view()->doc()->searchText(inputRange, pattern, enabledOptions);
        const Range & match = resultRanges[0];

        bool found = false;
        if (match.isValid()) {
            selectRange(view(), match);
            const bool NOT_WRAPPED = false;
            indicateMatch(NOT_WRAPPED);
            found = true;
        } else {
            // Wrap if it makes sense
            if (fromCursor) {
                // Find, second try
                inputRange = view()->doc()->documentRange();
                const QVector<Range> resultRanges2 = view()->doc()->searchText(inputRange, pattern, enabledOptions);
                const Range & match2 = resultRanges2[0];
                if (match2.isValid()) {
                    selectRange(view(), match2);
                    const bool WRAPPED = true;
                    indicateMatch(WRAPPED);
                    found = true;
                } else {
                    indicateMismatch();
                }
            } else {
                indicateMismatch();
            }
        }

        // Highlight all
        if (isChecked(m_incMenuHighlightAll)) {
            if (found ) {
                highlightAllMatches(pattern, enabledOptions);
            } else {
                resetHighlights();
            }
        }
        if (!found) {
          view()->setSelection(Range::invalid());
        }
    }
}



void KateSearchBar::onIncNext() {
    const bool FIND = false;
    onStep(FIND);
}



void KateSearchBar::onIncPrev() {
    const bool FIND = false;
    const bool BACKWARDS = false;
    onStep(FIND, BACKWARDS);
}



void KateSearchBar::onIncMatchCaseToggle(bool invokedByUserAction) {
    if (invokedByUserAction) {
        sendConfig();

        // Re-search with new settings
        const QString pattern = m_incUi->pattern->displayText();
        onIncPatternChanged(pattern);
    }
}



void KateSearchBar::onIncHighlightAllToggle(bool checked, bool invokedByUserAction) {
    if (invokedByUserAction) {
        sendConfig();

        if (checked) {
            const QString pattern = m_incUi->pattern->displayText();
            if (!pattern.isEmpty()) {
                // How to search while highlighting?
                Search::SearchOptions enabledOptions(KTextEditor::Search::Default);
                const bool matchCase = isChecked(m_incMenuMatchCase);
                if (!matchCase) {
                    enabledOptions |= Search::CaseInsensitive;
                }

                // Highlight them all
                resetHighlights();
                highlightAllMatches(pattern, enabledOptions);
            }
        } else {
            resetHighlights();
        }
    }
}



void KateSearchBar::onIncFromCursorToggle(bool invokedByUserAction) {
    if (invokedByUserAction) {
        sendConfig();
    }
}



void KateSearchBar::fixForSingleLine(Range & range, bool forwards) {
    FAST_DEBUG("Single-line workaround checking BEFORE" << range);
    if (forwards) {
        const int line = range.start().line();
        const int col = range.start().column();
        const int maxColWithNewline = view()->doc()->lineLength(line) + 1;
        if (col == maxColWithNewline) {
            FAST_DEBUG("Starting on a newline" << range);
            const int maxLine = view()->doc()->lines() - 1;
            if (line < maxLine) {
                range.setRange(Cursor(line + 1, 0), range.end());
                FAST_DEBUG("Search range fixed to " << range);
            } else {
                FAST_DEBUG("Already at last line");
                range = Range::invalid();
            }
        }
    } else {
        const int col = range.end().column();
        if (col == 0) {
            FAST_DEBUG("Ending after a newline" << range);
            const int line = range.end().line();
            if (line > 0) {
                const int maxColWithNewline = view()->doc()->lineLength(line - 1);
                range.setRange(range.start(), Cursor(line - 1, maxColWithNewline));
                FAST_DEBUG("Search range fixed to " << range);
            } else {
                FAST_DEBUG("Already at first line");
                range = Range::invalid();
            }
        }
    }
    FAST_DEBUG("Single-line workaround checking  AFTER" << range);
}



void KateSearchBar::onReturnPressed() {
    const Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
    const bool shiftDown = (modifiers & Qt::ShiftModifier) != 0;
    const bool controlDown = (modifiers & Qt::ControlModifier) != 0;

    if (shiftDown) {
        // Shift down, search backwards
        if (m_powerUi != NULL) {
            onPowerFindPrev();
        } else {
            onIncPrev();
        }
    } else {
        // Shift up, search forwards
        if (m_powerUi != NULL) {
            onPowerFindNext();
        } else {
            onIncNext();
        }
    }

    if (controlDown) {
        emit hideMe();
    }
}



bool KateSearchBar::onStep(bool replace, bool forwards) {
    // What to find?
    const QString pattern = (m_powerUi != NULL)
            ? m_powerUi->pattern->currentText()
            : m_incUi->pattern->displayText();
    if (pattern.isEmpty()) {
        return false; // == Pattern error
    }

    // How to find?
    Search::SearchOptions enabledOptions(KTextEditor::Search::Default);
    const bool matchCase = (m_powerUi != NULL)
            ? isChecked(m_powerUi->matchCase)
            : isChecked(m_incMenuMatchCase);
    if (!matchCase) {
        enabledOptions |= Search::CaseInsensitive;
    }

    if (!forwards) {
        enabledOptions |= Search::Backwards;
    }

    bool multiLinePattern = false;
    bool regexMode = false;
    if (m_powerUi != NULL) {
        switch (m_powerUi->searchMode->currentIndex()) {
        case MODE_WHOLE_WORDS:
            enabledOptions |= Search::WholeWords;
            break;

        case MODE_ESCAPE_SEQUENCES:
            enabledOptions |= Search::EscapeSequences;
            break;

        case MODE_REGEX:
            {
                // Check if pattern multi-line
                QString patternCopy(pattern);
                KateDocument::repairPattern(patternCopy, multiLinePattern);
                regexMode = true;
            }
            enabledOptions |= Search::Regex;
            break;

        case MODE_PLAIN_TEXT: // FALLTHROUGH
        default:
            break;

        }
    }


    // Where to find?
    Range inputRange;
    Range selection;
    const bool selected = view()->selection();
    const bool selectionOnly = (m_powerUi != NULL)
            ? isChecked(m_powerMenuSelectionOnly)
            : false;
    if (selected) {
        selection = view()->selectionRange();
        if (selectionOnly) {
            // First match in selection
            inputRange = selection;
        } else {
            // Next match after/before selection if a match was selected before
            if (forwards) {
                inputRange.setRange(selection.start(), view()->doc()->documentEnd());
            } else {
                inputRange.setRange(Cursor(0, 0), selection.end());
            }
        }
    } else {
        // No selection
        const bool fromCursor = (m_powerUi != NULL)
                ? isChecked(m_powerMenuFromCursor)
                : isChecked(m_incMenuFromCursor);
        if (fromCursor) {
            const Cursor cursorPos = view()->cursorPosition();
            if (forwards) {
                // if the vi input mode is used, the cursor will stay a the first character of the
                // matched pattern (no selection will be made), so the next search should start from
                // match column + 1
                if (!view()->viInputMode()) {
                    inputRange.setRange(cursorPos, view()->doc()->documentEnd());
                } else {
                    inputRange.setRange(Cursor(cursorPos.line(), cursorPos.column()+1), view()->doc()->documentEnd());
                }
            } else {
                inputRange.setRange(Cursor(0, 0), cursorPos);
            }
        } else {
            inputRange = view()->doc()->documentRange();
        }
    }
    FAST_DEBUG("Search range is" << inputRange);

    // Single-line pattern workaround
    if (regexMode && !multiLinePattern) {
        fixForSingleLine(inputRange, forwards);
    }


    // Find, first try
    const QVector<Range> resultRanges = view()->doc()->searchText(inputRange, pattern, enabledOptions);
    const Range & match = resultRanges[0];
    bool wrap = false;
    bool found = false;
    SmartRange * afterReplace = NULL;
    if (match.isValid()) {
        // Previously selected match again?
        if (selected && (match == selection) && (!selectionOnly || replace)) {
            // Same match again
            if (replace) {
                // Selection is match -> replace
                const QString replacement = m_powerUi->replacement->currentText();
                afterReplace = view()->doc()->newSmartRange(match);
                afterReplace->setInsertBehavior(SmartRange::ExpandRight | SmartRange::ExpandLeft);
                replaceMatch(resultRanges, replacement);

                // Find, second try after replaced text
                if (forwards) {
                    inputRange.setRange(afterReplace->end(), inputRange.end());
                } else {
                    inputRange.setRange(inputRange.start(), afterReplace->start());
                }
            } else {
                // Find, second try after old selection
                if (forwards) {
                    inputRange.setRange(selection.end(), inputRange.end());
                } else {
                    inputRange.setRange(inputRange.start(), selection.start());
                }
            }

            // Single-line pattern workaround
            fixForSingleLine(inputRange, forwards);

            const QVector<Range> resultRanges2 = view()->doc()->searchText(inputRange, pattern, enabledOptions);
            const Range & match2 = resultRanges2[0];
            if (match2.isValid()) {
                selectRange(view(), match2);
                found = true;
                const bool NOT_WRAPPED = false;
                indicateMatch(NOT_WRAPPED);
            } else {
                // Find, third try from doc start on
                wrap = true;
            }
        } else {
            selectRange(view(), match);
            found = true;
            const bool NOT_WRAPPED = false;
            indicateMatch(NOT_WRAPPED);
        }
    } else if (!selected || !selectionOnly) {
        // Find, second try from doc start on
        wrap = true;
    }

    // Wrap around
    if (wrap) {
        inputRange = view()->doc()->documentRange();
        const QVector<Range> resultRanges3 = view()->doc()->searchText(inputRange, pattern, enabledOptions);
        const Range & match3 = resultRanges3[0];
        if (match3.isValid()) {
            // Previously selected match again?
            if (selected && !selectionOnly && (match3 == selection)) {
                // NOOP, same match again
            } else {
                selectRange(view(), match3);
                found = true;
            }
            const bool WRAPPED = true;
            indicateMatch(WRAPPED);
        } else {
            indicateMismatch();
        }
    }

    // Highlight all matches and/or replacement
    const bool highlightAll = (m_powerUi != NULL)
            ? isChecked(m_powerMenuHighlightAll)
            : isChecked(m_incMenuHighlightAll);
    if ((found && highlightAll) || (afterReplace != NULL)) {
        // Highlight all matches
        if (found && highlightAll) {
            highlightAllMatches(pattern, enabledOptions);
        }

        // Highlight replacement (on top if overlapping) if new match selected
        if (found && (afterReplace != NULL)) {
            // Note: highlightAllMatches already reset for us
            if (!(found && highlightAll)) {
                resetHighlights();
            }

            highlightReplacement(*afterReplace);
        }

    }

    delete afterReplace;

    return true; // == No pattern error
}



void KateSearchBar::onPowerPatternChanged(const QString & pattern) {
    givePatternFeedback(pattern);
    indicateNothing();
}



void KateSearchBar::givePatternFeedback(const QString & pattern) {
    bool enabled = true;

    if (pattern.isEmpty()) {
        enabled = false;
    } else {
        switch (m_powerUi->searchMode->currentIndex()) {
        case MODE_WHOLE_WORDS:
            if (pattern.trimmed() != pattern) {
                enabled = false;
            }
            break;

        case MODE_REGEX:
            m_patternTester.setPattern(pattern);
            enabled = m_patternTester.isValid();
            break;

        case MODE_ESCAPE_SEQUENCES: // FALLTHROUGH
        case MODE_PLAIN_TEXT: // FALLTHROUGH
        default:
            ; // NOOP

        }
    }

    // Enable/disable next/prev and replace next/all
    m_powerUi->findNext->setDisabled(!enabled);
    m_powerUi->findPrev->setDisabled(!enabled);
    m_powerUi->replaceNext->setDisabled(!enabled);
    m_powerUi->replaceAll->setDisabled(!enabled);
}



void KateSearchBar::addCurrentTextToHistory(QComboBox * combo) {
    const QString text = combo->currentText();
    const int index = combo->findText(text);
    if (index != -1) {
        combo->removeItem(index);
    }
    combo->insertItem(0, text);
    combo->setCurrentIndex(0);
}



void KateSearchBar::backupConfig(bool ofPower) {
    if (ofPower) {
        m_powerMatchCase = isChecked(m_powerUi->matchCase);
        m_powerFromCursor = isChecked(m_powerMenuFromCursor);
        m_powerHighlightAll = isChecked(m_powerMenuHighlightAll);
        m_powerMode = m_powerUi->searchMode->currentIndex();
    } else {
        m_incHighlightAll = isChecked(m_incMenuHighlightAll);
        m_incFromCursor = isChecked(m_incMenuFromCursor);
        m_incMatchCase = isChecked(m_incMenuMatchCase);
    }
}



void KateSearchBar::sendConfig() {
    KateViewConfig * const globalConfig = KateGlobal::self()->viewConfig();
    const long pastFlags = globalConfig->searchFlags();
    long futureFlags = pastFlags;

    if (m_powerUi != NULL) {
        const bool OF_POWER = true;
        backupConfig(OF_POWER);

        // Update power search flags only
        const long incFlagsOnly = pastFlags
                & (KateViewConfig::IncHighlightAll
                    | KateViewConfig::IncFromCursor
                    | KateViewConfig::IncMatchCase);

        futureFlags = incFlagsOnly
            | (m_powerMatchCase ? KateViewConfig::PowerMatchCase : 0)
            | (m_powerFromCursor ? KateViewConfig::PowerFromCursor : 0)
            | (m_powerHighlightAll ? KateViewConfig::PowerHighlightAll : 0)
            | ((m_powerMode == MODE_REGEX)
                ? KateViewConfig::PowerModeRegularExpression
                : ((m_powerMode == MODE_ESCAPE_SEQUENCES)
                    ? KateViewConfig::PowerModeEscapeSequences
                    : ((m_powerMode == MODE_WHOLE_WORDS)
                        ? KateViewConfig::PowerModeWholeWords
                        : KateViewConfig::PowerModePlainText)));

    } else if (m_incUi != NULL) {
        const bool OF_INCREMENTAL = false;
        backupConfig(OF_INCREMENTAL);

        // Update incremental search flags only
        const long powerFlagsOnly = pastFlags
                & (KateViewConfig::PowerMatchCase
                    | KateViewConfig::PowerFromCursor
                    | KateViewConfig::PowerHighlightAll
                    | KateViewConfig::PowerModeRegularExpression
                    | KateViewConfig::PowerModeEscapeSequences
                    | KateViewConfig::PowerModeWholeWords
                    | KateViewConfig::PowerModePlainText);

        futureFlags = powerFlagsOnly
                | (m_incHighlightAll ? KateViewConfig::IncHighlightAll : 0)
                | (m_incFromCursor ? KateViewConfig::IncFromCursor : 0)
                | (m_incMatchCase ? KateViewConfig::IncMatchCase : 0);
    }

    // Adjust global config
    globalConfig->setSearchFlags(futureFlags);
}



void KateSearchBar::onPowerFindNext() {
    const bool FIND = false;
    if (onStep(FIND)) {
        // Add to search history
        addCurrentTextToHistory(m_powerUi->pattern);
    }
}



void KateSearchBar::onPowerFindPrev() {
    const bool FIND = false;
    const bool BACKWARDS = false;
    if (onStep(FIND, BACKWARDS)) {
        // Add to search history
        addCurrentTextToHistory(m_powerUi->pattern);
    }
}



void KateSearchBar::onPowerReplaceNext() {
    const bool REPLACE = true;
    if (onStep(REPLACE)) {
        // Add to search history
        addCurrentTextToHistory(m_powerUi->pattern);

        // Add to replace history
        addCurrentTextToHistory(m_powerUi->replacement);
    }
}



// replacement == NULL --> Highlight all matches
// replacement != NULL --> Replace and highlight all matches
void KateSearchBar::onForAll(const QString & pattern, Range inputRange,
        Search::SearchOptions enabledOptions,
        const QString * replacement) {
    bool multiLinePattern = false;
    const bool regexMode = enabledOptions.testFlag(Search::Regex);
    if (regexMode) {
        // Check if pattern multi-line
        QString patternCopy(pattern);
        KateDocument::repairPattern(patternCopy, multiLinePattern);
    }

    // Clear backwards flag, this algorithm is for forward mode
    if (enabledOptions.testFlag(Search::Backwards)) {
        enabledOptions &= ~Search::SearchOptions(Search::Backwards);
    }

    // Before first match
    resetHighlights();

    SmartRange * const workingRange = view()->doc()->newSmartRange(inputRange);
    QList<Range> highlightRanges;
    int matchCounter = 0;
    for (;;) {
        const QVector<Range> resultRanges = view()->doc()->searchText(*workingRange, pattern, enabledOptions);
        Range match = resultRanges[0];
        if (!match.isValid()) {
            break;
        }
        bool const originalMatchEmpty = match.isEmpty();

        // Work with the match
        if (replacement != NULL) {
            if (matchCounter == 0) {
                view()->doc()->editBegin();
            }

            // Track replacement operation
            SmartRange * const afterReplace = view()->doc()->newSmartRange(match);
            afterReplace->setInsertBehavior(SmartRange::ExpandRight | SmartRange::ExpandLeft);

            // Replace
            replaceMatch(resultRanges, *replacement, ++matchCounter);

            // Highlight and continue after adjusted match
            //highlightReplacement(*afterReplace);
            match = *afterReplace;
            highlightRanges << match;
            delete afterReplace;
        } else {
            // Highlight and continue after original match
            //highlightMatch(match);
            highlightRanges << match;
            matchCounter++;
        }

        // Continue after match
        SmartCursor & workingStart = workingRange->smartStart();
        workingStart.setPosition(match.end());
        if (originalMatchEmpty) {
            // Can happen for regex patterns like "^".
            // If we don't advance here we will loop forever...
            workingStart.advance(1);
        } else if (regexMode && !multiLinePattern && workingStart.atEndOfLine()) {
            // single-line regexps might match the naked line end
            // therefore we better advance to the next line
            workingStart.advance(1);
        }

        // Are we done?
        if (!workingRange->isValid() || workingStart.atEndOfDocument()) {
            break;
        }
    }

    // After last match
    if (matchCounter > 0) {
        if (replacement != NULL) {
            view()->doc()->editEnd();
        }
    }

    if (replacement != NULL)
        foreach (Range r, highlightRanges) {
            highlightMatch(r);
        }
    else
        foreach (Range r, highlightRanges) {
            highlightReplacement(r);
        }

    delete workingRange;
}



void KateSearchBar::onPowerReplaceAll() {
    // What to find/replace?
    const QString pattern = m_powerUi->pattern->currentText();
    const QString replacement = m_powerUi->replacement->currentText();


    // How to find?
    Search::SearchOptions enabledOptions(KTextEditor::Search::Default);
    const bool matchCase = isChecked(m_powerUi->matchCase);
    if (!matchCase) {
        enabledOptions |= Search::CaseInsensitive;
    }

    if (m_powerUi != NULL) {
        switch (m_powerUi->searchMode->currentIndex()) {
        case MODE_WHOLE_WORDS:
            enabledOptions |= Search::WholeWords;
            break;

        case MODE_ESCAPE_SEQUENCES:
            enabledOptions |= Search::EscapeSequences;
            break;

        case MODE_REGEX:
            enabledOptions |= Search::Regex;
            break;

        case MODE_PLAIN_TEXT: // FALLTHROUGH
        default:
            break;

        }
    }


    // Where to replace?
    Range selection;
    const bool selected = view()->selection();
    const bool selectionOnly = isChecked(m_powerMenuSelectionOnly);
    Range inputRange = (selected && selectionOnly)
            ? view()->selectionRange()
            : view()->doc()->documentRange();


    // Pass on the hard work
    onForAll(pattern, inputRange, enabledOptions, &replacement);


    // Add to search history
    addCurrentTextToHistory(m_powerUi->pattern);

    // Add to replace history
    addCurrentTextToHistory(m_powerUi->replacement);
}



struct ParInfo {
    int openIndex;
    bool capturing;
    int captureNumber; // 1..9
};



QVector<QString> KateSearchBar::getCapturePatterns(const QString & pattern) {
    QVector<QString> capturePatterns;
    capturePatterns.reserve(9);
    QStack<ParInfo> parInfos;

    const int inputLen = pattern.length();
    int input = 0; // walker index
    bool insideClass = false;
    int captureCount = 0;

    while (input < inputLen) {
        if (insideClass) {
            // Wait for closing, unescaped ']'
            if (pattern[input].unicode() == L']') {
                insideClass = false;
            }
            input++;
        }
        else
        {
            switch (pattern[input].unicode())
            {
            case L'\\':
                // Skip this and any next character
                input += 2;
                break;

            case L'(':
                ParInfo curInfo;
                curInfo.openIndex = input;
                curInfo.capturing = (input + 1 >= inputLen) || (pattern[input + 1].unicode() != '?');
                if (curInfo.capturing) {
                    captureCount++;
                }
                curInfo.captureNumber = captureCount;
                parInfos.push(curInfo);

                input++;
                break;

            case L')':
                if (!parInfos.empty()) {
                    ParInfo & top = parInfos.top();
                    if (top.capturing && (top.captureNumber <= 9)) {
                        const int start = top.openIndex + 1;
                        const int len = input - start;
                        if (capturePatterns.size() < top.captureNumber) {
                            capturePatterns.resize(top.captureNumber);
                        }
                        capturePatterns[top.captureNumber - 1] = pattern.mid(start, len);
                    }
                    parInfos.pop();
                }

                input++;
                break;

            case L'[':
                input++;
                insideClass = true;
                break;

            default:
                input++;
                break;

            }
        }
    }

    return capturePatterns;
}



void KateSearchBar::showExtendedContextMenu(bool forPattern) {
    // Make original menu
    QMenu * const contextMenu = m_powerUi->pattern->lineEdit()->createStandardContextMenu();
    if (contextMenu == NULL) {
        return;
    }

    bool extendMenu = false;
    bool regexMode = false;
    switch (m_powerUi->searchMode->currentIndex()) {
    case MODE_REGEX: 
        regexMode = true;
        // FALLTHROUGH

    case MODE_ESCAPE_SEQUENCES:
        extendMenu = true;
        break;

    default:
        break;
    }

    AddMenuManager addMenuManager(contextMenu, 35);
    if (!extendMenu) {
        addMenuManager.enableMenu(extendMenu);
    } else {
        // Build menu
        if (forPattern) {
            if (regexMode) {
                addMenuManager.addEntry("^", "", i18n("Beginning of line"));
                addMenuManager.addEntry("$", "", i18n("End of line"));
                addMenuManager.addSeparator();
                addMenuManager.addEntry(".", "", i18n("Any single character (excluding line breaks)"));
                addMenuManager.addSeparator();
                addMenuManager.addEntry("+", "", i18n("One or more occurrences"));
                addMenuManager.addEntry("*", "", i18n("Zero or more occurrences"));
                addMenuManager.addEntry("?", "", i18n("Zero or one occurrences"));
                addMenuManager.addEntry("{a", ",b}", i18n("<a> through <b> occurrences"), "{", ",}");
                addMenuManager.addSeparator();
                addMenuManager.addEntry("(", ")", i18n("Group, capturing"));
                addMenuManager.addEntry("|", "", i18n("Or"));
                addMenuManager.addEntry("[", "]", i18n("Set of characters"));
                addMenuManager.addEntry("[^", "]", i18n("Negative set of characters"));
                addMenuManager.addSeparator();
            }
        } else {
            addMenuManager.addEntry("\\0", "", i18n("Whole match reference"));
            addMenuManager.addSeparator();
            if (regexMode) {
                const QString pattern = m_powerUi->pattern->currentText();
                const QVector<QString> capturePatterns = getCapturePatterns(pattern);
    
                const int captureCount = capturePatterns.count();
                for (int i = 1; i <= 9; i++) {
                    const QString number = QString::number(i);
                    const QString & captureDetails = (i <= captureCount)
                            ? (QString(" = (") + capturePatterns[i - 1].left(30)) + QString(")")
                            : QString();
                    addMenuManager.addEntry("\\" + number, "",
                            i18n("Reference") + ' ' + number + captureDetails);
                }
    
                addMenuManager.addSeparator();
            }
        }
    
        addMenuManager.addEntry("\\n", "", i18n("Line break"));
        addMenuManager.addEntry("\\t", "", i18n("Tab"));
    
        if (forPattern && regexMode) {
            addMenuManager.addEntry("\\b", "", i18n("Word boundary"));
            addMenuManager.addEntry("\\B", "", i18n("Not word boundary"));
            addMenuManager.addEntry("\\d", "", i18n("Digit"));
            addMenuManager.addEntry("\\D", "", i18n("Non-digit"));
            addMenuManager.addEntry("\\s", "", i18n("Whitespace (excluding line breaks)"));
            addMenuManager.addEntry("\\S", "", i18n("Non-whitespace (excluding line breaks)"));
            addMenuManager.addEntry("\\w", "", i18n("Word character (alphanumerics plus '_')"));
            addMenuManager.addEntry("\\W", "", i18n("Non-word character"));
        }
    
        addMenuManager.addEntry("\\0???", "", i18n("Octal character 000 to 377 (2^8-1)"), "\\0");
        addMenuManager.addEntry("\\x????", "", i18n("Hex character 0000 to FFFF (2^16-1)"), "\\x");
        addMenuManager.addEntry("\\\\", "", i18n("Backslash"));
    
        if (forPattern && regexMode) {
            addMenuManager.addSeparator();
            addMenuManager.addEntry("(?:E", ")", i18n("Group, non-capturing"), "(?:");
            addMenuManager.addEntry("(?=E", ")", i18n("Lookahead"), "(?=");
            addMenuManager.addEntry("(?!E", ")", i18n("Negative lookahead"), "(?!");
        }
    
        if (!forPattern) {
            addMenuManager.addSeparator();
            addMenuManager.addEntry("\\L", "", i18n("Begin lowercase conversion"));
            addMenuManager.addEntry("\\U", "", i18n("Begin uppercase conversion"));
            addMenuManager.addEntry("\\E", "", i18n("End case conversion"));
            addMenuManager.addEntry("\\#[#..]", "", i18n("Replacement counter (for Replace All)"), "\\#");
        }
    }

    // Show menu
    QAction * const result = contextMenu->exec(QCursor::pos());
    if (result != NULL) {
        QLineEdit * const lineEdit = forPattern
                ? m_powerUi->pattern->lineEdit()
                : m_powerUi->replacement->lineEdit();
        Q_ASSERT(lineEdit != NULL);
        addMenuManager.handle(result, lineEdit);
    }
}



void KateSearchBar::onPowerMatchCaseToggle(bool invokedByUserAction) {
    if (invokedByUserAction) {
        sendConfig();
        indicateNothing();
    }
}



void KateSearchBar::onPowerHighlightAllToggle(bool checked, bool invokedByUserAction) {
    if (invokedByUserAction) {
        sendConfig();

        if (checked) {
            const QString pattern = m_powerUi->pattern->currentText();
            if (!pattern.isEmpty()) {
                // How to search while highlighting?
                Search::SearchOptions enabledOptions(KTextEditor::Search::Default);
                const bool matchCase = isChecked(m_powerUi->matchCase);
                if (!matchCase) {
                    enabledOptions |= Search::CaseInsensitive;
                }

                switch (m_powerUi->searchMode->currentIndex()) {
                case MODE_WHOLE_WORDS:
                    enabledOptions |= Search::WholeWords;
                    break;

                case MODE_ESCAPE_SEQUENCES:
                    enabledOptions |= Search::EscapeSequences;
                    break;

                case MODE_REGEX:
                    enabledOptions |= Search::Regex;
                    break;

                case MODE_PLAIN_TEXT: // FALLTHROUGH
                default:
                    break;

                }

                // Highlight them all
                resetHighlights();
                highlightAllMatches(pattern, enabledOptions);
            }
        } else {
            resetHighlights();
        }
    }
}



void KateSearchBar::onPowerFromCursorToggle(bool invokedByUserAction) {
    if (invokedByUserAction) {
        sendConfig();
    }
}



void KateSearchBar::onPowerModeChangedPlainText() {
    m_powerUi->searchMode->setCurrentIndex(MODE_PLAIN_TEXT);
    onPowerModeChanged();
}



void KateSearchBar::onPowerModeChangedWholeWords() {
    m_powerUi->searchMode->setCurrentIndex(MODE_WHOLE_WORDS);
    onPowerModeChanged();
}



void KateSearchBar::onPowerModeChangedEscapeSequences() {
    m_powerUi->searchMode->setCurrentIndex(MODE_ESCAPE_SEQUENCES);
    onPowerModeChanged();
}



void KateSearchBar::onPowerModeChangedRegularExpression() {
    m_powerUi->searchMode->setCurrentIndex(MODE_REGEX);
    onPowerModeChanged();
}



void KateSearchBar::onPowerModeChanged() {
    if (m_powerUi->searchMode->currentIndex() == MODE_REGEX) {
        setChecked(m_powerUi->matchCase, true);
    }

    sendConfig();
    indicateNothing();
}



void KateSearchBar::onPowerModeChanged(int /*index*/, bool invokedByUserAction) {
    if (invokedByUserAction) {
        onPowerModeChanged();
    }

    givePatternFeedback(m_powerUi->pattern->currentText());
}



/*static*/ void KateSearchBar::nextMatchForSelection(KateView * view, bool forwards) {
    const bool selected = view->selection();
    if (selected) {
        const QString pattern = view->selectionText();

        // How to find?
        Search::SearchOptions enabledOptions(KTextEditor::Search::Default);
        if (!forwards) {
            enabledOptions |= Search::Backwards;
        }

        // Where to find?
        const Range selRange = view->selectionRange();
        Range inputRange;
        if (forwards) {
            inputRange.setRange(selRange.end(), view->doc()->documentEnd());
        } else {
            inputRange.setRange(Cursor(0, 0), selRange.start());
        }

        // Find, first try
        const QVector<Range> resultRanges = view->doc()->searchText(inputRange, pattern, enabledOptions);
        const Range & match = resultRanges[0];

        if (match.isValid()) {
            selectRange(view, match);
        } else {
            // Find, second try
            if (forwards) {
                inputRange.setRange(Cursor(0, 0), selRange.start());
            } else {
                inputRange.setRange(selRange.end(), view->doc()->documentEnd());
            }
            const QVector<Range> resultRanges2 = view->doc()->searchText(inputRange, pattern, enabledOptions);
            const Range & match2 = resultRanges2[0];
            if (match2.isValid()) {
                selectRange(view, match2);
            }
        }
    } else {
        // Select current word so we can search for that the next time
        const Cursor cursorPos = view->cursorPosition();
        view->selectWord(cursorPos);
    }
}



void KateSearchBar::onMutatePower() {
    QString initialPattern;
    bool selectionOnly = false;

    // Guess settings from context: init pattern with current selection
    const bool selected = view()->selection();
    if (selected) {
        const Range & selection = view()->selectionRange();
        if (selection.onSingleLine()) {
            // ... with current selection
            initialPattern = view()->selectionText();
        } else {
            // Enable selection only
            selectionOnly = true;
        }
    }

    // If there's no new selection, we'll use the existing pattern
    if (initialPattern.isNull()) {
        // Coming from power search?
        const bool fromReplace = (m_powerUi != NULL) && (m_widget->isVisible());
        if (fromReplace) {
            QLineEdit * const patternLineEdit = m_powerUi->pattern->lineEdit();
            Q_ASSERT(patternLineEdit != NULL);
            patternLineEdit->selectAll();
            m_powerUi->pattern->setFocus(Qt::MouseFocusReason);
            return;
        }

        // Coming from incremental search?
        const bool fromIncremental = (m_incUi != NULL) && (m_widget->isVisible());
        if (fromIncremental) {
            initialPattern = m_incUi->pattern->displayText();
        }
    }

    // Create dialog
    const bool create = (m_powerUi == NULL);
    if (create) {
        // Kill incremental widget
        if (m_incUi != NULL) {
            // Backup current settings
            const bool OF_INCREMENTAL = false;
            backupConfig(OF_INCREMENTAL);

            // Kill widget
            delete m_incUi;
            delete m_incMenu;
            m_incUi = NULL;
            m_incMenu = NULL;
            m_incMenuMatchCase = NULL;
            m_incMenuFromCursor = NULL;
            m_incMenuHighlightAll = NULL;
            m_layout->removeWidget(m_widget);
            m_widget->deleteLater(); // I didn't get a crash here but for symmetrie to the other mutate slot^
        }

        // Add power widget
        m_widget = new QWidget(this);
        m_powerUi = new Ui::PowerSearchBar;
        m_powerUi->setupUi(m_widget);
        m_layout->addWidget(m_widget);

        // Bind to shared history models
        const int MAX_HISTORY_SIZE = 100; // Please don't lower this value! Thanks, Sebastian
        QStringListModel * const patternHistoryModel = KateHistoryModel::getPatternHistoryModel();
        QStringListModel * const replacementHistoryModel = KateHistoryModel::getReplacementHistoryModel();
        m_powerUi->pattern->setMaxCount(MAX_HISTORY_SIZE);
        m_powerUi->pattern->setModel(patternHistoryModel);
        m_powerUi->replacement->setMaxCount(MAX_HISTORY_SIZE);
        m_powerUi->replacement->setModel(replacementHistoryModel);

        // Fill options menu
        m_powerMenu = new QMenu();
        m_powerUi->options->setMenu(m_powerMenu);
        m_powerMenuFromCursor = m_powerMenu->addAction(i18n("From &cursor"));
        m_powerMenuFromCursor->setCheckable(true);
        m_powerMenuHighlightAll = m_powerMenu->addAction(i18n("Hi&ghlight all"));
        m_powerMenuHighlightAll->setCheckable(true);
        m_powerMenuSelectionOnly = m_powerMenu->addAction(i18n("Selection &only"));
        m_powerMenuSelectionOnly->setCheckable(true);

#if 0 // perhaps make actions for this, perhaps let be, don't seems to me that important to grab such prominent shortcuts
        // Grab Alt+1 .. Alt+4 for search mode switching
        connect(new QShortcut(QKeySequence(Qt::ALT + Qt::Key_1), m_widget,
                0, 0, Qt::WidgetWithChildrenShortcut), SIGNAL(activated()),
                this, SLOT(onPowerModeChangedPlainText()));
        connect(new QShortcut(QKeySequence(Qt::ALT + Qt::Key_2), m_widget,
                0, 0, Qt::WidgetWithChildrenShortcut), SIGNAL(activated()),
                this, SLOT(onPowerModeChangedWholeWords()));
        connect(new QShortcut(QKeySequence(Qt::ALT + Qt::Key_3), m_widget,
                0, 0, Qt::WidgetWithChildrenShortcut), SIGNAL(activated()),
                this, SLOT(onPowerModeChangedEscapeSequences()));
        connect(new QShortcut(QKeySequence(Qt::ALT + Qt::Key_4), m_widget,
                0, 0, Qt::WidgetWithChildrenShortcut), SIGNAL(activated()),
                this, SLOT(onPowerModeChangedRegularExpression()));
#endif

        // Icons
        m_powerUi->mutate->setIcon(KIcon("arrow-down-double"));
        m_powerUi->findNext->setIcon(KIcon("go-down"));
        m_powerUi->findPrev->setIcon(KIcon("go-up"));

        // Focus proxy
        centralWidget()->setFocusProxy(m_powerUi->pattern);

        // Make completers case-sensitive
        QLineEdit * const patternLineEdit = m_powerUi->pattern->lineEdit();
        Q_ASSERT(patternLineEdit != NULL);
        patternLineEdit->completer()->setCaseSensitivity(Qt::CaseSensitive);

        QLineEdit * const replacementLineEdit = m_powerUi->pattern->lineEdit();
        Q_ASSERT(replacementLineEdit != NULL);
        replacementLineEdit->completer()->setCaseSensitivity(Qt::CaseSensitive);
    }

    setChecked(m_powerMenuSelectionOnly, selectionOnly);

    // Restore previous settings
    if (create) {
        setChecked(m_powerUi->matchCase, m_powerMatchCase);
        setChecked(m_powerMenuHighlightAll, m_powerHighlightAll);
        setChecked(m_powerMenuFromCursor, m_powerFromCursor);
        m_powerUi->searchMode->setCurrentIndex(m_powerMode);
    }

    // Set initial search pattern
    QLineEdit * const patternLineEdit = m_powerUi->pattern->lineEdit();
    Q_ASSERT(patternLineEdit != NULL);
    patternLineEdit->setText(initialPattern);
    patternLineEdit->selectAll();

    // Set initial replacement text
    QLineEdit * const replacementLineEdit = m_powerUi->replacement->lineEdit();
    Q_ASSERT(replacementLineEdit != NULL);
    replacementLineEdit->setText("");

    // Propagate settings (slots are still inactive on purpose)
    onPowerPatternChanged(initialPattern);
    const bool NOT_INVOKED_BY_USER_ACTION = false;
    onPowerModeChanged(m_powerUi->searchMode->currentIndex(), NOT_INVOKED_BY_USER_ACTION);

    if (create) {
        // Slots
        connect(m_powerUi->mutate, SIGNAL(clicked()), this, SLOT(onMutateIncremental()));
        connect(patternLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(onPowerPatternChanged(const QString &)));
        connect(m_powerUi->findNext, SIGNAL(clicked()), this, SLOT(onPowerFindNext()));
        connect(m_powerUi->findPrev, SIGNAL(clicked()), this, SLOT(onPowerFindPrev()));
        connect(m_powerUi->replaceNext, SIGNAL(clicked()), this, SLOT(onPowerReplaceNext()));
        connect(m_powerUi->replaceAll, SIGNAL(clicked()), this, SLOT(onPowerReplaceAll()));
        connect(m_powerUi->searchMode, SIGNAL(currentIndexChanged(int)), this, SLOT(onPowerModeChanged(int)));
        connect(m_powerUi->matchCase, SIGNAL(stateChanged(int)), this, SLOT(onPowerMatchCaseToggle()));
        connect(m_powerMenuHighlightAll, SIGNAL(toggled(bool)), this, SLOT(onPowerHighlightAllToggle(bool)));
        connect(m_powerMenuFromCursor, SIGNAL(changed()), this, SLOT(onPowerFromCursorToggle()));

        // Make button click open the menu as well. IMHO with the dropdown arrow present the button
        // better shows his nature than in instant popup mode.
        connect(m_powerUi->options, SIGNAL(clicked()), m_powerUi->options, SLOT(showMenu()));

        // Make [return] in pattern line edit trigger <find next> action
        connect(patternLineEdit, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
        connect(replacementLineEdit, SIGNAL(returnPressed()), this, SLOT(onPowerReplaceNext()));

        // Hook into line edit context menus
        patternLineEdit->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(patternLineEdit, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onPowerPatternContextMenuRequest()));
        replacementLineEdit->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(replacementLineEdit, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onPowerReplacmentContextMenuRequest()));
    }

    // Focus
    if (m_widget->isVisible()) {
        m_powerUi->pattern->setFocus(Qt::MouseFocusReason);
    }
}



void KateSearchBar::onMutateIncremental() {
    QString initialPattern;

    // Guess settings from context: init pattern with current selection
    const bool selected = view()->selection();
    if (selected) {
        const Range & selection = view()->selectionRange();
        if (selection.onSingleLine()) {
            // ... with current selection
            initialPattern = view()->selectionText();
        }
    }

    // If there's no new selection, we'll use the existing pattern
    if (initialPattern.isNull()) {
        // Coming from incremental search?
        const bool fromIncremental = (m_incUi != NULL) && (m_widget->isVisible());
        if (fromIncremental) {
            m_incUi->pattern->selectAll();
            m_incUi->pattern->setFocus(Qt::MouseFocusReason);
            return;
        }

        // Coming from power search?
        const bool fromReplace = (m_powerUi != NULL) && (m_widget->isVisible());
        if (fromReplace) {
            initialPattern = m_powerUi->pattern->currentText();
        }
    }

    // Create dialog
    const bool create = (m_incUi == NULL);
    if (create) {
        // Kill power widget
        if (m_powerUi != NULL) {
            // Backup current settings
            const bool OF_POWER = true;
            backupConfig(OF_POWER);

            // Kill widget
            delete m_powerUi;
            m_powerUi = NULL;
            m_layout->removeWidget(m_widget);
            m_widget->deleteLater(); //deleteLater, because it's not a good idea too delete the widget and there for the button triggering this slot
        }

        // Add incremental widget
        m_widget = new QWidget(this);
        m_incUi = new Ui::IncrementalSearchBar;
        m_incUi->setupUi(m_widget);
        m_layout->addWidget(m_widget);

        new QShortcut(KStandardShortcut::paste().primary(), m_incUi->pattern, SLOT(paste()), 0, Qt::WidgetWithChildrenShortcut);
        if (!KStandardShortcut::paste().alternate().isEmpty())
            new QShortcut(KStandardShortcut::paste().alternate(), m_incUi->pattern, SLOT(paste()), 0, Qt::WidgetWithChildrenShortcut);


        // Fill options menu
        m_incMenu = new QMenu();
        m_incUi->options->setMenu(m_incMenu);
        m_incMenuFromCursor = m_incMenu->addAction(i18n("From &cursor"));
        m_incMenuFromCursor->setCheckable(true);
        m_incMenuHighlightAll = m_incMenu->addAction(i18n("Hi&ghlight all"));
        m_incMenuHighlightAll->setCheckable(true);
        m_incMenuMatchCase = m_incMenu->addAction(i18n("&Match case"));
        m_incMenuMatchCase->setCheckable(true);

        // Icons
        m_incUi->mutate->setIcon(KIcon("arrow-up-double"));
        m_incUi->next->setIcon(KIcon("go-down"));
        m_incUi->prev->setIcon(KIcon("go-up"));

	// Customize status area
	m_incUi->status->setTextElideMode(Qt::ElideLeft);

        // Focus proxy
        centralWidget()->setFocusProxy(m_incUi->pattern);
    }

    // Restore previous settings
    if (create) {
        setChecked(m_incMenuHighlightAll, m_incHighlightAll);
        setChecked(m_incMenuFromCursor, m_incFromCursor);
        setChecked(m_incMenuMatchCase, m_incMatchCase);
    }

    // Set initial search pattern
    m_incUi->pattern->setText(initialPattern);
    m_incUi->pattern->selectAll();

    // Propagate settings (slots are still inactive on purpose)
    const bool NOT_INVOKED_BY_USER_ACTION = false;
    onIncPatternChanged(initialPattern, NOT_INVOKED_BY_USER_ACTION);

    if (create) {
        // Slots
        connect(m_incUi->mutate, SIGNAL(clicked()), this, SLOT(onMutatePower()));
        connect(m_incUi->pattern, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
        connect(m_incUi->pattern, SIGNAL(textChanged(const QString &)), this, SLOT(onIncPatternChanged(const QString &)));
        connect(m_incUi->next, SIGNAL(clicked()), this, SLOT(onIncNext()));
        connect(m_incUi->prev, SIGNAL(clicked()), this, SLOT(onIncPrev()));
        connect(m_incMenuMatchCase, SIGNAL(changed()), this, SLOT(onIncMatchCaseToggle()));
        connect(m_incMenuFromCursor, SIGNAL(changed()), this, SLOT(onIncFromCursorToggle()));
        connect(m_incMenuHighlightAll, SIGNAL(toggled(bool)), this, SLOT(onIncHighlightAllToggle(bool)));

        // Make button click open the menu as well. IMHO with the dropdown arrow present the button
        // better shows his nature than in instant popup mode.
        connect(m_incUi->options, SIGNAL(clicked()), m_incUi->options, SLOT(showMenu()));
    }

    // Focus
    if (m_widget->isVisible()) {
        m_incUi->pattern->setFocus(Qt::MouseFocusReason);
    }
}



bool KateSearchBar::isChecked(QCheckBox * checkbox) {
    Q_ASSERT(checkbox != NULL);
    return checkbox->checkState() == Qt::Checked;
}



bool KateSearchBar::isChecked(QAction * menuAction) {
    Q_ASSERT(menuAction != NULL);
    return menuAction->isChecked();
}



void KateSearchBar::setChecked(QCheckBox * checkbox, bool checked) {
    Q_ASSERT(checkbox != NULL);
    checkbox->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}



void KateSearchBar::setChecked(QAction * menuAction, bool checked) {
    Q_ASSERT(menuAction != NULL);
    menuAction->setChecked(checked);
}



void KateSearchBar::enableHighlights(bool enable) {
    if (enable) {
        view()->addInternalHighlight(m_topRange);
    } else {
        view()->removeInternalHighlight(m_topRange);
        m_topRange->deleteChildRanges();
    }
}



void KateSearchBar::resetHighlights() {
    enableHighlights(false);
    enableHighlights(true);
}



void KateSearchBar::showEvent(QShowEvent * event) {
    // Update init cursor
    if (m_incUi != NULL) {
        m_incInitCursor = view()->cursorPosition();
    }

    connect(view(), SIGNAL(selectionChanged(KTextEditor::View *)),
            this, SLOT(onSelectionChanged()));
    connect(view(), SIGNAL(cursorPositionChanged(KTextEditor::View *, KTextEditor::Cursor const &)),
            this, SLOT(onCursorPositionChanged()));

    enableHighlights(true);
    KateViewBarWidget::showEvent(event);
}



void KateSearchBar::closed() {
    disconnect(view(), SIGNAL(selectionChanged(KTextEditor::View *)),
            this, SLOT(onSelectionChanged()));
    disconnect(view(), SIGNAL(cursorPositionChanged(KTextEditor::View *, KTextEditor::Cursor const &)),
            this, SLOT(onCursorPositionChanged()));

    enableHighlights(false);
}



void KateSearchBar::onSelectionChanged() {
    if (m_powerUi == NULL) {
        return;
    }

    // Re-init "Selection only" checkbox if power search bar open
    const bool selected = view()->selection();
    bool selectionOnly = selected;
    if (selected) {
        Range const & selection = view()->selectionRange();
        selectionOnly = !selection.onSingleLine();
    }
    setChecked(m_powerMenuSelectionOnly, selectionOnly);
}


void KateSearchBar::onCursorPositionChanged() {
    if (m_incUi == NULL) {
        return;
    }

    // Update init cursor
    m_incInitCursor = view()->cursorPosition();
}


void KateSearchBar::onPowerPatternContextMenuRequest() {
    const bool FOR_PATTERN = true;
    showExtendedContextMenu(FOR_PATTERN);
}



void KateSearchBar::onPowerReplacmentContextMenuRequest() {
    const bool FOR_REPLACEMENT = false;
    showExtendedContextMenu(FOR_REPLACEMENT);
}


#include "katesearchbar.moc"

// kate: space-indent on; indent-width 2; replace-tabs on;
