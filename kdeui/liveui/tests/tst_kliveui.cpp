
#include "tst_kliveui.h"
#include <kmenu.h>
#include <kmenubar.h>

class SampleComponent : public KLiveUiComponent
{
public:
    inline SampleComponent(QObject *parent) : KLiveUiComponent(parent) {}

    virtual void createComponentGui();
};

void SampleComponent::createComponentGui()
{
    KLiveUiBuilder builder(this);

    builder.beginMenuBar();
    builder.beginMenu("Components", "components");
    builder.beginMenu("SampleComponent", "samplecomponent");
    builder.addAction("Test Action");
}

void tst_GuiEditor::init()
{
    mw = new KMainWindow;
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
    KMenu *menu = editor.beginMenu("&File", "ui.file");

    editor.addAction(testAction1);
    editor.addMergePlaceholder("group");
    editor.addAction(testAction3);

    editor.beginMerge("group");
    editor.addAction(testAction2);
    editor.endMerge();

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
    QPointer<KMenu> menu;

    {
        KLiveUiBuilder editor(mw);

        editor.beginMenuBar();
        menu = editor.beginMenu("&File", "ui.file");

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
    QMenu *menu = editor.beginMenu("Test", "testmenu");
    editor.addAction("First");
    editor.end();
    
    QMenuBar *menuBar = mw->menuBar();
    QCOMPARE(menuBar->actions().count(), 1);
    QVERIFY(menuBar->actions().first()->menu() == menu);
    
    editor.begin(mw);
    editor.beginMenuBar();
    QMenu *menu2 = editor.beginMenu("Test", "testmenu");
    QVERIFY(menu == menu2);
    editor.end();
}

void tst_GuiEditor::implicitBeginMenuBar()
{
    KLiveUiBuilder builder(mw);
    QMenu *menu = builder.beginMenu(KLiveUi::FileMenu);
    QCOMPARE(mw->menuBar()->actions().count(), 1);
    QVERIFY(mw->menuBar()->actions().first()->menu() == menu);
}

void tst_GuiEditor::subComponents()
{
    KLiveUiComponent *parent = new KLiveUiComponent(mw);
    KLiveUiComponent *child = new KLiveUiComponent;

    parent->addSubComponent(child);
    QCOMPARE(parent->subComponents().count(), 1);
    QCOMPARE(parent->subComponents().first(), child);

    KLiveUiBuilder builder(child);
    builder.beginMenuBar();
    builder.beginMenu("Test", "testmenu");
    builder.addAction("Test");
    builder.end();

    parent->createGui();
    QCOMPARE(mw->menuBar()->actions().count(), 1);
    QAction *menuAction = mw->menuBar()->actions().first();
    QVERIFY(menuAction);
    QCOMPARE(menuAction->text(), QString("Test"));
    QMenu *m = menuAction->menu();
    QVERIFY(m);
    QCOMPARE(m->actions().count(), 1);
    QCOMPARE(m->actions().first()->text(), QString("Test"));

    QVERIFY(!child->activeActions().isEmpty());

    parent->removeGui();
    QVERIFY(mw->menuBar()->actions().isEmpty());
    QVERIFY(child->activeActions().isEmpty());

    parent->removeSubComponent(child);
    QVERIFY(parent->subComponents().isEmpty());

    parent->addSubComponent(child);
    QCOMPARE(parent->subComponents().count(), 1);
    QCOMPARE(parent->subComponents().first(), child);
    delete child;
    QCOMPARE(parent->subComponents().count(), 0);
}

void tst_GuiEditor::deleteActions()
{
    SampleComponent plugin(mw);

    QVERIFY(plugin.activeActions().isEmpty());
    plugin.createGui();
    QVERIFY(!plugin.activeActions().isEmpty());

    foreach (QAction *a, plugin.activeActions())
        delete a;

    QVERIFY(plugin.activeActions().isEmpty());
}

QTEST_KDEMAIN(tst_GuiEditor, GUI)
#include "tst_kliveui.moc"
