/*
    Requires the Qt widget libraries, available at no cost at
    http://www.troll.no

    Copyright (C) 2007 Chusslove Illich <caslav.ilic@gmx.net>

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
#ifndef SAMPLEEDIT_P_H
#define SAMPLEEDIT_P_H

#include <ktextedit.h>
#include <QFont>
#include <QString>
#include <QStringList>
#include <QSize>

#include <kdebug.h>

// Derived to keep text centered when there is not enough of it for scrolling.
// Needed in KFontChooser for sample text.
class SampleEdit : public KTextEdit
{
    Q_OBJECT

    public:

    SampleEdit (QWidget *parent = 0);

    // Reimplemented to recenter on font change.
    void setFont (const QFont &font);

    // Reimplemented to assure centered alignment.
    void setPlainText (const QString &text);

    // Reimplemented to always state the current size as preferred.
    QSize sizeHint () const;

    private:

    bool m_signalsAllowed;

    void showEvent (QShowEvent *);
    void resizeEvent (QResizeEvent *);

    private Q_SLOTS:

    void m_setMargins ();
};

SampleEdit::SampleEdit (QWidget *parent_) :
    KTextEdit(parent_),
    m_signalsAllowed(true)
{
    connect(this, SIGNAL(textChanged()),
            this, SLOT(m_setMargins()));
}

void SampleEdit::m_setMargins ()
{
    // The margins are determined in a bit roundabout manner,
    // in order to avoid resetting margins each time, to avoid flicker.

    m_signalsAllowed = false;

    // Repeat setting margins until they no longer change between iterations.
    int maxIterations=6;
    while (maxIterations--) {
        if (document()->isEmpty()) {
            QTextCursor cursor(document());
        }
        QTextFrame *root = document()->rootFrame();
        QTextFrameFormat format = root->frameFormat();
        int documentHeight = document()->size().height() - format.topMargin() - format.bottomMargin();
        if (documentHeight == 0) {
            documentHeight = fontMetrics().height();
        }
        int lrMargin = viewport()->width() / 100;
        int tbMargin = (viewport()->height() - documentHeight) / 2;
        tbMargin = tbMargin > lrMargin ? tbMargin : lrMargin;
        if (tbMargin != format.topMargin() || lrMargin != format.leftMargin()) {
            // Set new margins.
            format.setLeftMargin(lrMargin);
            format.setRightMargin(lrMargin);
            format.setTopMargin(tbMargin);
            format.setBottomMargin(tbMargin);
            // Setting the new margins triggers textChanged(), avoid recursion
            disconnect(this, SIGNAL(textChanged()), this, SLOT(m_setMargins()));
            root->setFrameFormat(format);
            connect(this, SIGNAL(textChanged()), this, SLOT(m_setMargins()));

            // To remind vertical scrollbar to disappear if not needed.
            setLineWrapMode(KTextEdit::WidgetWidth);
            setLineWrapColumnOrWidth(viewport()->width());
        }
        else {
            break;
        }
    }

    m_signalsAllowed = true;
}

void SampleEdit::setFont (const QFont &font)
{
    KTextEdit::setFont(font);
    m_setMargins();
}

void SampleEdit::setPlainText (const QString &text)
{
    // Add text line by line, to keep centering each.
    clear();
    QStringList lines = text.split("\n");
    foreach (const QString &line, lines) {
        append(line);
        setAlignment(Qt::AlignCenter);
    }
    // Clear undo stack.
    setUndoRedoEnabled(false);
    setUndoRedoEnabled(true);
}

void SampleEdit::showEvent (QShowEvent *e)
{
    if (!m_signalsAllowed) {
        return;
    }
    m_setMargins();
    KTextEdit::showEvent(e);
}

void SampleEdit::resizeEvent (QResizeEvent *e)
{
    if (!m_signalsAllowed) {
        return;
    }
    m_setMargins();
    KTextEdit::resizeEvent(e);
}

QSize SampleEdit::sizeHint () const
{
    return QSize(width(), height());
}

#endif
