#include "kio_cache.h"
#include "kpixmapcache.h"
#include "kmimetypes.h"

#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include <qdatetime.h>
#include <qtextstream.h>
#include <qfile.h>

#include <ksimpleconfig.h>
#include <kurl.h>
#include <kapp.h>
#include <kdebug.h>

/************************************************
 *
 * KIOCacheEntry
 *
 ************************************************/

KIOCacheEntry::KIOCacheEntry (const KIOCacheEntry &ref)
{
    expires = ref.expires;
    created = ref.created;
    lastAccess = ref.lastAccess; 
    lastModified = ref.lastModified;
    data.setBuffer( ref.data.buffer() );
    docURL = ref.docURL;
    localName = ref.localName;
}

KIOCacheEntry::KIOCacheEntry (const QString& _url)
{
    docURL = _url;
}

QByteArray KIOCacheEntry::content() const
{
    if (docURL.isEmpty()) return QByteArray();
    if (!data.buffer().isEmpty()) return data.buffer();

    if (localName.isEmpty() || KIOCache::cacheRoot().isEmpty())
	return QByteArray();

    QFile content_file( KIOCache::cacheRoot() + localName );    
    QByteArray retval;

    if ( !content_file.open( IO_ReadOnly ) ) return retval;
    // Add a trailing 0 => "+1"
    retval.resize( content_file.size() + 1 );
    if ( content_file.readBlock(retval.data(), retval.size() != retval.size() ) )
    {
      content_file.close();
      return QByteArray();
    }
    content_file.close();
    // Add the trailing 0
    retval.data()[ content_file.size() ] = 0;
    return retval;
}

void KIOCacheEntry::addData(const QByteArray &_data)
{ 
  addData( _data.data(), _data.size() );
}

void KIOCacheEntry::addData( const char *_data, int _len )
{ 
  if ( !data.isOpen() )
    data.open( IO_WriteOnly );

  data.writeBlock( _data, _len );
}

bool KIOCacheEntry::storeData()
{
  if ( data.isOpen() )
  {
    // Put a trailing 0 in here. We need that to terminate a string.
    data.putch( 0 );
    data.close();
  }
  
  if ( localName.isEmpty() || KIOCache::cacheRoot().isNull() || docURL.isEmpty())
    return false;
  
  QFile content_file( KIOCache::cacheRoot() + localName );    
  if ( !content_file.open( IO_WriteOnly ) )
    return false;
  if ( !data.buffer().isEmpty() )
    // Dont save the trailing 0
    if ( content_file.writeBlock( data.buffer().data(), data.buffer().size() - 1 ) != (int)data.buffer().size() - 1 )
    {
      content_file.close();
      return false;
    }
  content_file.close();

  return true;
}


/*************************************************
 *
 * KIOCache
 *
 *************************************************/

QString KIOCache::cachePath;
QDict<KIOCacheEntry> KIOCache::cacheDict;
QStrList KIOCache::cacheProtocols;
QStrList KIOCache::excludeHosts;
bool KIOCache::cacheEnabled = false;
bool KIOCache::saveCacheEnabled = false;
unsigned int KIOCache::maxURLLength = 80;

/**
 * Constructs a QDateTime from a string of the form
 * 'Sat May 20 1995 03:40:13'. This is the same format
 * QDateTime::toString() produces, and it is used to read dates and
 * times from the on-disk cache storage. This really should allow for
 * parsing more than this single format.
 */
QDateTime parseDateTime(const QString &s)
{
    const char* months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    int month, year, day, hour, min, sec;

    // "Parse" the string. Quick hack, just handles the format
    // specified above.
    QString monthStr = s.mid(4, 3);    
    for (month = 0; monthStr != months[month] && month < 12; month++);
    month++;
    day  = s.mid(9,2).toInt();
    year = s.mid(12,4).toInt();
    hour = s.mid(17,2).toInt();
    min  = s.mid(20,2).toInt();
    sec  = s.mid(23,2).toInt();
    
    return QDateTime(QDate(year, month, day), QTime(hour, min, sec));
}

