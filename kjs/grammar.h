typedef union {
  int                 ival;
  double              dval;
  UString             *ustr;
  void                *rxp; /* TODO */
  Node                *node;
  StatementNode       *stat;
  ParameterNode       *param;
  FuncDeclNode        *func;
  ProgramNode         *prog;
  AssignExprNode      *init;
  SourceElementNode   *src;
  SourceElementsNode  *srcs;
  StatListNode        *slist;
  ArgumentsNode       *args;
  ArgumentListNode    *alist;
  VarDeclNode         *decl;
  VarDeclListNode     *vlist;
  CaseBlockNode       *cblk;
  ClauseListNode      *clist;
  CaseClauseNode      *ccl;
  ElementNode         *elm;
  ElisionNode         *eli;
  Operator            op;
} YYSTYPE;
#define	NULLTOKEN	258
#define	TRUETOKEN	259
#define	FALSETOKEN	260
#define	STRING	261
#define	DECIMAL	262
#define	INTEGER	263
#define	REGEXP	264
#define	BREAK	265
#define	CASE	266
#define	DEFAULT	267
#define	FOR	268
#define	NEW	269
#define	VAR	270
#define	CONTINUE	271
#define	FUNCTION	272
#define	RETURN	273
#define	VOID	274
#define	DELETE	275
#define	IF	276
#define	THIS	277
#define	DO	278
#define	WHILE	279
#define	ELSE	280
#define	IN	281
#define	INSTANCEOF	282
#define	TYPEOF	283
#define	SWITCH	284
#define	WITH	285
#define	RESERVED	286
#define	THROW	287
#define	TRY	288
#define	CATCH	289
#define	FINALLY	290
#define	EQEQ	291
#define	NE	292
#define	STREQ	293
#define	STRNEQ	294
#define	LE	295
#define	GE	296
#define	OR	297
#define	AND	298
#define	PLUSPLUS	299
#define	MINUSMINUS	300
#define	LSHIFT	301
#define	RSHIFT	302
#define	URSHIFT	303
#define	PLUSEQUAL	304
#define	MINUSEQUAL	305
#define	MULTEQUAL	306
#define	DIVEQUAL	307
#define	LSHIFTEQUAL	308
#define	RSHIFTEQUAL	309
#define	URSHIFTEQUAL	310
#define	ANDEQUAL	311
#define	MODEQUAL	312
#define	XOREQUAL	313
#define	OREQUAL	314
#define	DOUBLE	315
#define	IDENT	316
#define	AUTO	317


extern YYSTYPE kjsyylval;
