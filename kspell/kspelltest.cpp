#include "kspell.h"
#include "kapp.h"

#include <qstring.h>
#include <qlabel.h>

int main(int argc, char **argv)
{
    KApplication *app = new KApplication(argc, argv, "KSpellTest");

    QString text( "I have noo idee of how to wride englisch or englisch" );
    
    KSpell::modalCheck( text );
    
    qDebug("Returned %s", text.latin1() );
    
    QLabel* l = new QLabel( text, (QWidget*)0 );
    l->show();
    
    return(app->exec());
}

