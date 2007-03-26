addSignal("length(qint64)");
addMethod("qint64", "remainingTime()", true);
addSignal("stateChanged(Phonon::State, Phonon::State)");
addSignal("finished()");
addSignal("aboutToFinish(qint32)");
addMethod("qint32", "aboutToFinishTime()");
addMethod("", "setAboutToFinishTime(qint32)");
