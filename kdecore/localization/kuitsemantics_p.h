/*  This file is part of the KDE libraries
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

#ifndef KUITSEMANTICS_P_H
#define KUITSEMANTICS_P_H

#include <QString>

class KuitSemantics;
class KuitSemanticsPrivate;

/**
  * @internal
  * (used by KLocalizedString)
  *
  * KuitSemantics resolves semantic markup in user interface text
  * into appropriate visual formatting.
  *
  * @author Chusslove Illich <caslav.ilic@gmx.net>
  * @short class for formatting semantic markup in UI messages
  */
class KuitSemantics
{
    public:

    /**
     * Constructor.
     *
     * @param lang language to create the formatter for
     */
    KuitSemantics (const QString &lang);

    /**
     * Transforms the semantic markup in the given text into visual formatting.
     * The appropriate visual formatting is decided based on the semantic
     * context marker provided in the context string.
     *
     * @param text text containing the semantic markup
     * @param ctxt context of the text
     *
     * @see More information on semantic markup in UI messages in the
     * <a href="http://techbase.kde.org/Development/Tutorials/Localization/i18n_Semantics">article on Techbase</a>.
     */
    QString format (const QString &text, const QString &ctxt) const;

    /**
     * Destructor.
     */
    ~KuitSemantics ();

    /**
     * Poor man's version of Qt::mightBeRichText() (cannot link to QtGui).
     */
    static bool mightBeRichText (const QString &text);

    /**
     * Convert &, ", ', <, > characters into XML entities
     * &amp;, &lt;, &gt;, &apos;, &quot;, respectively.
     */
    static QString escape (const QString &text);

    private:

    KuitSemantics (const KuitSemantics &t);
    KuitSemantics &operator= (const KuitSemantics &t);

    KuitSemanticsPrivate *d;
};

// Some stuff needed in klocalizedstring.cpp too.
#define KUIT_NUMINTG "numintg"
#define KUIT_NUMREAL "numreal"

#endif
