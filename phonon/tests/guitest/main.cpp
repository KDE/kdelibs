/*  This file is part of the KDE project
    Copyright (C) 2004-2007 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include <QtGui/QAction>
#include <QtGui/QGraphicsView>
#include <QtGui/QMainWindow>
#include "mediaobjectitem.h"
#include "mygraphicsscene.h"
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kicon.h>
#include "audiooutputitem.h"
#include "videowidgetitem.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
    public:
        MainWindow();

    private slots:
        void addMediaObject();
        void addAudioOutput();
        void addVideoWidget();

    private:
        QGraphicsView *m_view;
        MyGraphicsScene *m_scene;
};

/*
PathWidget::PathWidget(QWidget *parent)
    : QFrame(parent)
{
    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Raised);

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_effectComboBox = new QComboBox(this);
    layout->addWidget(m_effectComboBox);
    QList<EffectDescription> effectList = BackendCapabilities::availableAudioEffects();
    m_effectComboBox->setModel(new AudioEffectDescriptionModel(effectList, m_effectComboBox));

    QPushButton *addButton = new QPushButton(this);
    layout->addWidget(addButton);
    addButton->setText("add effect");
    connect(addButton, SIGNAL(clicked()), SLOT(addEffect()));

    QPushButton *button = new QPushButton(this);
    layout->addWidget(button);
    button->setText("add VolumeFader");
    connect(button, SIGNAL(clicked()), SLOT(addVolumeFader()));
}

void PathWidget::addEffect()
{
    int current = m_effectComboBox->currentIndex();
    if (current < 0) {
        return;
    }
    QList<EffectDescription> effectList = BackendCapabilities::availableAudioEffects();
    if (current < effectList.size()) {
        Effect *effect = m_path.insertEffect(effectList[current]);
        QGroupBox *gb = new QGroupBox(effectList[current].name(), this);
        layout()->addWidget(gb);
        gb->setFlat(true);
        gb->setCheckable(true);
        gb->setChecked(true);
        (new QHBoxLayout(gb))->addWidget(new EffectWidget(effect, gb));
        gb->setProperty("AudioEffect", QVariant::fromValue(static_cast<QObject *>(effect)));
        connect(gb, SIGNAL(toggled(bool)), SLOT(effectToggled(bool)));
    }
}

void PathWidget::effectToggled(bool checked)
{
    if (checked) {
        return;
    }
    QVariant v = sender()->property("AudioEffect");
    if (!v.isValid()) {
        return;
    }
    QObject *effect = v.value<QObject *>();
    if (!effect) {
        return;
    }
    delete effect;
    sender()->deleteLater();
}

void PathWidget::addVolumeFader()
{
    VolumeFaderEffect *effect = new VolumeFaderEffect(this);
    QGroupBox *gb = new QGroupBox("VolumeFader", this);
    layout()->addWidget(gb);
    gb->setFlat(true);
    gb->setCheckable(true);
    gb->setChecked(true);
    (new QHBoxLayout(gb))->addWidget(new EffectWidget(effect, gb));
    m_path.insertEffect(effect);
    gb->setProperty("AudioEffect", QVariant::fromValue(static_cast<QObject *>(effect)));
    connect(gb, SIGNAL(toggled(bool)), SLOT(effectToggled(bool)));
}

bool PathWidget::connectOutput(OutputWidget *w)
{
    if (m_sink && m_path.isValid()) {
        m_path.disconnect();
    }
    m_sink = w->output();
    if (m_source) {
        m_path = createPath(m_source, m_sink);
        return m_path.isValid();
    }
    return true;
}

bool PathWidget::connectInput(MediaObject *m)
{
    m_source = m;
}

bool ProducerWidget::connectPath(PathWidget *w)
{
    if (m_audioPath)
    m_audioPath = w->path;
    return w->connectInput(m_media);
    if (m_media)
    {
        if (m_media->addAudioPath(w->path())) {
            m_audioPaths.append(w->path());
            return true;
        }
    }
    if (m_audioPaths.contains(w->path()))
        return false;
    m_audioPaths.append(w->path());
    return true;
}

ConnectionWidget::ConnectionWidget(QWidget *parent)
    : QFrame(parent)
{
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Sunken);
    setMinimumWidth(120);
}

void ConnectionWidget::mousePressEvent(QMouseEvent *me)
{
    m_pressPos = me->pos();
}

void ConnectionWidget::mouseReleaseEvent(QMouseEvent *me)
{
    emit madeConnection(m_pressPos, me->pos());
    m_pressPos = QPoint();
    m_currentLine = QLine();
    update();
}

void ConnectionWidget::mouseMoveEvent(QMouseEvent *me)
{
    if (m_pressPos.isNull())
        return;

    m_currentLine = QLine(m_pressPos, me->pos());
    update();
}

void ConnectionWidget::addConnection(QWidget *a, QWidget *b)
{
    m_lines.append(WidgetConnection(a, b));
    update();
}

void ConnectionWidget::paintEvent(QPaintEvent *pe)
{
    QFrame::paintEvent(pe);
    QPainter p(this);
    foreach (WidgetConnection x, m_lines)
    {
        QLine l(0, x.a->geometry().center().y(),
                width(), x.b->geometry().center().y());
        p.drawLine(l);
    }
    p.setPen(Qt::darkRed);
    p.drawLine(m_currentLine);
}
*/

