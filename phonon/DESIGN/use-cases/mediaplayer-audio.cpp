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
        QComboBox *m_visComboBox;
};

MediaPlayer::MediaPlayer()
    : m_vis(0), m_visComboBox(0)
{
    //setCentralWidget(&m_fileView);
    m_fileView.setModel(&m_model);
    m_fileView.setFrameStyle(QFrame::NoFrame);

    m_media = new MediaObject(this);
    AudioOutput *output = new AudioOutput(Phonon::Music, this);
    Phonon::createPath(m_media, output);

    QList<VisualizationDescription> list = BackendCapabilities::availableVisualizationEffects();
    if (!list.isEmpty()) {
        VideoWidget *visWidget = new VideoWidget(this);
        m_vis = new Visualization(this);
        Phonon::createPath(m_media, m_vis);
        Phonon::createPath(m_vis, visWidget);
        m_vis->setVisualization(list.first());

        m_visComboBox = new QComboBox(this);
        m_visComboBox->setModel(new VisualizationDescriptionModel(list, m_visComboBox));
        connect(m_visComboBox, SIGNAL(activated(int)), SLOT(visEffectChanged(int)));
    }

    connect(&m_fileView, SIGNAL(updatePreviewWidget(const QModelIndex &)), SLOT(play(const QModelIndex &)));
}

void MediaPlayer::play(const QModelIndex &modelIndex)
{
    m_media->setCurrentSource(url);
    m_media->play();
}

void MediaPlayer::visEffectChanged(int index)
{
    QList<VisualizationDescription> list = BackendCapabilities::availableVisualizationEffects();
    Q_ASSERT(index < list.size() && index >= 0);
    m_vis->setVisualization(list.at(index));
}


