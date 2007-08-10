#include <QtGui/QMainWindow>

class MediaPlayer : public QMainWindow
{
    Q_OBJECT
    public:
        MediaPlayer();

    private slots:
        void play(const QModelIndex &modelIndex);
        void visEffectChanged(int index);

    private:
        QColumnView m_fileView;
        QDirModel m_model;

        Phonon::MediaObject *m_media;
        Phonon::Visualization *m_vis;
        Phonon::SubtitleStreamDescriptionModel m_subtitleModel;
        QComboBox *m_visComboBox;
};

MediaPlayer::MediaPlayer()
    : m_vis(0), m_visComboBox(0)
{
    //setCentralWidget(&m_fileView);
    m_fileView.setModel(&m_model);
    m_fileView.setFrameStyle(QFrame::NoFrame);

    m_media = new MediaObject(this);

    AudioPath *apath = new AudioPath(this);
    AudioOutput *aoutput = new AudioOutput(Phonon::Music, this);
    m_media->addAudioPath(apath);
    apath->addOutput(aoutput);

    VideoPath *vpath = new VideoPath(this);
    VideoWidget *vwidget = new VideoWidget(this);
    m_media->addVideoPath(vpath);
    vpath->addOutput(vwidget);

    // the following signal does not exist yet, but should, IMHO:
    connect(m_media, SIGNAL(availableSubtitleStreamsChanged()), SLOT(updateSubtitleSelector()));

    m_subtitleSelector = new QListView(this);
    m_subtitleSelector->setModel(&m_subtitleModel);
    connect(m_subtitleSelector, SIGNAL(activated(const QModelIndex &)), SLOT(changeSubtitle(const QModelIndex &)));

    connect(&m_fileView, SIGNAL(updatePreviewWidget(const QModelIndex &)), SLOT(play(const QModelIndex &)));
}

void MediaPlayer::play(const QModelIndex &modelIndex)
{
    m_media->setCurrentSource(url);
    m_media->play();
}

void MediaPlayer::updateSubtitleSelector()
{
    QList<SubtitleStreamDescription> list = m_media->availableSubtitleStreams();
    m_subtitleModel.setModelData(list);
}

void MediaPlayer::changeSubtitle(const QModelIndex &modelIndex)
{
    const int index = modelIndex.row();
    QList<SubtitleStreamDescription> list = m_media->availableSubtitleStreams();
    Q_ASSERT(index < list.size() && index >= 0);
    m_media->setCurrentSubtitleStream(list.at(index));
}
