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

#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kapp.h>

#include <kmdbase.h>

#define TEST_BLOCK_LEN 1000             // Length of test blocks.
#define TEST_BLOCK_COUNT 10000          // Number of test blocks.

void MD5_timeTrial ();
void MD5_testSuite ();
void MD5_string ( const char *, const char *expected = 0);
void MD5_file ( const char * );
void MD5_verify( const char*, const char*, bool );
void Base64Encode( const char*, bool );
void Base64Decode( const char*, bool );
void UUEncode( const char*, bool );
void UUDecode( const char*, bool );

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

int readContent( int fd, const char* err_msg, QString& buf, bool closefd = true )
{
    char* buffer = new char[1024];
    int result = read( fd, buffer, 1024 );    
    if ( result == 0 )
    {
        if ( closefd )
            if ( close(fd) == -1 )
                kdDebug() << "WARNING: unable to close the file properly!" << endl;
    }
    else if ( result == -1 )
        kdDebug() << err_msg << endl
                  << "Could not read the file!" << endl;
    else
        buf = buffer;
    return result;
}

void Base64Encode( const char* msg, bool isFile )
{
    if ( isFile )
    {
        QString data, encoded_data;
        const char* err_msg = "Encoding with \"base64\" failed!";	    
        int res, fd = openFile( msg, err_msg );
        if ( fd == -1 ) return;
        while (1) 
        {
            res = readContent( fd, err_msg, data );
            if ( res == -1 ) return;
            else if ( res == 0 ) break;
            encoded_data += KCodecs::base64Encode( data );

        }
        kdDebug() << "Base64 Encoded data: " << endl
                  << encoded_data << endl;
    }
    else
        kdDebug() << "Base64 Encoded data: " << endl
                  << KCodecs::base64Encode(msg) << endl;
}

void Base64Decode( const char* msg, bool isFile )
{
    if ( isFile )
    {
        QString data, decoded_data;
        const char* err_msg = "Decoding with \"base64\" failed!";
        int res, fd = openFile( msg, err_msg );
        if ( fd == -1 ) return;
        while (1) 
        {
            res = readContent( fd, err_msg, data );
            if ( res == -1 ) return;
            else if ( res == 0 ) break;
            decoded_data += KCodecs::base64Decode( data );
        }
        kdDebug() << "Decoded data (base64): " << endl
                  << decoded_data << endl;
    }
    else
        kdDebug() << "Decoded data (base64): " << endl
                  << KCodecs::base64Decode( msg ) << endl;
}

void UUEncode( const char* msg, bool isFile )
{
    if ( isFile )
    {
        QString data, encoded_data;
        const char* err_msg = "Encoding with \"uuencode\" failed!";
        int res, fd = openFile( msg, err_msg );
        if ( fd == -1 ) return;
        while (1) 
        {
            res = readContent( fd, err_msg, data );
            if ( res == -1 ) return;
            else if ( res == 0 ) break;
            encoded_data += KCodecs::uuencode( data );
        }
        kdDebug() << "UUEncoded data: " << endl
                  << encoded_data << endl;
    }
    else
        kdDebug() << "UUEncoded data: " << endl
                  << KCodecs::uuencode(msg) << endl;
}

