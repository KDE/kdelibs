   #include <kio/slavebase.h>
   #include <kinstance.h>
   #include <kdebug.h>
   #include <stdlib.h>
   #include <qtextstream.h>
   #include <klocale.h>
   #include <sys/stat.h>
   #include <dcopclient.h>  
   #include <qdatastream.h>

   class HelloProtocol : public KIO::SlaveBase
   {
   public:
      HelloProtocol( const QCString &pool, const QCString &app);
      virtual ~HelloProtocol();
#if 0
      virtual void get( const KURL& url );
#endif
      virtual void stat(const KURL& url);
      virtual void listDir(const KURL& url);
      void listRoot();
   private:
	DCOPClient *m_dcopClient;
	uint mountpointMappingCount();
	QString deviceNode(uint id);
	bool deviceMounted(const QString dev);
	bool deviceMounted(int);
	QString mountPoint(const QString dev);
	QString mountPoint(int);
	QString deviceType(int);
  };
  
  extern "C" {
      int kdemain( int, char **argv )
      {
	  kdDebug()<<"kdemain for mycomputer"<<endl;
          KInstance instance( "kio_hello" );
          HelloProtocol slave(argv[2], argv[3]);
          slave.dispatchLoop();
          return 0;
      }
  }
  


static void createFileEntry(KIO::UDSEntry& entry, const QString& name, const QString& url, const QString& mime);
static void createDirEntry(KIO::UDSEntry& entry, const QString& name, const QString& url, const QString& mime);

HelloProtocol::HelloProtocol( const QCString &pool, const QCString &app): SlaveBase( "mycomputer", pool, app )
{
	m_dcopClient=new DCOPClient();
	if (!m_dcopClient->attach())
	{
		kdDebug()<<"ERROR WHILE CONNECTING TO DCOPSERVER"<<endl;
	}
}

HelloProtocol::~HelloProtocol()
{
	delete m_dcopClient;
}

void HelloProtocol::stat(const KURL& url)
{
        QStringList     path = QStringList::split('/', url.encodedPathAndQuery(-1), false);
        KIO::UDSEntry   entry;
        QString mime;

	switch (path.count())
	{
		case 0:
		        createDirEntry(entry, i18n("My Computer"), "mycomputer:/", "inode/directory");
		        statEntry(entry);
			break;
		default:
			redirection("file:/");
//			createFileEntry(entry,i18n("blah"),url,"application/x-desktop");
			break;
	};
	

        finished();
}

void HelloProtocol::listDir(const KURL& url)
{
	kdDebug()<<"HELLO PROTOCOLL::listdir: "<<url.url()<<endl;

	if (url==KURL("mycomputer:/"))
		listRoot();
	else
	{
		QString device=url.queryItem("dev");
		if (url.queryItem("mounted")=="true")
		{
			QString mp=url.queryItem("mp");
			if (mp=="/") mp="";
			redirection("file:/"+mp);
		}
		else
			redirection("mycomputer:/");
		finished();
	}
	
}

uint HelloProtocol::mountpointMappingCount()
{
	QByteArray data;
	QByteArray param;
	QCString retType;
	uint count=0;
      if ( m_dcopClient->call( "kded",
		 "mountwatcher", "mountpointMappingCount()", param,retType,data,false ) )
      {
	QDataStream stream1(data,IO_ReadOnly);
	stream1>>count;
      }
      return count;
}

QString HelloProtocol::deviceNode(uint id)
{
	QByteArray data;
	QByteArray param;
	QCString retType;
	QString retVal;
	QDataStream streamout(param,IO_WriteOnly);
	streamout<<id;
	if ( m_dcopClient->call( "kded",
		 "mountwatcher", "devicenode(int)", param,retType,data,false ) )
      {
	QDataStream streamin(data,IO_ReadOnly);
	streamin>>retVal;
      }
      return retVal;

}

bool HelloProtocol::deviceMounted(const QString dev)
{
        QByteArray data;
        QByteArray param;
        QCString retType;
        bool retVal=false;
        QDataStream streamout(param,IO_WriteOnly);
        streamout<<dev;
        if ( m_dcopClient->call( "kded",
                 "mountwatcher", "mounted(QString)", param,retType,data,false ) )
      {
        QDataStream streamin(data,IO_ReadOnly);
        streamin>>retVal;
      }
      return retVal;
}

bool HelloProtocol::deviceMounted(int id)
{
        QByteArray data;
        QByteArray param;
        QCString retType;
        bool retVal=false;
        QDataStream streamout(param,IO_WriteOnly);
        streamout<<id;
        if ( m_dcopClient->call( "kded",
                 "mountwatcher", "mounted(int)", param,retType,data,false ) )
      {
        QDataStream streamin(data,IO_ReadOnly);
        streamin>>retVal;
      }
      return retVal;
}


