/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef KABC_RESOURCEFILE_H
#define KABC_RESOURCEFILE_H

#include <kconfig.h>
#include <kdirwatch.h>

#include <sys/types.h>

#include "resource.h"

class QTimer;

namespace KABC {

class FormatPlugin;
class ResourceConfigWidget;

/**
  @internal
*/
class ResourceFile : public QObject, public Resource
{
  Q_OBJECT

public:

  /**
   * Constructor.
   *
   * @param ab  The address book where the addressees should be stored.
   * @param cfg The config object where custom resource settings are stored.
   */
  ResourceFile( AddressBook *ab, const KConfig *cfg );

  /**
   * Constructor.
   *
   * @param ab       The address book where the addressees should be stored.
   * @param filename The name of the file, the addressees should be read from.
   * @param format   The pointer to a format handler.
   */
  ResourceFile( AddressBook *ab, const QString &filename,
                FormatPlugin *format = 0 );

  /**
   * Destructor.
   */
  ~ResourceFile();

  /**
   * Tries to open the file and checks for the proper format.
   * This method should be called before @ref load().
   */
  bool open();

  /**
   * Closes the file again.
   */
  void close();
  
  /**
   * Requests a save ticket, that is used by @ref save()
   */
  Ticket *requestSaveTicket();

  /**
   * Loads all addressees from file to the address book.
   * Returns true if all addressees could be loaded otherwise false.
   */
  bool load();

  /**
   * Saves all addresses from address book to file.
   * Returns true if all addressees could be saved otherwise false.
   *
   * @param ticket  The ticket returned by @ref requestSaveTicket()
   */
  bool save( Ticket *ticket );

  /**
   * Set name of file to be used for saving.
   */
  void setFileName( const QString & );

  /**
   * Return name of file used for loading and saving the address book.
   */
  QString fileName() const;

  /**
   * Returns the unique identifier.
   */
  virtual QString identifier() const;

  /**
   * Remove a addressee from its source.
   * This method is mainly called by KABC::AddressBook.
   */
  void removeAddressee( const Addressee& addr );

  /**
   * This method is called by an error handler if the application
   * crashed
   */
  virtual void cleanUp();

protected slots:
  void fileChanged();

protected:
  bool lock( const QString &fileName );
  void unlock( const QString &fileName );

private:
  void init( const QString &filename, FormatPlugin *format );

  QString mFileName;
  FormatPlugin *mFormat;

  QString mLockUniqueName;
    
  KDirWatch mDirWatch;
};

}
#endif
