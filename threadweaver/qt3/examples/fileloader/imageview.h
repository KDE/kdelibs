#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <qiconview.h>
#include <fileloaderjob.h>
#include <qimageloaderjob.h>
#include "imageview_base.h" // designer made

using ThreadWeaver::Job;

class ComputeThumbNailJob : public ThreadWeaver::Job
{
    Q_OBJECT
public:
    ComputeThumbNailJob ( ThreadWeaver::QImageLoaderJob *imageLoader,
                       QObject *parent = 0);
    /** Returns the "ready made" thumbnail. */
    const QPixmap* thumb();

protected:
    void run();
    QPixmap m_thumb;
    const ThreadWeaver::QImageLoaderJob *m_image;
};

/** Have the file loaded, then create a thumbnail. Interdependant jobs are
    used to implement the task, leaving no job scheduling code in the
    class. */
class ThumbNail : public QObject
{
    Q_OBJECT
public:
    ThumbNail ( ThreadWeaver::Weaver *weaver,
                QString filename,
		QObject *parent);
    ~ThumbNail();
    const QPixmap* thumb();
protected slots:
    void slotImageReady( Job* ); // delete the file data
    void slotThumbReady( Job* ); // display it
signals:
    void thumbReady ( ThumbNail*, QString);
protected:
    ThreadWeaver::FileLoaderJob *m_fileLoader;
    ThreadWeaver::QImageLoaderJob *m_imageLoader;
    ThreadWeaver::Weaver *m_weaver;
    ComputeThumbNailJob *m_thumb;
    QString m_text;
};

class ImageView : public ImageViewBase
{
    Q_OBJECT
public:
    ImageView ( QWidget *parent = 0, const char *name = 0);
    ~ImageView();
protected:
    void slotQuit();
    void slotSelectFiles();
public slots:
    void slotThumbReady ( ThumbNail*, QString);
};

#endif

