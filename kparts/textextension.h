/* This file is part of the KDE project
   Copyright (C) 2010 David Faure <faure@kde.org>

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

#ifndef KPARTS_TEXTEXTENSION_H
#define KPARTS_TEXTEXTENSION_H

#include <QtCore/QObject>
#include <kparts/kparts_export.h>

namespace KParts
{

class ReadOnlyPart;
class TextExtensionPrivate;

/**
 * @short an extension for KParts that allows to retrieve text from the part.
 *
 * For instance, the text-to-speech plugin uses this to speak the whole text
 * from the part or the selected text. The translation plugin uses it for
 * translating the selected text, and so on.
 *
 * @since 4.6
 */
class KPARTS_EXPORT TextExtension : public QObject
{
    Q_OBJECT
public:
    TextExtension(KParts::ReadOnlyPart* parent);
    ~TextExtension();

    /**
     * Queries @p obj for a child object which inherits from this
     * TextExtension class.
     */
    static TextExtension *childObject( QObject *obj );

    enum Format { PlainText, HTML };

    /**
     * Returns true if the user selected text in the part.
     */
    virtual bool hasSelection() const;
    /**
     * Returns the selected text, in the requested format.
     * If the format is not supported, the part must return an empty string.
     */
    virtual QString selectedText(Format format) const;
    /**
     * Returns the complete text shown in the part, in the requested format.
     * If the format is not supported, the part must return an empty string.
     */
    virtual QString completeText(Format format) const;

    // for future extensions can be made via slots

Q_SIGNALS:
    /**
     * This signal is emitted when the selection changes.
     */
    void selectionChanged();

private:
    // for future extensions
    TextExtensionPrivate* const d;
};

}

#endif /* KPARTS_TEXTEXTENSION_H */
