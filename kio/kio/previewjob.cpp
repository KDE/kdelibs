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
#ifdef __FreeBSD__
    #include <machine/param.h>
#endif
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <qdir.h>
#include <qfile.h>
#include <qimage.h>
#include <qtimer.h>
#include <qregexp.h>

#include <kdatastream.h> // Do not remove, needed for correct bool serialization
#include <kfileitem.h>
#include <kapplication.h>
#include <kdebug.h>
#include <ktempfile.h>
#include <ktrader.h>
#include <kmdcodec.h>
#include <kglobal.h>
#include <kstandarddirs.h>

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
    enum { STATE_STATORIG, // if the thumbnail exists
           STATE_GETORIG, // if we create it
           STATE_CREATETHUMB // thumbnail:/ slave
    } state;
    KFileItemList initialItems;
    const QStringList *enabledPlugins;
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
    // named thumbnail size for caching
    QString sizeName;
    // Whether the thumbnail should be scaled
    bool bScale;
    // Whether we should save the thumbnail
    bool bSave;
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
    bool succeeded;
};

PreviewJob::PreviewJob( const KFileItemList &items, int width, int height,
    int iconSize, int iconAlpha, bool scale, bool save,
    const QStringList *enabledPlugins, bool deleteItems )
    : KIO::Job( false /* no GUI */ )
{
    d = new PreviewJobPrivate;
    d->tOrig = 0;
    d->shmid = -1;
    d->shmaddr = 0;
    d->initialItems = items;
    d->enabledPlugins = enabledPlugins;
    d->width = width;
    d->height = height ? height : width;
    d->iconSize = iconSize;
    d->iconAlpha = iconAlpha;
    d->deleteItems = deleteItems;
    d->bScale = scale;
    d->bSave = save && scale;
    d->succeeded = false;
    d->currentItem.item = 0;

    // Return to event loop first, determineNextFile() might delete this;
    QTimer::singleShot(0, this, SLOT(startPreview()));
}

PreviewJob::~PreviewJob()
{
    if (d->shmaddr) {
        shmdt((char*)d->shmaddr);
        shmctl(d->shmid, IPC_RMID, 0);
    }
    delete d;
}

void PreviewJob::startPreview()
{
    // Load the list of plugins to determine which mimetypes are supported
    KTrader::OfferList plugins = KTrader::self()->query("ThumbCreator");
    QMap<QString, KService::Ptr> mimeMap;

    for (KTrader::OfferList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
        if (!d->enabledPlugins || d->enabledPlugins->contains((*it)->desktopEntryName()))
    {
        QStringList mimeTypes = (*it)->property("MimeTypes").toStringList();
        for (QStringList::ConstIterator mt = mimeTypes.begin(); mt != mimeTypes.end(); ++mt)
            mimeMap.insert(*mt, *it);
    }

    // Look for images and store the items in our todo list :)
    bool bNeedCache = false;
    for (KFileItemListIterator it(d->initialItems); it.current(); ++it )
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
            if (d->bSave && !bNeedCache && (*plugin)->property("CacheThumbnail").toBool())
                bNeedCache = true;
        }
        else
        {
            emitFailed(it.current());
            if (d->deleteItems)
                delete it.current();
        }
    }

  // Read configuration value for the maximum allowed size
    KConfig * config = KGlobal::config();
    KConfigGroupSaver cgs( config, "PreviewSettings" );
    d->maximumSize = config->readNumEntry( "MaximumSize", 1024*1024 /* 1MB */ );

    if (bNeedCache)
    {
        KGlobal::dirs()->addResourceType( "thumbnails", "share/thumbnails/" );
        if (d->width == d->height)
        {
            if (d->width == 48)
                d->sizeName = "small";
            else if (d->width == 64)
                d->sizeName = "med";
            else if (d->width == 90)
                d->sizeName = "large";
            else if (d->width == 112)
                d->sizeName = "xxl";
            else
                d->sizeName.setNum(d->width);
        }
        else
            d->sizeName = QString::fromLatin1("%1x%2").arg(d->width).arg(d->height);
    }
    else
        d->bSave = false;
