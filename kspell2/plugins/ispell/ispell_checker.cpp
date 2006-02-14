/* vim: set sw=8: -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* kspell2 - adopted from Enchant
 * Copyright (C) 2003 Dom Lachowicz
 * Copyright (C) 2004 Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * In addition, as a special exception, Dom Lachowicz
 * gives permission to link the code of this program with
 * non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU Lesser General Public License in all
 * respects for all of the code used other than said providers.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "sp_spell.h"
#include "ispell_checker.h"

#include <qmap.h>
#include <qdir.h>
#include <qfileinfo.h>

/***************************************************************************/

typedef struct str_ispell_map
{
	const char * lang;
	const char * dict;
	const char * enc;
} IspellMap;

static const char *ispell_dirs [] = {
	"/usr/lib/ispell",
	"/usr/local/lib/ispell",
	"/usr/local/share/ispell",
	"/usr/share/ispell",
	0
};
static const IspellMap ispell_map [] = {
	{"ca"    ,"catala.hash"         ,"iso-8859-1" },
	{"ca_ES" ,"catala.hash"         ,"iso-8859-1" },
	{"cs"    ,"czech.hash"          ,"iso-8859-2" },
	{"cs_CZ" ,"czech.hash"          ,"iso-8859-2" },
	{"da"    ,"dansk.hash"          ,"iso-8859-1" },
	{"da_DK" ,"dansk.hash"          ,"iso-8859-1" },
	{"de"    ,"deutsch.hash"        ,"iso-8859-1" },
	{"de_CH" ,"swiss.hash"          ,"iso-8859-1" },
	{"de_AT" ,"deutsch.hash"        ,"iso-8859-1" },
	{"de_DE" ,"deutsch.hash"        ,"iso-8859-1" },
	{"el"    ,"ellhnika.hash"       ,"iso-8859-7" },
	{"el_GR" ,"ellhnika.hash"       ,"iso-8859-7" },
	{"en"    ,"british.hash"        ,"iso-8859-1" },
	{"en_AU" ,"british.hash"        ,"iso-8859-1" },
	{"en_BZ" ,"british.hash"        ,"iso-8859-1" },
	{"en_CA" ,"british.hash"        ,"iso-8859-1" },
	{"en_GB" ,"british.hash"        ,"iso-8859-1" },
	{"en_IE" ,"british.hash"        ,"iso-8859-1" },
	{"en_JM" ,"british.hash"        ,"iso-8859-1" },
	{"en_NZ" ,"british.hash"        ,"iso-8859-1" },
	{"en_TT" ,"british.hash"        ,"iso-8859-1" },
	{"en_ZA" ,"british.hash"        ,"iso-8859-1" },
	{"en_ZW" ,"british.hash"        ,"iso-8859-1" },
	{"en_PH" ,"american.hash"       ,"iso-8859-1" },
	{"en_US" ,"american.hash"       ,"iso-8859-1" },
	{"eo"    ,"esperanto.hash"      ,"iso-8859-3" },
	{"es"    ,"espanol.hash"        ,"iso-8859-1" },
	{"es_AR" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_BO" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_CL" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_CO" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_CR" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_DO" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_EC" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_ES" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_GT" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_HN" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_MX" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_NI" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_PA" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_PE" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_PR" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_PY" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_SV" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_UY" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_VE" ,"espanol.hash"        ,"iso-8859-1" },
	{"fi"    ,"finnish.hash"        ,"iso-8859-1" },
	{"fi_FI" ,"finnish.hash"        ,"iso-8859-1" },
	{"fr"    ,"francais.hash"       ,"iso-8859-1" },
	{"fr_BE" ,"francais.hash"       ,"iso-8859-1" },
	{"fr_CA" ,"francais.hash"       ,"iso-8859-1" },
	{"fr_CH" ,"francais.hash"       ,"iso-8859-1" },
	{"fr_FR" ,"francais.hash"       ,"iso-8859-1" },
	{"fr_LU" ,"francais.hash"       ,"iso-8859-1" },
	{"fr_MC" ,"francais.hash"       ,"iso-8859-1" },
	{"hu"    ,"hungarian.hash"      ,"iso-8859-2" },
	{"hu_HU" ,"hungarian.hash"      ,"iso-8859-2" },
	{"ga"    ,"irish.hash"          ,"iso-8859-1" },
	{"ga_IE" ,"irish.hash"          ,"iso-8859-1" },
	{"gl"    ,"galician.hash"       ,"iso-8859-1" },
	{"gl_ES" ,"galician.hash"       ,"iso-8859-1" },
	{"ia"    ,"interlingua.hash"    ,"iso-8859-1" },
	{"it"    ,"italian.hash"        ,"iso-8859-1" },
	{"it_IT" ,"italian.hash"        ,"iso-8859-1" },
	{"it_CH" ,"italian.hash"        ,"iso-8859-1" },
	{"la"    ,"mlatin.hash"         ,"iso-8859-1" },
	{"la_IT" ,"mlatin.hash"         ,"iso-8859-1" },
	{"lt"    ,"lietuviu.hash"       ,"iso-8859-13" },
	{"lt_LT" ,"lietuviu.hash"       ,"iso-8859-13" },
	{"nl"    ,"nederlands.hash"     ,"iso-8859-1" },
	{"nl_NL" ,"nederlands.hash"     ,"iso-8859-1" },
	{"nl_BE" ,"nederlands.hash"     ,"iso-8859-1" },
	{"nb"    ,"norsk.hash"          ,"iso-8859-1" },
	{"nb_NO" ,"norsk.hash"          ,"iso-8859-1" },
	{"nn"    ,"nynorsk.hash"        ,"iso-8859-1" },
	{"nn_NO" ,"nynorsk.hash"        ,"iso-8859-1" },
	{"no"    ,"norsk.hash"          ,"iso-8859-1" },
	{"no_NO" ,"norsk.hash"          ,"iso-8859-1" },
	{"pl"    ,"polish.hash"         ,"iso-8859-2" },
	{"pl_PL" ,"polish.hash"         ,"iso-8859-2" },
	{"pt"    ,"brazilian.hash"      ,"iso-8859-1" },
	{"pt_BR" ,"brazilian.hash"      ,"iso-8859-1" },
	{"pt_PT" ,"portugues.hash"      ,"iso-8859-1" },
	{"ru"    ,"russian.hash"        ,"koi8-r" },
	{"ru_MD" ,"russian.hash"        ,"koi8-r" },
	{"ru_RU" ,"russian.hash"        ,"koi8-r" },
	{"sc"    ,"sardinian.hash"      ,"iso-8859-1" },
	{"sc_IT" ,"sardinian.hash"      ,"iso-8859-1" },
	{"sk"    ,"slovak.hash"         ,"iso-8859-2" },
	{"sk_SK" ,"slovak.hash"         ,"iso-8859-2" },
	{"sl"    ,"slovensko.hash"      ,"iso-8859-2" },
	{"sl_SI" ,"slovensko.hash"      ,"iso-8859-2" },
	{"sv"    ,"svenska.hash"        ,"iso-8859-1" },
	{"sv_SE" ,"svenska.hash"        ,"iso-8859-1" },
	{"uk"    ,"ukrainian.hash"      ,"koi8-u" },
	{"uk_UA" ,"ukrainian.hash"      ,"koi8-u" },
	{"yi"    ,"yiddish-yivo.hash"   ,"utf-8" }
};

