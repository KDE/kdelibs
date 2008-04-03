class SpeechToText
{
    Q_OBJECT
    public:
        SpeechToText();

    private slots:
        void processCaptureData();

    private:
        PcmCapture *m_capture;
};

SpeechToText::SpeechToText();
{
    // imagining Phonon API that's not there yet...
    m_capture = new PcmCapture(Phonon::Communication, this);
    Q_ASSERT(qobject_cast<QIODevice *>(m_capture));
    Q_ASSERT(!m_capture->isOpen());
    // hmm, probably not good to do this:
    if (!m_capture->open(PcmFormat(PcmFormat::NativeByteOrder, 16, 1, 48000, PcmFormat::DisallowSoftResampling))) {
        // handle error possibly trying a different format
        // (read m_capture->errorString())
        return;
    }

    connect(m_capture, SIGNAL(readyRead()), SLOT(processCaptureData()));
}

void SpeechToText::processCaptureData()
{
    const int toProcess = m_capture->bytesAvailable();
    const QByteArray = 
}
