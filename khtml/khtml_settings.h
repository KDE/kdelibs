/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __konq_htmlsettings_h__
#define __konq_htmlsettings_h__

class KConfig;
#include <qcolor.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qfont.h>
#include <qmap.h>

/**
 * Settings for the HTML view.
 * We need this in konqueror since we need to call reparseConfiguration
 * on the static instance (even when we don't have an HTML view any longer).
 */
class KHTMLSettings
{
public:

  /**
   * This enum specifies whether Java/JavaScript execution is allowed.
   */
  enum KJavaScriptAdvice {
    KJavaScriptDunno=0,
    KJavaScriptAccept,
    KJavaScriptReject
  };

  /**
   * @internal Constructor
   */
  KHTMLSettings();

  void init();

  /** Called by constructor and reparseConfiguration */
  void init( KConfig * config, bool reset = true );

  /** Destructor. Don't delete any instance by yourself. */
  virtual ~KHTMLSettings() {};

  // Behaviour settings
  bool changeCursor() { return m_bChangeCursor; }
  bool underlineLink() { return m_underlineLink; }

  // Font settings
  const QString& stdFontName() const { return m_strStdFontName; }
  const QString& fixedFontName() const { return m_strFixedFontName; }
  const QString& serifFontName() const { return m_strSerifFontName; }
    const QString& sansSerifFontName() const { return m_strSansSerifFontName; }
    const QString& cursiveFontName() const { return m_strCursiveFontName; }
    const QString& fantasyFontName() const { return m_strFantasyFontName; }

    // these two can be set. Mainly for historical reasons (the method in KHTMLPart exists...)
  void setStdFontName(const QString &n) { m_strStdFontName = n; }
  void setFixedFontName(const QString &n) { m_strFixedFontName = n; }

    const QValueList<int> &fontSizes() const { return m_fontSizes; }
    void setFontSizes(const QValueList<int> &newFontSizes );
    void resetFontSizes();

    int minFontSize() const { return m_minFontSize; }

    // the charset used to display the current document.
    QFont::CharSet charset() const { return m_charset; }
    void setCharset( QFont::CharSet c) { if(!enforceCharset) m_charset = c; }

    const QString &encoding() const { return m_encoding; }

  // Color settings
  const QColor& textColor() { return m_textColor; }
  const QColor& linkColor() { return m_linkColor; }
  const QColor& vLinkColor() { return m_vLinkColor; }

  // Autoload images
  bool autoLoadImages() { return m_bAutoLoadImages; }

  // Java and JavaScript
  bool isJavaEnabled( const QString& hostname = QString::null );
  bool isJavaScriptEnabled( const QString& hostname = QString::null );

  // helpers for parsing domain-specific configuration, used in KControl module as well
  static KJavaScriptAdvice strToAdvice(const QString& _str);
  static void splitDomainAdvice(const QString& configStr, QString &domain, 
								KJavaScriptAdvice &javaAdvice, KJavaScriptAdvice& javaScriptAdvice);
  static const char* adviceToStr(KJavaScriptAdvice _advice);


private:
  bool m_bChangeCursor;
  bool m_underlineLink;

  QString m_strStdFontName;
  QString m_strFixedFontName;
  QString m_strSerifFontName;
    QString m_strSansSerifFontName;
    QString m_strCursiveFontName;
    QString m_strFantasyFontName;

    QValueList<int>     m_fontSizes;
    int m_minFontSize;

    QFont::CharSet m_charset;
    bool enforceCharset;
    QString m_encoding;

  QColor m_textColor;
  QColor m_linkColor;
  QColor m_vLinkColor;

  bool m_bAutoLoadImages;
  bool m_bEnableJava;
  bool m_bEnableJavaScript;
  QMap<QString,KJavaScriptAdvice> javaDomainPolicy;
  QMap<QString,KJavaScriptAdvice> javaScriptDomainPolicy;

  //  static KonqHTMLSettings * s_HTMLSettings;
};

#endif
