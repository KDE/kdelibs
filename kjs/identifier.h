/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Apple Computer, Inc
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef KJS_IDENTIFIER_H
#define KJS_IDENTIFIER_H

#include "ustring.h"

namespace KJS {

  /**
   * Represents an Identifier for a Javascript object.
   */
    class KJS_EXPORT Identifier {
        friend class PropertyMap;
    public:
	/** 
	* Creates an empty identifier
	*/
        Identifier() { }
	/**
	* Creates an identifier with the name of the string 
	* @code
	* KJS::Identifier method("someJSMethod");
	* @endcode
	*/
        Identifier(const char *s) : _ustring(add(s)) { }
        Identifier(const UChar *s, int length) : _ustring(add(s, length)) { }
        explicit Identifier(const UString &s) : _ustring(add(s.rep)) { }

	/**
	* returns a UString of the identifier
	*/
        const UString &ustring() const { return _ustring; }
        DOM::DOMString string() const;
        /**
	* returns a QString of the identifier
	*/
	QString qstring() const;

	/**
	* returns a UChar pointer to the string of the identifier with a size defined by @ref size().
	*/
        const UChar *data() const { return _ustring.data(); }
	/**
	* The size of the UChar string returned.
	*/
        int size() const { return _ustring.size(); }

	/**
	* Char * of the identifier's string.
	*/
        const char *ascii() const { return _ustring.ascii(); }

        static Identifier from(unsigned y) { return Identifier(UString::from(y)); }

	/**
	* Returns the identfiers state of being unset.
	*/
        bool isNull() const { return _ustring.isNull(); }
	/**
	* Returns that the identifiers string is set, but is empty.
	*/
        bool isEmpty() const { return _ustring.isEmpty(); }

        unsigned long toULong(bool *ok) const { return _ustring.toULong(ok); }
        unsigned toStrictUInt32(bool *ok) const { return _ustring.toStrictUInt32(ok); }
        unsigned toArrayIndex(bool *ok) const { return _ustring.toArrayIndex(ok); }

        double toDouble() const { return _ustring.toDouble(); }

	/**
	* Creates an empty Identifier
	*/
        static const Identifier &null();

        friend bool operator==(const Identifier &, const Identifier &);
        friend bool operator!=(const Identifier &, const Identifier &);

        friend bool operator==(const Identifier &, const char *);

        static void remove(UString::Rep *);

    private:
        UString _ustring;

        static bool equal(UString::Rep *, const char *);
        static bool equal(UString::Rep *, const UChar *, int length);
        static bool equal(UString::Rep *, UString::Rep *);

        static bool equal(const Identifier &a, const Identifier &b)
            { return a._ustring.rep == b._ustring.rep; }
        static bool equal(const Identifier &a, const char *b)
            { return equal(a._ustring.rep, b); }

        static UString::Rep *add(const char *);
        static UString::Rep *add(const UChar *, int length);
        static UString::Rep *add(UString::Rep *);

        static void insert(UString::Rep *);

        static void rehash(int newTableSize);
        static void expand();
        static void shrink();

	// TODO: move into .cpp file
        static UString::Rep **_table;
        static int _tableSize;
        static int _tableSizeMask;
        static int _keyCount;
    };

    inline bool operator==(const Identifier &a, const Identifier &b)
        { return Identifier::equal(a, b); }

    inline bool operator!=(const Identifier &a, const Identifier &b)
        { return !Identifier::equal(a, b); }

    inline bool operator==(const Identifier &a, const char *b)
        { return Identifier::equal(a, b); }

    KJS_EXPORT extern const Identifier argumentsPropertyName;
    KJS_EXPORT extern const Identifier calleePropertyName;
    KJS_EXPORT extern const Identifier constructorPropertyName;
    KJS_EXPORT extern const Identifier lengthPropertyName;
    KJS_EXPORT extern const Identifier messagePropertyName;
    KJS_EXPORT extern const Identifier namePropertyName;
    KJS_EXPORT extern const Identifier prototypePropertyName;
    KJS_EXPORT extern const Identifier specialPrototypePropertyName;
    KJS_EXPORT extern const Identifier toLocaleStringPropertyName;
    KJS_EXPORT extern const Identifier toStringPropertyName;
    KJS_EXPORT extern const Identifier valueOfPropertyName;

}

#endif
