/*  -*- C++ -*-
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Thiago Macieira <thiagom@mail.com>
 *
 *  $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <qcstring.h>
#include <qstring.h>

#include "resolver.h"		// knetwork/resolver.h
#include "resolver_p.h"
#include "resolverworkerbase.h"	// knetwork/resolverworkerbase.h


using KDE::Network::ResolverWorkerBase;
using KDE::Internal::Resolver::Manager;

ResolverWorkerBase::ResolverWorkerBase()
  : th(0L)
{
}

ResolverWorkerBase::~ResolverWorkerBase()
{
}

int ResolverWorkerBase::familyMask() const
{
  if (th)
    return th->data->input->familyMask;
  return 0;
}

int ResolverWorkerBase::socketType() const
{
  if (th)
    return th->data->input->socktype;
  return 0;
}

int ResolverWorkerBase::protocol() const
{
  if (th)
    return th->data->input->protocol;
  return 0;
}

QCString ResolverWorkerBase::protocolName() const
{
  QCString res;
  if (th)
    res = th->data->input->protocolName;
  return res;
}

