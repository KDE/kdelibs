#include "panelmenutest.h"
#include <kapp.h>
#include <kstddirs.h>
#include <dcopclient.h>

TestWidget::TestWidget(QWidget *parent, const char *name)
    : QLabel(parent, name)
{
    testMenu = new PanelMenu(locate("mini", "x.png"), "Client Test", this,
                              "menu1");

    connect(testMenu, SIGNAL(activated(int)), SLOT(slotMenuCalled(int)));

    init();
    
    setText("We added \"Client Test\" to kicker. Click the K Menu to check.");
    resize(sizeHint());
}

void TestWidget::init()
{
    testMenu->insertItem(locate("mini", "bx2.png"),
                                "Add another entry!", 1);
    testMenu->insertItem("Attention, this will clear the menu", 2);
}

void TestWidget::slotMenuCalled(int id)
{
    switch ( id ) {
    case 1:
	setText("Selected \"Add another entry!\" ");
	testMenu->insertItem("Another entry");
    break;
    case 2:
	setText("Selected \"Attention, this will clear the menu\"");
	testMenu->clear();
	init();
    break;
    default:
	setText("Selected \"Another entry...\"");
    }
}

int main(int argc, char **argv)
{
    KApplication *app = new KApplication(argc, argv, "menutest");
    
    TestWidget w;
    app->setMainWidget(&w);
    w.show();
    
    
    return(app->exec());
}

#include "panelmenutest.moc"



