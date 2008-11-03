/**
  * This file is part of the KDE libraries
  * Copyright (C) 2007 Rafael Fernández López <ereslibre@kde.org>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License version 2 as published by the Free Software Foundation.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#include "autobrace.h"

#include <kpluginfactory.h>
#include <kpluginloader.h>

AutoBracePlugin *AutoBracePlugin::plugin = 0;

K_PLUGIN_FACTORY_DEFINITION(AutoBracePluginFactory,
        registerPlugin<AutoBracePlugin>("ktexteditor_autobrace");
        )
K_EXPORT_PLUGIN(AutoBracePluginFactory("ktexteditor_autobrace", "ktexteditor_plugins"))

AutoBracePlugin::AutoBracePlugin(QObject *parent, const QVariantList &args)
    : KTextEditor::Plugin(parent)
{
    Q_UNUSED(args);
    plugin = this;
}

AutoBracePlugin::~AutoBracePlugin()
{
    plugin = 0;
}

void AutoBracePlugin::addView(KTextEditor::View *view)
{
    AutoBracePluginDocument *docplugin = new AutoBracePluginDocument(view->document());
    m_docplugins.insert(view, docplugin);
}

void AutoBracePlugin::removeView(KTextEditor::View *view)
{
    if (m_docplugins.contains(view))
    {
        AutoBracePluginDocument *docplugin = m_docplugins.value(view);
        m_docplugins.remove(view);

        // Only detach from the document if it was the last view pointing to that.
        if (m_docplugins.keys(docplugin).empty()) {
            delete docplugin;
        }
    }
}


AutoBracePluginDocument::AutoBracePluginDocument(KTextEditor::Document *document)
  : QObject(document), m_insertionLine(0)
{
    connect(document, SIGNAL(textInserted(KTextEditor::Document*, const KTextEditor::Range&)),
            this, SLOT(slotTextInserted(KTextEditor::Document*, const KTextEditor::Range&)));
}

AutoBracePluginDocument::~AutoBracePluginDocument()
{
    disconnect(parent() /* == document */, 0, this, 0);
}

void AutoBracePluginDocument::slotTextChanged(KTextEditor::Document *document) {
    if (m_insertionLine != 0) {
        disconnect(document, 0, this, 0);

        if (isInsertionCandidate(document, m_insertionLine - 1)) {
            KTextEditor::Cursor cursor = document->endOfLine(m_insertionLine);

            document->startEditing();
            document->insertText(cursor, "\n" + m_indentation + "}");
            document->endEditing();

            KTextEditor::View *view = document->activeView();
            view->setCursorPosition(cursor);

            /* [requires a config option, otherwise it clashes with the C indenter]
            // If the document's View is a KateView then it's able to indent.
            if (view->inherits("KateView")) {
                document->startEditing();
                connect(this, SIGNAL(indent()), view, SLOT(indent()));
                emit indent();
                disconnect(this, SIGNAL(indent()), view, SLOT(indent()));
                document->endEditing();
            }*/
        }
        m_insertionLine = 0;

        connect(document, SIGNAL(textInserted(KTextEditor::Document*, const KTextEditor::Range&)),
                this, SLOT(slotTextInserted(KTextEditor::Document*, const KTextEditor::Range&)));
    }
}

void AutoBracePluginDocument::slotTextInserted(KTextEditor::Document *document,
                                           const KTextEditor::Range& range)
{
    // Make sure we don't even try to handle other events than brace openers.
    if (document->text(range) != "\n") {
        return;
    }

    // Remember this position as insertion candidate.
    // We don't directly insert this here because of KatePart specifics:
    // a) Setting the cursor position crashes at this point, and
    // b) textChanged() only gets called once per edit operation, so we can
    //    ignore the same braces when they're being inserted via paste.
    if (isInsertionCandidate(document, range.start().line())) {
        m_insertionLine = range.end().line();
        connect(document, SIGNAL(textChanged(KTextEditor::Document*)),
                this, SLOT(slotTextChanged(KTextEditor::Document*)));
    }
}

bool AutoBracePluginDocument::isInsertionCandidate(KTextEditor::Document *document, int openingBraceLine) {
    QString line = document->line(openingBraceLine);
    if (line.isEmpty() || !line.endsWith('{')) {
        return false;
    }

    // Get the indentation prefix.
    QRegExp rx("^(\\s+)");
    m_indentation = (rx.indexIn(line) == -1) ? "" : rx.cap(1);

    // Determine whether to insert a brace or not, depending on the indentation
    // of the upcoming (non-empty) line.
    bool isCandidate = true;
    QString indentationLength = QString::number(m_indentation.length());
    QString indentationLengthMinusOne = QString::number(m_indentation.length() - 1);

    for (int i = openingBraceLine + 1; i < document->lines(); ++i)
    {
        line = document->line(i);
        if (line.trimmed().isEmpty()) {
            continue; // Empty lines are not a reliable source of information.
        }

        rx.setPattern("^(?:"
            // Inserting a brace is ok if there is a closing brace with
            // less indentation than the opener line.
            "[\\s]{0," + indentationLengthMinusOne + "}\\}"
            "|"
            // Inserting a brace is ok if there is a line (not starting with a
            // brace) with less or similar indentation as the original line.
            "[\\s]{0," + indentationLength + "}[^\\}\\s]"
            ")"
        );
        if (rx.indexIn(line) == -1) {
            // There is already a brace, or the line is indented more than the
            // opener line (which means we expect a brace somewhere further down).
            // So don't insert the brace, and just indent the line.
            isCandidate = false;
        }
        // Quit the loop - a non-empty line always leads to a definitive decision.
        break;
    }

    return isCandidate;
}

#include "autobrace.moc"