static void testDir( const char *_name )
{
    DIR *dp;
    QString c = kapp->localkdedir().data();
    c += _name;
    dp = opendir( c.data() );
    if ( dp == NULL )
	::mkdir( c.data(), S_IRWXU );
    else
	closedir( dp );
}

void KIOCache::initStatic()
{
  testDir( "/share" );
  testDir( "/share/apps" );
  testDir( "/share/apps/kio" );
  testDir( "/share/apps/kio/cache" );
  
  cachePath = kapp->localkdedir().data();
  cachePath += "/share/apps/kio/cache/";

  readConfig( *( kapp->getConfig() ) );
}

void KIOCache::readConfig( KConfig &config ) 
{
    KConfigGroupSaver gs(&config, "Cache");

    QString path = kapp->localkdedir().data();
    path += "/share/apps/kio/cache/";
    cachePath = config.readEntry( "CachePath", path );
    if ( cachePath.right(1).at(1) != '/')
	cachePath += "/";
    cacheEnabled = config.readBoolEntry("UseCache", true );
    maxURLLength = config.readNumEntry("maxURLLength", 80 );
    
    config.readListEntry("ExcludeHosts", excludeHosts);
    if ( config.hasKey("Protocols") )
    {
      config.readListEntry("Protocols", cacheProtocols);
    }
    else
    {
      cacheProtocols.inSort("cgi");
      cacheProtocols.inSort("http");
    }
}


KConfig &KIOCache::storeConfig(KConfig &config) 
{
    KConfigGroupSaver gs(&config, "Cache");

    config.writeEntry("CachePath", cachePath);
    config.writeEntry("UseCache", cacheEnabled);
    
    config.writeEntry("ExcludeHosts", excludeHosts);
    config.writeEntry("Protocols", cacheProtocols);
    config.writeEntry("maxURLLength", maxURLLength);

    return config;
}

bool KIOCache::readCache()
{
    if (cachePath.isEmpty()) return false;

    KSimpleConfig cacheInfo(cachePath + "index.kdelnk", true);
    unsigned long idx;
    QString entryPrefix("Cacheentry ");
    QString entryNum;
    QString url;
    QDateTime date;
    KIOCacheEntry *entry;

    // Read the individual entries and put them into the
    // dictionary.
    idx = 1;
    entryNum.setNum(idx);
    cacheInfo.setGroup(entryPrefix + entryNum);
    url = cacheInfo.readEntry("URL");
    cacheDict.setAutoDelete(true);
    while (!url.isEmpty()) {
	entry = new KIOCacheEntry(url);
	entry->setLocalKey(cacheInfo.readEntry("LocalKey"));

	date = parseDateTime(cacheInfo.readEntry("Expires"));
	entry->setExpiresAt(date);
	date = parseDateTime(cacheInfo.readEntry("Created"));
	entry->setCreatedAt(date);
	date = parseDateTime(cacheInfo.readEntry("LastModified"));
	entry->setLastModifiedAt(date);
	date = parseDateTime(cacheInfo.readEntry("LastAccess"));
	entry->setLastAccessedAt(date);
	cacheDict.insert(url, entry);
	entryNum.setNum(++idx);
	cacheInfo.setGroup(entryPrefix + entryNum);
	url = cacheInfo.readEntry("URL");
    }
    return true;
}

