/*
  kmd5test.cc - a test suite for KDE's implementation of MD2, MD4 and MD5
  Copyright (C) 2000 Dawit Alemayehu <adawit@kde.org>

  This work is completely based on the work done by
  Copyright (C) 1995 by Mordechai T. Abzug and
  Copyright (C) 1990-1992, RSA Data Security, Inc. Created 1990. All rights reserved.
  See the "License" file for details.
*/

#include <time.h>
#include <stdio.h>
#include <string.h>

#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kapp.h>

#include <kmdbase.h>

#define TEST_BLOCK_LEN 1000             // Length of test blocks.
#define TEST_BLOCK_COUNT 10000          // Number of test blocks.

void MD5_timeTrial ();
void MD5_testSuite ();
void MD5_string (const char *input);
void MD5_file (const char *filename);
void MD5_verify(const char* input, const char* digest, bool isFile);


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
  kdDebug() << "MD5 test suite:" << endl;

  MD5_string ( "" );
  MD5_string ( "a" );
  MD5_string ( "abc" );
  MD5_string ( "message digest" );
  MD5_string ( "abcdefghijklmnopqrstuvwxyz" );
  MD5_string ( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" );
  MD5_string ( "12345678901234567890123456789012345678901234567890123456789012345678901234567890" );
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

void MD5_string (const char *input)
{

  KMD5 context;
  Q_UINT32 len = strlen ( input );

  context.update ( (Q_UINT8*) input, len );
  context.finalize ();

  kdDebug() << "MD5 (\"" << input << "\") = " << context.hexDigest() << endl;
}

int main (int argc, char *argv[])
{
    const char *version = "v0.0.5";
    const char *description = I18N_NOOP("Unit test for KDE's MD5 implementation...");

    KCmdLineOptions options[] =
    {
	{ "c <digest>", I18N_NOOP("compare this digest with digest of a string or file."), 0 },
        { "f", I18N_NOOP("calculate digest for the file(s)"), "default" },
        { "s", I18N_NOOP("calculate digest for the string(s)"), 0 },
        { "t", I18N_NOOP("perform a timed test"), 0 },
        { "x", I18N_NOOP("run a preset test-suite"), 0 },
        { "+command", I18N_NOOP("[input1, input2,...]"), 0 },
        { 0, 0, 0 }
    };

    KCmdLineArgs::init( argc, argv, "kmdbasetest", description, version, true );
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    int count = args->count();
    KApplication app;
   
    if (!count)
    {
        if ( args->isSet( "t" ) )
            MD5_timeTrial ();
        else if ( args->isSet( "x" ) )
            MD5_testSuite ();
        else
            args->usage();
    }
    else
    {
        if ( args->isSet("c") )
        {
            if ( args->isSet("f") && args->isSet("s") )
                args->usage();

            const char* opt = args->getOption( "c" ).data();
            for ( int i=0 ; i < count; i++ )
                MD5_verify ( QCString(args->arg(i)), opt, !args->isSet("s") );
        }
        else if ( args->isSet( "s" ) )
        {
            if ( args->isSet( "f" ) )
                args->usage();

            for ( int i=0 ; i < count; i++ )
                MD5_string( args->arg( i ) );
        }
        else
        {
            for ( int i=0 ; i < count; i++ )
                MD5_file( args->arg( i ) );
        }
    }
    return (0);
}
