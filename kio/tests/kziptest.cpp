#include "kzip.h"
#include <stdio.h>
#include <kinstance.h>
#include <kdebug.h>
#include <qfile.h>

#include <assert.h>

void recursive_print( const KArchiveDirectory * dir, const QString & path )
{
  QStringList l = dir->entries();
  QStringList::Iterator it = l.begin();
  for( ; it != l.end(); ++it )
  {
    const KArchiveEntry* entry = dir->entry( (*it) );
    printf("mode=%07o %s %s size: %d pos: %d %s%s isdir=%d", entry->permissions(),
	entry->user().latin1(), entry->group().latin1(),
	entry->isDirectory() ? 0 : ((KArchiveFile*)entry)->size(),
	entry->isDirectory() ? 0 : ((KArchiveFile*)entry)->position(),
	path.latin1(), (*it).latin1(), entry->isDirectory());

//    if (!entry->isDirectory()) printf("%d", ((KArchiveFile*)entry)->size());
    printf("\n");
    if (entry->isDirectory())
      recursive_print( (KArchiveDirectory *)entry, path+(*it)+"/" );
  }
}


void recursive_transfer(const KArchiveDirectory * dir,
	    const QString & path, KZip * zip)
{
    QStringList l = dir->entries();
    QStringList::Iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
	kdDebug(7122) << "hi folks!" << endl;
        const KArchiveEntry* e = dir->entry( (*it) );
	kdDebug(7122) << "actual file: " << e->name() << endl;
	if (e->isFile())
	{
    	    Q_ASSERT( e && e->isFile() );
    	    const KArchiveFile* f = (KArchiveFile*)e;
    	    printf("FILE=%s\n", e->name().latin1());

    	    QByteArray arr( f->data() );
    	    printf("SIZE=%i\n",arr.size() );
    	    QString str( arr );
    	    printf("DATA=%s\n", str.latin1());

	    zip->writeFile( path+e->name().latin1(),
			    "holgi", "holgrp",
			    arr.size() , f->data() );
	}
	else if (e->isDirectory())
	{
	    recursive_transfer((KArchiveDirectory *)e ,
			path+e->name()+"/", zip);
	}
    }
}

