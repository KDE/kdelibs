
#include "tst_kliveui.h"

void tst_GuiEditor::init()
{
    mw = new QMainWindow;
    testAction1 = new QAction(mw);
    testAction1->setText("Test1");
    testAction2 = new QAction(mw);
    testAction1->setText("Test2");
    testAction3 = new QAction(mw);
    testAction1->setText("Test3");
}

void tst_GuiEditor::cleanup()
{
    delete mw;
    testAction1 = 0;
    testAction2 = 0;
    testAction3 = 0;
}

void tst_GuiEditor::actionGroups()
{
    KLiveUiBuilder editor(mw);

    editor.beginMenuBar();
    QMenu *menu = editor.beginMenu("ui.file", "&File");

    editor.addAction(testAction1);
    editor.addActionGroup("group");
    editor.addAction(testAction3);

    editor.beginActionGroup("group");
    editor.addAction(testAction2);
    editor.endActionGroup();

    editor.endMenu();

    QList<QAction *> actions = menu->actions();
    QVERIFY(actions.indexOf(testAction1) < actions.indexOf(testAction2));
    QVERIFY(actions.indexOf(testAction2) < actions.indexOf(testAction3));
}

/*
void tst_GuiEditor::recursiveActionGroups()
{
    GuiEditor editor(mw);

    editor.beginMenuBar();

    QMenu *fileMenu = editor.beginMenu("ui.file", "&File");
    editor.addActionGroup("group");
    editor.endMenu();

    QMenu *editMenu = editor.beginMenu("ui.edit", "&Edit");
    editor.addActionGroup("group");
    editor.endMenu();

    editor.end();
    editor.begin(mw);

    editor.beginActionGroup("group");
    editor.addAction(testAction1);
    editor.endActionGroup();

    QList<QAction *> actions = menu->actions();
    QVERIFY(actions.indexOf(testAction1) < actions.indexOf(testAction2));
    QVERIFY(actions.indexOf(testAction2) < actions.indexOf(testAction3));
}
*/

void tst_GuiEditor::menuDeletion()
{
    QPointer<QMenu> menu;

    {
        KLiveUiBuilder editor(mw);

        editor.beginMenuBar();
        menu = editor.beginMenu("ui.file", "&File");

        editor.endMenu();
    }

    QVERIFY(!menu.isNull());

    delete mw;
    mw = 0;

    QVERIFY(menu.isNull());
}

void tst_GuiEditor::changeExistingMenu()
{
    KLiveUiBuilder editor(mw);
    editor.beginMenuBar();
    QMenu *menu = editor.beginMenu("testmenu", "Test");
    editor.addAction("First");
    editor.end();
    
    QMenuBar *menuBar = mw->menuBar();
    QCOMPARE(menuBar->actions().count(), 1);
    QVERIFY(menuBar->actions().first()->menu() == menu);
    
    editor.begin(mw);
    editor.beginMenuBar();
    QMenu *menu2 = editor.beginMenu("testmenu", "Test");
    QVERIFY(menu == menu2);
    editor.end();
}

QTEST_MAIN(tst_GuiEditor)
#include "tst_kliveui.moc"