void UUDecode( const char* msg, bool isFile )
{
    if ( isFile )
    {
        QString data, encoded_data;
        const char* err_msg = "Encoding with \"uudecode\" failed!";
        int res, fd = openFile( msg, err_msg );
        if ( fd == -1 ) return;
        while (1) 
        {
            res = readContent( fd, err_msg, data );
            if ( res == -1 ) return;
            else if ( res == 0 ) break;
            encoded_data += KCodecs::uudecode( data );
        }
        kdDebug() << "Decoded data (uudecode): " << endl
                  << encoded_data << endl;
    }
    else
        kdDebug() << "Decoded data (uudecode): " << endl
                  << KCodecs::uudecode( msg ) << endl;
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

void MD5_file (const char *filename)
{
    FILE* f = fopen( filename, "r" );

    if (!f)
    {
        kdError() << "(" << filename << ") cannot be opened!" << endl;
    }
    else
    {
        KMD5 context( f );
        kdDebug() <<  "MD5 ("  << filename <<  ") = "  <<  context.hexDigest() << endl;
    }
}

void MD5_string (const char *input, const char* expected )
{
  KMD5 context;
  Q_UINT32 len = strlen (input);

  context.update ( reinterpret_cast<Q_UINT8*>(const_cast<char*>(input)), len );
  context.finalize ();
  kdDebug() << endl << "Result: MD5 (\"" << input << "\") = " << context.hexDigest() << endl;
  if ( expected )
    kdDebug() << "Expected: MD5 (\"" << input << "\") = " << expected << endl
              << "Result is a match: " << context.verify( expected ) << endl << endl;

}

int main (int argc, char *argv[])
{
    const char *version = "v0.0.5";
    const char *description = "Unit test for md5, base64encode/decode & uuencode/decode facilities";
    KCmdLineOptions options[] =
    {
        { "c <digest>", "compare <digest> with the calculated digest for a string or file.", 0 },
        { "d", "decode the given string or file using base64", 0 },
        { "e", "encode the given string or file using base64", 0 },
        { "f", "the filename to be used as input", "default" },
        { "s", "the string to be used as input", 0 },
        { "t", "perform a timed message-digest test", 0 },
        { "u", "uuencode the given string or file", 0 },
        { "x", "uudecode the given string or file", 0 },
        { "z", "run a preset message-digest test", 0 },
        { "+command", "[input1, input2,...]", 0 },
        { 0, 0, 0 }
    };

    KCmdLineArgs::init( argc, argv, "kmdncodectest", description, version, true );
    KCmdLineArgs::addCmdLineOptions(options);
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
	    bool isBase64Encode = args->isSet("e");
	    bool isBase64Decode = args->isSet("d");
		bool isUUEncode = args->isSet("u");
	    bool isUUDecode = args->isSet("x");        
		if ( isVerify )
        {
            if ( (args->isSet("f") && isString) ||
			     isUUEncode || isUUDecode ||
				 isBase64Encode || isBase64Decode )
                args->usage();

            const char* opt = args->getOption( "c" ).data();
            for ( int i=0 ; i < count; i++ )
                MD5_verify ( QCString(args->arg(i)), opt, !isString );
        }    
        else if ( isString )
        {
            if ( args->isSet("f") ||
                 (isUUEncode && isBase64Encode) ||
                 (isUUDecode && isBase64Decode) ||
                 (isUUEncode && isUUDecode) ||
                 (isBase64Decode && isBase64Encode) )
                args->usage();
            
            for ( int i=0 ; i < count; i++ )
            {
                if ( isBase64Decode )
                    Base64Decode( args->arg( i ), false );
                else if ( isBase64Encode )
                    Base64Encode( args->arg( i ), false );
                else if ( isUUEncode )
                    UUEncode( args->arg( i ), false );
                else if ( isUUDecode )
                    UUDecode( args->arg( i ), false );
                else
                    MD5_string( args->arg( i ) );
             }
        }
        else
        {
            if ( (isUUEncode && isBase64Encode) ||
                 (isUUDecode && isBase64Decode) ||
                 (isUUEncode && isUUDecode)     ||
                 (isBase64Decode && isBase64Encode) )
                args->usage();

            for ( int i=0 ; i < count; i++ )
            {
                if ( isBase64Decode )
                    Base64Decode( args->arg( i ), true );
                else if ( isBase64Encode )
                    Base64Encode( args->arg( i ), true );
                else if ( isUUEncode )
                    UUEncode( args->arg( i ), true );
                else if ( isUUDecode )
                    UUDecode( args->arg( i ), true );
                else
                    MD5_file( args->arg( i ) );
             }
        }
    }
    return (0);
}