static const size_t size_ispell_map = ( sizeof(ispell_map) / sizeof((ispell_map)[0]) );
static QMap<QString, QString> ispell_dict_map;


void
ISpellChecker::try_autodetect_charset(const char * const inEncoding)
{
	if (inEncoding && strlen(inEncoding))
		{
			m_translate_in = QTextCodec::codecForName(inEncoding);
		}
}

/***************************************************************************/
/***************************************************************************/

ISpellChecker::ISpellChecker()
	: deftflag(-1),
     prefstringchar(-1),
     m_bSuccessfulInit(false),
     m_BC(NULL),
     m_cd(NULL),
     m_cl(NULL),
     m_cm(NULL),
     m_ho(NULL),
     m_nd(NULL),
     m_so(NULL),
     m_se(NULL),
     m_ti(NULL),
     m_te(NULL),
     m_hashstrings(NULL),
     m_hashtbl(NULL),
     m_pflaglist(NULL),
     m_sflaglist(NULL),
     m_chartypes(NULL),
     m_infile(NULL),
     m_outfile(NULL),
     m_askfilename(NULL),
     m_Trynum(0),
     m_translate_in(0)
{
	memset(m_sflagindex,0,sizeof(m_sflagindex));
	memset(m_pflagindex,0,sizeof(m_pflagindex));
}