/*    // Check if we're in a thumbnail dir already
    if ( m_iconView->url().isLocalFile() )
    {
      // there's exactly one path
      QString cachePath = QDir::cleanDirPath( KGlobal::dirs()->resourceDirs( "thumbnails" )[0] );
      QString dir = QDir::cleanDirPath( m_iconView->url().directory() );
      if ( dir.startsWith( cachePath ) )
        thumbPath = dir.mid( cachePath.length() );
    }*/
    determineNextFile();
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
    KURL oldURL;
    if (d->currentItem.item)
    {
        if (!d->succeeded)
            emitFailed();
        if (d->deleteItems)
            delete d->currentItem.item;
    }
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
        d->succeeded = false;
        d->items.remove(d->items.begin());
        if (d->bSave)
        {
            oldURL.setPath(QDir::cleanDirPath(oldURL.directory()));
            KURL newURL = d->currentItem.item->url();
            newURL.setPath(QDir::cleanDirPath(newURL.directory()));
            // different path, determine cache dir
            if (oldURL != newURL)
            {
                KMD5 md5(QFile::encodeName(newURL.url()));
                QCString hash = md5.hexDigest();
                d->thumbPath = locateLocal("thumbnails",
                    QString::fromLatin1( hash.data(), 4 ) + "/" +
                    QString::fromLatin1( hash.data()+4, 4 ) + "/" +
                    QString::fromLatin1( hash.data()+8 ) + "/" +
                    d->sizeName + "/");
            }
        }
        KIO::Job *job = KIO::stat( d->currentItem.item->url(), false );
        addSubjob(job);
    }
}

void PreviewJob::slotResult( KIO::Job *job )
{
    subjobs.remove( job );
    Q_ASSERT ( subjobs.isEmpty() ); // We should have only one job at a time ...
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
                    if ( static_cast<unsigned long>((*it).m_long) > d->maximumSize && !d->currentItem.plugin->property("IgnoreMaximumSize").toBool() )
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

bool PreviewJob::statResultThumbnail()
{
    if ( d->thumbPath.isEmpty() )
        return false;

    QString thumbPath = d->thumbPath + d->currentItem.item->url().fileName();
    struct stat st;
    if ( ::stat( QFile::encodeName( thumbPath ), &st ) != 0 ||
         st.st_mtime < d->tOrig )
        return false;

    QPixmap pix;
    if ( !pix.load( thumbPath ) )
        return false;

    // Found it, use it
    emitPreview(pix);
    d->succeeded = true;
    determineNextFile();
    return true;
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
        job->addMetaData("thumbnail","1");
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
    addSubjob(job);
    connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)), SLOT(slotThumbData(KIO::Job *, const QByteArray &)));
    job->addMetaData("mimeType", d->currentItem.item->mimetype());
    job->addMetaData("width", QString().setNum(d->width));
    job->addMetaData("height", QString().setNum(d->height));
    job->addMetaData("iconSize", QString().setNum(d->iconSize));
    job->addMetaData("iconAlpha", QString().setNum(d->iconAlpha));
    job->addMetaData("plugin", d->currentItem.plugin->library());
    if (d->shmid == -1)
    {
        if (d->shmaddr) {
            shmdt((char*)d->shmaddr);
            shmctl(d->shmid, IPC_RMID, 0);
        }
        d->shmid = shmget(IPC_PRIVATE, d->width * d->height * 4, IPC_CREAT|0600);
        if (d->shmid != -1)
        {
            d->shmaddr = static_cast<uchar *>(shmat(d->shmid, 0, SHM_RDONLY));
            if (d->shmaddr == (uchar *)-1)
            {
                shmctl(d->shmid, IPC_RMID, 0);
                d->shmaddr = 0;
                d->shmid = -1;
            }
        }
        else
            d->shmaddr = 0;
    }
    if (d->shmid != -1)
        job->addMetaData("shmid", QString().setNum(d->shmid));
}

void PreviewJob::slotThumbData(KIO::Job *, const QByteArray &data)
{
    bool save = d->bSave && d->currentItem.plugin->property("CacheThumbnail").toBool();
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
    emitPreview(pix);
    d->succeeded = true;
}

void PreviewJob::emitPreview(const QPixmap &pix)
{
    emit gotPreview(d->currentItem.item, pix);
}

void PreviewJob::emitFailed(const KFileItem *item)
{
    if (!item)
        item = d->currentItem.item;
    emit failed(item);
}

void PreviewJob::saveThumbnail(const QByteArray &imgData)
{
    QFile file( d->thumbPath + d->currentItem.item->url().fileName() );
    if ( file.open(IO_WriteOnly) )
    {
        file.writeBlock( imgData.data(), imgData.size() );
        file.close();
    }
}

QStringList PreviewJob::availablePlugins()
{
    QStringList result;
    KTrader::OfferList plugins = KTrader::self()->query("ThumbCreator");
    for (KTrader::OfferList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
        result.append((*it)->desktopEntryName());
    return result;
}

QStringList PreviewJob::supportedMimeTypes()
{
    QStringList result;
    KTrader::OfferList plugins = KTrader::self()->query("ThumbCreator");
    for (KTrader::OfferList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
        result += (*it)->property("MimeTypes").toStringList();
    return result;
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
        fileItems.append(new KFileItem(KFileItem::Unknown, KFileItem::Unknown, *it, true));
    return new PreviewJob(fileItems, width, height, iconSize, iconAlpha,
                          scale, save, enabledPlugins, true);
}

void PreviewJob::virtual_hook( int id, void* data )
{ KIO::Job::virtual_hook( id, data ); }

