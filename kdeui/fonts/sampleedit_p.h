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
#ifndef K_SAMPLEEDIT_H
#define K_SAMPLEEDIT_H

#include <QTextEdit>
#include <QFont>
#include <QString>
#include <QStringList>
#include <QSize>

#include <kdebug.h>

// Derived to keep text centered when there is not enough of it for scrolling.
// Needed in KFontChooser for sample text.
class SampleEdit : public QTextEdit
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
    bool m_resetMargins;

    int m_maxViewportHeight;
    int m_tbMargin;
    int m_lrMargin;

    void showEvent (QShowEvent *);
    void resizeEvent (QResizeEvent *);

    private Q_SLOTS:

    void m_setMargins ();
};

SampleEdit::SampleEdit (QWidget *parent_) :
    QTextEdit(parent_),
    m_signalsAllowed(true),
    m_resetMargins(true)
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
    while (true) {
        if (m_resetMargins) {
            // Reset viewport margins to determine max viewport height.
            m_tbMargin = 0;
            m_lrMargin = 0;
            setViewportMargins(0, 0, 0, 0);
            m_maxViewportHeight = viewport()->height();
            m_resetMargins = false;
        }
        else {
            // Collect max viewport height from current plus margin.
            m_maxViewportHeight = viewport()->height() + 2 * m_tbMargin;
        }
        int lrMargin = viewport()->width() / 100;
        int tbMargin = (m_maxViewportHeight - document()->size().height()) / 2;
        tbMargin = tbMargin > lrMargin ? tbMargin : lrMargin;
        if (tbMargin != m_tbMargin || lrMargin != m_lrMargin) {
            // Set new margins.
            m_tbMargin = tbMargin;
            m_lrMargin = lrMargin;
            setViewportMargins(m_lrMargin, m_tbMargin, m_lrMargin, m_tbMargin);

            // To remind vertical scrollbar to disappear if not needed.
            setLineWrapMode(QTextEdit::WidgetWidth);
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
    QTextEdit::setFont(font);
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
    QTextEdit::showEvent(e);
}

void SampleEdit::resizeEvent (QResizeEvent *e)
{
    if (!m_signalsAllowed) {
        return;
    }
    m_setMargins();
    QTextEdit::resizeEvent(e);
}

QSize SampleEdit::sizeHint () const
{
    return QSize(width(), height());
}

#endif
