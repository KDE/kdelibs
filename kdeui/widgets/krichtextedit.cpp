/*
 * krichtextedit
 *
 * Copyright 2007 Laurent Montel <montel@kde.org>
 * Copyright 2008 Thomas McGuire <thomas.mcguire@gmx.net>
 * Copyright 2008 Stephen Kelly  <steveire@gmail.com>
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

#include "krichtextedit.h"

// Own includes
#include "nestedlisthelper.h"
#include "klinkdialog.h"

// kdelibs includes
#include <kcursor.h>

// Qt includes
#include <QtGui/QTextDocumentFragment>
#include <QtGui/QMouseEvent>

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KRichTextEditPrivate : public QObject
{
public:
    KRichTextEditPrivate(KRichTextEdit *parent)
            : q(parent),
            mMode(KRichTextEdit::Plain) {
        nestedListHelper = new NestedListHelper(q);
    }

    ~KRichTextEditPrivate() {
        delete nestedListHelper;
    }

    //
    // Normal functions
    //

    // If the text under the cursor is a link, the cursor's selection is set to
    // the complete link text. Otherwise selects the current word if there is no
    // selection.
    void selectLinkText() const;

    void init();

    // Switches to rich text mode and emits the mode changed signal if the
    // mode really changed.
    void activateRichText();

    // Applies formatting to the current word if there is no selection.
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);

    void setTextCursor(QTextCursor &cursor);


    // Data members

    KRichTextEdit *q;
    KRichTextEdit::Mode mMode;

    NestedListHelper *nestedListHelper;

};

void KRichTextEditPrivate::activateRichText()
{
    if (mMode == KRichTextEdit::Plain) {
        q->setAcceptRichText(true);
        mMode = KRichTextEdit::Rich;
        emit q->textModeChanged(mMode);
    }
}

void KRichTextEditPrivate::setTextCursor(QTextCursor &cursor)
{
    q->setTextCursor(cursor);
}

void KRichTextEditPrivate::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = q->textCursor();
    cursor.beginEditBlock();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    q->mergeCurrentCharFormat(format);
    cursor.endEditBlock();
}
//@endcond

KRichTextEdit::KRichTextEdit(const QString& text, QWidget *parent)
        : KTextEdit(text, parent), d(new KRichTextEditPrivate(this))
{
    d->init();
}

KRichTextEdit::KRichTextEdit(QWidget *parent)
        : KTextEdit(parent), d(new KRichTextEditPrivate(this))
{
    d->init();
}

KRichTextEdit::~KRichTextEdit()
{
    delete d;
}

//@cond PRIVATE
void KRichTextEditPrivate::init()
{
    KCursor::setAutoHideCursor(q, true, true);
}
//@endcond

void KRichTextEdit::setListStyle(int _styleIndex)
{
    d->nestedListHelper->handleOnBulletType(-_styleIndex);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::indentListMore()
{
    d->nestedListHelper->handleOnIndentMore();
    d->activateRichText();
}

void KRichTextEdit::indentListLess()
{
    d->nestedListHelper->handleOnIndentLess();
}

void KRichTextEdit::insertHorizontalRule()
{
    QTextCursor cursor = textCursor();
    QTextBlockFormat bf = cursor.blockFormat();
    QTextCharFormat cf = cursor.charFormat();

    cursor.beginEditBlock();
    cursor.insertHtml("<hr>");
    cursor.insertBlock(bf, cf);
    setTextCursor(cursor);
    d->activateRichText();
    cursor.endEditBlock();
}

void KRichTextEdit::alignLeft()
{
    setAlignment(Qt::AlignLeft);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::alignCenter()
{
    setAlignment(Qt::AlignHCenter);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::alignRight()
{
    setAlignment(Qt::AlignRight);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::alignJustify()
{
    setAlignment(Qt::AlignJustify);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setTextBold(bool bold)
{
    QTextCharFormat fmt;
    fmt.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setTextItalic(bool italic)
{
    QTextCharFormat fmt;
    fmt.setFontItalic(italic);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setTextUnderline(bool underline)
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(underline);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setTextStrikeOut(bool strikeOut)
{
    QTextCharFormat fmt;
    fmt.setFontStrikeOut(strikeOut);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setTextForegroundColor(const QColor &color)
{
    QTextCharFormat fmt;
    fmt.setForeground(color);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setTextBackgroundColor(const QColor &color)
{
    QTextCharFormat fmt;
    fmt.setBackground(color);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setFontFamily(const QString &fontFamily)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(fontFamily);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setFontSize(int size)
{
    QTextCharFormat fmt;
    fmt.setFontPointSize(size);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setFont(const QFont &font)
{
    QTextCharFormat fmt;
    fmt.setFont(font);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::switchToPlainText()
{
    if (d->mMode == Rich) {
        d->mMode = Plain;
        // TODO: Warn the user about this?
        document()->setPlainText(document()->toPlainText());
        setAcceptRichText(false);
        emit textModeChanged(d->mMode);
    }
}

void KRichTextEdit::enableRichTextMode()
{
    d->activateRichText();
}

KRichTextEdit::Mode KRichTextEdit::textMode() const
{
    return d->mMode;
}

QString KRichTextEdit::textOrHtml() const
{
    if (textMode() == Rich)
        return toCleanHtml();
    else
        return toPlainText();
}

void KRichTextEdit::setTextOrHtml(const QString &text)
{
    // might be rich text
    if (Qt::mightBeRichText(text)) {
        if (d->mMode == KRichTextEdit::Plain) {
            d->activateRichText();
        }
        setHtml(text);
    } else {
        setPlainText(text);
    }
}

QString KRichTextEdit::currentLinkText() const
{
    QTextCursor cursor = textCursor();
    selectLinkText(&cursor);
    return cursor.selectedText();
}

void KRichTextEdit::selectLinkText() const
{
    QTextCursor cursor = textCursor();
    selectLinkText(&cursor);
    d->setTextCursor(cursor);
}

void KRichTextEdit::selectLinkText(QTextCursor *cursor) const
{
    // If the cursor is on a link, select the text of the link.
    if (cursor->charFormat().isAnchor()) {
        QString aHref = cursor->charFormat().anchorHref();

        // Move cursor to start of link
        while (cursor->charFormat().anchorHref() == aHref) {
            if (cursor->atStart())
                break;
            cursor->setPosition(cursor->position() - 1);
        }
        if (cursor->charFormat().anchorHref() != aHref)
            cursor->setPosition(cursor->position() + 1, QTextCursor::KeepAnchor);

        // Move selection to the end of the link
        while (cursor->charFormat().anchorHref() == aHref) {
            if (cursor->atEnd())
                break;
            cursor->setPosition(cursor->position() + 1, QTextCursor::KeepAnchor);
        }
        if (cursor->charFormat().anchorHref() != aHref)
            cursor->setPosition(cursor->position() - 1, QTextCursor::KeepAnchor);
    } else if (cursor->hasSelection()) {
        // Nothing to to. Using the currently selected text as the link text.
    } else {

        // Select current word
        cursor->movePosition(QTextCursor::StartOfWord);
        cursor->movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    }
}

QString KRichTextEdit::currentLinkUrl() const
{
    return textCursor().charFormat().anchorHref();
}

void KRichTextEdit::updateLink(const QString &linkUrl, const QString &linkText)
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    QTextCharFormat format = cursor.charFormat();

    selectLinkText();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }

    if (!linkUrl.isEmpty()) {
        format.setAnchor(true);
        format.setAnchorHref(linkUrl);
    } else {
        format = cursor.block().charFormat();
        format.setAnchor(false);
        format.setAnchorHref(QString());
    }

    QString _linkText;

    int lowPos = qMin(cursor.selectionStart(), cursor.selectionEnd());
    if (!linkText.isEmpty()) {
        _linkText = linkText;
    } else {
        _linkText = linkUrl;
    }
    // Can't simply insertHtml("<a href=\"%1\">%2</a>").arg(linkUrl).arg(_linkText);
    // That would remove all existing text formatting on the selection (bold etc).
    // The href information is stored in the QTextCharFormat, but qt bugs must
    // still be worked around below.
    cursor.insertText(_linkText, format);


    // Workaround for qt bug 203510:
    // Link formatting does not get applied immediately. Removing and reinserting
    // the marked up html does format the text correctly.
    // -- Stephen Kelly, 15th March 2008
    if (!linkUrl.isEmpty()) {
        cursor.setPosition(lowPos);
        cursor.setPosition(lowPos + _linkText.length(), QTextCursor::KeepAnchor);

        if (!cursor.currentList()) {
            cursor.insertHtml(cursor.selection().toHtml());
        } else {
            // Workaround for qt bug 215576:
            // If the cursor is currently on a list, inserting html will create a new block.
            // This seems to be because toHtml() does not create a <!-- StartFragment --> tag in
            // this case and text style information is stored in the list item rather than a span tag.
            // -- Stephen Kelly, 8th June 2008

            QString selectionHtml = cursor.selection().toHtml();
            QString style = selectionHtml.split("<li style=\"").takeAt(1).split("\"").first();
            QString linkTag = "<a" + selectionHtml.split("<a").takeAt(1).split('>').first() + '>'
                + "<span style=\"" + style + "\">" + _linkText + "</span></a>";
            cursor.insertHtml(linkTag);
        }

        // Insert a space after the link if at the end of the block so that
        // typing some text after the link does not carry link formatting
        if (cursor.position() == cursor.block().position() + cursor.block().length() - 1) {
            cursor.setCharFormat(cursor.block().charFormat());
            cursor.insertText(QString(' '));
        }

        d->activateRichText();
    } else {
        // Remove link formatting. This is a workaround for the same qt bug (203510).
        // Just remove all formatting from the link text.
        QTextCharFormat charFormat;
        cursor.setCharFormat(charFormat);
    }

    cursor.endEditBlock();
}

void KRichTextEdit::keyPressEvent(QKeyEvent *event)
{
    bool handled = false;
    if (textCursor().currentList()) {
        // handled is False if the key press event was not handled or not completely
        // handled by the Helper class.
        handled = d->nestedListHelper->handleBeforeKeyPressEvent(event);
    }

    if (!handled) {
        KTextEdit::keyPressEvent(event);
    }

    if (textCursor().currentList()) {
        d->nestedListHelper->handleAfterKeyPressEvent(event);
    }
    emit cursorPositionChanged();
}

// void KRichTextEdit::dropEvent(QDropEvent *event)
// {
//     int dropSize = event->mimeData()->text().size();
//
//     dropEvent( event );
//     QTextCursor cursor = textCursor();
//     int cursorPosition = cursor.position();
//     cursor.setPosition( cursorPosition - dropSize );
//     cursor.setPosition( cursorPosition, QTextCursor::KeepAnchor );
//     setTextCursor( cursor );
//     d->nestedListHelper->handleAfterDropEvent( event );
// }


bool KRichTextEdit::canIndentList() const
{
    return d->nestedListHelper->canIndent();
}

bool KRichTextEdit::canDedentList() const
{
    return d->nestedListHelper->canDedent();
}

QString KRichTextEdit::toCleanHtml() const
{
    static QString evilline = "<p style=\" margin-top:0px; margin-bottom:0px; "
                      "margin-left:0px; margin-right:0px; -qt-block-indent:0; "
                      "text-indent:0px; -qt-user-state:0;\">";

    QString result;
    QStringList lines = toHtml().split("\n");
    foreach(QString tempLine, lines ) {
        if (tempLine.startsWith(evilline)) { 
            tempLine.remove(evilline);
            if (tempLine.endsWith("</p>")) {
                tempLine.remove(QRegExp("</p>$"));
                tempLine.append("<br>\n");
            }
            result += tempLine;
        } else {
            result += tempLine;
        }
    }

    // ### HACK to fix bug 86925: A completely empty line is ignored in HTML-mode
    int offset = 0;
    QRegExp paragraphFinder("<p.*>(.*)</p>");
    QRegExp paragraphEnd("</p>");
    paragraphFinder.setMinimal(true);

    while (offset != -1) {

        // Find the next paragraph
        offset = paragraphFinder.indexIn(result, offset);

        if (offset != -1) {

            // If the text in the paragraph is empty, add a &nbsp there.
            if (paragraphFinder.capturedTexts().size() == 2 &&
                paragraphFinder.capturedTexts()[1].isEmpty()) {
                int end = paragraphEnd.indexIn(result, offset);
                Q_ASSERT(end != -1 && end > offset);
                result.replace(end, paragraphEnd.pattern().length(), "<br></p>");
            }

            // Avoid finding the same match again
            offset++;
        }
    }

    return result;
}

#include "krichtextedit.moc"
