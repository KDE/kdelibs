#include <kmimetype.h>
#include <kmimemagic.h>
#include <kinstance.h>
#include <stdio.h>

int main( int argc, char** argv )
{
  if (argc != 2)
  {
      printf("\n"
             "Usage : ./kmimemagictest file\n");
      return 1;
  }
  KInstance blah("kmimemagictest");

  QString file = QString::fromLocal8Bit( argv[1] );

  KMimeMagicResult * result = KMimeMagic::self()->findFileType( file );

  if ( result->isValid() )
      printf( "Found %s, accuracy %d\n", result->mimeType().latin1(), result->accuracy() );
  else
      printf( "Invalid result\n");

  KMimeType::Format f = KMimeType::findFormatByFileContent( file );
  if (f.text)
    printf("Text\n");
  else
    printf("Binary\n");

  if (f.compression == KMimeType::Format::GZipCompression)
    printf("GZipped\n");

  return 0;
}
