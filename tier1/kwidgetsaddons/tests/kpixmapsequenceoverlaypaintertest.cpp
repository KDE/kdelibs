#include "kpixmapsequenceoverlaypaintertest.h"
#include "kpixmapsequenceoverlaypainter.h"
#include "kpixmapsequence.h"
#include "kpixmapsequence_config.h"

#include <QApplication>
#include <QGridLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QVariant>
#include <QEvent>

#include <QDebug>
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

    m_alignment->addItem(QLatin1Literal("Center"), QVariant::fromValue(Qt::Alignment(Qt::AlignCenter)));
    m_alignment->addItem(QLatin1Literal("Top-left"), QVariant::fromValue(Qt::Alignment(Qt::AlignTop|Qt::AlignLeft)));
    m_alignment->addItem(QLatin1Literal("Top"), QVariant::fromValue(Qt::Alignment(Qt::AlignTop|Qt::AlignHCenter)));
    m_alignment->addItem(QLatin1Literal("Top-right"), QVariant::fromValue(Qt::Alignment(Qt::AlignTop|Qt::AlignRight)));
    m_alignment->addItem(QLatin1Literal("Right"), QVariant::fromValue(Qt::Alignment(Qt::AlignRight|Qt::AlignVCenter)));
    m_alignment->addItem(QLatin1Literal("Bottom-right"), QVariant::fromValue(Qt::Alignment(Qt::AlignRight|Qt::AlignBottom)));
    m_alignment->addItem(QLatin1Literal("Bottom"), QVariant::fromValue(Qt::Alignment(Qt::AlignHCenter|Qt::AlignBottom)));
    m_alignment->addItem(QLatin1Literal("Bottom-left"), QVariant::fromValue(Qt::Alignment(Qt::AlignLeft|Qt::AlignBottom)));
    m_alignment->addItem(QLatin1Literal("Left"), QVariant::fromValue(Qt::Alignment(Qt::AlignLeft|Qt::AlignVCenter)));

    connect(m_alignment, SIGNAL(activated(int)), this, SLOT(alignementChanged(int)));
    connect(m_offsetX, SIGNAL(valueChanged(int)), this, SLOT(offsetChanged()));
    connect(m_offsetY, SIGNAL(valueChanged(int)), this, SLOT(offsetChanged()));

    m_painter = new KPixmapSequenceOverlayPainter(KPixmapSequence(QString::fromUtf8(ANIMATION_FILE), 22), this);
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
  QApplication::setApplicationName(QLatin1Literal("test"));
  TestWidget   *window;

  QApplication testapp(argc, argv);
  window = new TestWidget();
  window->show();
  return testapp.exec();
}

