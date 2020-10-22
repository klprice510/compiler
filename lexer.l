/*  Intermediate Code
 *  Homework 4
 *  CS 445, Dr. Jeffery
 *  Nov. 24, 2014
 *  Kathryn Price
 */

/*  $Id: lexer.l,v 1.2 1997/11/19 15:13:15 sandro Exp $ */

/*
 * Copyright (c) 1997 Sandro Sigala <ssigala@globalnet.it>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * ISO C++ lexical analyzer.
 *
 * Based on the ISO C++ draft standard of December '96.
 */

%{
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "120++.h"
#include "parser.tab.h"

int lineno = 1;
int yywrap(void);
static int check_identifier(char *);
void print_ident_list(void);

extern YYSTYPE yylval;
extern int yychar;
struct tokenlist *tlist = 0;
struct token *yytoken = 0;
void lexerr(char *s);
int handle_include();
int newfile(char *fn);
int popfile(void);
int set_yytoken(int cat, char *txt);
int hash_key(char *s);
unsigned int DJBHash(char* str, unsigned int len);
struct typenametable_entry *createNode(char *s, int cat);
void insertToHash(char *s, int cat);
int searchInHash(char *s);
int check_type_exceptions(char *s);

int errors;
int StructKeywordSeen = 0; 
int namespace_found = 0;
/* include standard .h file flags: Soule pg. 134 */
int included_iostream = 0;
int included_cstdlib = 0;
int included_cmath = 0;
int included_ctime = 0;
int included_cstring = 0;
int included_fstream = 0;
int included_string = 0;
int included_iomanip = 0;

/* Nested includes code built from an
 * example in John Levine's "flex & bison" 
 * published by O'Reilly */
struct bufstack
{
  struct bufstack *prev; /* previous entry */
  YY_BUFFER_STATE bs;    /* saved buffer */
  int lineno;            /* saved line number */
  char *filename;        /* name of this file */
  FILE *f;               /* current file */
} *curbs = 0;

char *curfilename;       /* name of current input file */

/* Linked list code built from an example
 * in John Levine's (et al) "lex & yacc"
 * published by O'Reilly */

int add_token(struct token *tp);

%}

