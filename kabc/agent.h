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

#ifndef KABC_AGENT_H
#define KABC_AGENT_H

#include <kurl.h>

namespace KABC {

class Addressee;

/**
 * Important!!!
 *
 * At the moment the vcard format does not support saving and loading
 * this entity.
 */
class Agent
{
  friend QDataStream &operator<<( QDataStream &, const Agent & );
  friend QDataStream &operator>>( QDataStream &, Agent & );

public:

  /**
   * Consturctor. Creates an empty object.
   */
  Agent();

  /**
   * Consturctor.
   *
   * @param url  A URL that describes the position of the agent file.
   */
  Agent( const KURL &url );

  /**
   * Consturctor.
   *
   * @param data  The raw data of the agent.
   */
  Agent( Addressee *addressee );

  /**
   * Destructor.
   */
  ~Agent();


  bool operator==( const Agent & ) const;
  bool operator!=( const Agent & ) const;

  /**
   * Sets a URL for the location of the agent file. When using this
   * function, @ref isIntern() will return 'false' until you use
   * @ref setAddressee().
   *
   * @param url  The location URL of the agent file.
   */
  void setUrl( const KURL &url );

  /**
   * Sets the raw data of the agent. When using this function,
   * @ref isIntern() will return 'true' until you use @ref setUrl().
   *
   * @param data  The raw data of the agent.
   */
  void setAddressee( Addressee *addressee );

  /**
   * Returns whether the agent is described by a URL (extern) or
   * by the raw data (intern).
   * When this method returns 'true' you can use @ref addressee() to
   * get the raw data. Otherwise you can request the URL of this
   * agent by @ref url() and load the raw data from that location.
   */
  bool isIntern();

  /**
   * Returns the location URL of this agent.
   */
  KURL url();

  /**
   * Returns the raw data of this agent.
   */
  Addressee* addressee();

  /**
   * Returns string representation of the agent.
   */
  QString asString() const;

private:
  Addressee *mAddressee;
  KURL mUrl;

  int mIntern;
};

QDataStream &operator<<( QDataStream &, const Agent & );
QDataStream &operator>>( QDataStream &, Agent & );

}
#endif
