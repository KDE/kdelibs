/*
    This file is part of kdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KRESOURCES_IDMAPPER_H
#define KRESOURCES_IDMAPPER_H

#include <QMap>
#include <QString>

#include <kdelibs_export.h>

namespace KRES {

class IdMapperPrivate;

/**
    An Id Mapper maps Ids. What to or what for is not entirely
    clear, but maps have categories. This is probably an
    adjoint functor, since adjoint functors are everywhere.
*/
class KRESOURCES_EXPORT IdMapper
{
  public:
    /**
      Create Id mapper. You have to set path and identifier before you can call
      load() or save().
    */
    IdMapper();
    /**
      Create Id mapper. The path specifies the category of mapping, the
      identifier the concrete object.

      If you don't pass an identifier you have to set it before calling load()
      or save().

      The current implementation stores the data at
      $(KDEHOME)/share/apps/\<path\>/\<identifier\>.

      @param path Category of mapping (path into the mapping namespace)
      @param identifier The concrete mapping object (filename in namespace)
    */
    IdMapper( const QString &path, const QString &identifier = QString() );
    /** Destructor. */
    ~IdMapper();

    /**
      Set id map path.
      @param path Path to use into mapping namespace.
      @see IdMapper()
    */
    void setPath( const QString &path );
    /**
      Return id map path.
    */
    QString path() const;

    /**
      Set id map identifier.
      @param identifier the identifier (filename) within the mapping namespace
    */
    void setIdentifier( const QString &identifier );
    /**
      Return id map identifier.
    */
    QString identifier() const;

    /**
      Loads the map.
     */
    bool load();

    /**
      Saves the map.
     */
    bool save();

    /**
      Clears the map.
     */
    void clear();

    /**
      Stores the remote id for the given local id.
      @param localId Local Id to set remote for.
      @param remoteId Remote Id to associate with this local Id.
      @see remoteId()
      @see localId()
      @todo What happens when you set the same remote Id for more than
            one localId?
     */
    void setRemoteId( const QString &localId, const QString &remoteId );

    /**
      Removes the remote id.
      @param remoteId remote Id to remove.
      @todo So what does that do? Remove local Ids with only that
            one remote Id? Remove the remote Id from all local Ids
            that have it?
      @see setRemoteId()
     */
    void removeRemoteId( const QString &remoteId );

    /**
      Returns the remote id of the given local id.
      @param localId Local Id to get the remote Id from.
      @see setRemoteId()
      @see removeRemoteId()
     */
    QString remoteId( const QString &localId ) const;

    /**
      Returns the local id for the given remote id.
      @param remoteId Remote Id to get the local Id for.
     */
    QString localId( const QString &remoteId ) const;


    /**
     * Stores a fingerprint for an id which can be used to detect if
     * the locally held version differs from what is on the server.
     * This can be a sequence number of an md5 hash depending on what
     * the server provides.
     *
     * @param localId Local Id to set the fingerprint on.
     * @param fingerprint Fingerprint (any string will do, though
     *        an md5 hash is probably a good idea) of the Id.
     */
    void setFingerprint( const QString &localId, const QString &fingerprint );

    /**
     * Returns the fingerprint for the map.
     *
     * @param localId Local Id to get the fingerprint for.
     * @todo Figure out if this returns the last fingerprint set
     *       by setFingerprint() only or if anything else can change it.
     */
    QString fingerprint( const QString &localId ) const;


    /**
     * Returns the entire map of local-to-remote Ids.
     */
    QMap<QString, QString> remoteIdMap() const;

    /**
      Returns a string representation of the id pairs, that's useful
      for debugging.
     */
    QString asString() const;

  protected:
    /**
     * Filename of the map when stored on disk. Used in save() and load(),
     * see also documentation for constructor IdMapper( const QString &path, const QString &identifier )
     */
    QString filename();

  private:
    IdMapperPrivate* const d;
};

}

#endif