bool KIOCache::storeCache()
{
    if (!saveCacheEnabled || cachePath.isEmpty()) return false;

    KSimpleConfig cacheInfo(cachePath + "index.kdelnk");
    unsigned long idx;
    QString entryPrefix("Cacheentry ");
    QString entryNum;
    QDictIterator<KIOCacheEntry> entry(cacheDict);

    // Delete all [Cacheentry #] groups
    idx = 1;
    do {
	entryNum.setNum(idx++);
    } while (cacheInfo.deleteGroup(entryPrefix + entryNum));
    
    // Write a [Cacheentry #] group for every cache entry.
    idx = 1;

    for (entry.toFirst(); entry != 0L; ++entry) {
	entryNum.setNum(idx++);
	cacheInfo.setGroup(entryPrefix + entryNum);
	cacheInfo.writeEntry("URL", entry.current()->url());
	cacheInfo.writeEntry("Expires", 
			     entry.current()->expiresAt().toString());
	cacheInfo.writeEntry("Created", 
			     entry.current()->createdAt().toString());
	cacheInfo.writeEntry("LastModified", 
			     entry.current()->lastModifiedAt().toString());
	cacheInfo.writeEntry("LastAccess", 
			     entry.current()->lastAccessedAt().toString());
    }

    if ( storeIndex().isEmpty() )
      return false;

    return true;
}

QString KIOCache::storeIndex()
{
  kdebug( KDEBUG_INFO, 7002, "STORING INDEX" );
  
  // No need to write the config file, KSimpleConfig handles this at
  // destruction time. But write a new index.html for the cache directory
  QString file = cacheRoot() + "index.html";
  QFile index_file( file );
  if ( !index_file.open( IO_WriteOnly ) )
    return QString();

  kdebug( KDEBUG_INFO, 7002, "WRITING INDEX" );

  QString html = htmlIndex();
  if ( html.isEmpty() )
    return QString();
  
  kdebug( KDEBUG_INFO, 7002, "WRITING INDEX 2" );

  if ( index_file.writeBlock( html.ascii(), html.length() + 1 ) != (int)html.length() + 1 )
  {
    index_file.close();
    return QString();
  }
  kdebug( KDEBUG_INFO, 7002, "CLOSING" );
  index_file.close();
  kdebug( KDEBUG_INFO, 7002, "INDEX DONE" );
  return file;
};


bool KIOCache::clear()
{
    QStrList todie;
    
    QDictIterator<KIOCacheEntry> it( cacheDict );
    for ( ; it.current() != 0L; ++it )
	if ( unlink( cachePath + it.current()->localKey() ) == 0 )
	    todie.append( it.current()->url() );

    const char *s;
    for ( s = todie.first(); s != 0L; s = todie.next() )
	cacheDict.remove( s );
    
    return storeCache();
}
 
