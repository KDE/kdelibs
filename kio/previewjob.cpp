// -*- c++ -*-
// vim: ts=4 sw=4 et
/*  This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>
                  2000 Carsten Pfeiffer <pfeiffer@kde.org>
                  2001 Malte Starostik <malte.starostik@t-online.de>

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

#include <sys/stat.h>
#include <sys/shm.h>

#include <qdir.h>
#include <qfile.h>
#include <qimage.h>
#include <qtimer.h>

#include <kdatastream.h> // Do not remove, needed for correct bool serialization
#include <kfileitem.h>
#include <kapp.h>
#include <kdebug.h>
#include <ktempfile.h>
#include <ktrader.h>
#include <kmdcodec.h>
#include <kglobal.h>
#include <kstddirs.h>

#include <kio/kservice.h>

#include "previewjob.moc"

namespace KIO { struct PreviewItem; }
using namespace KIO;

struct KIO::PreviewItem
{
    KFileItem *item;
    KService::Ptr plugin;
};

struct KIO::PreviewJobPrivate
{
    enum { STATE_STATORIG, STATE_GETTHUMB, // if the thumbnail exists
           STATE_GETORIG, // if we create it
           STATE_CREATETHUMB // thumbnail:/ slave
    } state;
    // Our todo list :)
    QValueList<PreviewItem> items;
    // The current item
    PreviewItem currentItem;
    // The modification time of that URL
    time_t tOrig;
    // Path to thumbnail cache for this directory
    QString thumbPath;
    // Size of thumbnail
    int width;
    int height;
    // Whether we can save the thumbnail
    bool bCanSave;
    // If the file to create a thumb for was a temp file, this is its name
    QString tempName;
    // Over that, it's too much
    unsigned long maximumSize;
    // the size for the icon overlay
    int iconSize;
    // the transparency of the blended mimetype icon
    int iconAlpha;
	// Shared memory segment Id. The segment is allocated to a size
	// of extent x extent x 4 (32 bit image) on first need.
	int shmid;
	// And the data area
	uchar *shmaddr;
    // Delete the KFileItems when done?
    bool deleteItems;
};
    
PreviewJob::PreviewJob( const KFileItemList &items, int width, int height,
    int iconSize, int iconAlpha, bool scale, bool save,
    const QStringList *enabledPlugins, bool deleteItems )
    : KIO::Job( false /* no GUI */ )
{
    d = new PreviewJobPrivate;
    d->bCanSave = false;
    d->shmid = -1;
    d->shmaddr = 0;
    d->width = width;
    d->height = height ? height : width;
    d->iconSize = iconSize;
    d->iconAlpha = iconAlpha;
    d->deleteItems = deleteItems;

    // Load the list of plugins to determine which mimetypes are supported
    KTrader::OfferList plugins = KTrader::self()->query("ThumbCreator");
    QMap<QString, KService::Ptr> mimeMap;

    for (KTrader::OfferList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
        if (!enabledPlugins || enabledPlugins->contains((*it)->desktopEntryName()))
    {
        QStringList mimeTypes = (*it)->property("MimeTypes").toStringList();
        for (QStringList::ConstIterator mt = mimeTypes.begin(); mt != mimeTypes.end(); ++mt)
            mimeMap.insert(*mt, *it);
    }
  
    // Look for images and store the items in our todo list :)
    bool bNeedCache = false;
    for (KFileItemListIterator it(items); it.current(); ++it )
    {
        PreviewItem item;
        item.item = it.current();
        QMap<QString, KService::Ptr>::ConstIterator plugin = mimeMap.find(it.current()->mimetype());
        if (plugin == mimeMap.end())
        {
            QString mimeType = it.current()->mimetype();
            plugin = mimeMap.find(mimeType.replace(QRegExp("/.*"), "/*"));
        }
        if (plugin != mimeMap.end())
        {
            item.plugin = *plugin;
            d->items.append(item);
            if (!bNeedCache && (*plugin)->property("CacheThumbnail").toBool())
                bNeedCache = true;
        }
        else if (deleteItems)
            delete it.current();
    }

  // Read configuration value for the maximum allowed size
//  KConfig * config = KGlobal::config();
//  KConfigGroupSaver cgs( config, "FMSettings" );
//  m_maximumSize = config->readNumEntry( "MaximumImageSize", 1024*1024 /* 1MB */ );
    d->maximumSize = 1024 * 1024;
  
/*  int size = m_iconView->iconSize() ? m_iconView->iconSize()
    : KGlobal::iconLoader()->currentSize( KIcon::Desktop ); // if 0

  QString sizeName;
  if (size < 28)
  {
    m_extent = 48;
    sizeName = "small";
  }
  else if (size < 40)
  {
    m_extent = 64;
    sizeName = "med";
  }
  else
  {
    m_extent = 90;
    sizeName = "large";
  }

  if ( bNeedCache )
  {
    QString thumbPath;
    KGlobal::dirs()->addResourceType( "thumbnails", "share/thumbnails/" );
    // Check if we're in a thumbnail dir already
    if ( m_iconView->url().isLocalFile() )
    {
      // there's exactly one path
      QString cachePath = QDir::cleanDirPath( KGlobal::dirs()->resourceDirs( "thumbnails" )[0] );
      QString dir = QDir::cleanDirPath( m_iconView->url().directory() );
      if ( dir.startsWith( cachePath ) )
        thumbPath = dir.mid( cachePath.length() );
    }

    if ( thumbPath.isEmpty() ) // not a thumbnail dir, generate a name
    {
      KURL cleanURL( m_iconView->url() );
      // clean out the path to avoid multiple cache dirs for the same
      // location (e.g. file:/foo/bar vs. file:/foo//bar)
      cleanURL.setPath( QDir::cleanDirPath( m_iconView->url().path() ) );
      HASHHEX hash;
      KMD5 md5( QFile::encodeName( cleanURL.url() ) );
      md5.hexDigest( hash );
      thumbPath = QString::fromLatin1( hash, 4 ) + "/" +
                  QString::fromLatin1( &hash[4], 4 ) + "/" +
                  QString::fromLatin1( &hash[8] ) + "/";
    }

    m_thumbPath = locateLocal( "thumbnails", thumbPath + "/" + sizeName + "/" );
  }*/
    d->currentItem.item = 0;

    // Return to event loop first, determineNextFile() might delete this;
    QTimer::singleShot(0, this, SLOT(determineNextFile()));
}

PreviewJob::~PreviewJob()
{
    if (d->shmaddr)
        shmdt((char*)d->shmaddr);
    delete d;
}

void PreviewJob::removeItem( const KFileItem *item )
{
    for (QValueList<PreviewItem>::Iterator it = d->items.begin(); it != d->items.end(); ++it)
        if ((*it).item == item)
        {
            d->items.remove(it);
            break;
        }

    if (d->currentItem.item == item)
    {
        subjobs.first()->kill();
        subjobs.removeFirst();
        determineNextFile();
    }
}

void PreviewJob::determineNextFile()
{
    if (d->deleteItems && d->currentItem.item){kdDebug() << "*************** deleting " << d->currentItem.item->url().url() << endl;
        delete d->currentItem.item;}
    // No more items ?
    if ( d->items.isEmpty() )
    {
        emitResult();
        return;
    }
    else
    {
        // First, stat the orig file
        d->state = PreviewJobPrivate::STATE_STATORIG;
        d->currentItem = d->items.first();
        d->items.remove(d->items.begin());
        KIO::Job *job = KIO::stat( d->currentItem.item->url(), false );
        addSubjob(job);
    }
}

void PreviewJob::slotResult( KIO::Job *job )
{
    subjobs.remove( job );
    ASSERT ( subjobs.isEmpty() ); // We should have only one job at a time ...
    switch ( d->state )
    {
        case PreviewJobPrivate::STATE_STATORIG:
        {
            if (job->error()) // that's no good news...
            {
                // Drop this one and move on to the next one
                determineNextFile();
                return;
            }
            KIO::UDSEntry entry = ((KIO::StatJob*)job)->statResult();
            KIO::UDSEntry::ConstIterator it = entry.begin();
            d->tOrig = 0;
            int found = 0;
            for( ; it != entry.end() && found < 2; it++ )
            {
                if ( (*it).m_uds == KIO::UDS_MODIFICATION_TIME )
                {
                    d->tOrig = (time_t)((*it).m_long);
                    found++;
                }
                else if ( (*it).m_uds == KIO::UDS_SIZE )
                    {
                    if ( static_cast<unsigned long>((*it).m_long) > d->maximumSize )
                    {
                        determineNextFile();
                        return;
                    }
                    found++;
                }
            }

            if ( !d->currentItem.plugin->property( "CacheThumbnail" ).toBool() )
            {
                // This preview will not be cached, no need to look for a saved thumbnail
                // Just create it, and be done
                getOrCreateThumbnail();
                return;
            }

            if ( statResultThumbnail() )
                return;

            getOrCreateThumbnail();
            return;
        }
        case PreviewJobPrivate::STATE_GETTHUMB:
        {
            // We arrive here if statResultThumbnail found a remote thumbnail
            if (job->error())
            {
                // Drop this one (if get fails, I wouldn't be on mkdir and put)
                determineNextFile();
                return;
            }
            QString localFile( static_cast<KIO::FileCopyJob*>(job)->destURL().path() );
            QPixmap pix;
            if ( pix.load( localFile ) )
            {
                // Found it, use it
                emit gotPreview( d->currentItem.item, pix );
                emit gotPreview( d->currentItem.item->url(), pix );
            }
            QFile::remove( localFile );
            // Whether we suceeded or not, move to next one
            determineNextFile();
            return;
        }
        case PreviewJobPrivate::STATE_GETORIG:
        {
            if (job->error())
            {
                determineNextFile();
                return;
            }
      
            createThumbnail( static_cast<KIO::FileCopyJob*>(job)->destURL().path() );
            return;
        }
        case PreviewJobPrivate::STATE_CREATETHUMB:
        {
            if (!d->tempName.isEmpty())
            {
                QFile::remove(d->tempName);
                d->tempName = QString::null;
            }
            determineNextFile();
            return;
        }
    }
}

bool PreviewJob::statResultThumbnail( KIO::StatJob * job )
{
/*    QString thumbPath = m_thumbPath + m_currentURL.fileName();
    time_t tThumb = 0;
    if ( job ) // had a job (.xvpics)
    {
      if (!job->error()) // found a thumbnail
      {
          KIO::UDSEntry entry = job->statResult();
          KIO::UDSEntry::ConstIterator it = entry.begin();
          for( ; it != entry.end(); it++ ) {
            if ( (*it).m_uds == KIO::UDS_MODIFICATION_TIME ) {
              tThumb = (time_t)((*it).m_long);
            }
          }
          // Only ok if newer than the file
      }
    }
    else
    {
      struct stat st;
      if ( stat( QFile::encodeName( thumbPath ), &st ) == 0 )
        tThumb = st.st_mtime;
    }

    if ( tThumb < m_tOrig )
        return false;

    if ( !job || job->url().isLocalFile() )
    {
        QPixmap pix;
        if ( pix.load( job ? job->url().url() : thumbPath ) )
        {
            // Found it, use it
            m_iconView->setThumbnailPixmap( m_currentItem, pix );
            determineNextIcon();
            return true;
        }
    }
    else
    {
        m_state = PreviewJobPrivate::STATE_GETTHUMB;
        KTempFile localFile;
        KURL localURL;
        localURL.setPath( localFile.name() );*/
//        KIO::Job * getJob = KIO::file_copy( job->url(), localURL, -1, true, false, false /* No GUI */ );
/*        kdDebug(1203) << "KonqImagePreviewJob:: KIO::file_copy thumb " << job->url().url() << " to " << localFile.name() << endl;
        addSubjob(getJob);
        return true;
    }
    return false;*/
    return false;
}


void PreviewJob::getOrCreateThumbnail()
{
    // We still need to load the orig file ! (This is getting tedious) :)
    KURL currentURL = d->currentItem.item->url();
    if ( currentURL.isLocalFile() )
        createThumbnail( currentURL.path() );
    else
    {
        d->state = PreviewJobPrivate::STATE_GETORIG;
        KTempFile localFile;
        KURL localURL;
        localURL.setPath( d->tempName = localFile.name() );
        KIO::Job * job = KIO::file_copy( currentURL, localURL, -1, true,
                                         false, false /* No GUI */ );
        addSubjob(job);
    }
}

void PreviewJob::createThumbnail( QString pixPath )
{
    d->state = PreviewJobPrivate::STATE_CREATETHUMB;
    KURL thumbURL;
    thumbURL.setProtocol("thumbnail");
    thumbURL.setPath(pixPath);
    KIO::TransferJob *job = KIO::get(thumbURL, false, false);
    connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)), SLOT(slotThumbData(KIO::Job *, const QByteArray &)));
    job->addMetaData("mimeType", d->currentItem.item->mimetype());
    job->addMetaData("width", QString().setNum(d->width));
    job->addMetaData("height", QString().setNum(d->height));
    job->addMetaData("iconSize", QString().setNum(d->iconSize));
    job->addMetaData("iconAlpha", QString().setNum(d->iconAlpha));
    job->addMetaData("plugin", d->currentItem.plugin->library());
    if (d->shmid == -1)
    {
        if (d->shmaddr)
            shmdt((char*)d->shmaddr);
        d->shmid = shmget(IPC_PRIVATE, d->width * d->height * 4, IPC_CREAT|0777);
        if (d->shmid != -1)
        {
            d->shmaddr = static_cast<uchar *>(shmat(d->shmid, 0, SHM_RDONLY));
            shmctl(d->shmid, IPC_RMID, 0);
            if (d->shmaddr == (uchar *)-1)
            {
                d->shmaddr = 0;
                d->shmid = -1;
            }
        }
        else
            d->shmaddr = 0;
    }
    if (d->shmid != -1)
        job->addMetaData("shmid", QString().setNum(d->shmid));
    addSubjob(job);
}

