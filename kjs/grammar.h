#ifndef BISON_GRAMMAR_TAB_H
# define BISON_GRAMMAR_TAB_H

#ifndef YYSTYPE
typedef union {
  int                 ival;
  double              dval;
  UString             *ustr;
  Identifier          *ident;
  Node                *node;
  StatementNode       *stat;
  ParameterNode       *param;
  FunctionBodyNode    *body;
  FuncDeclNode        *func;
  FunctionBodyNode    *prog;
  AssignExprNode      *init;
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
  Operator            op;
  PropertyValueNode   *plist;
  PropertyNode        *pnode;
  CatchNode           *cnode;
  FinallyNode         *fnode;
  VarStatementNode::Type vtype;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif

#ifndef YYLTYPE
typedef struct yyltype
{
  int first_line;
  int first_column;

  int last_line;
  int last_column;
} yyltype;

# define YYLTYPE yyltype
# define YYLTYPE_IS_TRIVIAL 1
#endif

# define	NULLTOKEN	257
# define	TRUETOKEN	258
# define	FALSETOKEN	259
# define	STRING	260
# define	NUMBER	261
# define	BREAK	262
# define	CASE	263
# define	DEFAULT	264
# define	FOR	265
# define	NEW	266
# define	VAR	267
# define	CONST	268
# define	CONTINUE	269
# define	FUNCTION	270
# define	RETURN	271
# define	VOID	272
# define	DELETE	273
# define	IF	274
# define	THIS	275
# define	DO	276
# define	WHILE	277
# define	ELSE	278
# define	IN	279
# define	INSTANCEOF	280
# define	TYPEOF	281
# define	SWITCH	282
# define	WITH	283
# define	RESERVED	284
# define	THROW	285
# define	TRY	286
# define	CATCH	287
# define	FINALLY	288
# define	EQEQ	289
# define	NE	290
# define	STREQ	291
# define	STRNEQ	292
# define	LE	293
# define	GE	294
# define	OR	295
# define	AND	296
# define	PLUSPLUS	297
# define	MINUSMINUS	298
# define	LSHIFT	299
# define	RSHIFT	300
# define	URSHIFT	301
# define	PLUSEQUAL	302
# define	MINUSEQUAL	303
# define	MULTEQUAL	304
# define	DIVEQUAL	305
# define	LSHIFTEQUAL	306
# define	RSHIFTEQUAL	307
# define	URSHIFTEQUAL	308
# define	ANDEQUAL	309
# define	MODEQUAL	310
# define	XOREQUAL	311
# define	OREQUAL	312
# define	IDENT	313
# define	AUTOPLUSPLUS	314
# define	AUTOMINUSMINUS	315


extern YYSTYPE kjsyylval;

#endif /* not BISON_GRAMMAR_TAB_H */
