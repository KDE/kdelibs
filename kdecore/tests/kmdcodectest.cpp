/*
  kmd5test.cc - a test suite for KDE's implementation of MD2, MD4 and MD5
  Copyright (C) 2000 Dawit Alemayehu <adawit@kde.org>

  This work is completely based on the work done by
  Copyright (C) 1995 by Mordechai T. Abzug and
  Copyright (C) 1990-1992, RSA Data Security, Inc. Created 1990. All rights reserved.
  See the "License" file for details.
*/

#include <config.h>

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <qbuffer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kapp.h>

#include <kmdcodec.h>

#define TEST_BLOCK_LEN 1000             // Length of test blocks.
#define TEST_BLOCK_COUNT 10000          // Number of test blocks.
#define MAX_READ_BUF_SIZE 8192

enum Codec {
    Unspecified=0,
    Base64Encode,
    Base64Decode,
    UUEncode,
    UUDecode,
    QPEncode,
    QPDecode
};

void MD5_timeTrial ();
void MD5_testSuite ();
void MD5_string ( const char *, const char *expected = 0, bool rawOutput = false);
void MD5_file ( const char * , bool rawOutput = false );
void MD5_verify( const char*, const char*, bool );
void testCodec( const char*, Codec, bool );


// Returns the file descriptor if successful, -1 otherwise.
int openFile( const char* filename, const char* err_msg )
{
    struct stat st;
    if ( stat( filename, &st ) )
    {
        kdDebug() << err_msg << endl
                  << "Could not find a file named: " << filename << endl;
        return -1;
    }

    if ( !S_ISREG( st.st_mode ) )
    {
        kdDebug() << "{" << filename << "} is not a file!" << endl;
        return -1;
    }

    FILE* f = fopen( filename, "rb" );
    if( !f )
    {
        kdDebug() << err_msg << endl
                  << "Could not open a file named: " << filename << endl;
        return -1;
    }
    return fileno(f);
}

int readContent( int fd, QByteArray& buf, int count, const char* err_msg, bool closefd = true )
{
    int old_size = buf.size();
    buf.resize(old_size+count);
    int result = read( fd, buf.data()+old_size, count );
    if ( result > 0 && result < count )
    {
      buf.resize( old_size + result );
    }
    else if ( result == 0 )
    {
      buf.resize( old_size );
      if ( closefd && close(fd) == -1 )
          kdDebug() << "WARNING: unable to close the file properly!" << endl;
    }
    else if ( result == -1 )
    {
      kdDebug() << err_msg << endl
                << "Could not read the file!" << endl;
      if ( close(fd) == -1 )
        kdDebug() << "WARNING: unable to close the file properly!" << endl;
    }
    return result;
}

void testCodec( const char* msg, Codec type, bool isFile )
{
    QByteArray output;
    if ( isFile )
    {
        // Open requested file...
        const char* err_msg = "Encoding with \"base64\" failed!";
        int fd = openFile( msg, err_msg );
        if ( fd == -1 ) return;

        // Read contents of file...
        int count=0;
        QByteArray data;
        while((count=readContent(fd, data, MAX_READ_BUF_SIZE, err_msg)) > 0);

        // Error! Exit!
        if ( count == -1 ) return;

        // Perform the requested encoding or decoding...
        switch (type)
        {
            case Base64Encode:
                KCodecs::base64Encode(data, output, true);
                break;
            case Base64Decode:
                KCodecs::base64Decode(data, output);
                break;
            case UUEncode:
                KCodecs::uuencode(data, output);
                break;
            case UUDecode:
                KCodecs::uudecode(data, output);
                break;
#if 0
            case QPEncode:
                KCodecs::quotedPrintableEncode(data, output);
                break;
            case QPDecode:
                KCodecs::quotedPrintableDecode(data, ouput);
                break;
#endif
            default:
                break;
        }
        char* result = output.data();
        int size = output.size();
        // NULL terminate string before printing!
        if ( size && result[size-1] != '\0' )
            result[size] = '\0';
        kdDebug() << "Result: \n" << result << endl;
    }
    else
    {
        int size = strlen(msg);
        output.resize(size);
        memcpy( output.data(), msg, size );
        QCString out;
        switch (type)
        {
          case Base64Encode:
            out = KCodecs::base64Encode(output);
            break;
          case Base64Decode:
            kdDebug() << "Decoding base64 data..." << endl;
            out = KCodecs::base64Decode(output);
            kdDebug() << "Base64 decoding completed!" << endl;
            break;
          case UUEncode:
            out = KCodecs::uuencode(output);
            break;
          case UUDecode:
            out = KCodecs::uudecode(output);
            break;
#if 0
          case QPEncode:
            out = KCodecs::quotedPrintableEncode(output);
            break;
          case QPDecode:
            out = KCodecs::quotedPrintableDecode(output);
            break;
#endif
          default:
            break;
        }
        kdDebug() << out << endl;
    }
}

// Measures the time to digest TEST_BLOCK_COUNT TEST_BLOCK_LEN-byte blocks.
void MD5_timeTrial ()
{
    KMD5 context;
    time_t endTime, startTime;
    Q_UINT8 block[TEST_BLOCK_LEN];
    Q_UINT32 i;

    kdDebug() << endl << "Timing test. Digesting "
              << TEST_BLOCK_COUNT << " blocks of "
              << TEST_BLOCK_LEN << "-byte..." << endl;

    // Initialize block
    for (i = 0; i < TEST_BLOCK_LEN; i++)
        block[i] = (Q_UINT8)(i & 0xff);

    // Start timer
    time (&startTime);

    // Digest blocks
    for (i = 0; i < TEST_BLOCK_COUNT; i++)
        context.update (block, TEST_BLOCK_LEN);

    context.finalize();

    // Stop timer
    time (&endTime);

    long duration = endTime - startTime;
    long speed = duration ? (TEST_BLOCK_LEN * (TEST_BLOCK_COUNT/duration)) : TEST_BLOCK_COUNT;
    kdDebug() << endl << "Result: " << endl
              << "  Digest = " << context.hexDigest() << endl
              << "  Time   = "<< duration << " seconds" << endl
              << "  Speed  = " << speed << " bytes/second" << endl << endl;
}