void PreviewJob::slotThumbData(KIO::Job *, const QByteArray &data)
{
    bool save = false;//d->bCanSave && m_plugins[m_currentItem->thumbnailName()]->property("CacheThumbnail").toBool();
    QPixmap pix;
    if (d->shmaddr)
    {
        QDataStream str(data, IO_ReadOnly);
        int width, height, depth;
        bool alpha;
        str >> width >> height >> depth >> alpha;
        QImage img(d->shmaddr, width, height, depth, 0, 0, QImage::IgnoreEndian);
        img.setAlphaBuffer(alpha);
        pix.convertFromImage(img);
        if (save)
        {
            QByteArray saveData;
            QDataStream saveStr(saveData, IO_WriteOnly);
            saveStr << img;
            saveThumbnail(saveData);
        }
    }
    else
    {
        pix.loadFromData(data);
        if (save)
            saveThumbnail(data);
    }
    emit gotPreview(d->currentItem.item->url(), pix);
}

void PreviewJob::saveThumbnail(const QByteArray &imgData)
{
/*    QFile file( m_thumbPath + m_currentURL.fileName() );
    if ( file.open(IO_WriteOnly) )
    {
        file.writeBlock( imgData.data(), imgData.size() );
        file.close();
    }*/
}

QStringList PreviewJob::availablePlugins()
{
    QStringList result;
    KTrader::OfferList plugins = KTrader::self()->query("ThumbCreator");
    for (KTrader::OfferList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
        result.append((*it)->desktopEntryName());
}

PreviewJob *KIO::filePreview( const KFileItemList &items, int width, int height,
    int iconSize, int iconAlpha, bool scale, bool save,
    const QStringList *enabledPlugins )
{
    return new PreviewJob(items, width, height, iconSize, iconAlpha,
                          scale, save, enabledPlugins);
}

PreviewJob *KIO::filePreview( const KURL::List &items, int width, int height,
    int iconSize, int iconAlpha, bool scale, bool save,
    const QStringList *enabledPlugins )
{
    KFileItemList fileItems;
    for (KURL::List::ConstIterator it = items.begin(); it != items.end(); ++it)
        fileItems.append(new KFileItem(-1, -1, *it, true));
    return new PreviewJob(fileItems, width, height, iconSize, iconAlpha,
                          scale, save, enabledPlugins, true);
}

