#include <qstring.h>
#include <qfile.h>
#include <qcstring.h>

void kidlParse( const char *_code );

int idl_line_no;

int main( int argc, char** argv )
{
    QFile file( argv[1] );
    if ( !file.open( IO_ReadOnly ) )
	ASSERT( 0 );
    
    QByteArray arr = file.readAll();
    int len = arr.size();
    arr.resize( len + 1 );
    arr[ len ] = 0;
    
    kidlParse( arr.data() );
		     
    file.close();
}
