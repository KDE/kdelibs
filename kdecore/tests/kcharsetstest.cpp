
#include <kcharsets.h>

#include <assert.h>

int main()
{
    QString input( "&lt;Hello &amp;World&gt;" );
    QString ouput = KCharsets::resolveEntities( input );
    assert( ouput == "<Hello &World>" );
    return 0;
}