QString KIOCache::htmlIndex()
{
    QString cacheIndex;

    cacheIndex = "<HTML><HEAD><TITLE>";
    cacheIndex += i18n("KIO Cache");
    cacheIndex += "</TITLE></HEAD><BODY><H2>";
    cacheIndex += i18n("Contents of the cache");
    cacheIndex += "</H2>\n";
    cacheIndex += "<p>";
    // I know this line should be split to look better, but I don't
    // want to split this over different i18n entries.
    cacheIndex += i18n ("Use the icons for drag and drop actions, since they refer to the cache files on your hard disk, while you should use the textual links for browsing");
    cacheIndex += "</p><hr>\n";
    cacheIndex += "<table><tr><td>&nbsp;</td><td><b>URL</b></td><td><b>";
    cacheIndex += i18n("Creation Date");
    cacheIndex += "</b></td><td><b>";
    cacheIndex += i18n("Expiration Date");
    cacheIndex += "</b></td></tr>";

    QDictIterator<KIOCacheEntry> it( cacheDict );
    // !!! Really should sort the cache list. Just spitting out
    // entries in the order they appear in the dictionary is stupid.
    // Maybe sort by expiration date? 
    for ( ; it.current(); ++it )
    {
	// Abbreviate large URL's: First try to replace components of the
	// reference part by ellipses (...) until the abbreviated URL
	// fits the size, leaving the beginning of the URL and the
	// filename intact. If this still doesn't fit the limit, cut
	// the URL off at maxURLLength-3 and append an ellipsis.
	QString urlText = it.current()->url();
	
	if ( urlText.length() > maxURLLength )
	{
	    int ellipsisStart, ellipsisEnd;

	    ellipsisEnd = urlText.findRev('/');
	    ellipsisStart = ellipsisEnd;
	    while ( ellipsisStart > 0 &&
		    urlText.length()-ellipsisEnd+ellipsisStart+5 > maxURLLength )
	    {
		ellipsisStart = urlText.findRev('/', ellipsisStart);
	    }
	    if (ellipsisStart > 0 
		&&  urlText.length()-ellipsisEnd+ellipsisStart+5 <= maxURLLength)
	    {
	      urlText.replace(ellipsisStart+1, ellipsisEnd-ellipsisStart-2, "...");
	    }
	    else
	    {
	      urlText.replace(maxURLLength-3, urlText.length(), "...");
	    }
	}
	QString cachedFile = cachePath + it.current()->localFile();
	KURL::encode( cachedFile );
	// First table entry is a href to the local file,
	// displaying a mini icon of the document's mime type.
	cacheIndex += "<tr><td><a href=\"";
	cacheIndex += cachedFile;
	cacheIndex += "\"><img border=0 src=\"";
	if ( !it.current()->mimeType().isEmpty() )
	  cacheIndex += KPixmapCache::pixmapFileForMimeType( it.current()->mimeType(), true );
	else
	  cacheIndex += KPixmapCache::pixmapFileForURL( cachedFile, 0, true, true );
	
	cacheIndex += "\"></a></td> ";
	// Second entry is a href to the actual URL, displaying the
	// abbreviated version of the URL
	cacheIndex += "<td><a href=\"";
	cacheIndex += it.current()->url();
	cacheIndex += "\">\n";
	cacheIndex += urlText;
	cacheIndex += "\n</a></td>";

	// And last not least the creation and expiration dates of
	// the document.
	cacheIndex += "<td>";
	if ( it.current()->createdAt().isValid() )
	  cacheIndex += it.current()->createdAt().toString();
	else
	  cacheIndex += i18n("Unknown" );
	cacheIndex += "</td>";
	cacheIndex += "<td>";
	if ( it.current()->expiresAt().isValid() )
	  cacheIndex += it.current()->expiresAt().toString();
	else
	  cacheIndex += i18n("Never" );
	cacheIndex += "</td></tr>\n";
    }
    
    cacheIndex += "</table></BODY></HTML>\n";

    return cacheIndex;
}

bool KIOCache::isValid(const QString &url)
{
    if ( !isCached(url) )
      return false;
    QDateTime expires = lookup(url).expiresAt();
    return ( expires.isNull() || ( expires > QDateTime::currentDateTime() ) );
}

bool KIOCache::isCached(const QString &url) 
{
    return !lookup( url ).isEmpty();
}

const KIOCacheEntry& KIOCache::lookup(const QString &url)
{
    static const KIOCacheEntry NOTHING;
    KIOCacheEntry *entry;

    entry = cacheDict[trimURL(url)];
    return (entry?*entry:NOTHING);
}

