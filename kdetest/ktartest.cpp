#include "ktar.h"
#include <stdio.h>

/*
 * Usage : if you define TORBEN_TEST, run with ./ktartest /tmp/whatever.tar.gz
 * It will create the tar.gz, then close and reopen it
 *
 * if you don't, run with ./ktartest /path/to/existing_file.tar.gz
 * it will list its contents
 *
 */

#define TORBEN_TEST

void recursive_print( const KTarDirectory * dir, const QString & path )
{
  QStringList l = dir->entries();
  QStringList::Iterator it = l.begin();
  for( ; it != l.end(); ++it )
  {
    printf("%s%s\n", path.latin1(), (*it).latin1());
    const KTarEntry* entry = dir->entry( (*it) );
    if (entry->isDirectory())
      recursive_print( (KTarDirectory *)entry, path+(*it)+"/" );
  }
}

#ifdef TORBEN_TEST

int main( int argc, char** argv )
{
  KTar tar( argv[1] );

  if ( !tar.open( IO_WriteOnly ) )
  {
    printf("Could not open %s for writing\n", argv[1] );
    return 1;
  }

  tar.writeFile( "test1", "weis", "users", 5, "Hallo" );
  tar.writeFile( "test2", "weis", "users", 8, "Hallo Du" );
  tar.writeFile( "mydir/test3", "weis", "users", 13, "Noch so einer" );
  tar.writeFile( "my/dir/test3", "dfaure", "hackers", 13, "I don't speak German (David)" );
  // This last one should, in theory, create /my and /my/dir, but only creates /my/dir
  // It doesn't really matter since we handle that when reopening the file
  // See note near findOrCreate in ktar.h

  tar.close();

  printf("-----------------------\n");

  if ( !tar.open( IO_ReadOnly ) )
  {
    printf("Could not open %s for reading\n", argv[1] );
    return 1;
  }

  const KTarDirectory* dir = tar.directory();
  recursive_print(dir, "/");

  const KTarEntry* e = dir->entry( "mydir/test3" );
  ASSERT( e && e->isFile() );
  const KTarFile* f = (KTarFile*)e;

  QByteArray arr( f->data() );
  printf("SIZE=%i\n",arr.size() );
  QString str( arr );
  printf("DATA=%s\n", str.latin1());

  tar.close();

  return 0;
}

#else
// David's test

int main( int argc, char** argv )
{
  KTar tar( argv[1] );

  if ( !tar.open( IO_ReadOnly ) )
  {
    printf("Could not open %s for reading\n", argv[1] );
    return 1;
  }

  const KTarDirectory* dir = tar.directory();

  //printf("calling recursive_print\n");
  recursive_print( dir, "/" );
  //printf("recursive_print called\n");

  tar.close();

  return 0;
}

#endif 