MainWindow::MainWindow()
{
    setWindowIcon(KIcon("phonon"));
    m_scene = new MyGraphicsScene(this);
    m_view = new QGraphicsView(m_scene);
    setCentralWidget(m_view);
    QAction *action;
    action = new QAction(i18n("add MediaObject"), m_view);
    connect(action, SIGNAL(triggered()), SLOT(addMediaObject()));
    m_view->addAction(action);
    action = new QAction(i18n("add AudioOutput"), m_view);
    connect(action, SIGNAL(triggered()), SLOT(addAudioOutput()));
    m_view->addAction(action);
    action = new QAction(i18n("add VideoWidget"), m_view);
    connect(action, SIGNAL(triggered()), SLOT(addVideoWidget()));
    m_view->addAction(action);
    m_view->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void MainWindow::addMediaObject()
{
    kDebug();
    m_scene->addItem(new MediaObjectItem(QCursor::pos(), m_view));
}

void MainWindow::addAudioOutput()
{
    kDebug();
    m_scene->addItem(new AudioOutputItem(QCursor::pos(), m_view));
}

void MainWindow::addVideoWidget()
{
    kDebug();
    m_scene->addItem(new VideoWidgetItem(QCursor::pos(), m_view));
}


/*
    QHBoxLayout *topLayout = new QHBoxLayout(this);

    m_producerFrame = new QFrame(this);
    topLayout->addWidget(m_producerFrame);
    new QVBoxLayout(m_producerFrame);
    m_producerFrame->layout()->setMargin(0);

    m_connectionFrame1 = new ConnectionWidget(this);
    topLayout->addWidget(m_connectionFrame1);
    connect(m_connectionFrame1, SIGNAL(madeConnection(const QPoint &, const QPoint &)),
            SLOT(madeConnection1(const QPoint &, const QPoint &)));

    m_pathFrame = new QFrame(this);
    topLayout->addWidget(m_pathFrame);
    new QVBoxLayout(m_pathFrame);
    m_pathFrame->layout()->setMargin(0);

    m_connectionFrame2 = new ConnectionWidget(this);
    topLayout->addWidget(m_connectionFrame2);
    connect(m_connectionFrame2, SIGNAL(madeConnection(const QPoint &, const QPoint &)),
            SLOT(madeConnection2(const QPoint &, const QPoint &)));

    m_outputFrame = new QFrame(this);
    topLayout->addWidget(m_outputFrame);
    new QVBoxLayout(m_outputFrame);
    m_outputFrame->layout()->setMargin(0);

    QPushButton *addProducerButton = new QPushButton("add Producer", m_producerFrame);
    m_producerFrame->layout()->addWidget(addProducerButton);
    connect(addProducerButton, SIGNAL(clicked()), SLOT(addProducer()));

    QPushButton *addPathButton = new QPushButton("add Path", m_pathFrame);
    m_pathFrame->layout()->addWidget(addPathButton);
    connect(addPathButton, SIGNAL(clicked()), SLOT(addPath()));

    QPushButton *addOutputButton = new QPushButton("add Output", m_outputFrame);
    m_outputFrame->layout()->addWidget(addOutputButton);
    connect(addOutputButton, SIGNAL(clicked()), SLOT(addOutput()));

    addProducer();
    addPath();
    addOutput();
    if (m_producers.first()->connectPath(m_paths.first()))
        m_connectionFrame1->addConnection(m_producers.first(), m_paths.first());
    if (m_paths.first()->connectOutput(m_outputs.first()))
        m_connectionFrame2->addConnection(m_paths.first(), m_outputs.first());
}

void MainWindow::addProducer()
{
    ProducerWidget *x = new ProducerWidget(m_producerFrame);
    m_producerFrame->layout()->addWidget(x);
    m_producers << x;
    m_connectionFrame1->update();
}

void MainWindow::addPath()
{
    PathWidget *x = new PathWidget(m_pathFrame);
    m_pathFrame->layout()->addWidget(x);
    m_paths << x;
    m_connectionFrame1->update();
    m_connectionFrame2->update();
}

void MainWindow::addOutput()
{
    OutputWidget *x = new OutputWidget(m_outputFrame);
    m_outputFrame->layout()->addWidget(x);
    m_outputs << x;
    m_connectionFrame2->update();
}

void MainWindow::madeConnection1(const QPoint &a, const QPoint &b)
{
    QPoint left;
    QPoint right;
    if (a.x() < m_connectionFrame1->width() / 3)
        left = a;
    else if (a.x() > m_connectionFrame1->width() * 2 / 3)
        right = a;
    if (b.x() < m_connectionFrame1->width() / 3)
        left = b;
    else if (b.x() > m_connectionFrame1->width() * 2 / 3)
        right = b;
    if (left.isNull() || right.isNull())
        return;

    kDebug() << left << right;

    ProducerWidget *producer = 0;
    foreach (ProducerWidget *x, m_producers)
    {
        QRect rect = x->geometry();
        kDebug() << rect;
        left.setX(rect.center().x());
        if (rect.contains(left))
            producer = x;
    }
    if (!producer)
        return;

    PathWidget *path = 0;
    foreach (PathWidget *x, m_paths)
    {
        QRect rect = x->geometry();
        kDebug() << rect;
        right.setX(rect.center().x());
        if (rect.contains(right))
            path = x;
    }
    if (!path)
        return;

    if (producer->connectPath(path))
        m_connectionFrame1->addConnection(producer, path);
}

void MainWindow::madeConnection2(const QPoint &a, const QPoint &b)
{
    QPoint left;
    QPoint right;
    if (a.x() < m_connectionFrame2->width() / 3)
        left = a;
    else if (a.x() > m_connectionFrame2->width() * 2 / 3)
        right = a;
    if (b.x() < m_connectionFrame2->width() / 3)
        left = b;
    else if (b.x() > m_connectionFrame2->width() * 2 / 3)
        right = b;
    if (left.isNull() || right.isNull())
        return;

    kDebug() << left << right;

    PathWidget *path = 0;
    foreach (PathWidget *x, m_paths)
    {
        QRect rect = x->geometry();
        kDebug() << rect;
        left.setX(rect.center().x());
        if (rect.contains(left))
            path = x;
    }
    if (!path)
        return;

    OutputWidget *output = 0;
    foreach (OutputWidget *x, m_outputs)
    {
        QRect rect = x->geometry();
        kDebug() << rect;
        right.setX(rect.center().x());
        if (rect.contains(right))
            output = x;
    }
    if (!output)
        return;

    if (path->connectOutput(output))
        m_connectionFrame2->addConnection(path, output);
}
*/

int main(int argc, char **argv)
{
    KAboutData about("phonontester", 0, ki18n("KDE Multimedia Test"),
            "0.2", KLocalizedString(),
            KAboutData::License_LGPL);
    about.addAuthor(ki18n("Matthias Kretz"), KLocalizedString(), "kretz@kde.org");
    KCmdLineArgs::init(argc, argv, &about);
    KApplication app;
    MainWindow w;
    w.show();
    return app.exec();
}

#include "main.moc"

// vim: sw=4 ts=4