// Digests a reference suite of strings and prints the results.

void MD5_testSuite ()
{
  kdDebug() << "MD5 test preset test suite as defined in RFC 1321:" << endl;
  MD5_string ( "", "d41d8cd98f00b204e9800998ecf8427e" );
  MD5_string ( "a", "0cc175b9c0f1b6a831c399e269772661" );
  MD5_string ( "abc", "900150983cd24fb0d6963f7d28e17f72" );
  MD5_string ( "message digest", "f96b697d7cb7938d525a2f31aaf161d0" );
  MD5_string ( "abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b" );
  MD5_string ( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
               "d174ab98d277d9f5a5611c2c9f419d9f" );
  MD5_string ( "12345678901234567890123456789012345678901234567890123456789012"
               "345678901234567890", "57edf4a22be3c955ac49da2e2107b67a" );
}

void MD5_verify( const char *input, const char *digest, bool isFile )
{
    KMD5 context;
    bool result;
    if ( !isFile )
    {
        result = context.verify( QCString(input), digest );
        kdDebug() << endl << "Input string: " << input << endl;
    }
    else
    {
        FILE* f = fopen( input, "r" );
        if ( !f )
            kdFatal() << "Cannot open file for reading!"  << endl;
        result = context.verify ( f, digest );
        kdDebug() << endl << "Input filename: " << input << endl;
    }
    kdDebug() << "Calculated Digest = " <<  context.hexDigest() << endl
              << "Supplied Digest   = " << digest << endl
              << "Verified As: " << (result ? "A MATCH!!":"NOT A MATCH!!")
              << endl << endl;
}

void MD5_file (const char *filename, bool rawOutput )
{
    FILE* f = fopen( filename, "r" );

    if (!f)
    {
        kdError() << "(" << filename << ") cannot be opened!" << endl;
    }
    else
    {
       KMD5 context( f );
       if ( rawOutput )
          kdDebug() <<  "MD5 ("  << filename <<  ") = "  <<  context.rawDigest() << endl;
       else
          kdDebug() <<  "MD5 ("  << filename <<  ") = "  <<  context.hexDigest() << endl;
    }
}

void MD5_string (const char *input, const char* expected, bool rawOutput )
{
  KMD5 context;
  QCString data = input;

  context.update ( data );
  context.finalize ();
  if ( rawOutput )
    kdDebug() << endl << "Result: MD5 (\"" << input << "\") = " << context.rawDigest() << endl;
  else
    kdDebug() << endl << "Result: MD5 (\"" << input << "\") = " << context.hexDigest() << endl;
  if ( expected )
    kdDebug() << "Expected: MD5 (\"" << input << "\") = " << expected << endl
              << "Result is a match: " << context.verify( expected ) << endl << endl;
}

int main (int argc, char *argv[])
{
    const char *version = "1.0";
    const char *description = "Unit test for md5, base64 encode/decode and uuencode/decode facilities";
    KCmdLineOptions options[] =
    {
        { "c <digest>", "compare <digest> with the calculated digest for a string or file.", 0 },
        { "d", "decode the given string or file using base64", 0 },
        { "e", "encode the given string or file using base64", 0 },
        { "f", "the filename to be used as input", "default" },
#if 0
        { "p", "encode the given string or file using quoted-printable", 0},
        { "q", "decode the given string or file using quoted-printable", 0},
#endif
        { "r", "calculate the raw md5 for the given string or file", 0 },
        { "s", "the string to be used as input", 0 },
        { "t", "perform a timed message-digest test", 0 },
        { "u", "uuencode the given string or file", 0 },
        { "x", "uudecode the given string or file", 0 },
        { "z", "run a preset message-digest test", 0 },
        { "+command", "[input1, input2,...]", 0 },
        { 0, 0, 0 }
    };

    KCmdLineArgs::init( argc, argv, "kmdcodectest", description, version );
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    int count = args->count();
    KApplication app;

    if (!count)
    {
        if ( args->isSet("t") )
            MD5_timeTrial ();
        else if ( args->isSet("z") )
            MD5_testSuite ();
        else
            args->usage();
    }
    else
    {
       bool isVerify = args->isSet("c");
       bool isString = args->isSet("s");
       bool isFile = args->isSet( "f" );
       Codec type = Unspecified;
       if ( args->isSet("d") )
          type = Base64Decode;
       else if ( args->isSet("e") )
          type = Base64Encode;
       else if ( args->isSet("u") )
          type = UUEncode;
       else if ( args->isSet("x") )
          type = UUDecode;
       if ( isVerify )
       {
          const char* opt = args->getOption( "c" ).data();
          for ( int i=0 ; i < count; i++ )
            MD5_verify ( QCString(args->arg(i)), opt, (isString || !isFile) );
       }
       else
       {
          for ( int i=0 ; i < count; i++ )
          {
            if ( type != Unspecified )
              testCodec( args->arg(i), type, isFile );
            else
            {
              if ( isString )
                MD5_string( args->arg( i ), 0, args->isSet("r") );
              else
                MD5_file( args->arg( i ), args->isSet("r") );
            }
          }
       }
    }
    args->clear();
    return (0);
}
