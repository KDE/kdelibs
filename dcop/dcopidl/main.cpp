/*****************************************************************
Copyright (c) 1999 Torben Weis <weis@kde.org>
Copyright (c) 2000 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/
#include <qstring.h>
#include <qfile.h>
#include <qcstring.h>
#include <stdlib.h>

void dcopidlParse( const char *_code );

int idl_line_no;

#if YYDEBUG
extern int yydebug;
#endif

int main( int argc, char** argv )
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage : dcopidl input_file\n");
        return -1;
    }
    QFile file( argv[1] );
    if ( !file.open( IO_ReadOnly ) )
    {
        fprintf(stderr, "Can't open input file\n");
        return -1;
    }

    QByteArray arr = file.readAll();
    uint len = arr.size();
    uint j = 1;
    for (uint i = 1; i<len; i++, j++) {
        if (arr[ i-1 ] == '\r' && ((i+1)==len || arr[ i ] != '\n')) {
            // change single \r's (Mac OS line endings) to \n
            arr[ j-1 ] = '\n';
            if ((i+1)==len) //special case: cut last character
                j--;
        }
        else if (arr[ i-1 ] == '\r' && arr[ i ] == '\n') {
            // change \r\n's (win32 line endings) to \n
            arr[ j-1 ] = '\n';
            i++; //skip \n
        }
        else if (i!=j) {
            arr[ j-1 ] = arr[ i-1 ];
        }
    }
    len = j;
    arr.resize( len + 1 );
    arr[ len ] = 0;

#if YYDEBUG
    char *debug = getenv("DEBUG");
    if (debug)
	yydebug = 1;
#endif
    idl_line_no = 1;

    printf("<!DOCTYPE DCOP-IDL><DCOP-IDL>\n");
    printf("<SOURCE>%s</SOURCE>\n", argv[1] );
    dcopidlParse( arr.data() );
    printf("</DCOP-IDL>\n");

    file.close();
    return 0;
}
