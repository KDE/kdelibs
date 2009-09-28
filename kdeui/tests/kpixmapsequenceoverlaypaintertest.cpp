#include "kpixmapsequenceoverlaypaintertest.h"
#include "kpixmapsequenceoverlaypainter.h"

#include <QGridLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QVariant>
#include <QEvent>

#include <klocale.h>
#include <kapplication.h>
#include <kcmdlineargs.h>


Q_DECLARE_METATYPE(Qt::Alignment)

TestWidget::TestWidget()
    : QWidget()
{
    m_draggingLeft = false;
    m_draggingRight = false;
    m_draggingTop = false;
    m_draggingBottom = false;

    QGridLayout* layout = new QGridLayout(this);
    m_widget = new QWidget(this);
    m_alignment = new QComboBox(this);
    m_offsetX = new QSpinBox(this);
    m_offsetY = new QSpinBox(this);

    layout->addWidget(m_widget, 0, 0, 1, 2);
    layout->addWidget(m_alignment, 1, 0, 1, 2);
    layout->addWidget(m_offsetX, 2, 0, 1, 1);
    layout->addWidget(m_offsetY, 2, 1, 1, 1);

    m_alignment->addItem("Center", QVariant::fromValue(Qt::Alignment(Qt::AlignCenter)));
    m_alignment->addItem("Top-left", QVariant::fromValue(Qt::Alignment(Qt::AlignTop|Qt::AlignLeft)));
    m_alignment->addItem("Top", QVariant::fromValue(Qt::Alignment(Qt::AlignTop|Qt::AlignHCenter)));
    m_alignment->addItem("Top-right", QVariant::fromValue(Qt::Alignment(Qt::AlignTop|Qt::AlignRight)));
    m_alignment->addItem("Right", QVariant::fromValue(Qt::Alignment(Qt::AlignRight|Qt::AlignVCenter)));
    m_alignment->addItem("Bottom-right", QVariant::fromValue(Qt::Alignment(Qt::AlignRight|Qt::AlignBottom)));
    m_alignment->addItem("Bottom", QVariant::fromValue(Qt::Alignment(Qt::AlignHCenter|Qt::AlignBottom)));
    m_alignment->addItem("Bottom-left", QVariant::fromValue(Qt::Alignment(Qt::AlignLeft|Qt::AlignBottom)));
    m_alignment->addItem("Left", QVariant::fromValue(Qt::Alignment(Qt::AlignLeft|Qt::AlignVCenter)));

    connect(m_alignment, SIGNAL(activated(int)), this, SLOT(alignementChanged(int)));
    connect(m_offsetX, SIGNAL(valueChanged(int)), this, SLOT(offsetChanged()));
    connect(m_offsetY, SIGNAL(valueChanged(int)), this, SLOT(offsetChanged()));

    m_painter = new KPixmapSequenceOverlayPainter(this);
    m_painter->setWidget(m_widget);
    m_painter->start();
}


TestWidget::~TestWidget()
{
}


void TestWidget::alignementChanged(int i)
{
    m_painter->setAlignment(m_alignment->itemData(i).value<Qt::Alignment>());
}


void TestWidget::offsetChanged()
{
    m_painter->setOffset(QPoint(m_offsetX->value(), m_offsetY->value()));
}

bool TestWidget::eventFilter(QObject* o, QEvent* e)
{
    if(o == m_widget) {
        if(e->type() == QEvent::Paint) {

        }
        else if(e->type() == QEvent::MouseButtonPress) {

        }
        else if(e->type() == QEvent::MouseButtonRelease) {

        }
        else if(e->type() == QEvent::MouseMove) {

        }
    }
    return QWidget::eventFilter(o, e);
}



/* --- MAIN -----------------------*/
int main(int argc, char **argv)
{
  KCmdLineArgs::init( argc, argv, "test", 0, ki18n("Test"), "1.0", ki18n("test app"));
  TestWidget   *window;

  KApplication testapp;
  window = new TestWidget();
  window->show();
  return testapp.exec();
}

#include "kpixmapsequenceoverlaypaintertest.moc"
