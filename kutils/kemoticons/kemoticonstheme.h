/***************************************************************************
 *   Copyright (C) 2008 by Carlo Segato <brandon.ml@gmail.com>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef KEMOTICONS_THEME_H
#define KEMOTICONS_THEME_H

#include "kemoticons_export.h"
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QStringList>
#include <QtXml/QDomDocument>
#include <QtCore/QPair>

class QString;

    
class KEmoticonsThemePrivate
{
    public:
        KEmoticonsThemePrivate();
        QString m_themeName;
        QString m_fileName;
        QString m_themePath;
        QMap<QString, QStringList> m_emoticonsMap;
   
};

class KEMOTICONS_EXPORT KEmoticonsTheme : public QObject
{
    Q_OBJECT
    public:
        KEmoticonsTheme(QObject *parent, const QVariantList &args);

        virtual ~KEmoticonsTheme();
        
        
        /**
        * TokenType, a token might be an image ( emoticon ) or text.
        */
        enum TokenType { 
            Undefined, /** Undefined, for completeness only */
            Image,     /** Token contains a path to an image */
            Text       /** Token contains test */
        };

        /**
        * A token consists of a QString text which is either a regular text
        * or a path to image depending on the type.
        * If type is Image the text refers to an image path.
        * If type is Text the text refers to a regular text.
        */
        struct Token {
            Token() : type( Undefined ) {}
            Token( TokenType t, const QString &m ) : type( t ), text(m) {}
            Token( TokenType t, const QString &m, const QString &p, const QString &html )
                : type( t ), text( m ), picPath( p ), picHTMLCode( html ) {}
            TokenType   type;
            QString     text;
            QString     picPath;
            QString     picHTMLCode;
        };

        /**
        * The possible parse modes
        */
        enum ParseModeEnum {  
            DefaultParseMode = 0x0 , /**  Use strict or relaxed according the config  */
            StrictParse = 0x1,       /** Strict parsing requires a space between each emoticon */
            RelaxedParse = 0x2,      /** Parse mode where all possible emoticon matches are allowed */
            SkipHTML = 0x4           /** Skip emoticons within HTML */
         };

        Q_DECLARE_FLAGS(ParseMode, ParseModeEnum)
        
        typedef QPair<QString, int> EmoticonNode;
        
        QString parseEmoticons(const QString &text, ParseMode mode = DefaultParseMode, const QStringList &exclude = QStringList());
        QList<Token> tokenize(const QString &message, ParseMode mode = DefaultParseMode);
        
        virtual bool loadTheme(const QString &path);
        virtual bool removeEmoticon(const QString &emo);
        virtual bool addEmoticon(const QString &emo, const QString &text, bool copy);
        virtual void save();

        QString themeName();
        void setThemeName(const QString &name);

        QMap<QString, QStringList> emoticonsMap();
        
        virtual void createNew();
  
    protected:
        KEmoticonsThemePrivate * const d;
  
};

#endif /* KEMOTICONS_THEME_H */

// kate: space-indent on; indent-width 4; replace-tabs on;
