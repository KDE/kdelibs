/*
 * numbers.h: Implementation of the XSLT number functions
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 * Bjorn Reese <breese@users.sourceforge.net>
 */

#ifndef __XML_XSLT_NUMBERSINTERNALS_H__
#define __XML_XSLT_NUMBERSINTERNALS_H__

#include <libxml/tree.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * xsltNumberData:
 *
 * This data structure is just a wrapper to pass xsl:number data in
 */
typedef struct _xsltNumberData {
    xmlChar *level;
    xmlChar *count;
    xmlChar *from;
    xmlChar *value;
    xmlChar *format;
    int has_format;
    int digitsPerGroup;
    xmlChar groupingCharacter;
    xmlDocPtr doc;
    xmlNodePtr node;
} xsltNumberData, *xsltNumberDataPtr;

/**
 * xsltFormatNumberInfo,:
 *
 * This data structure lists the various parameters needed to format numbers
 */
typedef struct _xsltFormatNumberInfo {
    int	    integer_digits;	/* Number of '0' in integer part */
    int	    frac_digits;	/* Number of '0' in fractional part */
    int	    frac_hash;		/* Number of '#' in fractional part */
    int	    group;		/* Number of chars per display 'group' */
    int     multiplier;		/* Scaling for percent or permille */
    char    is_multiplier_set;	/* Flag to catch multiple occurences of percent/permille */
    char    is_negative_pattern;/* Flag for processing -ve prefix/suffix */
} xsltFormatNumberInfo, *xsltFormatNumberInfoPtr;

#ifdef __cplusplus
}
#endif
#endif /* __XML_XSLT_NUMBERSINTERNALS_H__ */
