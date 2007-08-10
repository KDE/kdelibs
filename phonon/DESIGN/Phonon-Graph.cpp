namespace Phonon
{


class MediaNode
{
public:
    // must be connected to the backend to get information about the format
    QList<Port> inputPorts(MediaStreamType type = Phonon::AllMedia); 

    QList<Port> outputPorts(MediaStreamType type = Phonon::AllMedia); 


};

class MediaSource
{
        //the parameter is used to filter
        QList<SubStreamDescription> subStreams(MediaStreamType = AllMedia)
        {
            //this just asks the source to give us back ots substream information
        }
};

class MediaObject : public QObject
{
    Q_OBJECT
    public:
        MediaObject(const MediaSource &, QObject *parent);
        QList<DecoderParameter> decoderParameters(const Port &port);
        /**
         * Returns 0 if there are no parameters for the given port/decoder.
         *
         * \code
         * media = new MediaObject;
         * media->setCurrentSource("/music/file.ogg");
         * output = new AudioOutput(Phonon::Music);
         * Path path(media, output);
         * QWidget *w = media->decoderParameterWidget(path.sourcePort());
         * if (w) {
         *     w->setParent(this);
         *     // or
         *     // w->show();
         * }
         * \endcode
         */
        QWidget *decoderParameterWidget(const Port &port);

        //the parameter is used to filter
        QList<SubStreamDescription> currentSubStreams(MediaStreamType = AllMedia)
        {
            //this just asks the source to give us back ots substream information
        }



        bool selectSubstream(SubStreamDescription, const Port &port = Port())
        {
            //check the type of the port...
            //if the port is invalid, it will do so for every suitable open ports
        }

        /*
         * returns an invalid Port in case of errors.
         * format is optional and can be defined later when the connection is done
         */
        Port openPort(MediaStreamType type, const char *format = 0)
        {
            //it gets the default stream
        }

        bool closePort(const Port &)
        {
        }

        bool isPortValid(const Port &p)
        {
            //checks if the port description can correspond to a port in this mediaobject
            //will most likely be used a lot internally
        }
};

class Path
{

    //addition for the ports

    Phonon::MediaStreamTypes types();
    QList<Port> inputPorts, outputPorts;
};


// post 4.0:
class Port
{
    //the real identifier of a port is its index ?
    //then the format is just information you either give when creating the port
    //or information you can retrieve once a connection is established

public:
    // a null format means that any might be used and one is automatically selected when
    // a connection is set up
    Port(MediaStreamTypes types, int index = 0, const char *format = 0) : m_supportedTypes(types), 
        m_index(index), m_format(format), m_type(0)
    {
    }

    Port(int index = -1) : m_supportedTypes(AllMedia), m_index(index), m_format(0), m_type(0)
    {
        //gets the port whatever the mediatype is
    }

    int isValid() const { return index != -1; }

    MediaStreamType type() const { return m_type; }
    int index() const { return m_index; }
    char *format() const { return m_format; }

private:
    const MediaStreamTypes m_supportedtypes;
    MediaStreamType m_type;
    const int m_index; //index (depends on the media type?)
    const char * const m_format;
};

Path createPath(MediaNode *source, const Port &srcport, MediaNode *sink, const Port &sinkport = Port(AllMedia))
{
    // if there's already a path to sink then return invalid Path
}

} // namespace Phonon