#ifndef FREEP
#define FREEP(p)        do { if (p) free(p); } while (0)
#endif

ISpellChecker::~ISpellChecker()
{
	if (m_bSuccessfulInit) {
		// only cleanup our mess if we were successfully initialized

		clearindex (m_pflagindex);
		clearindex (m_sflagindex);
	}

	FREEP(m_hashtbl);
	FREEP(m_hashstrings);
	FREEP(m_sflaglist);
	FREEP(m_chartypes);
}

bool
ISpellChecker::checkWord( const QString& utf8Word )
{
	ichar_t iWord[INPUTWORDLEN + MAXAFFIXLEN];
	if (!m_bSuccessfulInit)
		return false;

	if (utf8Word.isNull() || utf8Word.length() >= (INPUTWORDLEN + MAXAFFIXLEN) || utf8Word.isEmpty())
		return false;

	bool retVal = false;
	QByteArray out;
	if (!m_translate_in)
		return false;
	else {
		/* convert to 8bit string and null terminate */
		int len_out = utf8Word.length();

		out = m_translate_in->fromUnicode( utf8Word, len_out );
	}

	if (!strtoichar(iWord, out.data(), INPUTWORDLEN + MAXAFFIXLEN, 0))
		{
			if (good(iWord, 0, 0, 1, 0) == 1 ||
			    compoundgood(iWord, 1) == 1)
				{
					retVal = true;
				}
		}

	return retVal;
}

QStringList
ISpellChecker::suggestWord(const QString& utf8Word)
{
	ichar_t  iWord[INPUTWORDLEN + MAXAFFIXLEN];
	int  c;

	if (!m_bSuccessfulInit)
		return QStringList();

	if (utf8Word.isEmpty() || utf8Word.length() >= (INPUTWORDLEN + MAXAFFIXLEN) ||
			utf8Word.length() == 0)
		return QStringList();

	QByteArray out;
	if (!m_translate_in)
		return QStringList();
	else
		{
			/* convert to 8bit string and null terminate */

			int len_out = utf8Word.length();
			out = m_translate_in->fromUnicode( utf8Word, len_out );
		}

	if (!strtoichar(iWord, out.data(), INPUTWORDLEN + MAXAFFIXLEN, 0))
		makepossibilities(iWord);
	else
		return QStringList();

	QStringList sugg_arr;
	for (c = 0; c < m_pcount; c++)
	{
		QString utf8Word;

		if (!m_translate_in)
		{
			/* copy to 8bit string and null terminate */
			utf8Word = QString::fromUtf8( m_possibilities[c] );
		}
		else
		{
			/* convert to 32bit string and null terminate */
			utf8Word = m_translate_in->toUnicode( m_possibilities[c] );
		}

		sugg_arr.append( utf8Word );
	}

	return sugg_arr;
}

static void
s_buildHashNames (std::vector<std::string> & names, const char * dict)
{
	const char * tmp = 0;
	int i = 0;

	names.clear ();

	while ( (tmp = ispell_dirs[i++]) ) {
		QByteArray maybeFile = QByteArray( tmp ) + '/';
		maybeFile += dict;
		names.push_back( maybeFile.data() );
	}
}

static void
s_allDics()
{
	const char * tmp = 0;
	int i = 0;

	while ( (tmp = ispell_dirs[i++]) ) {
		QDir dir( tmp );
		QStringList lst = dir.entryList( "*.hash" );
		for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
			QFileInfo info( *it );
			for (size_t i = 0; i < size_ispell_map; i++)
			{
				const IspellMap * mapping = (const IspellMap *)(&(ispell_map[i]));
				if (!strcmp (info.fileName().toLatin1(), mapping->dict))
				{
					ispell_dict_map.insert( mapping->lang, *it );
				}
			}
		}
	}
}

