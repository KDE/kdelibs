#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <qiconview.h>
#include <fileloaderjob.h>
#include "imageview_base.h" // designer made

class ImageView : public ImageViewBase
{
    Q_OBJECT
public:
    ImageView ( QWidget *parent = 0, const char *name = 0);
    ~ImageView();
protected:
    void slotQuit();
    void slotSelectFiles();
};

class ComputeThumbNail : public ThreadWeaver::Job
{
    Q_OBJECT
public:
    ComputeThumbNail ( ThreadWeaver::FileLoaderJob *fileLoader,
                       QObject *parent = 0);
    /** Returns the "ready made" thumbnail. */
    const QPixmap& thumb();
protected:
    void run();
    QPixmap m_thumb;
    QImage m_image;
    const ThreadWeaver::FileLoaderJob *m_fileLoader;
};

/** Have the file loaded, then create a thumbnail. Interdependant jobs are
    used to implement the task, leaving no job scheduling code in the
    class. */
class ThumbNail : public QObject
{
    Q_OBJECT
public:
    ThumbNail ( ThreadWeaver::Weaver *weaver,
                QString filename, QIconView *iconview, 
		QObject *parent);
    ~ThumbNail();
protected slots:
    void thumbReady();
protected:
    ThreadWeaver::FileLoaderJob *m_fileLoader;
    ThreadWeaver::Weaver *m_weaver;
    QIconView *m_iconview;
    QIconViewItem *m_item;
    ComputeThumbNail *m_thumb;
    QString m_text;
};

#endif

