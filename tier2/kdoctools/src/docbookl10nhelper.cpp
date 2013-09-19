/*  This file is part of the KDE project
    Copyright (C) 2010 Luigi Toscano <luigi.toscano@tiscali.it>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QList>
#include <QPair>
#include <QRegExp>
#include <QStringList>
#include <QTextStream>


class LangListType: public QList<QPair<QString,QString> >
{
public:
   int searchLang( QString el ) {

      for ( int i = 0; i < size(); ++i ) {
         if ( at( i ).first == el )
            return i;
      }
      return -1;
   }
};

int writeLangFile( const QString &fname, const QString &dtdPath,
                   const LangListType &langMap ) {

   QFile outFile( fname );
   if ( ! outFile.open( QIODevice::WriteOnly ) ) {
      qCritical() << QStringLiteral( "Could not write %1" )
                     .arg( outFile.fileName() );
      return( 1 );
   }

   QTextStream outStream( &outFile );
   outStream << "<?xml version='1.0'?>" << endl;
   outStream << QStringLiteral( "<!DOCTYPE l:i18n SYSTEM \"%1\" [" )
               .arg( dtdPath ) << endl;

   LangListType::const_iterator i = langMap.constBegin();
   while ( i != langMap.constEnd() ) {
      //qDebug() << (*i).first << ": " << (*i).second;
      outStream << QStringLiteral( "<!ENTITY %1 SYSTEM \"%2\">" )
                   .arg( (*i).first ).arg( (*i).second ) << endl;
      ++i;
   }
   outStream << "]>" << endl;

   if ( langMap.size() > 0 ) {
      outStream
         << "<l:i18n xmlns:l=\"http://docbook.sourceforge.net/xmlns/l10n/1.0\">"
         << endl;
      i = langMap.constBegin();
      while ( i != langMap.constEnd() ) {
         outStream << QStringLiteral( "&%1;" )
                      .arg( (*i).first ) << endl;
         ++i;
      }
      outStream << "</l:i18n>" << endl;
   }

   outFile.close();

   return( 0 );
}

int writeLangFileNew( const QString &fname, const QString &dtdPath,
                      const LangListType &langMap ) {

   QFile outFile( fname );
   if ( ! outFile.open( QIODevice::WriteOnly ) ) {
      qCritical() << QStringLiteral( "Could not write %1" )
                     .arg( outFile.fileName() );
      return( 1 );
   }

   QTextStream outStream( &outFile );
   outStream << "<?xml version='1.0'?>" << endl;
   outStream << QStringLiteral( "<!DOCTYPE l:i18n SYSTEM \"%1\">" )
               .arg( dtdPath ) << endl;

   if ( langMap.size() > 0 ) {
      outStream
         << "<l:i18n xmlns:l=\"http://docbook.sourceforge.net/xmlns/l10n/1.0\">"
         << endl;
      LangListType::const_iterator i = langMap.constBegin();
      while ( i != langMap.constEnd() ) {
         outStream << QStringLiteral( "<l:l10n language=\"%1\" href=\"%2\"/>" )
                      .arg( (*i).first ).arg( (*i).second ) << endl;
         ++i;
      }
      outStream << "</l:i18n>" << endl;
   }

   outFile.close();

   return( 0 );
}

inline const QString addTrailingSlash( const QString &p ) {
    return p.endsWith( QStringLiteral("/") ) ? p : p + QStringLiteral("/");
}

int main( int argc, char **argv ) {
   QCoreApplication app( argc, argv );

   const QStringList arguments = app.arguments();
   if ( arguments.count() != 4 ) {
      qCritical() << "wrong argument count";
      return ( 1 );
   }

   const QString l10nDir = addTrailingSlash( arguments[1] );
   const QString l10nCustomDir = addTrailingSlash( arguments[2] );
   const QString destDir = addTrailingSlash( arguments[3] );

   QFile i18nFile( l10nDir + QStringLiteral("common/l10n.xml") );

   if ( ! i18nFile.open( QIODevice::ReadOnly ) ) {
      qCritical() << i18nFile.fileName() << " not found";
      return( 1 );
   }

   const QString all10nFName = destDir + QStringLiteral("all-l10n.xml");
   const QString customl10nFName = destDir + QStringLiteral("kde-custom-l10n.xml");

   /*
    * for each language defined in the original l10n.xml, copy
    * it into all-l10n.xml and store it in a list;
    **/
   QRegExp rxEntity, rxEntity2, rxDocType, rxDocType2;
   rxDocType.setPattern(QStringLiteral("^\\s*<!DOCTYPE\\s+l:i18n\\s+SYSTEM\\s+\"l10n\\.dtd\"\\s+\\[\\s*$"));
   rxDocType2.setPattern(QStringLiteral("^\\s*<!DOCTYPE\\s+l:i18n\\s+SYSTEM\\s+\"l10n\\.dtd\"\\s*>$"));
   rxEntity.setPattern(QStringLiteral("^\\s*<!ENTITY\\s+([^\\s]+)\\s+SYSTEM\\s+\"([^\\s]+)\">\\s*$"));
   rxEntity2.setPattern(QStringLiteral("^\\s*<l:l10n language=\"([^\\s]+)\"\\s+href=\"([^\\s]+)\"/>\\s*$"));
   QTextStream inStream( &i18nFile );
   int parsingState = 0;

   LangListType allLangs, customLangs;

   bool foundRxEntity = false;
   bool foundRxEntity2 = false;
   while ( ! inStream.atEnd() ) {
      QString line = inStream.readLine();

      switch ( parsingState ) {
       case 0:
         if ( rxDocType.indexIn( line ) != -1 ) {
            parsingState = 1;
            //qDebug() << "DTD found";
         } else if ( rxDocType2.indexIn( line ) != -1 ) {
            parsingState = 1;
            //qDebug() << "DTD found";
         }
         break;
       case 1:
         QString langCode, langFile;
         if ( rxEntity.indexIn( line ) != -1 && !foundRxEntity2 ) {
            foundRxEntity = true;
            langCode = rxEntity.cap( 1 );
            langFile = l10nDir + QStringLiteral("common/") + rxEntity.cap( 2 );
            allLangs += qMakePair( langCode, langFile );
            //qDebug() << langCode << " - " << langFile;
         } else if ( rxEntity2.indexIn( line ) != -1  && !foundRxEntity ) {
            foundRxEntity2 = true;
            langCode = rxEntity2.cap( 1 );
            langFile = l10nDir + QStringLiteral("common/") + rxEntity2.cap( 2 );
            allLangs += qMakePair( langCode, langFile );
            //qDebug() << langCode << " - " << langFile;
         }
         break;
      }

   }
   i18nFile.close();

   /* read the list of locally-available custom languages */
   QDir outDir( l10nCustomDir );

   QStringList dirFileFilters;
   dirFileFilters << QStringLiteral("*.xml");
   QStringList customLangFiles = outDir.entryList( dirFileFilters,
                           QDir::Files|QDir::NoSymLinks, QDir::Name );
   /* the following two calls to removeOne should not be needed, as
    * the customization directory from the sources should not contain
    * those files
    */
   customLangFiles.removeOne( QStringLiteral("all-l10n.xml") );
   customLangFiles.removeOne( QStringLiteral("kde-custom-l10n.xml") );
   //qDebug() << "customLangFiles:" << customLangFiles;

   /*
    * for each custom language (from directory listing), if it is not
    * in the list of upstream languages, add it to all-l10n.xml,
    * otherwise add it to kde-custom-l10n.xml
    */
   QStringList::const_iterator i = customLangFiles.constBegin();
   while ( i != customLangFiles.constEnd() ) {
      QString langFile = (*i);
      /* remove trailing .xml */
      QString langCode = langFile.left( langFile.length() - 4 );

      QPair<QString,QString> cl = qMakePair( langCode, langFile );
      if ( ( allLangs.searchLang( langCode ) ) > 0 ) {
      /* custom language found in upstream list */
         customLangs += cl;
      } else {
      /* custom language not found in upstream list */
         allLangs += cl;
      }
      ++i;
   }

   int res = 0;

   if ( foundRxEntity ) {
      /* old style (docbook-xsl<=1.75) */
      res = writeLangFile( all10nFName, l10nDir + QStringLiteral("common/l10n.dtd"),
                           allLangs );
   } else {
       res = writeLangFileNew( all10nFName, l10nDir + QStringLiteral("common/l10n.dtd"),
                              allLangs );
   }

   return( res );
}
