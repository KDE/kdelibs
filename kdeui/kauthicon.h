/*
 * KAuthIcon - an icon which shows whether privileges are in effect
 * Part of the KDE Project
 * Copyright (c) 1999 Preston Brown <pbrown@kde.org>
 */

#ifndef _KAUTHICON_H
#define _KAUTHICON_H

#include <qwidget.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qfileinfo.h>

class QPixmap;
class QHBoxLayout;

/**
 * This is the base class from which different authorization icon widget
 * which actually do something should be derived.  You can use these
 * widgets to show that the user has (or doesn't have) the ability to do
 * something, and why that is.
 *
 * One of the most useful things you can do with this is connect
 * authChanged(bool) to setEnabled(bool) for a widget to turn it on and
 * off depending on the status of whatever it is you are monitoring.
 *
 * @see KRootPermsIcon, KWritePermsIcon
 * @author Preston Brown <pbrown@kde.org>
 */
class KAuthIcon : public QWidget
{
  Q_OBJECT

public:
  /**
   * Constructor.
   */
  KAuthIcon(QWidget *parent = 0, const char *name = 0);
  ~KAuthIcon();

  virtual QSize sizeHint() const;
  /**
   * return the status of whatever is being monitored.
   */
  virtual bool status() const = 0;

public slots:
  /**
   * Re-implement this method if you want the icon to update itself
   * when something external has changed (i.e. a file on disk, uid/gid).
   */
  virtual void updateStatus() = 0;

signals:
  /**
   * this signal is emitted when authorization has changed from
   * it's previous state.
   * @param authorized will be true if the type of authorization
   * described by the icon is true, otherwise it will be false.
   */
  void authChanged(bool authorized);

protected:
  QHBoxLayout *layout;

  QLabel *lockBox;
  QLabel *lockLabel;
  QPixmap lockPM;
  QPixmap openLockPM;
  QString lockText;
  QString openLockText;
};

/* 
 * KRootPermsIcon tells whether or not root permissions are in effect.
 *
 * @see KAuthIcon
 * @author Preston Brown <pbrown@kde.org>
 */
class KRootPermsIcon : public KAuthIcon
{
  Q_OBJECT

public:
  KRootPermsIcon(QWidget *parent = 0, const char *name = 0);
  ~KRootPermsIcon();

  /**
   * return whether or not the current user has root permissions.
   */
  bool status() const { return root; }

public slots:
  void updateStatus();

protected:
  bool root;
};

/* 
 * KWritePermsIcon tells whether or not write permissions are in effect
 * for a particular file or directory.
 *
 * @see KAuthIcon
 * @author Preston Brown <pbrown@kde.org>
 */
class KWritePermsIcon : public KAuthIcon
{
  Q_OBJECT

public:
  KWritePermsIcon(QString fileName, QWidget *parent = 0, const char *name = 0);
  ~KWritePermsIcon();
  /**
   * @return whether or not the monitored file is writable.
   */
  bool status() const { return writable; }

  /**
   * make the icon watch a new filename.
   * @param fileName the new file to monitor / display status for.
   */
  void setFileName(QString fileName) { fi.setFile(fileName); updateStatus(); }
public slots:
  void updateStatus();

protected:
  bool writable; 
  QFileInfo fi;
};

#endif
