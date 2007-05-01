#include "iodevicestream.h"
#include "iodevicestream_p.h"

namespace Phonon
{

IODeviceStream::IODeviceStream(QIODevice *ioDevice, QObject *parent)
    : AbstractMediaStream(*new IODeviceStreamPrivate(ioDevice), parent)
{
}

IODeviceStream::~IODeviceStream()
{
}

void IODeviceStream::needData()
{
    Q_D(IODeviceStream);
    const QByteArray data = d->ioDevice->read(4096);
    if (data.isEmpty()) {
        // error handling
    }
    writeData(data);
    if (d->ioDevice->atEnd()) {
        endOfData();
    }
}

void IODeviceStream::enoughData()
{
}

void IODeviceStream::seekStream(qint64 offset)
{
    Q_D(IODeviceStream);
    d->ioDevice->seek(offset);
}

} // namespace Phonon
#include "iodevicestream.moc"
// vim: sw=4 sts=4 et tw=100
