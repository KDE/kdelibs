/************************/
/* Internal definitions */
/************************/

D       [0-9]
L       [a-zA-Z_]
W	[[:alnum:]_-]+
SP      [ \t]
NL	(\r)?\n
EX	(send|status|cancel|convert|filter|verify|filetype)

%{
#include "pdq.h"
#ifndef KDE_USE_FINAL
#include "parser.cpp.h"
#endif
#include <glob.h>
#include <qvaluestack.h>

static int brace_count = 0;
static int expand_include = 1;
static QValueStack<YY_BUFFER_STATE>	handle_stack;
static QValueStack<FILE*>		file_stack;

static void newStr(const char* s, bool b = false);
static void do_try_include(const char* s);
static int do_eof();
static void do_switch();
%}

%x exec_blk exec_start incl

%%

"#".*$			{ return COMMENT; }

"driver_opts"		{ newStr(yytext); return DRIVER_OPTS; }
"driver_args"		{ newStr(yytext); return DRIVER_ARGS; }
"interface_opts"	{ newStr(yytext); return INTERFACE_OPTS; }
"interface_args"	{ newStr(yytext); return INTERFACE_ARGS; }
"driver_command_path"	{ newStr(yytext); return DRIVER_COMMAND_PATH; }
"interface_command_path"	{ newStr(yytext); return INTERFACE_COMMAND_PATH; }
"default_printer"	{ newStr(yytext); return DEFAULT_PRINTER; }
"printer"		{ newStr(yytext); return PRINTER; }
"driver"		{ newStr(yytext); return DRIVER; }
"interface"		{ newStr(yytext); return INTERFACE; }
"location"		{ newStr(yytext); return LOCATION; }
"model"			{ newStr(yytext); return MODEL; }
"try_include"		{ if (expand_include) BEGIN(incl); else { newStr(yytext); return TRY_INCLUDE; } }
"include"		{ if (expand_include) BEGIN(incl); else { newStr(yytext); return INCLUDE; } }
"requires"		{ newStr(yytext); return REQUIRES; }
"required_args"		{ newStr(yytext); return REQUIRED_ARGS; }
"choice"		{ newStr(yytext); return CHOICE; }
"var"			{ newStr(yytext); return VAR; }
"desc"			{ newStr(yytext); return DESC; }
"help"			{ newStr(yytext); return HELP; }
"option"		{ newStr(yytext); return OPTION; }
"argument"		{ newStr(yytext); return ARGUMENT; }
"def_value"		{ newStr(yytext); return DEF_VALUE; }
"default_choice"	{ newStr(yytext); return DEFAULT_CHOICE; }
"value"			{ newStr(yytext); return VALUE; }
"language_driver"	{ newStr(yytext); return LANGUAGE_DRIVER; }
"filetype_regx"		{ newStr(yytext); return FILETYPE_REGX; }
"job_dir"		{ newStr(yytext); return JOB_DIR; }

{EX}"_exec"		{ newStr(yytext); BEGIN(exec_start); return SCRIPT_START; }
<exec_start>"{"		{ BEGIN(exec_blk); return '{'; }
<exec_blk>"{"		{ brace_count++; yymore(); }
<exec_blk>"}"		{ if (brace_count == 0) { BEGIN(INITIAL); yyless(yyleng-1); newStr(yytext); return SCRIPT_BODY; } else { brace_count--; yymore(); } }
<exec_blk>[^{}]*	{ yymore(); }

"{"			{ return '{'; }
"}"			{ return '}'; }
<INITIAL,exec_start>"="	{ return '='; }
","			{ return ','; }

<incl>\"((\\\")|([^"]))*\"	{ do_try_include(yytext); BEGIN(INITIAL); }
<incl>[ \t]*			/* eat up */

\"((\\\")|([^"]))*\"		{ newStr(yytext,1); return STRING; }
{W}			{ newStr(yytext); return IDENTIFIER; }

<<EOF>>			{ if (do_eof()) yyterminate(); }

<*>.|\n			/* eat all other characters in all start conditions */

%%

int yywrap()
{
	return 1;
}

void initPdqFlex(const QString& filename, bool expandincl)
{
	expand_include = (expandincl ? 1 : 0);
	FILE	*f = fopen(filename.latin1(),"r");
	if (f)
	{
		YY_BUFFER_STATE	st = yy_create_buffer( f, YY_BUF_SIZE );
		handle_stack.push(st);
		file_stack.push(f);

		do_switch();
	}
}

void newStr(const char* s, bool b)
{
	yylval.string = new QString(s);
	if (b)
	{
		*(yylval.string) = yylval.string->mid(1,yylval.string->length()-2);
	}
}

void do_try_include(const char *s)
{
	QString	str(s);
	str.replace(QRegExp("\""),"");
	glob_t	gl;
	if (glob(str.latin1(),0,NULL,&gl) == 0)
	{
		for (uint i=0;i<gl.gl_pathc;i++)
		{
			FILE	*f = fopen(gl.gl_pathv[i],"r");
			if (f)
			{
				YY_BUFFER_STATE	st = yy_create_buffer( f, YY_BUF_SIZE );
				handle_stack.push(st);
				file_stack.push(f);
			}
		}
		do_switch();
	}
}

int do_eof()
{
	YY_BUFFER_STATE	st = handle_stack.pop();
	FILE	*f = file_stack.pop();
	fclose(f);
	yy_delete_buffer(st);
	if (handle_stack.count() == 0)
	{
		return 1;
	}
	else
	{
		do_switch();
		return 0;
	}
}

void do_switch()
{
	if (handle_stack.count() > 0)
		yy_switch_to_buffer( handle_stack.top() );
}
