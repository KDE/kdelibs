#include <QToolBar>
#include <QTextCodec>

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kmenubar.h>
#include <kdebug.h>

#include "kcodecactiontest.h"

#include <kcodecaction.h>
#include <kactioncollection.h>

int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, "kcodecactiontest", "KCodecActionTest", "kselectaction test app", "1.0" );
    KApplication app;

    CodecActionTest* test = new CodecActionTest;
    test->show();

    return app.exec();
}

CodecActionTest::CodecActionTest(QWidget *parent)
    : KMainWindow(parent)
    , m_comboCodec(new KCodecAction("Combo Codec Action", this))
    , m_buttonCodec(new KCodecAction("Button Codec Action", this))
{
    actionCollection()->addAction("combo", m_comboCodec);
    actionCollection()->addAction("button", m_buttonCodec);
    m_comboCodec->setToolBarMode(KCodecAction::ComboBoxMode);
    connect(m_comboCodec, SIGNAL(triggered(QAction*)), SLOT(triggered(QAction*)));
    connect(m_comboCodec, SIGNAL(triggered(int)), SLOT(triggered(int)));
    connect(m_comboCodec, SIGNAL(triggered(const QString&)), SLOT(triggered(const QString&)));
    connect(m_comboCodec, SIGNAL(triggered(QTextCodec *)), SLOT(triggered(QTextCodec *)));

    m_buttonCodec->setToolBarMode(KCodecAction::MenuMode);
    connect(m_buttonCodec, SIGNAL(triggered(QAction*)), SLOT(triggered(QAction*)));
    connect(m_buttonCodec, SIGNAL(triggered(int)), SLOT(triggered(int)));
    connect(m_buttonCodec, SIGNAL(triggered(const QString&)), SLOT(triggered(const QString&)));
    connect(m_buttonCodec, SIGNAL(triggered(QTextCodec *)), SLOT(triggered(QTextCodec *)));

    menuBar()->addAction(m_comboCodec);
    menuBar()->addAction(m_buttonCodec);

    QToolBar* toolBar = addToolBar("Test");
    toolBar->addAction(m_comboCodec);
    toolBar->addAction(m_buttonCodec);
}

void CodecActionTest::triggered(QAction* action)
{
  kDebug() << k_funcinfo << action << endl;
}

void CodecActionTest::triggered(int index)
{
  kDebug() << k_funcinfo << index << endl;
}

void CodecActionTest::triggered(const QString& text)
{
  kDebug() << k_funcinfo << '"' << text << '"' << endl;
}

void CodecActionTest::triggered(QTextCodec *codec)
{
  kDebug() << k_funcinfo << codec->name() << ':' << codec->mibEnum() << endl;
}

void CodecActionTest::slotActionTriggered(bool state)
{
    kDebug() << k_funcinfo << sender() << " state " << state << endl;
}

#include "kcodecactiontest.moc"

