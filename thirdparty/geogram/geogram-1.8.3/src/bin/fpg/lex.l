D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*
S           [ \t\v\n\f]*


%{
#include <cstring>
#include <cstdio>
#include <FPG/AST.h>
#include <FPG/Declarator.h>
#include <FPG/SymbolEnvironment.h>

struct Group_rep;

#include "parse.tab.h"

extern void yyerror(char *s);
extern int lineno;
void count();
void comment();
int caller;

%}

%s LINE_DIRECTIVE_LINENO LINE_DIRECTIVE_FILENAME REST_OF_LINE
%%

"#"{S}          { BEGIN(LINE_DIRECTIVE_LINENO); }
<LINE_DIRECTIVE_LINENO>{D}+  {
                  AST::current_location.line = atoi(yytext); BEGIN(LINE_DIRECTIVE_FILENAME); }
<LINE_DIRECTIVE_FILENAME>{S}\"(\\.|[^\\"])*\" {
                  AST::current_location.filename = yytext; BEGIN(REST_OF_LINE); }
<REST_OF_LINE>.*[\n\r]+ { count(); BEGIN(INITIAL); }

"/*"			{ comment(); }

"double"		{ count(); return(DOUBLE); }
"else"			{ count(); return(ELSE);   }
"float"			{ count(); return(FLOAT);  }
"if"			{ count(); return(IF);     }
"int"			{ count(); return(INT);    }
"bool"			{ count(); return(BOOL);   }
"return"		{ count(); return(RETURN); }
"void"			{ count(); return(VOID);   }
"sign"          { count(); return(SIGN);   }
"abs"           { count(); return(ABS);    }
"sqrt"          { count(); return(SQRT);   }
"typedef"       { count(); return(TYPEDEF);}
"inline"        { count(); return(INLINE); }
"enum"          { count(); return(ENUM);   }
"false"         { count(); yylval.int_const = 0; return(INT_CONSTANT); }
"true"          { count(); yylval.int_const = 1; return(INT_CONSTANT); }
"extern"        { count(); return(EXTERN); }
"exact"         { count(); return(EXACT); }
"group"         { count(); return(GROUP); }
"degree"        { count(); return(DEGREE); }
"compare"       { count(); return(COMPARE); }

{L}({L}|{D})*   { count();
                  Type *t = symbol_env.findType( yytext );
                  if( t != NULL ) {
                      yylval.type = t;
                      return USER_TYPE;
                  } else {
                      yylval.string_const = strdup( yytext );
                      return IDENTIFIER;
                  }
                }

"-"?{D}+  		{ count(); yylval.int_const = atoi(yytext);
                          return(INT_CONSTANT); }


"-"?{D}+{E}{FS}?		{ count(); yylval.float_const = atof(yytext);
                          return(FLOAT_CONSTANT); }
"-"?{D}*"."{D}+({E})?{FS}?	{ count(); yylval.float_const = atof(yytext);
                          return(FLOAT_CONSTANT); }
"-"?{D}+"."{D}*({E})?{FS}?	{ count(); yylval.float_const = atof(yytext);
                          return(FLOAT_CONSTANT); }

\"(\\.|[^\\"])*\"	{ count(); yylval.string_const = strdup( yytext );
                        return(STRING_LITERAL); }

">>="			{ count(); return(RIGHT_ASSIGN); }
"<<="			{ count(); return(LEFT_ASSIGN); }
"+="			{ count(); return(ADD_ASSIGN); }
"-="			{ count(); return(SUB_ASSIGN); }
"*="			{ count(); return(MUL_ASSIGN); }
"/="			{ count(); return(DIV_ASSIGN); }
"%="			{ count(); return(MOD_ASSIGN); }
"&="			{ count(); return(AND_ASSIGN); }
"^="			{ count(); return(XOR_ASSIGN); }
"|="			{ count(); return(OR_ASSIGN); }
">>"			{ count(); return(RIGHT_OP); }
"<<"			{ count(); return(LEFT_OP); }
"++"			{ count(); return(INC_OP); }
"--"			{ count(); return(DEC_OP); }
"->"			{ count(); return(PTR_OP); }
"&&"			{ count(); return(AND_OP); }
"||"			{ count(); return(OR_OP); }
"<="			{ count(); return(LE_OP); }
">="			{ count(); return(GE_OP); }
"=="			{ count(); return(EQ_OP); }
"!="			{ count(); return(NE_OP); }
";"			{ count(); return(';'); }
("{"|"<%")		{ count(); return('{'); }
("}"|"%>")		{ count(); return('}'); }
","			{ count(); return(','); }
":"			{ count(); return(':'); }
"="			{ count(); return('='); }
"("			{ count(); return('('); }
")"			{ count(); return(')'); }
("["|"<:")		{ count(); return('['); }
("]"|":>")		{ count(); return(']'); }
"."			{ count(); return('.'); }
"&"			{ count(); return('&'); }
"!"			{ count(); return('!'); }
"~"			{ count(); return('~'); }
"-"			{ count(); return('-'); }
"+"			{ count(); return('+'); }
"*"			{ count(); return('*'); }
"/"			{ count(); return('/'); }
"%"			{ count(); return('%'); }
"<"			{ count(); return('<'); }
">"			{ count(); return('>'); }
"^"			{ count(); return('^'); }
"|"			{ count(); return('|'); }
"?"			{ count(); return('?'); }

[ \t\v\n\f]		{ count(); }

.			{ count(); yyerror((char*)("bad character")); }

%%


int yywrap() {
  return 1;
}

void comment() {
    char c, c1;
loop:
    while ((c = (char)yyinput()) != '*' && c != 0)
        /*putchar(c)*/;

    if ((c1 = (char)yyinput()) != '/' && c != 0)
    {
        //unput(c1);
        goto loop;
    }

    if (c != 0) {
    }
    /*putchar(c1) ;*/
}


int column = 0;

void count() {
    int i;

    for (i = 0; yytext[i] != '\0'; ++i)
        if (yytext[i] == '\n') {
            column = 0;
            ++lineno;
            ++AST::current_location.line;
        }
        else if (yytext[i] == '\t')
            column += 8 - (column % 8);
        else
            column++;
    AST::current_location.column = column;
}

int foo() { return yyunput(42,0),2; }

