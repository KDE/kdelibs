/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>

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
#ifndef KDCOPACTIONPROXY_H
#define KDCOPACTIONPROXY_H

#include <dcopobject.h>
#include <dcopref.h>
#include <kdelibs_export.h>

class KActionCollection;
class KAction;

/**
 * @short A proxy class publishing a DCOP interface for actions.
 *
 * The KDCOPActionProxy class provides an easy way to publish a collection of KAction objects
 * through DCOP. For the DCOP client the exported actions behave like full-fledged DCOP objects,
 * providing full access to the KAction object functionality in the server.
 *
 * This class can generate DCOP object ids for given action objects, which it automatically
 * processes, as being a DCOPObjectProxy .
 */
class KDEUI_EXPORT KDCOPActionProxy : public DCOPObjectProxy
{
public:
  /**
   * Constructs a dcop action proxy, being able to export the actions of the provided
   * KActionCollection through DCOP, using the parent DCOPObject's object id to
   * generate unique object ids for the actions.
   */
  KDCOPActionProxy( KActionCollection *actionCollection, DCOPObject *parent );
  /**
   * Use this constructor if do not want to provide the exportable actions through a
   * KActionCollection . You have to reimplement the virtual actions() and
   * action() methods if you use this constructor.
   */
  KDCOPActionProxy( DCOPObject *parent );
  /**
   * Destructor.
   */
  ~KDCOPActionProxy();

  /**
   * Returns a list of exportable actions. The default implementation returns a list of actions
   * provided by a KActionCollection, if the first constructor has been used.
   */
  virtual QList<KAction *> actions() const;
  /**
   * Returns an action object with the given name. The default implementation queries the action object
   * from the KActionCollection, if the first constructor has been used.
   */
  virtual KAction *action( const char *name ) const;

  /**
   * Use this method to retrieve a DCOP object id for an action with the given name.
   * This class automatically takes care of processing DCOP object requests for the returned
   * object id.
   *
   * You can construct a global DCOP object referenence using DCOPRef. For example like
   * DCOPRef( KApplication::dcopClient()->appId, actionProxy->actionObjectId( actionName ) );
   *
   * The action with the given name has to be available through the #action method.
   */
  virtual DCOPCString actionObjectId( const DCOPCString &name ) const;

  /**
   * Returns a map of all exported actions, with the action name as keys and a global DCOP reference
   * as data entries.
   * The appId argument is used to specify the appid component of the DCOP reference. By default the
   * global application id is used ( KApplication::dcopClient()->appId() ) .
   */
  virtual QMap<DCOPCString,DCOPRef> actionMap( const DCOPCString &appId = DCOPCString() ) const;

  /**
   * Internal reimplementation of DCOPObjectProxy::process .
   */
  virtual bool process( const DCOPCString &obj, const DCOPCString &fun, const QByteArray &data,
                        DCOPCString &replyType, QByteArray &replyData );

  /**
   * Called by the #process method and takes care of processing the object request for an
   * action object.
   */
  virtual bool processAction( const DCOPCString &obj, const DCOPCString &fun, const QByteArray &data,
                              DCOPCString &replyType, QByteArray &replyData, KAction *action );
private:
  void init( KActionCollection *collection, DCOPObject *parent );

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KDCOPActionProxyPrivate;
  KDCOPActionProxyPrivate *d;
};

#endif

