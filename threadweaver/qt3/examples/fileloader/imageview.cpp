#include <qapplication.h>
#include <qfiledialog.h>
#include <qimage.h>
#include <qdir.h>

#include <weaver.h>
#include <weavervisualizer.h>
#include <weaverlogger.h>
#include <fileloaderjob.h>

#include "imageview.h"

ThreadWeaver::Weaver *theWeaver;

ImageView::ImageView (QWidget *parent, const char *name)
    : ImageViewBase (parent, name)
{
}

ImageView::~ImageView()
{
}

void ImageView::slotQuit()
{
    // suspend queueing, wait for busy threads to finish, exit:
    // WORK_TODO
    close();
}

void ImageView::slotThumbReady ( ThumbNail *nail, QString name )
{
    new QIconViewItem ( ivThumbs, name, *(nail->thumb()) );
}

void ImageView::slotSelectFiles()
{
    ivThumbs->clear();

    QStringList files = QFileDialog::getOpenFileNames (
        "Images (*.png *.xpm *.jpg *.JPG)",
        QDir::homeDirPath(),
        this,
        "open file dialog",
        "Choose Files to display" );

    if (files.count() >0)
    {
        QStringList::Iterator it;
        for ( it = files.begin(); it != files.end(); ++it )
        {
            ThumbNail *thumb = new ThumbNail (theWeaver, *it, this);
            connect ( thumb, SIGNAL (thumbReady (ThumbNail*, QString) ),
                      SLOT ( slotThumbReady ( ThumbNail*, QString)) );
        }
    } else {
        qApp->beep();
    }
}

ThumbNail::ThumbNail ( ThreadWeaver::Weaver *weaver,
                       QString filename,
                       QObject *parent)
    : QObject (parent),
      m_weaver (weaver)
{
    m_text = filename.section ('/', -1);
    // load the image data from the file:
    m_fileLoader = new ThreadWeaver::FileLoaderJob (filename, this);
    // convert it to a QImage:
    m_imageLoader = new ThreadWeaver::QImageLoaderJob (m_fileLoader, this);
    connect (m_imageLoader, SIGNAL (done( Job* )),
             SLOT (slotImageReady( Job* )) );
    // make a thumbnail from it:
    m_thumb = new ComputeThumbNailJob (m_imageLoader, this);
    connect (m_thumb, SIGNAL (done( Job* )), SLOT (slotThumbReady( Job* )));
    weaver->enqueue (m_fileLoader);
    weaver->enqueue (m_imageLoader);
    weaver->enqueue (m_thumb);
}

ThumbNail::~ThumbNail ()
{
}

void ThumbNail::slotImageReady( Job* )
{
    // free the memory that holds the file contents:
    delete m_fileLoader;
}

const QPixmap* ThumbNail::thumb()
{
    if ( m_thumb->isFinished() )
    {
        return m_thumb->thumb();
    } else {
        return 0;
    }
}

void ThumbNail::slotThumbReady( Job* )
{
    delete m_imageLoader;
    // tell the icon view the thumbnail is ready:
    if (!m_thumb->thumb()->isNull())
    {
        emit ( thumbReady ( this, m_text ) );
    } else {
        ThreadWeaver::debug
            ( 0, "ComputeThumbNailJob::thumbReady: skipping image.\n" );
    }
}

ComputeThumbNailJob::ComputeThumbNailJob(
    ThreadWeaver::QImageLoaderJob *imageLoader, QObject *parent)
    : ThreadWeaver::Job (imageLoader, parent),
      m_image (imageLoader)
{
}

const QPixmap* ComputeThumbNailJob::thumb()
{
    return &m_thumb;
}

void ComputeThumbNailJob::run()
{
    if ( m_image->image() != 0)
    {
        m_thumb = m_image->image()->smoothScale ( 64, 48, QImage::ScaleMin );
    } else {
        ThreadWeaver::debug
            ( 0, "ComputeThumbNailJob::run: cannot load image.\n" );
        m_thumb.resize(0, 0);
    }
}

int main ( int argc, char ** argv)
{
    QApplication app ( argc, argv );
    ThreadWeaver::setDebugLevel (true, 0);
    ThreadWeaver::Weaver weaver (0, 0, 4);
    theWeaver = &weaver;
    ThreadWeaver::WeaverThreadVisualizer *vis
        = new ThreadWeaver::WeaverThreadVisualizer;
    vis->attach (&weaver);
//     ThreadWeaver::WeaverThreadLogger *log
//         = new ThreadWeaver::WeaverThreadLogger;
//     log->attach (&weaver);

    ImageView view;

    app.setMainWidget (&view);

    view.show();

    app.exec();

    return 0;
}

#include "imageview.moc"