QString HelloProtocol::mountPoint(const QString dev)
{
        QByteArray data;
        QByteArray param;
        QCString retType;
        QString retVal;
        QDataStream streamout(param,IO_WriteOnly);
        streamout<<dev;
        if ( m_dcopClient->call( "kded",
                 "mountwatcher", "mountpoint(QString)", param,retType,data,false ) )
      {
        QDataStream streamin(data,IO_ReadOnly);
        streamin>>retVal;
      }
      return retVal;
}

QString HelloProtocol::mountPoint(int id)
{
        QByteArray data;
        QByteArray param;
        QCString retType;
        QString retVal;
        QDataStream streamout(param,IO_WriteOnly);
        streamout<<id;
        if ( m_dcopClient->call( "kded",
                 "mountwatcher", "mountpoint(int)", param,retType,data,false ) )
      {
        QDataStream streamin(data,IO_ReadOnly);
        streamin>>retVal;
      }
      return retVal;
}



QString HelloProtocol::deviceType(int id)
{
        QByteArray data;
        QByteArray param;
        QCString retType;
        QString retVal;
        QDataStream streamout(param,IO_WriteOnly);
        streamout<<id;
        if ( m_dcopClient->call( "kded",
                 "mountwatcher", "type(int)", param,retType,data,false ) )
      {
        QDataStream streamin(data,IO_ReadOnly);
        streamin>>retVal;
      }
      return retVal;
}



void HelloProtocol::listRoot()
{
	KIO::UDSEntry   entry;
	uint count;
   {

      uint count=mountpointMappingCount();
	for (uint i=0;i<count;i++)
	{
		QString device=deviceNode(i);
		if (deviceMounted(i))
		        createFileEntry(entry, i18n("%1 mounted at %2").arg(deviceNode(i)).arg(mountPoint(i)), QString("mycomputer:/entries?dev=")+deviceNode(i)+"&mp="+mountPoint(i)+"&mounted=true", deviceType(i)+"_mounted");
		else
		        createFileEntry(entry, i18n("%1 (not mounted)").arg(deviceNode(i)), QString("mycomputer:/entries?dev=")+deviceNode(i)+"&mp="+mountPoint(i)+"&mounted=false", deviceType(i)+"_unmounted");
        	listEntry(entry, false);
		
	}
        totalSize(count);
        listEntry(entry, true);
   }



//        createFileEntry(entry, i18n("floppy"), "mycomputer:/blah", "kdedevice/floppy_unmounted");
  //      listEntry(entry, false);

        // Jobs entry

        // finish
        finished();
}

#if 0
 void HelloProtocol::get( const KURL& url )
 {
/*	mimeType("application/x-desktop");
	QCString output;
	output.sprintf("[Desktop Action Format]\n"
			"Exec=kfloppy\n"
			"Name=Format\n"
			"[Desktop Entry]\n"
			"Actions=Format\n"
			"Dev=/dev/fd0\n"
			"Encoding=UTF-8\n"
			"FSType=Default\n"
			"Icon=3floppy_mount\n"
			"MountPoint=/media/floppy\n"
			"ReadOnly=false\n"
			"Type=FSDevice\n"
			"UnmountIcon=3floppy_unmount\n"
			);
     data(output);
     finished();
 */
  redirection("file:/");
  //finished();
} 
#endif

void addAtom(KIO::UDSEntry& entry, unsigned int ID, long l, const QString& s = QString::null)
{
        KIO::UDSAtom    atom;
        atom.m_uds = ID;
        atom.m_long = l;
        atom.m_str = s;
        entry.append(atom);
}

static void createFileEntry(KIO::UDSEntry& entry, const QString& name, const QString& url, const QString& mime)
{
        entry.clear();
        addAtom(entry, KIO::UDS_NAME, 0, name);
        addAtom(entry, KIO::UDS_FILE_TYPE, S_IFDIR);//REG);
        addAtom(entry, KIO::UDS_URL, 0, url);
        addAtom(entry, KIO::UDS_ACCESS, 0500);
        addAtom(entry, KIO::UDS_MIME_TYPE, 0, mime);
        addAtom(entry, KIO::UDS_SIZE, 0);
        addAtom(entry, KIO::UDS_GUESSED_MIME_TYPE, 0, "inode/directory");
}


static void createDirEntry(KIO::UDSEntry& entry, const QString& name, const QString& url, const QString& mime)
{
        entry.clear();
        addAtom(entry, KIO::UDS_NAME, 0, name);
        addAtom(entry, KIO::UDS_FILE_TYPE, S_IFDIR);
        addAtom(entry, KIO::UDS_ACCESS, 0500);
        addAtom(entry, KIO::UDS_MIME_TYPE, 0, mime);
        addAtom(entry, KIO::UDS_URL, 0, url);
        addAtom(entry, KIO::UDS_SIZE, 0);
        addAtom(entry, KIO::UDS_GUESSED_MIME_TYPE, 0, "inode/directory");
//        addAtom(entry, KIO::UDS_GUESSED_MIME_TYPE, 0, "application/x-desktop");
}
