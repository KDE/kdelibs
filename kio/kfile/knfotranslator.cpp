/*****************************************************************************
 * Copyright (C) 2010 by Peter Penz <peter.penz@gmx.at>                      *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "knfotranslator_p.h"
#include <klocale.h>
#include <kstandarddirs.h>

#include <kurl.h>

#include <config-nepomuk.h>
#ifdef HAVE_NEPOMUK
#include "property.h"
#endif

struct TranslationItem {
    const char* const key;
    const char* const context;
    const char* const value;
};

// TODO: a lot of NFOs are missing yet
static const TranslationItem g_translations[] = {
    { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment", I18N_NOOP2_NOSTRIP("@label", "Comment") },
    { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated", I18N_NOOP2_NOSTRIP("@label creation date", "Created") },
    { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentSize", I18N_NOOP2_NOSTRIP("@label file content size", "Size") },
    { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#depends", I18N_NOOP2_NOSTRIP("@label file depends from", "Depends") },
    { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#isPartOf", I18N_NOOP2_NOSTRIP("@label parent directory", "Part of") },
    { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#lastModified", I18N_NOOP2_NOSTRIP("@label modified date of file", "Modified") },
    { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#mimeType", I18N_NOOP2_NOSTRIP("@label", "MIME Type") },
    { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#plainTextContent", I18N_NOOP2_NOSTRIP("@label", "Content") },
    { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title", I18N_NOOP2_NOSTRIP("@label music title", "Title") },
    { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url", I18N_NOOP2_NOSTRIP("@label file URL", "Location") },
    { "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator", I18N_NOOP2_NOSTRIP("@label", "Creator") },
    { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#averageBitrate", I18N_NOOP2_NOSTRIP("@label", "Average Bitrate") },
    { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#channels", I18N_NOOP2_NOSTRIP("@label", "Channels") },
    { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#characterCount", I18N_NOOP2_NOSTRIP("@label number of characters", "Characters") },
    { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#codec",  I18N_NOOP2_NOSTRIP("@label", "Codec") },
    { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#colorDepth", I18N_NOOP2_NOSTRIP("@label", "Color Depth") },
    { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName", I18N_NOOP2_NOSTRIP("@label", "Filename") },
    { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#height", I18N_NOOP2_NOSTRIP("@label", "Height") },
    { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#interlaceMode", I18N_NOOP2_NOSTRIP("@label", "Interlace Mode") },
    { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#lineCount", I18N_NOOP2_NOSTRIP("@label number of lines", "Lines") },
    { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#programmingLanguage", I18N_NOOP2_NOSTRIP("@label", "Programming Language") },
    { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#sampleRate", I18N_NOOP2_NOSTRIP("@label", "Sample Rate") },
    { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#width", I18N_NOOP2_NOSTRIP("@label", "Width") },
    { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#wordCount", I18N_NOOP2_NOSTRIP("@label number of words", "Words") },
    { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#apertureValue", I18N_NOOP2_NOSTRIP("@label EXIF aperture value", "Aperture") },
    { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureBiasValue", I18N_NOOP2_NOSTRIP("@label EXIF", "Exposure Bias Value") },
    { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureTime", I18N_NOOP2_NOSTRIP("@label EXIF", "Exposure Time") },
    { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#flash", I18N_NOOP2_NOSTRIP("@label EXIF", "Flash") },
    { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLength", I18N_NOOP2_NOSTRIP("@label EXIF", "Focal Length") },
    { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLengthIn35mmFilm", I18N_NOOP2_NOSTRIP("@label EXIF", "Focal Length 35 mm") },
    { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#isoSpeedRatings", I18N_NOOP2_NOSTRIP("@label EXIF", "ISO Speed Ratings") },
    { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#make", I18N_NOOP2_NOSTRIP("@label EXIF", "Make") },
    { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#meteringMode", I18N_NOOP2_NOSTRIP("@label EXIF", "Metering Mode") },
    { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#model", I18N_NOOP2_NOSTRIP("@label EXIF", "Model") },
    { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#orientation", I18N_NOOP2_NOSTRIP("@label EXIF", "Orientation") },
    { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#whiteBalance", I18N_NOOP2_NOSTRIP("@label EXIF", "White Balance") },
    { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#genre",  I18N_NOOP2_NOSTRIP("@label music genre", "Genre") },
    { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#musicAlbum", I18N_NOOP2_NOSTRIP("@label music album", "Album") },
    { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#trackNumber", I18N_NOOP2_NOSTRIP("@label music track number", "Track") },
    { "http://www.w3.org/1999/02/22-rdf-syntax-ns#type", I18N_NOOP2_NOSTRIP("@label file type", "Type") },
    { 0, 0, 0 } // mandatory last entry
};

class KNfoTranslatorSingleton
{
public:
    KNfoTranslator instance;
};
K_GLOBAL_STATIC(KNfoTranslatorSingleton, s_nfoTranslator)

KNfoTranslator& KNfoTranslator::instance()
{
    return s_nfoTranslator->instance;
}

QString KNfoTranslator::translation(const KUrl& uri) const
{
    const QString key = uri.url();
    if (m_hash.contains(key)) {
        return m_hash.value(key);
    }

    // fallback if the URI is not translated
#ifdef HAVE_NEPOMUK
    const QString label = Nepomuk::Types::Property(uri).label();
#else
    QString label;
    const int index = key.indexOf(QChar('#'));
    if (index >= 0) {
        label = key.right(key.size() - index - 1);
    }
#endif
    QString tunedLabel;
    const int labelLength = label.length();
    if (labelLength > 0) {
        tunedLabel.reserve(labelLength);
        tunedLabel = label[0].toUpper();
        for (int i = 1; i < labelLength; ++i) {
            if (label[i].isUpper() && !label[i - 1].isSpace() && !label[i - 1].isUpper()) {
                tunedLabel += ' ';
                tunedLabel += label[i].toLower();
            } else {
                tunedLabel += label[i];
            }
        }
    }
    return tunedLabel;
}

KNfoTranslator::KNfoTranslator() :
    m_hash()
{
    const TranslationItem* item = &g_translations[0];
    while (item->key != 0) {
        m_hash.insert(item->key, i18nc(item->context, item->value));
        ++item;
    }
}

KNfoTranslator::~KNfoTranslator()
{
}
