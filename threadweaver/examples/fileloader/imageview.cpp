#include <qapplication.h>
#include <qfiledialog.h>
#include <qimage.h>

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

void ImageView::slotSelectFiles()
{
    ivThumbs->clear();

    QStringList files = QFileDialog::getOpenFileNames (
        "Images (*.png *.xpm *.jpg *.JPG)",
        "/home",
        this,
        "open file dialog",
        "Choose Files to display" );

    if (files.count() >0)
    {
        QStringList::Iterator it;
        for ( it = files.begin(); it != files.end(); ++it )
        {
            new ThumbNail (theWeaver, *it, ivThumbs, this);
        }
    } else {
        qApp->beep();
    }
}

ThumbNail::ThumbNail ( ThreadWeaver::Weaver *weaver,
                       QString filename, QIconView *iconview,
                       QObject *parent)
    : QObject (parent),
      m_weaver (weaver),
      m_iconview (iconview)
{
    m_text = filename.section ('/', -1);
    m_fileLoader = new ThreadWeaver::FileLoaderJob (filename, this);
    m_thumb = new ComputeThumbNail (m_fileLoader, this);
    connect (m_thumb, SIGNAL (done()), SLOT (thumbReady()));
    weaver->enqueue (m_fileLoader);
    weaver->enqueue (m_thumb);
}

ThumbNail::~ThumbNail ()
{
}

void ThumbNail::thumbReady()
{
    // free the memory that holds the file contents
    delete m_fileLoader;
    // display the thumbnail
    m_item = new QIconViewItem (m_iconview, m_text, m_thumb->thumb());
}

ComputeThumbNail::ComputeThumbNail(
    ThreadWeaver::FileLoaderJob *fileLoader, QObject *parent)
    : ThreadWeaver::Job (fileLoader, parent),
      m_fileLoader (fileLoader)
{
}

const QPixmap& ComputeThumbNail::thumb()
{
    return m_thumb;
}

void ComputeThumbNail::run()
{
    if ( m_image.loadFromData ( (const uchar*) m_fileLoader->data(),
                                m_fileLoader->size() ) )
    {
        m_thumb = m_image.smoothScale ( 64, 48, QImage::ScaleMin );
    } else {
        ThreadWeaver::debug
            ( 0, "ComputeThumbNail::run: cannot load image.\n" );
    }
    m_image.reset();
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
