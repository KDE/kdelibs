#include <QObject>

namespace Phonon {
	class MediaObject;
	class AudioPath;
	class FaderEffect;
	class AudioOutput;
}
class KUrl;
using namespace Phonon;

class Crossfader : public QObject
{
	Q_OBJECT
	public:
		Crossfader( QObject* parent = 0 );

	signals:
		void needNextUrl( KUrl& nextUrl );

	public slots:
		void start( const KUrl& firstUrl );
		void stop();

	private slots:
		void crossfade( long );
		void setupNext();

	private:
		MediaObject *m1, *m2;
		AudioPath *a1, *a2;
		FaderEffect *f1, *f2;
		AudioOutput *output;
};
