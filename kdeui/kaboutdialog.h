/* -*- C++ -*-
 * This file declares a class for creating "About ..." dialogs
 * in a general way. It provides geometry management and some
 * options to connect for, like emailing the author or
 * maintainer.
 *
 * copyright:  (C) Mirko Sucker, 1999
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko@kdeorg>
 * requires:   at least Qt 1.4
 * $Revision$
 */

#ifndef KABOUTDIALOG_H
#define KABOUTDIALOG_H

#include <dialogbase.h>
#include <qframe.h>
#include <qstring.h>
#include <list>

class QLabel;
class KURLLabel;

/** Every contributor, the author and/or the maintainer of the application are
 *  stored in objects of this local class. Every single field may be empty. */
class KAboutContributor : public QFrame
{
  // ############################################################################
  Q_OBJECT
  // ----------------------------------------------------------------------------
public:
  /** The Qt constructor. */
  KAboutContributor(QWidget* parent=0, const char* name=0);
  /** Set the name (a literal string). */
  void setName(const QString&);
  /** Get the name. */
  QString getName();
  /** The email address (dito). */
  void setEmail(const QString&);
  /** Get the email address. */
  QString getEmail();
  /** The URL (dito). */
  void setURL(const QString&);
  /** Get the URL. */
  QString getURL();
  /** The tasks the person worked on (a literal string). More than one line is 
   *  possible, but very long texts might look ugly. */
  void setWork(const QString&);
  /** The size hint. Very important here, since KAboutWidget relies on it for
   *  geometry management. */
  QSize sizeHint();
  // ----------------------------------------------------------------------------
protected:
  // events:
  /** The resize event. */
  void resizeEvent(QResizeEvent*);
  /** The paint event. */
  void paintEvent(QPaintEvent*);
  /** The label showing the program version. */
  QLabel *name;
  /** The clickable URL label showing the email address. It is only visible if
   *  its text is not empty. */
  KURLLabel *email;
  /** Another interactive part that displays the homepage URL. */
  KURLLabel *url;
  /** The description of the contributions of the person. */
  QString work;
  // ----------------------------------------------------------------------------
protected slots:
  /** The homepage URL has been clicked. */
  void urlClickedSlot(const QString&);
  /** The email address has been clicked. */
  void emailClickedSlot(const QString& emailaddress);
  // ----------------------------------------------------------------------------
signals:
  /** The email address has been clicked. */
  void sendEmail(const QString& name, const QString& email);
  /** The URL has been clicked. */
  void openURL(const QString& url);
  // ############################################################################
};

/** KAboutWidget is the main widget for KAboutDialog.
 *  It has a minimum size set. */
class KAboutWidget : public QWidget
{
  // ############################################################################
  Q_OBJECT
  // ----------------------------------------------------------------------------
public:
  /** The Qt constructor. */
  KAboutWidget(QWidget* parent=0, const char* name=0);
  /** Adjust the minimum size (after setting the properties of the image and the
   *  labels. */
  void adjust();
  /** Set the image as the application logo. */
  void setLogo(const QPixmap&);
  /** Set the authors name and email address. */
  void setAuthor(const QString& name, const QString& email,
		 const QString& url, const QString& work);
  /** Set the maintainers name and email address. */
  void setMaintainer(const QString& name, const QString& email,
		     const QString& url, const QString& work);
  /** Show this person as one of the major contributors. */
  void addContributor(const QString& name, const QString& email,
		      const QString& url, const QString& work);
  /** Set the text describing the version. */
  void setVersion(const QString& name);
  // ----------------------------------------------------------------------------
protected slots:
  /** Catch the signals from the contributors elements. */
  void sendEmailSlot(const QString& name, const QString& email);
  /** Catch the clicked URLs. */
  void openURLSlot(const QString& url);
  // ----------------------------------------------------------------------------
signals:
  /** An email address has been selected by the user. */
  void sendEmail(const QString& name, const QString& email);
  /** An URL has been clicked. */
  void openURL(const QString& url);
  // ----------------------------------------------------------------------------
protected:
  // events:
  /** The resize event. */
  void resizeEvent(QResizeEvent*);
  /** The label showing the program version. */
  QLabel *version;
  /** The label showing the text "Other contributors:". */
  QLabel *cont;
  /** The frame showing the logo. */
  QLabel *logo;
  /** The application developer. */
  KAboutContributor *author;
  /** The application maintainer. */
  KAboutContributor *maintainer;
  /** Show the maintainer? */
  bool showMaintainer;
  /** A set of people who contributed to the application. */
  list<KAboutContributor*> contributors;
  // ############################################################################
};

/** KAboutDialog is a DialogBase with predefined main widget.
 *  Being a DialogBase it uses your application wide settings for DialogBase
 *  objects (base frame tiles, main frame tiles etc). */
class KAboutDialog : public DialogBase
{
  // ############################################################################
  Q_OBJECT
  // ----------------------------------------------------------------------------
public:
  /** The Qt constructor. */
  KAboutDialog(QWidget* parent=0, const char* name=0);
  /** Adjust the dialog. Call this method after you set up all the contents. */
  void adjust();
  /** Set the image as the application logo. */
  void setLogo(const QPixmap&);
  /** Set the authors name and email address. */
  void setAuthor(const QString& name, const QString& email,
		 const QString& url, const QString& work);
  /** Set the maintainers name and email address. */
  void setMaintainer(const QString& name, const QString& email,
		     const QString& url, const QString& work);
  /** Show this person as one of the major contributors. */
  void addContributor(const QString& name, const QString& email,
		      const QString& url, const QString& work);
  /** Set the text describing the version. */
  void setVersion(const QString& name);
  // ----------------------------------------------------------------------------
signals:
  /** Send an email to this person. The application must provide the
   *  functionality. */
  void sendEmail(const QString& name, const QString& email);
  /** Open the selected URL. */
  void openURL(const QString& url);
  // ----------------------------------------------------------------------------
protected:
  /** The main widget. */
  KAboutWidget * about;
  // ----------------------------------------------------------------------------
protected slots:
  /** Connected to widget->sendEmail. */
  void sendEmailSlot(const QString& name, const QString& email);
  /** Open this URL. */
  void openURLSlot(const QString& url);
  // ############################################################################
};

#endif // defined KABOUTDIALOG_H





