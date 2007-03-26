addMethod("int", "sampleRate()");
addSignal("dataReady(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16>>&)");
addMethod("Phonon::AudioDataOutput::Format", "format()");
addMethod("", "setFormat(Phonon::AudioDataOutput::Format)");
addSignal("endOfMedia(int)");
addMethod("int", "dataSize()");
addSignal("dataReady(const QMap<Phonon::AudioDataOutput::Channel, QVector<float>>&)");
addMethod("", "setDataSize(int)");
