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

#ifndef KTRANSLIT_P_H
#define KTRANSLIT_P_H

#include <QString>
#include <QList>
#include <QStringList>

#include "klocale.h"

class KTranslit;
class KTranslitPrivate;
class KTranslitSerbian;
class KTranslitSerbianPrivate;

/**
  * @internal
  * (used by KLocale, KLocalizedString)
  *
  * KTranslit provides support for on-the-fly transliteration of UI
  * messages to different scripts for particular languages.
  *
  * @author Chusslove Illich <caslav.ilic@gmx.net>
  * @short class for script transliteration in UI messages
  */
class KTranslit
{
    public:

    /**
     * Constructor, makes noop transliterator (use create()).
     */
    KTranslit ();

    /**
     * Constructs transliterators.
     *
     * @param lang language to create transliterator for
     * @return pointer to transliterator object if one can be made for given
     *         language, NULL otherwise
     */
    static KTranslit *create (const QString &lang);

    /**
     * Returns fallback languages for given language. The fallbacks are
     * supposed to be different scripts of same language, which can be
     * transliterated to produce given one.
     *
     * @param lang language to return fallbacks for
     * @return list of fallbacks
     */
    static QStringList fallbackList (const QString &lang);

    /**
     * Returns possible higher priority script for given language.
     *
     * @param lang language to check scripts for
     * @return higher priority script if found, empty string otherwise
     */
    static QString higherPriorityScript (const QString &lang,
                                         const KLocale *locale);

    /**
     * Transliterates string to given script.
     *
     * @param str string to transliterate
     * @param script script to transliterate into
     * @return transliterated string
     */
    virtual QString transliterate (const QString &str,
                                   const QString &script) const;

    /**
     * Destructor.
     */
    virtual ~KTranslit ();

    protected:

    /**
     * Resolve optional inserts of the form HEAD/foo/bar/ in the string,
     * by picking the ones with given index.
     * Any single character can be used consistently instead of the slash to
     * separate inserts (like in sed).
     *
     * @param str string to resolve inserts in
     * @param nins number of optional inserts
     * @param ind index of insert to choose (zero-based)
     * @param head start string of optional inserts
     * @return resolved string
     */
    QString resolveInserts (const QString &str, int nins, int ind,
                            const QString &head) const;

    private:

    KTranslit (const KTranslit &t);
    KTranslit &operator= (const KTranslit &t);

    KTranslitPrivate *d;
};

/**
  * @internal
  * (used by KLocale, KLocalizedString)
  * Transliterations in Serbian language.
  *
  * @author Chusslove Illich <caslav.ilic@gmx.net>
  * @short derived class for transliterations in Serbian
  */
class KTranslitSerbian : public KTranslit
{
    public:
    /**
     * Constructor.
     */
    KTranslitSerbian ();

    /**
     * Virtual implementation, see base class.
     */
    QString transliterate (const QString &str,
                           const QString &script) const;

    /**
     * Destructor.
     */
    ~KTranslitSerbian ();

    private:

    KTranslitSerbian (const KTranslitSerbian &t);
    KTranslitSerbian &operator= (const KTranslitSerbian &t);

    KTranslitSerbianPrivate *d;
};

#endif