MediaObject *media;
{
    media = new MediaObject("/video/foo.ogm");
    // post 4.0:
    /*
     Path p = Phonon::createPath(media, Port(Phonon::Audio, 0, "pcm"), writer);
     Path p = Phonon::createPath(media, writer);
     p.mediaStreamTypes() & (Phonon::Audio | Phonon::Video)

     //ports must always be valid... but they might be 'unconnectable' (depnding on the current source)
     Port audio_port = media.openPort(Phonon::Audio, "pcm");
     Port audio_port2 = media.openPort(Phonon::Audio);


     //pb: when to call this
     //is substream referring to the mediasource? Then you could possibly bind it any time?
     media.selectSubStream(src.subStream(Phonon::Audio).first(), audio_port); <- selects the auid stream for the specified audio_port
     media.selectSubStream(src.subStream(Phonon::Audio).first()); <-- does it for every audio port (of course only applies to 'src')
    
    //do this for every source? is the way it works obvious?
    //example
     MediaObject o;
     o.enqueue( QList<QUrl>() <<...
     foreach(const MediaSource &src, o.queue()) {
        // 'attach' would be better but what to do in case there is no port specified
        media.selectSubStream( src.substream(Phonon::Audio)[1] ); 
     }





     //do we want to have so many formats or are the formats basically raw or decoded (with a specific format)?

     AudioOutput *audioOut = new AudioOutput(Phonon::Music);
     Path p = Phonon::createPath(media, audio_port2, writer); // <-- the format of audioport2 is selected to coded?
     //shouldn't the port object be changed to reflect the port format has actually been set
     Path p = Phonon::createPath(media, audio_port2, audioOut);  // <-- this will fail
     Path p = Phonon::createPath(media, media.outputPorts(Phonon::Video)[0], writer);

    */

    AudioOutput *audioOut = new AudioOutput(Phonon::Music);
    Path audioPath = Phonon::createPath(media, audioOut);
    Effect *audioEffect = new Effect(audioEffectDescription, this);
    audioPath.insertEffect(audioEffect);

    VideoWidget *videoWidget = new VideoWidget;
    Path videoPath = Phonon::createPath(media, videoWidget);
    Effect *videoEffect = videoPath.insertEffect(videoEffectDescription);

    videoPath.disconnect();

    media->play();

    // post 4.0:
    QList<SubtitleStreamDescription> l = media->availableSubtitleStreams();
    media->setCurrentSubtitleStream(l.first());
    Port port = media->openPort(Phonon::Subtitles);
    media->setCurrentSubtitleStream(l.first(), port);
}
// Paths are not gone
delete media;
// Paths are gone now

// Mixing a second media
// =====================
//  _____________
// | o           |
// |audio1 in    |
// |           o |
// |    audio out|
// | o           |
// |audio2 in    |
// |_____________|
MediaObject *media2 = new MediaObject("/music/foo2.ogg");
AudioMixer *amix = new AudioMixer;
Path *audioPath2 = new Path(media2, /*"audio",*/ amix);

Path *audioPath3 = new Path(amix, audioOut); // this unplugs audioPath from audioOut, otherwise this Path would not be possible
audioPath->changeOutput(amix); // and here we plug the audioPath back in, but into amix

media2->play();


// Writing to a file
// =================
//  ____________
// | o          |
// |audio in    |
// |            |
// | o          |
// |video in    |
// |____________|
FileWriter *writer = new FileWriter;
writer->setContainerFormat(...);
Encoder *audioEnc = writer->createAudioStream("name", ...);
Encoder *audioEnc2 = writer->createAudioStream("name", ...);
Encoder *videoEnc = writer->createVideoStream(...);
Path *writerAudioPath = new Path(media, audioEnc);
Path *writerAudioPath2 = new Path(media, audioEnc2);
Path *writerVideoPath = new Path(media, videoEnc);
writerAudioPath->insertEffect(audioEffect);
writerVideoPath->insertEffect(videoEffect);
media->setCurrentAudioStream(streamDesc, writerAudioPath);
media->setCurrentAudioStream(streamDesc2, writerAudioPath2);


// Visualization
// =============
//  ____________
// |            |
// |            |
// | o        o |
// |audio  video|
// | in     out |
// |____________|
VideoWidget *visWidget = new VideoWidget;
Visualization *vis = new Visualization;
Path *visAudioInPath = new Path(media, vis);
Path *visVideoOutPath = new Path(vis, visWidget);


// Synchronizing two MediaObjects
// ==============================
Synchronizer *sync = new Synchronizer;
sync->addMediaObject(media);
sync->addMediaObject(media2);
media->pause();// ?
sync->pause();
// media->state() == Phonon::PausedState == media2->state()
sync->play();
// media->state() == Phonon::PlayingState == media2->state()
...
