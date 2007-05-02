
#include <kcharsets.h>
#include <QtCore/QString>

#include <assert.h>

int main()
{
    QString input( "&lt;Hello &amp;World&gt;" );
    QString output = KCharsets::resolveEntities( input );
    assert( output == "<Hello &World>" );
    return 0;
}
