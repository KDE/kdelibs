/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KABC_ERRORHANDLER_H
#define KABC_ERRORHANDLER_H

#include <qstring.h>

namespace KABC {

/**
 * Abstract class that provides displaying of error messages.
 * We need this to make libkabc gui independend on the one side
 * and provide user friendly error messages on the other side.
 * Use @p ConsoleErrorHandler or @p GUIErrorHandler in your
 * application.
*/
class ErrorHandler
{
public:
    virtual void error( const QString& msg ) = 0;
};

/**
 * This class prints the error messages to
 * stderr via kdError().
*/
class ConsoleErrorHandler : public ErrorHandler
{
public:
    virtual void error( const QString& msg );
};

/**
 * This class show messages boxes for every
 * error message.
*/
class GUIErrorHandler : public ErrorHandler
{
public:
    virtual void error( const QString& msg );
};

}

#endif
