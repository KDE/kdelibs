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

#ifndef KTRANSCRIPT_H
#define KTRANSCRIPT_H


#include <QList>
#include <QStringList>

/**
  * @internal
  * (used by KLocalizedString)
  *
  * KTranscript provides support for programmable translations.
  * The class is abstract in order to facilitate dynamic loading.
  *
  * @author Chusslove Illich <caslav.ilic@gmx.net>
  * @short class for supporting programmable translations
  */
class KTranscript
{
    public:

    /**
     * Evaluates interpolation.
     *
     * @param argv vector of interpolation tokens
     * @param lang language of the translation
     * @param msgctxt message context
     * @param msgid original message
     * @param subs substitutions for message placeholders
     * @param final finalized ordinary translation
     * @param mods extra scripting modules to load
     * @param error set to the message detailing the problem, if the script
                    failed; set to empty otherwise
     * @param fallback set to true if the script requested fallback to ordinary
                       translation; set to false otherwise
     * @return resolved interpolation if evaluation succeded,
     *         empty string otherwise
     */
    virtual const QString eval (const QStringList &argv,
                                const QString &lang,
                                const QString &msgctxt,
                                const QString &msgid,
                                const QStringList &subs,
                                const QString &final,
                                const QList<QStringList> &mods,
                                QString &error,
                                bool &fallback) = 0;

    /**
     * Destructor.
     */
    virtual ~KTranscript () {}
};

#endif