bool KIOCache::insert( KIOCacheEntry *entry )
{
  kdebug( KDEBUG_INFO, 7002, "########### CACHE 1"  );
  
    // Check for consistency. At least an URL must be set, and it must
    // be cacheable.
    if (!isCacheable(entry->url())) return false;

  kdebug( KDEBUG_INFO, 7002, "########### CACHE 2"  );

    // Calculate default values for expiration if no expiration date
    // is given, check expiration date if it is given.
    QDateTime expires = entry->expiresAt();
    if ( expires.isValid() )
    {
      if ( expires <= QDateTime::currentDateTime() )
	return false;
    }
    else
    {
	entry->setExpiresAt( defaultExpire( entry ) );
    }

  kdebug( KDEBUG_INFO, 7002, "########### CACHE 3"  );

    // Do we have an entry already? If yes, delete the local file.
    KIOCacheEntry oldEntry = lookup( entry->url() );
    if ( !oldEntry.isEmpty() )
    {
      unlink( cachePath + oldEntry.localFile() );
    }
    
  kdebug( KDEBUG_INFO, 7002, "########### CACHE 4"  );

    // OK, put the new entry into the cache.
    entry->setLocalKey( localKey( entry ) );
    entry->storeData();

  kdebug( KDEBUG_INFO, 7002, "########### CACHE 5"  );

    // Check if there is a mime type, if not determine one using
    // KMimeMagic on the cached file. Can't do it before this since
    // there is no cached file before entry->storeData()
    if ( entry->mimeType().isEmpty() )
    {
      QString file( cachePath.data() );
      file += entry->localFile();
      KURL::encode( file );
      KURL u( file );
      
      KMimeType* mime = KMimeType::findByURL( u, 0, true );
      entry->setMimeType( mime->mimeType() );
    }

  kdebug( KDEBUG_INFO, 7002, "########### CACHE 6"  );

    cacheDict.replace( trimURL( entry->url() ), entry );
    return true;
}

bool KIOCache::isCacheable(const QString &_url)
{
    KURL url(_url);
    
    if (url.isMalformed()) return false;

    const char *entry;

    for (entry = excludeHosts.first(); entry != 0;
	 entry = excludeHosts.next()) {
	if (QString(url.host()).findRev(entry, -1, false) >= 0) return false;
    }

    for (entry = cacheProtocols.first(); entry != 0;
	 entry = cacheProtocols.next()) {
	if (strcmp(entry, url.protocol()) == 0) return true;
    }
    return false;
}

QString KIOCache::trimURL(const QString &url)
{
  // Delete the reference of the right most (sub)protocol.
  KURL u( url );
  u.setHTMLRef( QString::null );
  return u.url();
}

QDateTime KIOCache::defaultExpire(const KIOCacheEntry *entry)
{
    // Not really implemented yet, simply return an invalid date
    // meaning 'expire never'. 
    return QDateTime();
}

QString KIOCache::localKey(const KIOCacheEntry *entry)
{
    static time_t lastCall;    
    static int sequence = 0;
    time_t timestamp = time(0l);
    KURL url(entry->url());

    if ( timestamp != lastCall ) 
      sequence = 0;
    
    QString localName( "entry-%1.%2-%3" );
    localName = localName.arg( timestamp ).arg(sequence).arg(url.filename() );

    lastCall = timestamp;
    
    return localName;    
}

/**************************************************
 *
 * CachedKIOJob
 *
 **************************************************/

CachedKIOJob::CachedKIOJob() : KIOJob()
{
  m_bLookInCache = true;
  m_pCurrentDoc = 0L;
  m_step = STEP_NONE;
}

CachedKIOJob::~CachedKIOJob()
{
  kdebug( KDEBUG_INFO, 7002, "RUNNING destructor %p", this );
  if ( m_pCurrentDoc )
    delete m_pCurrentDoc;
}

