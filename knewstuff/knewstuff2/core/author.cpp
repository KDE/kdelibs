/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2006 Josef Spillner <spillner@kde.org>

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

#include "author.h"

using namespace KNS;

class AuthorPrivate
{
  public:
  AuthorPrivate(){}
};

Author::Author()
{
  d = NULL;
}

Author::~Author()
{
}

void Author::setName(const QString& name)
{
  mName = name;
}

QString Author::name() const
{
  return mName;
}

void Author::setEmail(const QString& email)
{
  mEmail = email;
}

QString Author::email() const
{
  return mEmail;
}

void Author::setJabber(const QString& jabber)
{
  mJabber = jabber;
}

QString Author::jabber() const
{
  return mJabber;
}

void Author::setHomepage(const QString& homepage)
{
  mHomepage = homepage;
}

QString Author::homepage() const
{
  return mHomepage;
}

