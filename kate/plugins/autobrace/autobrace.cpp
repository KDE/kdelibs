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

#include <ktexteditor/document.h>

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
    connect(view->document(), SIGNAL(textInserted(KTextEditor::Document*, const KTextEditor::Range&)),
            this, SLOT(slotTextInserted(KTextEditor::Document*, const KTextEditor::Range&)));
}

void AutoBracePlugin::removeView(KTextEditor::View *view)
{
    disconnect(view->document(), 0, this, 0);
}

void AutoBracePlugin::slotTextInserted(KTextEditor::Document *document,
                                       const KTextEditor::Range& range)
{
    // Make sure we don't even try to handle other events than brace openers.
    if (document->text(range) != "\n") {
        return;
    }
    QString line = document->line(range.start().line());
    if (line.isEmpty() || !line.endsWith('{')) {
        return;
    }

    // Get the indentation prefix.
    QRegExp rx("^(\\s+)");
    QString indentation = (rx.indexIn(line) == -1) ? "" : rx.cap(1);

    // Determine whether to insert a brace or not, depending on the indentation
    // of the upcoming (non-empty) line.
    bool insertBrace = true;
    QString indentationLength = QString::number(indentation.length());
    QString indentationLengthMinusOne = QString::number(indentation.length() - 1);

    for (int i = range.start().line() + 1; i < document->lines(); ++i)
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
        insertBrace = false;
      }
      // Quit the loop - a non-empty line always leads to a definitive decision.
      break;
    }

    // Insert the empty line + brace, and adjust the cursor position.
    if (insertBrace) {
      document->insertText(range.end(), "\n" + indentation + "}");
    }
    document->activeView()->setCursorPosition(document->endOfLine(range.end().line()));
}

#include "autobrace.moc"
