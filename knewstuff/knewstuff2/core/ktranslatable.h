/*
    This file is part of KNewStuff2.
    Copyright (c) 2006, 2007 Josef Spillner <spillner@kde.org>

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
#ifndef KTRANSLATABLE_H
#define KTRANSLATABLE_H

#include <kdelibs_export.h>

#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>

namespace KNS {

/**
 * @short String class with multiple localized representations.
 *
 * KTranslatable is used as a drop-in replacement for QString
 * whenever human-visible information needs to be stored and this
 * information depends on the current language.
 *
 * @author Josef Spillner (spillner@kde.org)
 */
class KDE_EXPORT KTranslatable
{
public:
	typedef QMap<QString, QString> Map;
	typedef QMapIterator<QString, QString> Iterator;

	/**
	 * Constructor.
	 */
	KTranslatable();

	/**
	 * Constructor.
	 * Initializes the contents with a single string.
	 */
	KTranslatable(QString string);

	/**
	 * Adds a string to the contents of this object.
	 *
	 * @param lang Language code in 'xx_XX' format
	 * @param string The string to be stored for this language
	 */
	void addString(QString lang, QString string);

	/**
	 * Returns the string which matches most closely the current language.
	 *
	 * @return Contents for the current language
	 */
	QString representation();

	/**
	 * Returns the string which matches most closely the specified language.
	 *
	 * @param lang Language the contents should appear in, in 'xx_XX' format
	 * @return Contents for the specified language
	 */
	QString translated(QString lang);

	/**
	 * Returns the list of all strings stored for all languages.
	 *
	 * @return All strings stored in this object
	 */
	QStringList strings();

	/**
	 * Returns the list of all languages for which strings are stored.
	 *
	 * @return All languages the content is available for
	 */
	QStringList languages();

	/**
	 * Returns the language which most likely resembles the current language.
	 *
	 * @return Current language, or one close to it
	 */
	QString language();

	/**
	 * Returns a map of all languages and their strings.
	 *
	 * @return Map of all languages as keys and all strings as values
	 */
	QMap<QString, QString> stringmap();

	/**
	 * Returns whether the content provides alternative (translated) strings.
	 *
	 * @return \b true if contents are translated, \b false otherwise
	 */
	bool isTranslated();

	/**
	 * Returns whether no content is set yet
	 *
	 * @return \b true if no content has been added yet, \b false otherwise
	 */
	bool isEmpty();
private:
	QMap<QString, QString> m_strings;
};

}

#endif