intsuffix                               ([uU][lL]?)|([lL][uU]?)
fracconst                               ([0-9]*\.[0-9]+)|([0-9]+\.)
exppart                                 [eE][-+]?[0-9]+
floatsuffix                             [fFlL]
chartext                                ([^'])|(\\.)
stringtext                              ([^"])|(\\.)

%%

"\n"                { ++lineno; }
[\t\f\v\r ]+        { /* Ignore whitespace. */ }
"{"                 { return set_yytoken(LC, yytext); }
"<%"                { return set_yytoken(LC, yytext); }
"}"                 { return set_yytoken(RC, yytext); }
"%>"                { return set_yytoken(RC, yytext); }
"["                 { return set_yytoken(LB, yytext); }
"<:"                { return set_yytoken(LB, yytext); }
"]"                 { return set_yytoken(RB, yytext); }
":>"                { return set_yytoken(RB, yytext); }
"("                 { return set_yytoken(LP, yytext); }
")"                 { return set_yytoken(RP, yytext); }
";"                 { return set_yytoken(SM, yytext); }
":"                 { return set_yytoken(COLON, yytext); }
"..."               { return set_yytoken(ELLIPSIS, yytext); }
"?"                 { return set_yytoken(QUEST, yytext); }
"::"                { return set_yytoken(COLONCOLON, yytext); }
"."                 { return set_yytoken(DOT, yytext); }
".*"                { return set_yytoken(DOTSTAR, yytext); }
"+"                 { return set_yytoken(PLUS, yytext); }
"-"                 { return set_yytoken(MINUS, yytext); }
"*"                 { return set_yytoken(STAR, yytext); }
"/"                 { return set_yytoken(DIV, yytext); }
"%"                 { return set_yytoken(MOD, yytext); }
"^"                 { return set_yytoken(ER, yytext); }
"xor"               { return set_yytoken(ER, yytext); }
"&"                 { return set_yytoken(AND, yytext); }
"bitand"            { return set_yytoken(AND, yytext); }
"|"                 { return set_yytoken(OR, yytext); }
"bitor"             { return set_yytoken(OR, yytext); }
"~"                 { return set_yytoken(NOT, yytext); }
"compl"             { return set_yytoken(NOT, yytext); }
"!"                 { return set_yytoken(BANG, yytext); }
"not"               { return set_yytoken(BANG, yytext); }
"="                 { return set_yytoken(ASN, yytext); }
"<"                 { return set_yytoken(LT, yytext); }
">"                 { return set_yytoken(GT, yytext); }
"+="                { return set_yytoken(ADDEQ, yytext); }
"-="                { return set_yytoken(SUBEQ, yytext); }
"*="                { return set_yytoken(MULEQ, yytext); }
"/="                { return set_yytoken(DIVEQ, yytext); }
"%="                { return set_yytoken(MODEQ, yytext); }
"^="                { return set_yytoken(XOREQ, yytext); }
"xor_eq"            { return set_yytoken(XOREQ, yytext); }
"&="                { return set_yytoken(ANDEQ, yytext); }
"and_eq"            { return set_yytoken(ANDEQ, yytext); }
"|="                { return set_yytoken(OREQ, yytext); }
"or_eq"             { return set_yytoken(OREQ, yytext); }
"<<"                { return set_yytoken(SL, yytext); }
">>"                { return set_yytoken(SR, yytext); }
"<<="               { return set_yytoken(SLEQ, yytext); }
">>="               { return set_yytoken(SREQ, yytext); }
"=="                { return set_yytoken(EQ, yytext); }
"!="                { return set_yytoken(NOTEQ, yytext); }
"not_eq"            { return set_yytoken(NOTEQ, yytext); }
"<="                { return set_yytoken(LTEQ, yytext); }
">="                { return set_yytoken(GTEQ, yytext); }
"&&"                { return set_yytoken(ANDAND, yytext); }
"and"               { return set_yytoken(ANDAND, yytext); }
"||"                { return set_yytoken(OROR, yytext); }
"or"                { return set_yytoken(OROR, yytext); }
"++"                { return set_yytoken(PLUSPLUS, yytext); }
"--"                { return set_yytoken(MINUSMINUS, yytext); }
","                 { return set_yytoken(COMMA, yytext); }
"->*"               { return set_yytoken(ARROWSTAR, yytext); }
"->"                { return set_yytoken(ARROW, yytext); }
"asm"               { return set_yytoken(ASM, yytext); }
"auto"              { return set_yytoken(AUTO, yytext); }
"bool"              { return set_yytoken(BOOL, yytext); }
"break"             { return set_yytoken(BREAK, yytext); }
"case"              { return set_yytoken(CASE, yytext); }
"catch"             { return set_yytoken(CATCH, yytext); }
"char"              { return set_yytoken(CHAR, yytext); }
"class"             { return set_yytoken(CLASS, yytext); }
"const"             { return set_yytoken(CONST, yytext); }
"const_cast"        { return set_yytoken(CONST_CAST, yytext); }
"continue"          { return set_yytoken(CONTINUE, yytext); }
"default"           { return set_yytoken(DEFAULT, yytext); }
"delete"            { return set_yytoken(DELETE, yytext); }
"do"                { return set_yytoken(DO, yytext); }
"double"            { return set_yytoken(DOUBLE, yytext); }
"dynamic_cast"      { return set_yytoken(DYNAMIC_CAST, yytext); }
"else"              { return set_yytoken(ELSE, yytext); }
"enum"              { return set_yytoken(ENUM, yytext); }
"explicit"          { return set_yytoken(EXPLICIT, yytext); }
"export"            { return set_yytoken(EXPORT, yytext); }
"extern"            { return set_yytoken(EXTERN, yytext); }
"false"             { return set_yytoken(FALSE, yytext); }
"float"             { return set_yytoken(FLOAT, yytext); }
"for"               { return set_yytoken(FOR, yytext); }
"friend"            { return set_yytoken(FRIEND, yytext); }
"goto"              { return set_yytoken(GOTO, yytext); }
"if"                { return set_yytoken(IF, yytext); }
"inline"            { return set_yytoken(INLINE, yytext); }
"int"               { return set_yytoken(INT, yytext); }
"long"              { return set_yytoken(LONG, yytext); }
"mutable"           { return set_yytoken(MUTABLE, yytext); }
"namespace"         { return set_yytoken(NAMESPACE, yytext); }
"new"               { return set_yytoken(NEW, yytext); }
"operator"          { return set_yytoken(OPERATOR, yytext); }
"private"           { return set_yytoken(PRIVATE, yytext); }
"public"            { return set_yytoken(PUBLIC, yytext); }
"register"          { return set_yytoken(REGISTER, yytext); }
"reinterpret_cast"  { return set_yytoken(REINTERPRET_CAST, yytext); }
"return"            { return set_yytoken(RETURN, yytext); }
"short"             { return set_yytoken(SHORT, yytext); }
"signed"            { return set_yytoken(SIGNED, yytext); }
"sizeof"            { return set_yytoken(SIZEOF, yytext); }
"static"            { return set_yytoken(STATIC, yytext); }
"static_cast"       { return set_yytoken(STATIC_CAST, yytext); }
"struct"            { StructKeywordSeen = 1; 
                      return set_yytoken(STRUCT, yytext); }
"switch"            { return set_yytoken(SWITCH, yytext); }
"template"          { return set_yytoken(TEMPLATE, yytext); }
"this"              { return set_yytoken(THIS, yytext); }
"throw"             { return set_yytoken(THROW, yytext); }
"true"              { return set_yytoken(TRUE, yytext); }
"try"               { return set_yytoken(TRY, yytext); }
"typedef"           { return set_yytoken(TYPEDEF, yytext); }
"typeid"            { return set_yytoken(TYPEID, yytext); }
"typename"          { return set_yytoken(TYPENAME, yytext); }
"union"             { return set_yytoken(UNION, yytext); }
"unsigned"          { return set_yytoken(UNSIGNED, yytext); }
"using"             { return set_yytoken(USING, yytext); }
"virtual"           { return set_yytoken(VIRTUAL, yytext); }
"void"              { return set_yytoken(VOID, yytext); }
"volatile"          { return set_yytoken(VOLATILE, yytext); }
"wchar_t"           { return set_yytoken(WCHAR_T, yytext); }
"while"             { return set_yytoken(WHILE, yytext); }
"#include"[ ]*\"[^"]+\"              { handle_include(); }
"#include"[ ]*"<iostream"(\.h)?">"   { included_iostream = 1; }
"#include"[ ]*"<cstdlib"(\.h)?">"    { included_cstdlib = 1; }
"#include"[ ]*"<cmath"(\.h)?">"      { included_cmath = 1; }
"#include"[ ]*"<ctime"(\.h)?">"      { included_ctime = 1; }
"#include"[ ]*"<cstring"(\.h)?">"    { included_cstring = 1; }
"#include"[ ]*"<fstream"(\.h)?">"    { included_fstream = 1; }
"#include"[ ]*"<string"(\.h)?">"     { included_string = 1; }
"#include"[ ]*"<iomanip"(\.h)?">"    { included_iomanip = 1; }
"#include"[ ]*"<"[^>]+">"    { lexerr("Unsupported include file specified"); }
"//".*                     { /* ignore comments */ }
"/*"([^*]|"*"[^/])*"*/"    { /* ignore comments */ }
"/*"([^*]|"*"[^/])*[^*/]   { lexerr("Non-terminated comment"); }
[a-zA-Z_][a-zA-Z_0-9]* { return set_yytoken(check_identifier(yytext), yytext);}
"0"[xX][0-9a-fA-F]+{intsuffix}? { return set_yytoken(INTEGER, yytext); }
"0"[0-7]+{intsuffix}?           { return set_yytoken(INTEGER, yytext); }
[0-9]+{intsuffix}?  { return set_yytoken(INTEGER, yytext); }
{fracconst}{exppart}?{floatsuffix}? { return set_yytoken(FLOATING, yytext); }
[0-9]+{exppart}{floatsuffix}?       { return set_yytoken(FLOATING, yytext); }
"'"{chartext}*"'"   { return set_yytoken(CHARACTER, yytext); }
"L'"{chartext}*"'"  { return set_yytoken(CHARACTER, yytext); }
"\""{stringtext}*"\""       { return set_yytoken(STRING, yytext); }
"L\""{stringtext}*"\""      { return set_yytoken(STRING, yytext); }
.    { fprintf(stderr, "%d: unexpected character `%c'\n", lineno, yytext[0]); }

%%

void lexerr(char *s)
{
  errors++;
  fprintf(stderr, "LEXICAL ERROR: %s\n", s);
  fprintf(stderr, "  Error found in %s on line %d\n", curfilename, lineno);
  fprintf(stderr, "  Error causing token = \"%s\"\n", yytext);
  /* exit 1 is lexical error */
  /* Fail stop! */
  exit(1);
}

/* hash table code from examples in:
 * http://see-programming.blogspot.com/2013/05/
        chain-hashing-separate-chaining-with.html
 */

/* From stackoverflow.com DJB hash functions */ 
unsigned int DJBHash(char* str, unsigned int len)
{
   unsigned int hash = 5381;
   unsigned int i    = 0;
   for(i = 0; i < len; str++, i++)
   {   
      hash = ((hash << 5) + hash) + (*str);
   }   
   return hash;
}
 
int hash_key(char *s)
{
    unsigned int j;
    j = DJBHash(s, strlen(s));
    return j;
}

struct typenametable_entry *createNode(char *s, int cat) 
{
    struct typenametable_entry *newnode;
    newnode = (struct typenametable_entry *)
        calloc(1, sizeof(struct typenametable_entry));
    if(newnode == NULL)
    {
        fprintf(stderr, "ERROR: out of memory\n"); 
        exit(10);       
    }
    newnode->category = cat;
    newnode->name = strdup(s);
    if(newnode->name == NULL)
    {
        fprintf(stderr, "ERROR: out of memory\n"); 
        exit(10);       
    }
    newnode->next = NULL;
    return newnode;
}

void insertToHash(char *s, int cat) 
{
    unsigned int hashIndex;
    hashIndex = hash_key(s);
    hashIndex %= TYPEHASHSIZE;
    struct typenametable_entry *newnode =  createNode( s, cat);
    /* head of list for the bucket with index hashIndex */
    if (!identhashTable[hashIndex].head) 
    {
        /* first entry for the bucket! */
        identhashTable[hashIndex].head = newnode;
        identhashTable[hashIndex].count = 1;
        return;
    }
    /* adding new node to the list */
    newnode->next = (identhashTable[hashIndex].head);
    /*
     * update the head of the list and no of
     * nodes in the current bucket
     */
    identhashTable[hashIndex].head = newnode;
    identhashTable[hashIndex].count++;
    return;
}

int searchInHash(char *s) 
{
    unsigned int hashIndex;
    int retval = 0;
    hashIndex = hash_key(s);
    hashIndex %= TYPEHASHSIZE;
    struct typenametable_entry *myNode;
    myNode = identhashTable[hashIndex].head;
    if(myNode) 
    {
        while (myNode != NULL) 
        {
            if(!strcmp(myNode->name, s)) 
            {
                retval = myNode->category;
                break;
            }
            myNode = myNode->next;
        }
    }
    if (!retval)
    {
        retval = check_type_exceptions(s);
    }
    return retval;
}

int check_type_exceptions(char *s)
{
   if(!strcmp(s, "std"))
   {
      insertToHash("std", NAMESPACE_NAME);
      return NAMESPACE_NAME;
   }
   /*
   if(!strcmp(s, "string") && namespace_found
      && (included_string || included_iostream || included_fstream))
   {
      insertToHash("string", CLASS_NAME);
      return CLASS_NAME;
   }
   if((!strcmp(s, "ifstream") || !strcmp(s, "ofstream") 
    || !strcmp(s, "fstream"))
      && namespace_found && included_fstream)
   {
      insertToHash(s, CLASS_NAME);
      return CLASS_NAME;
   }
   if(!strcmp(s, "cin") && namespace_found && included_iostream)
   {
        insertToHash("cin", CLASS_NAME);
        return CLASS_NAME;
   }
   if(!strcmp(s, "cout") && namespace_found && included_iostream)
   {
        insertToHash("cout", CLASS_NAME);
        return CLASS_NAME;
   }
   if(!strcmp(s, "endl") && namespace_found && included_iostream)
   {
        insertToHash("endl", CLASS_NAME);
        return CLASS_NAME;
   }
*/
   return IDENTIFIER;
}

static int
check_identifier(char *s)
{
    return searchInHash(s);
}

void print_ident_list() 
{
    struct typenametable_entry *myNode;
    int i;
    fprintf(stdout, "\n\nTYPE NAME LIST\n");
    for (i = 0; i < TYPEHASHSIZE; i++) 
    {
        if (identhashTable[i].count == 0)
        {
           continue;
        }
        myNode = identhashTable[i].head;
        if (!myNode)
        {
           continue;
        }
        while (myNode != NULL) 
        {
            fprintf(stdout, "%s\t%d\n", myNode->name, myNode->category);
            myNode = myNode->next;
        }
    }
    fprintf(stdout, "\n");
    return;
}

/*
 * Return -1 if done, 0 if yyin points at more input
 */
int yywrap()
{
  int retval;
  if(popfile() == 0)
  {
    retval = -1;
  }
  else
  {
    retval = 0;
  }
  return retval;
}

int set_yytoken(int cat, char *txt)
{
  /* sets the global struct yytoken with the current found lexeme info */
  /* these ptrs are used for string manipulation */
  char *str_start;
  char *str_copy;
  char *r;
  char *e;
  char *p;
  int i;
  
  struct token *tp;
  tp = (struct token *)calloc(1, sizeof(struct token));
  if(tp == NULL)
  {
    fprintf(stderr, "ERROR: out of memory for token list.\n");
    exit(10);
  }
  tp->category = cat;
  if((tp->text = strdup(txt)) == NULL)
  {
    fprintf(stderr, "ERROR: out of memory for token list.\n");
    exit(10);
  }
  tp->lineno = lineno;
  if((tp->filename = strdup(curfilename)) == NULL)
  {
    fprintf(stderr, "ERROR: out of memory for token list.\n");
    exit(10);
  }
  
  /* deal with constants int and string values per hw instructions */
  switch(cat)
  {
    case INTEGER:
      tp->ival = atoi(txt);
      break;
    case STRING:
      if((str_copy = strdup(txt)) == NULL)
      {
        fprintf(stderr, "ERROR: out of memory for token list.\n");
        exit(10);
      } 
      /* get rid of quotes for sval */
      str_start = strchr(str_copy, '"');
      if(str_start == NULL)
      {
        /* error string with no start quote */
        lexerr("ERROR: cannot find start quote for string.\n");
      }
      str_start++; /* move to the next char after " */
      r = strrchr(str_start, '"');
      if(r == NULL)
      {
        /* ERROR string with no end quote */
        lexerr("ERROR: cannot find end quote for string.\n");
      }
      *r = '\0'; /* set the last " to null */
      /* now process escapes so sval will print correctly */
      r = str_start; /* set read loop pointer */
      while(*r != '\0')
      {
        i = 0; /* set no escape found */
        if(*r == '\\')
        {
          /* possible escape found */
          e = r + 1; /* look at next char */
          switch (*e) /* escape sequences: Soule pg. 26 */
          {
            case 'n':
            case 'N':
              *r = 0x0a; /* set newline */
              i++; /* escape found! */
              break;
            case 't':
            case 'T':
              *r = 0x09; /* set tab */
              i++; /* escape found! */
              break;
            case 0x27: /* single quote ' */
              *r = 0x27;
              i++;
              break;
            case '"': /* double quote " */
              *r = 0x22;
              i++;
              break;
            case '\\': /* backslash \ */
              *r = 0x5c;
              i++;
              break;
          }
          if(i > 0)
          {
            /* escape found, need to reset pointers */
            p = r; /* remember where we are in the loop */
            e++; /* eat the tab or newline char */
            r++; /* move off the ascii tab or newline */
            while(*e != '\0')
            {
              /* copy the rest of the string as is */
              *r = *e;
              e++;
              r++;
            }
            *r = '\0';
            r = p; /* reset to looping point */
          }
        }
        r++; /* advance read pointer to next char */
      }
      if((tp->sval = strdup(str_start)) == NULL)
      {
        fprintf(stderr, "ERROR: out of memory for token list.\n");
        exit(10);
      }
      free(str_copy);
      break;
  }
  if(add_token(tp))
  {
    yytoken = tp;
  }
  else
  {
    fprintf(stderr, "ERROR: token list memory error.\n");
    exit(10); 
  }
  /* Set the value of yylval */
  yylval.tokptr = tlist->t;
  return cat;
}

int add_token(struct token *tp)
{
  /* add yytoken to the token list */
  struct tokenlist *lp;
  lp = (struct tokenlist *)calloc(1, sizeof(struct tokenlist));
  if(lp == NULL)
  {
    fprintf(stderr, "ERROR: out of memory for token list.\n");
    exit(10);
  } 
  lp->t = tp;
  lp->next = tlist;
  tlist = lp;
  return 1;
}

int handle_include()
{
  char *fname = strchr(yytext, '\"') + 1;
  fname[strlen(fname) - 1] = '\0'; 
  if(!newfile(fname))
  {
    yyterminate(); /* no such file */
  } 
}

int newfile(char *fn)
{
  FILE *f = fopen(fn, "r");
  struct bufstack *bs = calloc(1, sizeof(struct bufstack));
  
  /* die if no file or no room */
  if(!f)
  {  
    fprintf(stderr, "ERROR: unable to open file %s.\n", fn);
    exit(11);
  }
  if(!bs)
  {
    fprintf(stderr, "ERROR: out of buffer memory.\n"); 
    exit(10);
  }
  
  /* remember state */
  if(curbs)
  {
    curbs->lineno = lineno;
    lineno = 1;
  }
  bs->prev = curbs;
  /* set up current entry */
  bs->bs = yy_create_buffer(f, YY_BUF_SIZE);
  bs->f = f;
  bs->filename = fn;
  yy_switch_to_buffer(bs->bs);
  curbs = bs;
  curfilename = fn;
  return 1;
}

int popfile(void)
{
  struct bufstack *bs = curbs;
  struct bufstack *prevbs;

  if(!bs)
  {
    return 0;
  }

  /* get rid of current entry */
  fclose(bs->f);
  yy_delete_buffer(bs->bs);

  /* switch back to previous */
  prevbs = bs->prev;
  free(bs);

  if(!prevbs)
  {
    curbs = 0;
    return 0;
  }

  yy_switch_to_buffer(prevbs->bs);
  curbs = prevbs;
  lineno = curbs->lineno;
  curfilename = curbs->filename;
  return 1;
}
