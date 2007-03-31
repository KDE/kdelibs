addSignal("dataReady(const QMap<Phonon::Experimental::AudioDataOutput::Channel, QVector<qint16>>&)");
addMethod("int", "sampleRate()");
addMethod("", "setFormat(Phonon::Experimental::AudioDataOutput::Format)");
addMethod("Phonon::Experimental::AudioDataOutput::Format", "format()");
addSignal("endOfMedia(int)");
addMethod("int", "dataSize()");
addSignal("dataReady(const QMap<Phonon::Experimental::AudioDataOutput::Channel, QVector<float>>&)");
addMethod("", "setDataSize(int)");