QStringList
ISpellChecker::allDics()
{
	if ( ispell_dict_map.empty() )
		s_allDics();

	return ispell_dict_map.keys();
}

QString
ISpellChecker::loadDictionary (const char * szdict)
{
	std::vector<std::string> dict_names;

	s_buildHashNames (dict_names, szdict);

	for (size_t i = 0; i < dict_names.size(); i++)
		{
			if (linit(const_cast<char*>(dict_names[i].c_str())) >= 0)
				return dict_names[i].c_str();
		}

	return QString();
}

/*!
 * Load ispell dictionary hash file for given language.
 *
 * \param szLang -  The language tag ("en-US") we want to use
 * \return The name of the dictionary file
 */
bool
ISpellChecker::loadDictionaryForLanguage ( const char * szLang )
{
	QString hashname;

	const char * encoding = NULL;
	const char * szFile = NULL;

	for (size_t i = 0; i < size_ispell_map; i++)
		{
			const IspellMap * mapping = (const IspellMap *)(&(ispell_map[i]));
			if (!strcmp (szLang, mapping->lang))
				{
					szFile = mapping->dict;
					encoding = mapping->enc;
					break;
				}
		}

	if (!szFile || !strlen(szFile))
		return false;

	alloc_ispell_struct();

	hashname = loadDictionary(szFile);
	if (hashname.isEmpty())
		return false;

	// one of the two above calls succeeded
	setDictionaryEncoding (hashname, encoding);

	return true;
}

void
ISpellChecker::setDictionaryEncoding( const QString& hashname, const char * encoding )
{
	/* Get Hash encoding from XML file. This should always work! */
	try_autodetect_charset(encoding);

	if (m_translate_in)
		{
			/* We still have to setup prefstringchar*/
			prefstringchar = findfiletype("utf8", 1, deftflag < 0 ? &deftflag
						      : static_cast<int *>(NULL));

			if (prefstringchar < 0)
				{
					std::string teststring;
					for(int n1 = 1; n1 <= 15; n1++)
						{
							teststring = "latin" + n1;
							prefstringchar = findfiletype(teststring.c_str(), 1,
										      deftflag < 0 ? &deftflag : static_cast<int *>(NULL));
							if (prefstringchar >= 0)
								break;
						}
				}

			return; /* success */
		}

	/* Test for UTF-8 first */
	prefstringchar = findfiletype("utf8", 1, deftflag < 0 ? &deftflag : static_cast<int *>(NULL));
	if (prefstringchar >= 0)
		{
			m_translate_in = QTextCodec::codecForName("utf8");
		}

	if (m_translate_in)
		return; /* success */

	/* Test for "latinN" */
	if (!m_translate_in)
		{
			/* Look for "altstringtype" names from latin1 to latin15 */
			for(int n1 = 1; n1 <= 15; n1++)
				{
					QString teststring = QString("latin%1").arg(n1);
					prefstringchar = findfiletype(teststring.toLatin1(), 1,
								      deftflag < 0 ? &deftflag : static_cast<int *>(NULL));
					if (prefstringchar >= 0)
						{
							//FIXME: latin1 might be wrong
							m_translate_in = QTextCodec::codecForName( teststring.toLatin1() );
							break;
						}
				}
		}

	/* If nothing found, use latin1 */
	if (!m_translate_in)
		{
			m_translate_in = QTextCodec::codecForName("latin1");
		}
}

bool
ISpellChecker::requestDictionary(const char *szLang)
{
	if (!loadDictionaryForLanguage (szLang))
		{
			// handle a shortened version of the language tag: en_US => en
			std::string shortened_dict (szLang);
			size_t uscore_pos;

			if ((uscore_pos = shortened_dict.rfind ('_')) != ((size_t)-1)) {
				shortened_dict = shortened_dict.substr(0, uscore_pos);
				if (!loadDictionaryForLanguage (shortened_dict.c_str()))
					return false;
			} else
				return false;
		}

	m_bSuccessfulInit = true;

	if (prefstringchar < 0)
		m_defdupchar = 0;
	else
		m_defdupchar = prefstringchar;

	return true;
}
