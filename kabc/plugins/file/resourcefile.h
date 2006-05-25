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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KABC_RESOURCEFILE_H
#define KABC_RESOURCEFILE_H

#include <kconfig.h>
#include <kdirwatch.h>

#include <sys/types.h>

#include <kabc/resource.h>

class QFile;
class QTimer;
class KTempFile;

class KJob;

namespace KABC {

class Format;
class ResourceConfigWidget;
class Lock;

/**
  This resource allows access to a local file.
*/
class KABC_FILE_CORE_EXPORT ResourceFile : public Resource
{
  Q_OBJECT

  public:
    /**
      Constructor.

      @param cfg The config object where custom resource settings are stored.
     */
    explicit ResourceFile( const KConfig *cfg );

    /**
      Construct file resource on file @arg fileName using format @arg formatName.
     */
    explicit ResourceFile( const QString &fileName, const QString &formatName = QLatin1String( "vcard" ) );

    /**
      Destructor.
     */
    ~ResourceFile();

    /**
      Writes the config back.
     */
    virtual void writeConfig( KConfig *cfg );

    /**
      Tries to open the file and checks for the proper format.
      This method should be called before load().
     */
    virtual bool doOpen();

    /**
      Closes the file again.
     */
    virtual void doClose();

    /**
      Requests a save ticket, that is used by save()
     */
    virtual Ticket *requestSaveTicket();

    virtual void releaseSaveTicket( Ticket* );

    /**
      Loads all addressees from file to the address book.
      Returns true if all addressees could be loaded otherwise false.
     */
    virtual bool load();

    virtual bool asyncLoad();

    /**
      Saves all addresses from address book to file.
      Returns true if all addressees could be saved otherwise false.

      @param ticket  The ticket returned by requestSaveTicket()
     */
    virtual bool save( Ticket *ticket );

    virtual bool asyncSave( Ticket *ticket );

    /**
      Set name of file to be used for saving.
     */
    void setFileName( const QString & );

    /**
      Return name of file used for loading and saving the address book.
     */
    QString fileName() const;

    /**
      Sets a new format by name.
     */
    void setFormat( const QString &name );

    /**
      Returns the format name.
     */
    QString format() const;

    /**
      Remove a addressee from its source.
      This method is mainly called by KABC::AddressBook.
     */
    virtual void removeAddressee( const Addressee& addr );

  private Q_SLOTS:
    void downloadFinished( KJob* );
    void uploadFinished( KJob* );

  protected Q_SLOTS:
    void fileChanged();

  protected:
    void init( const QString &fileName, const QString &format );

    bool lock( const QString &fileName );
    void unlock( const QString &fileName );

  private:
    bool clearAndLoad( QFile *file );
    void saveToFile( QFile *file );
    void abortAsyncLoading();
    void abortAsyncSaving();
    bool createLocalTempFile();
    void deleteLocalTempFile();
    void deleteStaleTempFile();
    bool hasTempFile() const { return mTempFile != 0; }

    QString mFileName;
    QString mFormatName;

    Format *mFormat;

    Lock *mLock;

    KDirWatch mDirWatch;

    KTempFile *mTempFile;

    bool mAsynchronous;

    class ResourceFilePrivate;
    ResourceFilePrivate *d;
};

}

#endif