void CachedKIOJob::get( const char *_url, bool _reload )
{
  // Get the document from cache if possible
  if ( m_bLookInCache && !_reload && KIOCache::isValid( _url ) )
  {
    m_pCurrentDoc = new KIOCacheEntry( KIOCache::lookup( _url ) );
    // Lets feed the client with data, but return to the X11 loop first
    // since this would be the expected behaviour if the data is not cached.
    m_step = STEP_REDIRECTION;
    connect( &m_timer, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
    m_timer.start( 0, true );
    return;
  } 

  if ( !_reload )
  {    
    // Save the current document content to a cache entry even if the
    // document may not be cacheable at all.
    m_pCurrentDoc = new KIOCacheEntry( _url );
  }
  
  KIOJob::get( _url );
}

void CachedKIOJob::slotFinished()
{
  if ( !m_pCurrentDoc )
  {
    KIOJob::slotFinished();
    return;
  }
  
  kdebug( KDEBUG_INFO, 7002, "#### CACHE COMMIT 1" );

  if ( !KIOCache::insert(m_pCurrentDoc) )
  {
    kdebug( KDEBUG_INFO, 7002, "#### CACHE COMMIT 2" );
    delete m_pCurrentDoc;
    m_pCurrentDoc = 0L;
    // Should notify clients that something went wrong. But since
    // KIOCache::insert() can fail on perfectly valid documents
    // (e.g. because they are not cacheable), an error is
    // too strong here. Maybe a warning signal?
  }
  else
    m_pCurrentDoc = 0L;

  kdebug( KDEBUG_INFO, 7002, "#### CACHE COMMIT 3" );

  KIOJob::slotFinished();
}
    
void CachedKIOJob::slotData( void *_data, int _len )
{
  kdebug( KDEBUG_INFO, 7002, "#### CACHE ADD 1" );

  if ( m_pCurrentDoc )
    m_pCurrentDoc->addData( (const char*)_data, _len );

  kdebug( KDEBUG_INFO, 7002, "#### CACHE ADD 2" );

  KIOJob::slotData( _data, _len );

  kdebug( KDEBUG_INFO, 7002, "#### CACHE ADD 3" );
}
    
void CachedKIOJob::slotRedirection( const char *_url )
{
  if ( m_pCurrentDoc )
    m_pCurrentDoc->setRedirection( _url );

  KIOJob::slotRedirection( _url );
}

void CachedKIOJob::slotMimeType( const char *_mime )
{
  if ( m_pCurrentDoc )
    m_pCurrentDoc->setMimeType( _mime );

  KIOJob::slotMimeType( _mime );
}

void CachedKIOJob::slotError( int _errid, const char* _errortext )
{
  if ( m_pCurrentDoc )
  {
    // Forget the whole story
    delete m_pCurrentDoc;
    m_pCurrentDoc = 0L;
  }

  KIOJob::slotError( _errid, _errortext );
}

void CachedKIOJob::slotTimeout()
{
  kdebug( KDEBUG_INFO, 7002, "#### TIMER 1" );
  
  assert( m_pCurrentDoc );
  
  kdebug( KDEBUG_INFO, 7002, "#### TIMER 2" );

  switch( m_step )
    {
    case STEP_REDIRECTION:
      m_step = STEP_DATE;
      if ( !m_pCurrentDoc->redirection().isEmpty() )
	emit sigRedirection( m_id, m_pCurrentDoc->redirection() );
      else
      {  
	slotTimeout();
	return;
      }
      break;
    case STEP_DATE:
      m_step = STEP_MIME;
      // Not implemented yet
      slotTimeout();
      return;
      break;
    case STEP_MIME:
      m_step = STEP_DATA;
      if ( !m_pCurrentDoc->mimeType().isEmpty() )
	emit sigMimeType( m_id,  m_pCurrentDoc->mimeType() );
      else
      {  
	slotTimeout();
	return;
      }
      break;
    case STEP_DATA:
      m_step = STEP_FINISHED;
      // The trailing 0 does not belong to the data itself => "-1"
      emit sigData( m_id, m_pCurrentDoc->content().data(), m_pCurrentDoc->content().size() - 1 );
      break;
    case STEP_FINISHED:
      m_step = STEP_NONE;
      emit sigFinished( m_id );
      break;
    default:
      assert( 0 );
    }
  
  kdebug( KDEBUG_INFO, 7002, "#### TIMER 3" );

  if ( m_step != STEP_NONE )
    m_timer.start( 0, true );
  else if ( m_bAutoDelete )
  {
    kdebug( KDEBUG_INFO, 7002, "#### TIMER 4a %p", this );
    delete this;
    kdebug( KDEBUG_INFO, 7002, "#### TIMER 5" );
    return;
  }

  kdebug( KDEBUG_INFO, 7002, "#### TIMER 4b" );
}

#include "kio_cache.moc"

