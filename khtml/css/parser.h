typedef union {
    CSSRuleImpl *rule;
    CSSSelector *selector;
    QPtrList<CSSSelector> *selectorList;
    bool ok;
    MediaListImpl *mediaList;
    CSSMediaRuleImpl *mediaRule;
    CSSRuleListImpl *ruleList;
    ParseString string;
    float val;
    int prop_id;
    int attribute;
    int element;
    CSSSelector::Relation relation;
    bool b;
    char tok;
    Value value;
    ValueList *valueList;
} YYSTYPE;
#define	S	257
#define	SGML_CD	258
#define	INCLUDES	259
#define	DASHMATCH	260
#define	STRING	261
#define	IDENT	262
#define	HASH	263
#define	IMPORT_SYM	264
#define	PAGE_SYM	265
#define	MEDIA_SYM	266
#define	FONT_FACE_SYM	267
#define	CHARSET_SYM	268
#define	KONQ_RULE_SYM	269
#define	KONQ_DECLS_SYM	270
#define	KONQ_VALUE_SYM	271
#define	IMPORTANT_SYM	272
#define	EMS	273
#define	EXS	274
#define	PXS	275
#define	CMS	276
#define	MMS	277
#define	INS	278
#define	PTS	279
#define	PCS	280
#define	DEGS	281
#define	RADS	282
#define	GRADS	283
#define	MSECS	284
#define	SECS	285
#define	HERZ	286
#define	KHERZ	287
#define	DIMEN	288
#define	PERCENTAGE	289
#define	NUMBER	290
#define	URI	291
#define	FUNCTION	292
#define	UNICODERANGE	293

