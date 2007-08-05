/* This file is part of the KDE libraries
    Copyright (C) 2000,2001 Dawit Alemayehu <adawit@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <config.h>
#include <unistd.h>
#include <time.h>

#include <iostream>

#include <QtCore/QBuffer>
#include <QtCore/QFile>

#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
//#include <kapplication.h>

#include <kcodecs.h>

using namespace std;

#define TEST_BLOCK_LEN 1000             // Length of test blocks.
#define TEST_BLOCK_COUNT 10000          // Number of test blocks.
#define MAX_READ_BUF_SIZE 8192

enum Codec
{
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
void testCodec (const char*, Codec, bool);
void MD5_verify (const char*, const char*, bool);
void MD5_file (const char * , bool rawOutput = false);
void MD5_string (const char *, const char *expected = 0, bool rawOutput = false);

long readContent (const QFile& f, long count, QByteArray& buf)
{
    long result;
    int old_size;

    old_size = buf.size();
    buf.resize(old_size+count);

    result = read (f.handle (), buf.data()+old_size, count);

    if ( result > 0 && result < count )
    {
      buf.resize( old_size + result );
    }
    else if ( result == 0 )
    {
      buf.resize( old_size );
    }
    else if ( result == -1 )
    {
      kError() << "Could not read the file!" << endl;
    }

    return result;
}

void testCodec (const char* msg, Codec type, bool isFile)
{
    QByteArray output;

    if ( isFile )
    {
        int count;
        QByteArray data;

        QFile f (QFile::encodeName(msg));

        if (!f.exists())
        {
          kError() << "Could not find: " << qPrintable(f.fileName()) << endl;
          return;
        }

        if (!f.open(QIODevice::ReadOnly))
        {
          f.close ();
          kError() << "Could not open: " << qPrintable(f.fileName()) << endl;
          return;
        }

        // Read contents of file...
        count = 0;

        while ((count= readContent(f, MAX_READ_BUF_SIZE, data)) > 0) {
            ;
        }

        // Error! Exit!
        if ( count == -1 )
        {
          kError () << "Error reading from: " << qPrintable(f.fileName()) << endl;
          f.close ();
          return;
        }

        f.close ();

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
            case QPEncode:
                KCodecs::quotedPrintableEncode(data, output, true);
                break;
            case QPDecode:
                KCodecs::quotedPrintableDecode(data, output);
                break;
            default:
                break;
        }

        cout << "Result: " << endl << output.data() << endl;
    }
    else
    {
        QByteArray result;

        memcpy (output.data(), msg, strlen(msg));

        switch (type)
        {
          case Base64Encode:
            result = KCodecs::base64Encode(output);
            break;
          case Base64Decode:
            result = KCodecs::base64Decode(output);
            break;
          case UUEncode:
            result = KCodecs::uuencode(output);
            break;
          case UUDecode:
            result = KCodecs::uudecode(output);
            break;
          case QPEncode:
            result = KCodecs::quotedPrintableEncode(output);
            break;
          case QPDecode:
            result = KCodecs::quotedPrintableDecode(output);
            break;
          default:
            break;
        }
        cout << result.data() << endl;
    }
}

void MD5_timeTrial ()
{
    KMD5 context;

    time_t endTime;
    time_t startTime;

    quint8 block[TEST_BLOCK_LEN];
    quint32 i;

    cout << "Timing test. Digesting " << TEST_BLOCK_COUNT << " blocks of "
         << TEST_BLOCK_LEN << "-byte..." << endl;

    // Initialize block
    for (i = 0; i < TEST_BLOCK_LEN; i++)
        block[i] = (quint8)(i & 0xff);

    // Start timer
    time (&startTime);

    // Digest blocks
    for (i = 0; i < TEST_BLOCK_COUNT; i++)
        context.update (block, TEST_BLOCK_LEN);

    // Stop timer
    time (&endTime);

    long duration = endTime - startTime;
    long speed;
    if (duration)
      speed = (TEST_BLOCK_LEN * (TEST_BLOCK_COUNT/duration));
    else
      speed = TEST_BLOCK_COUNT;

    cout << "Result: " << endl;
    cout << "  Time   = " << duration << " seconds" << endl;
    cout << "  Speed  = " << speed << " bytes/second" << endl;
    cout << "  Digest = " << context.hexDigest().data() << endl;
}

void MD5_testSuite ()
{
  cout << "MD5 preset test suite as defined in RFC 1321:" << endl;
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
  bool result;
  KMD5 context;

  if ( !isFile )
  {
    context.update (QByteArray(input));
    result = context.verify( digest );
    cout << "Input string: " << input << endl;
  }
  else
  {
    QFile f (input);

    if (!f.open (QIODevice::ReadOnly))
    {
      f.close ();
      kFatal() << "Cannot open file for reading!";
    }

    result = context.verify (digest);
    f.close ();

    cout << "Input filename: " << input << endl;
  }

  cout << "Calculated Digest = " <<  context.hexDigest().data() << endl;
  cout << "Supplied Digest   = " << digest << endl;
  cout << "Matches: " << (result ? "TRUE":"FALSE") << endl;
}

void MD5_file (const char *filename, bool rawOutput )
{
  QFile f (QFile::encodeName(filename));

  if (!f.open(QIODevice::ReadOnly))
  {
    f.close();
    kError() << "(" << filename << ") cannot be opened!" << endl;
    return;
  }

  KMD5 context;
  context.update( f );

  if ( rawOutput )
    cout << "MD5 (" << filename << ") = " << context.rawDigest() << endl;
  else
    cout << "MD5 (" << filename << ") = " << context.hexDigest().data() << endl;

  f.close ();
}

void MD5_string (const char *input, const char* expected, bool rawOutput )
{
  KMD5 context;
  context.update (QByteArray(input));

  cout << "Checking MD5 for: " << input << endl;

  if ( rawOutput )
    cout << "Result: " << context.rawDigest() << endl;
  else
    cout << "Result: " << context.hexDigest().data() << endl;

  if ( expected )
  {
    cout << "Expected: " << expected << endl;
    cout << "Status: " << context.verify (expected) << endl;
  }
}

int main (int argc, char *argv[])
{
    KCmdLineOptions options;
    options.add("c <digest>", ki18n("compare <digest> with the calculated digest for a string or file."));
    options.add("d", ki18n("decode the given string or file using base64"));
    options.add("e", ki18n("encode the given string or file using base64"));
    options.add("f", ki18n("the filename to be used as input"), "default");
    options.add("p", ki18n("encode the given string or file using quoted-printable"));
    options.add("q", ki18n("decode the given string or file using quoted-printable"));
    options.add("r", ki18n("calculate the raw md5 for the given string or file"));
    options.add("s", ki18n("the string to be used as input"));
    options.add("t", ki18n("perform a timed message-digest test"));
    options.add("u", ki18n("uuencode the given string or file"));
    options.add("x", ki18n("uudecode the given string or file"));
    options.add("z", ki18n("run a preset message-digest test"));
    options.add("+command", ki18n("[input1, input2,...]"));

    KCmdLineArgs::init( argc, argv, "kmdcodectest", 0,
                        ki18n("KMDCodecTest"), "1.0",
                        ki18n("Unit test for md5, base64 encode/decode "
                                   "and uuencode/decode facilities" ) );
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    int count = args->count();

    //KApplication app;

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
       else if ( args->isSet("p") )
          type = QPEncode;
       else if ( args->isSet("q") )
          type = QPDecode;
       if ( isVerify )
       {
          const char* opt = args->getOption( "c" ).toLocal8Bit().data();
          for ( int i=0 ; i < count; i++ )
            MD5_verify ( args->arg(i).toLocal8Bit(), opt, (isString || !isFile) );
       }
       else
       {
          for ( int i=0 ; i < count; i++ )
          {
            if ( type != Unspecified )
              testCodec( args->arg(i).toLocal8Bit(), type, isFile );
            else
            {
              if ( isString )
                MD5_string( args->arg(i).toLocal8Bit(), 0, args->isSet("r") );
              else
                MD5_file( args->arg(i).toLocal8Bit(), args->isSet("r") );
            }
          }
       }
    }
    args->clear();
    return (0);
}