int main( int argc, char** argv )
{
  if (argc < 3)
  {
    printf("\n"
 " Usage :\n"
 " ./kziptest list /path/to/existing_file.zip       tests listing an existing zip\n"
 " ./kziptest readwrite newfile.zip                 will create the zip, then close and reopen it.\n"
 " ./kziptest maxlength newfile.zip                 tests the maximum filename length allowed.\n"
 " ./kziptest print file.zip                        prints contents of all files.\n"
 " ./kziptest print2 file.zip filename              prints contents of one file.\n"
 " ./kziptest update file.zip filename              updates contents of one file.\n"
 " ./kziptest transfer file.zip newfile.zip         complete transfer.\n"
 " ./kziptest iodevice /path/to/existing_file.zip   tests KArchiveFile::device()\n");
    return 1;
  }
  KInstance instance("kziptest");
  QString command = argv[1];
  if ( command == "list" )
  {
    KZip zip( argv[2] );

    if ( !zip.open( IO_ReadOnly ) )
    {
      printf("Could not open %s for reading\n", argv[2] );
      return 1;
    }

    const KArchiveDirectory* dir = zip.directory();

    //printf("calling recursive_print\n");
    recursive_print( dir, "" );
    //printf("recursive_print called\n");

    zip.close();

    return 0;
  }
  else if (command == "readwrite" )
  {
    KZip zip( argv[2] );

    if ( !zip.open( IO_WriteOnly ) )
    {
      printf("Could not open %s for writing\n", argv[2]);
      return 1;
    }

    zip.setCompression( KZip::NoCompression );
    zip.writeFile( "typeid", "", "", 19, "application/x-kword" );
    zip.setCompression( KZip::DeflateCompression );
    zip.writeFile( "empty", "weis", "users", 0, "" );
    zip.writeFile( "test1", "weis", "users", 5, "Hallo" );
    zip.writeFile( "test2", "weis", "users", 8, "Hallo Du" );
    zip.writeFile( "mydir/test3", "weis", "users", 13, "Noch so einer" );
    zip.writeFile( "my/dir/test3", "dfaure", "hackers", 29, "I don't speak German (David)" );

#define SIZE1 100
    // Now a medium file : 100 null bytes
    char medium[ SIZE1 ];
    memset( medium, 0, SIZE1 );
    zip.writeFile( "mediumfile", "user", "group", SIZE1, medium );
    // Another one, with an absolute path
    zip.writeFile( "/dir/subdir/mediumfile2", "user", "group", SIZE1, medium );

    // Now a huge file : 20000 null bytes
    int n = 20000;
    char * huge = new char[ n ];
    memset( huge, 0, n );
    zip.writeFile( "hugefile", "user", "group", n, huge );
    delete [] huge;

    zip.close();

    printf("-----------------------\n");

    if ( !zip.open( IO_ReadOnly ) )
    {
      printf("Could not open %s for reading\n", argv[2] );
      return 1;
    }

    const KArchiveDirectory* dir = zip.directory();
    recursive_print(dir, "");

    const KArchiveEntry* e = dir->entry( "mydir/test3" );
    Q_ASSERT( e && e->isFile() );
    const KArchiveFile* f = (KArchiveFile*)e;

    QByteArray arr( f->data() );
    Q_ASSERT( arr.size() == 13 );
    printf("SIZE=%i\n",arr.size() );
    QString str( arr );
    Q_ASSERT( str == "Noch so einer" );
    printf("DATA=%s\n", str.latin1());

    e = dir->entry( "mediumfile" );
    Q_ASSERT( e && e->isFile() );
    f = (KArchiveFile*)e;
    Q_ASSERT( f->data().size() == SIZE1 );

    e = dir->entry( "hugefile" );
    Q_ASSERT( e && e->isFile() );
    f = (KArchiveFile*)e;
    Q_ASSERT( f->data().size() == 20000 );

    zip.close();

    return 0;
  }
  else if ( command == "maxlength" )
  {
    KZip zip( argv[2] );

    if ( !zip.open( IO_WriteOnly ) )
    {
      printf("Could not open %s for writing\n", argv[2]);
      return 1;
    }
    // Generate long filenames of each possible length bigger than 98...
    for (int i = 98; i < 500 ; i++ )
    {
      QString str, num;
      str.fill( 'a', i-10 );
      num.setNum( i );
      num = num.rightJustify( 10, '0' );
      zip.writeFile( str+num, "testu", "testg", 3, "hum" );
    }
    // Result of this test : it fails at 482 (instead of 154 previously).
    // Ok, I think we can do with that :)
    zip.close();
    printf("Now run 'unzip -l %s'\n", argv[2]);
    return 0;
  }
  else if ( command == "iodevice" )
  {
    KZip zip( argv[2] );
    if ( !zip.open( IO_ReadOnly ) )
      return 1;
    const KArchiveDirectory* dir = zip.directory();
    assert(dir);
    const KArchiveEntry* entry = dir->entry( "my/dir/test3" );
    if ( entry && entry->isFile() )
    {
        QIODevice *dev = static_cast<const KZipFileEntry *>(entry)->device();
        QByteArray contents = dev->readAll();
        printf("contents='%s'\n", QCString(contents).data());
    } else
        printf("entry=%p - not found if 0, otherwise not a file\n", (void*)entry);
    return 0;
  }
  else if (command == "print" )
  {
    KZip zip( argv[2] );
    if ( !zip.open( IO_ReadOnly ) )
    {
      printf("Could not open %s for reading\n", argv[2] );
      return 1;
    }
    const KArchiveDirectory* dir = zip.directory();
    QStringList l = dir->entries();
    QStringList::Iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
	kdDebug(7122) << "hi folks!" << endl;
        const KArchiveEntry* e = dir->entry( (*it) );
	if (e->isFile())
	{
    	    Q_ASSERT( e && e->isFile() );
    	    const KArchiveFile* f = (KArchiveFile*)e;

    	    QByteArray arr( f->data() );
    	    printf("SIZE=%i\n",arr.size() );
    	    QString str( arr );
    	    printf("DATA=%s\n", str.latin1());
	}
    }
    zip.close();
    return 0;
  }
  else if (command == "print2" )
  {
    if (argc != 4)
    {
        printf("usage: kziptest.cpp print2 archivename filename");
	return 1;
    }
    KZip zip( argv[2] );
    if ( !zip.open( IO_ReadOnly ) )
    {
      printf("Could not open %s for reading\n", argv[2] );
      return 1;
    }
    const KArchiveDirectory* dir = zip.directory();
    const KArchiveEntry* e = dir->entry( argv[3] );
    Q_ASSERT( e && e->isFile() );
    const KArchiveFile* f = (KArchiveFile*)e;

    QByteArray arr( f->data() );
    printf("SIZE=%i\n",arr.size() );
    QString str( arr );
//    printf("DATA=%s\n", str.latin1());
    printf("%s", str.latin1());
    zip.close();

    return 0;

  }
  else if (command == "update" )
  {
    if (argc != 4)
    {
        printf("usage: kziptest.cpp update archivename filename");
       return 1;
    }
    KZip zip( argv[2] );
    if ( !zip.open( IO_ReadWrite ) )
    {
      printf("Could not open %s for read/write\n", argv[2] );
      return 1;
    }
    const KArchiveEntry* e = zip.directory()->entry( argv[3] );
//    Q_ASSERT( e && e->isFile() );
//    const KArchiveFile* f = (KArchiveFile*)e;

//    QCString data( "This is some new data that goes into " );
  //  data += argv[3];
    QFile f ( argv[3] );
    if (!f.open( IO_ReadOnly )) 
    {
      printf("Could not open %s for reading\n", argv[2] );
      return 1;
    }
    	
    QDataStream s( &f );
    
    
//    zip.writeFile( argv[3], "", "", data.size(), data.data() );
    zip.writeFile( argv[3], "", "", f.size(), f.readAll() );
    zip.close();

    return 0;

  }
  else if (command == "transfer" )
  {
    if (argc != 4)
    {
        printf("usage: kziptest.cpp transfer sourcefile destfile");
	return 1;
    }
    KZip zip1( argv[2] );
    KZip zip2( argv[3] );
    if ( !zip1.open( IO_ReadOnly ) )
    {
      printf("Could not open %s for reading\n", argv[2] );
      return 1;
    }
    if ( !zip2.open( IO_WriteOnly ) )
    {
      printf("Could not open %s for writing\n", argv[3] );
      return 1;
    }
    const KArchiveDirectory* dir1 = zip1.directory();

    recursive_transfer(dir1, "", &zip2 );

    zip1.close();
    zip2.close();

/*
    zip.writeFile( "empty", "weis", "users", 0, "" );
    zip.writeFile( "test1", "weis", "users", 5, "Hallo" );
    zip.writeFile( "test2", "weis", "users", 8, "Hallo Du" );
    zip.writeFile( "mydir/test3", "weis", "users", 13, "Noch so einer" );
    zip.writeFile( "my/dir/test3", "dfaure", "hackers", 29, "I don't speak German (David)" );

#define SIZE1 100
    // Now a medium file : 100 null bytes
    char medium[ SIZE1 ];
    memset( medium, 0, SIZE1 );
    zip.writeFile( "mediumfile", "user", "group", SIZE1, medium );
    // Another one, with an absolute path
    zip.writeFile( "/dir/subdir/mediumfile2", "user", "group", SIZE1, medium );

    // Now a huge file : 20000 null bytes
    int n = 20000;
    char * huge = new char[ n ];
    memset( huge, 0, n );
    zip.writeFile( "hugefile", "user", "group", n, huge );
    delete [] huge;

    zip.close();

    printf("-----------------------\n");

    if ( !zip.open( IO_ReadOnly ) )
    {
      printf("Could not open %s for reading\n", argv[2] );
      return 1;
    }

    const KArchiveDirectory* dir = zip.directory();
    recursive_print(dir, "");

    const KArchiveEntry* e = dir->entry( "mydir/test3" );
    Q_ASSERT( e && e->isFile() );
    const KArchiveFile* f = (KArchiveFile*)e;

    QByteArray arr( f->data() );
    printf("SIZE=%i\n",arr.size() );
    QString str( arr );
    printf("DATA=%s\n", str.latin1());

    zip.close();

    return 0;





    const KArchiveEntry* e = dir1->entry( argv[3] );
    Q_ASSERT( e && e->isFile() );
    const KArchiveFile* f = (KArchiveFile*)e;

    QByteArray arr( f->data() );
//    printf("SIZE=%i\n",arr.size() );
    QString str( arr );
//    printf("DATA=%s\n", str.latin1());
    printf("%s", str.latin1());
    zip.close();

*/
    return 0;

  }
  else
    printf("Unknown command\n");
}
