/*  Intermediate Code
 *  Homework 4
 *  CS 445, Dr. Jeffery
 *  Nov. 24, 2014
 *  Kathryn Price
 */
 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "rules.h"

extern int semantic_debug;
extern int parser_debug;

char *readable_rule(int rule);
void init_rules(void);

void init_rules(void)
{
    int i;
    for(i = 1; i < NUMOFRULES; i++)
    {
        ruleStr[i] = 0;
        ruleArray[i] = 0;
    }
    //ruleArray[0] =  IDENTIFIERS_TYPEDEF;
    //ruleStr[0] = strdup("typedef_name: TYPEDEF_NAME");
    
    //ruleArray[1] =  IDENTIFIERS_NAMESPACE; 
    ruleArray[2] =  IDENTIFIERS_ORIG_NS; 
    ruleStr[2] = strdup("original_namespace_name: NAMESPACE_NAME");
    
    ruleArray[3] =  IDENTIFIERS_CLASS ;
    ruleStr[3] = strdup("class_name: CLASS_NAME");
    
    ruleArray[4] =  LEXICAL_IDENT  ;
    ruleStr[4] = strdup("identifier: IDENTIFIER");

    //ruleArray[5] =  LEXICAL_LITERAL  ;
    //ruleArray[6] =  LEXICAL_LITERAL2 ;
    //ruleArray[7] =  LEXICAL_LITERAL3 ;
    //ruleArray[8] =  LEXICAL_LITERAL4 ;
    //ruleArray[9] =  LEXICAL_LITERAL5 ;
    ruleArray[10] =  LEXICAL_INTLIT ;
    ruleStr[10] = strdup("integer_literal: INTEGER");
    
    ruleArray[11] =  LEXICAL_CHARLIT ;
    ruleStr[11] = strdup("character_literal: CHARACTER");
    
    ruleArray[12] =  LEXICAL_FLOATLIT ; 
    ruleStr[12] = strdup("floating_literal: FLOATING");
    
    ruleArray[13] =  LEXICAL_STRINGLIT  ;
    ruleStr[13] = strdup("string_literal: STRING");
    
    ruleArray[14] =  LEXICAL_BOOLLIT  ;
    ruleStr[14] = strdup("boolean_literal: TRUE");
    
    ruleArray[15] =  LEXICAL_BOOLLIT2 ;
    ruleStr[15] = strdup("boolean_literal: FALSE");

    //ruleArray[16] =  TRANSLATION  ;
    //ruleArray[17] =  EXPRESSION_PRIMARY  ;
    //ruleArray[18] =  EXPRESSION_PRIMARY2 ;
    //ruleArray[19] =  EXPRESSION_PRIMARY3 ;
    //ruleStr[19] = strdup("primary_expression: LP expression RP");
    
    //ruleArray[20] =  EXPRESSION_PRIMARY4 ;
    //ruleArray[21] =  EXPRESSION_ID ;
    //ruleArray[22] =  EXPRESSION_ID2 ;
    //ruleArray[23] =  EXPRESSION_UQID ;
    //ruleArray[24] =  EXPRESSION_UQID2 ;
    //ruleArray[25] =  EXPRESSION_UQID3 ;
    ruleArray[26] =  EXPRESSION_QID ;
    ruleStr[26] = 
    strdup("qualified_id: nested_name_specifier unqualified_id");
    
    //ruleArray[27] =  EXPRESSION_NESTED_NAME ;
    //ruleArray[28] =  EXPRESSION_NESTED_NAME2 ;
    //ruleArray[29] =  EXPRESSION_NESTED_NAME3 ;
    //ruleArray[30] =  EXPRESSION_POSTFIX ;
    ruleArray[31] =  EXPRESSION_POSTFIX2 ;
    ruleStr[31] = 
    strdup("postfix_expression: postfix_expression LB expression RB");
    
    ruleArray[32] =  EXPRESSION_POSTFIX3;
    ruleStr[32] = strdup(
     "postfix_expression: postfix_expression LP expression_list_opt RP");
     
    //ruleArray[33] =  EXPRESSION_POSTFIX4 ;
    ruleArray[34] =  EXPRESSION_POSTFIX5 ;
    ruleStr[34] = strdup(
     "postfix_expression: postfix_expression DOT id_expression");
    
    //ruleArray[35] =  EXPRESSION_POSTFIX6 ;
    //ruleArray[36] =  EXPRESSION_POSTFIX7 ;
    //ruleStr[36] = strdup(
     //"postfix_expression: postfix_expression ARROW id_expression");
    
    ruleArray[37] =  EXPRESSION_POSTFIX8 ;
    ruleStr[37] = strdup("postfix_expression: postfix_expression PLUSPLUS");
    
    ruleArray[38] =  EXPRESSION_POSTFIX9 ;
    ruleStr[38] = strdup("postfix_expression: postfix_expression MINUSMINUS");
    
    //ruleArray[39] =  EXPRESSION_POSTFIX10 ;
    //ruleArray[40] =  EXPRESSION_POSTFIX11 ;
    //ruleStr[40] = strdup("postfix_expression: TYPEID LP expression RP");
    
    //ruleArray[41] =  EXPRESSION_POSTFIX12 ;
    //ruleStr[41] = strdup("postfix_expression: TYPEID LP type_id RP");
    
    //ruleArray[42] =  EXPRESSION_LIST ;
    ruleArray[43] =  EXPRESSION_LIST2 ;
    ruleStr[43] = strdup(
     "expression_list: expression_list COMMA assignment_expression");
    
    //ruleArray[44] =  EXPRESSION_UNEX ;
    ruleArray[45] =  EXPRESSION_UNEX2 ;
    ruleStr[45] = strdup("unary_expression: PLUSPLUS cast_expression");
    
    ruleArray[46] =  EXPRESSION_UNEX3 ;
    ruleStr[46] = strdup("unary_expression: MINUSMINUS cast_expression");
    
    ruleArray[47] =  EXPRESSION_UNEX4 ;
    ruleStr[47] = strdup("unary_expression: STAR cast_expression");
    
    ruleArray[48] =  EXPRESSION_UNEX5;
    ruleStr[48] = strdup("unary_expression: AND cast_expression");
     
    ruleArray[49] =  EXPRESSION_UNEX6 ;
    ruleStr[49] = strdup("unary_expression: unary_operator cast_expression");
    
    //ruleArray[50] =  EXPRESSION_UNEX7 ;
    //ruleArray[51] =  EXPRESSION_UNEX8 ;
    //ruleArray[52] =  EXPRESSION_UNEX9 ;
    //ruleArray[53] =  EXPRESSION_UNEX10 ;
    ruleArray[54] =  EXPRESSION_UNOP ;
    ruleStr[54] = strdup("unary_operator: PLUS");
      
    ruleArray[55] =  EXPRESSION_UNOP2 ;
    ruleStr[55] = strdup("unary_operator: MINUS");

    ruleArray[56] =  EXPRESSION_UNOP3 ;
    ruleStr[56] = strdup("unary_operator: BANG");

    ruleArray[57] =  EXPRESSION_UNOP4 ;
    ruleStr[57] = strdup("unary_operator: NOT");

    ruleArray[58] =  EXPRESSION_NEWEX ;
    ruleStr[58] = strdup(
     "new_expression: NEW new_placement_opt new_type_id new_initializer_opt");
      
    //ruleArray[59] =  EXPRESSION_NEWEX2 ;
    ruleArray[60] =  EXPRESSION_NEWEX3 ;
    ruleStr[60] = strdup(
     "new_expression: NEW new_placement_opt LP type_id RP new_initializer_opt"
     );
    
    //ruleArray[61] =  EXPRESSION_NEWEX4 ;
    //ruleArray[62] =  EXPRESSION_NEWPL  ;
    //ruleStr[62] = strdup("new_placement: LP expression_list RP");
    
    ruleArray[63] =  EXPRESSION_NEWTYPE  ;
    ruleStr[63] = strdup("new_type_id: type_specifier_seq new_declarator_opt");
     
    ruleArray[64] =  EXPRESSION_NEWDEC  ;
    ruleStr[64] = strdup("new_declarator: ptr_operator new_declarator_opt");
    
    //ruleArray[65] =  EXPRESSION_NEWDEC2 ;
    //ruleArray[66] =  EXPRESSION_DIRDEC  ;
    //ruleStr[66] = strdup("direct_new_declarator: LB expression RB");
    
    ruleArray[67] =  EXPRESSION_DIRDEC2 ;
    ruleStr[67] = strdup(
     "direct_new_declarator: direct_new_declarator LB constant_expression RB");
    
    //ruleArray[68] =  EXPRESSION_NEWINIT  ;
    //ruleStr[68] = strdup("new_initializer: LP expression_list_opt RP");
    
    ruleArray[69] =  EXPRESSION_DELEX  ;
    ruleStr[69] = strdup("delete_expression: DELETE cast_expression");
      
    //ruleArray[70] =  EXPRESSION_DELEX2 ;
    //ruleArray[71] =  EXPRESSION_DELEX3 ;
    //ruleStr[71] = strdup("delete_expression: DELETE LB RB cast_expression");
    
    //ruleArray[72] =  EXPRESSION_DELEX4 ;
    //ruleArray[73] =  EXPRESSION_CAST  ;
    ruleArray[74] =  EXPRESSION_CAST2 ;
    ruleStr[74] = strdup("cast_expression: LP type_id RP cast_expression");
    
    //ruleArray[75] =  EXPRESSION_PM  ;
    ruleArray[76] =  EXPRESSION_PM2 ;
    ruleStr[76] = strdup(
     "pm_expression: pm_expression DOTSTAR cast_expression");
    
    ruleArray[77] =  EXPRESSION_PM3 ;
    ruleStr[77] = strdup(
     "pm_expression: pm_expression ARROWSTAR cast_expression");
    
    //ruleArray[78] =  EXPRESSION_MULT ;
    ruleArray[79] =  EXPRESSION_MULT2;
    ruleStr[79] = strdup(
     "multiplicative_expression: multiplicative_expression STAR pm_expression"
     );
    
    ruleArray[80] =  EXPRESSION_MULT3 ;
    ruleStr[80] = strdup(
     "multiplicative_expression: multiplicative_expression DIV pm_expression");
    
    ruleArray[81] =  EXPRESSION_MULT4 ;
    ruleStr[81] = strdup(
     "multiplicative_expression: multiplicative_expression MOD pm_expression");
    
    //ruleArray[82] =  EXPRESSION_ADD  ;
    ruleArray[83] =  EXPRESSION_ADD2 ;
    ruleStr[83] = strdup(
     "additive_expression: additive_expression PLUS multiplicative_expression"
     );
    
    ruleArray[84] =  EXPRESSION_ADD3 ;
    ruleStr[84] = strdup(
     "additive_expression: additive_expression MINUS multiplicative_expression"
     );
    
    //ruleArray[85] =  EXPRESSION_SHIFT ;
    ruleArray[86] =  EXPRESSION_SHIFT2 ;
    ruleStr[86] = strdup(
     "shift_expression: shift_expression SL additive_expression");
    
    ruleArray[87] =  EXPRESSION_SHIFT3;
    ruleStr[87] = strdup(
     "shift_expression: shift_expression SR additive_expression"); 
    
    //ruleArray[88] =  EXPRESSION_REL  ;
    ruleArray[89] =  EXPRESSION_REL2 ;
    ruleStr[89] = strdup(
     "relational_expression: relational_expression LT shift_expression");
    
    ruleArray[90] =  EXPRESSION_REL3 ;
    ruleStr[90] = strdup(
     "relational_expression: relational_expression GT shift_expression");
    
    ruleArray[91] =  EXPRESSION_REL4 ;
    ruleStr[91] = strdup(
     "relational_expression: relational_expression LTEQ shift_expression");
    
    ruleArray[92] =  EXPRESSION_REL5 ;
    ruleStr[92] = strdup(
     "relational_expression: relational_expression GTEQ shift_expression");
    
    //ruleArray[93] =  EXPRESSION_EQ  ;
    ruleArray[94] =  EXPRESSION_EQ2 ;
    ruleStr[94] = strdup( 
     "equality_expression: equality_expression EQ relational_expression");
    
    ruleArray[95] =  EXPRESSION_EQ3 ;
    ruleStr[95] = strdup(
     "equality_expression: equality_expression NOTEQ relational_expression");
    
    //ruleArray[96] =  EXPRESSION_AND  ;
    //ruleArray[97] =  EXPRESSION_AND2 ;
    //ruleArray[98] =  EXPRESSION_OR  ;
    //ruleArray[99] =  EXPRESSION_OR2 ;
    //ruleArray[100] =  EXPRESSION_IOR  ;
    //ruleArray[101] =  EXPRESSION_IOR2 ;
    //ruleArray[102] =  EXPRESSION_LOGAND  ;
    ruleArray[103] =  EXPRESSION_LOGAND2 ;
    ruleStr[103] = strdup(
"logical_and_expression: logical_and_expression ANDAND inclusive_or_expression"
    );
    
    //ruleArray[104] =  EXPRESSION_LOGOR ;
    ruleArray[105] =  EXPRESSION_LOGOR2;
    ruleStr[105] = strdup(
     "logical_or_expression: logical_or_expression OROR logical_and_expression"
     );
    
    ruleArray[106] =  EXPRESSION_COND ;
     
    ruleStr[106] = strdup(
"conditional_expression: logical_or_expression QUEST expression COLON assignment_expression"
    );
    
    //ruleArray[107] =  EXPRESSION_COND2 ;
    //ruleArray[108] =  EXPRESSION_ASN  ;
    ruleArray[109] =  EXPRESSION_ASN2 ;
    ruleStr[109] =  strdup(
"assignment_expression: logical_or_expression assignment_operator assignment_expression"
    );
    
    //ruleArray[110] =  EXPRESSION_ASN3 ;
    ruleArray[111] =  EXPRESSION_OP ;
    ruleStr[111] = strdup("assignment_operator: ASN");
    
    ruleArray[112] =  EXPRESSION_OP2 ;
    ruleStr[112] = strdup("assignment_operator: MULEQ");
    
    ruleArray[113] =  EXPRESSION_OP3 ;
    ruleStr[113] = strdup("assignment_operator: DIVEQ");
    
    ruleArray[114] =  EXPRESSION_OP4 ;
    ruleStr[114] = strdup("assignment_operator: MODEQ");
    
    ruleArray[115] =  EXPRESSION_OP5 ;
    ruleStr[115] = strdup("assignment_operator: ADDEQ");
    
    ruleArray[116] =  EXPRESSION_OP6 ;
    ruleStr[116] = strdup("assignment_operator: SUBEQ");
    
    //ruleArray[117] =  EXPRESSION_OP7 ;
    //ruleArray[118] =  EXPRESSION_OP8 ;
    //ruleArray[119] =  EXPRESSION_OP9 ;
    //ruleArray[120] =  EXPRESSION_OP10 ;
    //ruleArray[121] =  EXPRESSION_OP11 ;
    //ruleArray[122] =  EXPRESSION_EX ;
    ruleArray[123] =  EXPRESSION_EX2 ;
    ruleStr[123] = strdup(
     "expression: expression COMMA assignment_expression");
    
    //ruleArray[124] =  EXPRESSION_CONSTEX ;
    ruleArray[125] =  STATEMENTS_STMT  ;
    ruleStr[125] = strdup("statement: labeled_statement");

    ruleArray[126] =  STATEMENTS_STMT2 ;
    ruleStr[126] = strdup("statement: expression_statement");

    ruleArray[127] =  STATEMENTS_STMT3 ;
    ruleStr[127] = strdup("statement: compound_statement");
    
    ruleArray[128] =  STATEMENTS_STMT4 ;
    ruleStr[128] = strdup("statement: selection_statement");
    
    ruleArray[129] =  STATEMENTS_STMT5 ;
    ruleStr[129] = strdup("statement: iteration_statement");
    
    ruleArray[130] =  STATEMENTS_STMT6 ;
    ruleStr[130] = strdup("statement: jump_statement");
    
    ruleArray[131] =  STATEMENTS_STMT7 ;
    ruleStr[131] = strdup("statement: declaration_statement");
    
    ruleArray[132] =  STATEMENTS_LABEL  ;
    ruleStr[132] = strdup("labeled_statement: identifier COLON statement");

    ruleArray[133] =  STATEMENTS_LABEL2 ;
    ruleStr[133] = strdup(
     "labeled_statement: CASE constant_expression COLON statement");
    
    //ruleArray[134] =  STATEMENTS_LABEL3 ;
    
    ruleArray[135] =  STATEMENTS_EX  ;
    ruleStr[135] = strdup("expression_opt: expression");
    
    //ruleStr[135] = strdup("expression_statement: expression_opt SM");
    
    //ruleArray[136] =  STATEMENTS_COMP  ;
    //ruleStr[136] = strdup("compound_statement: LC statement_seq_opt RC");
    
    //ruleArray[137] =  STATEMENTS_STSEQ  ;
    ruleArray[138] =  STATEMENTS_STSEQ2 ;
    ruleStr[138] = strdup("statement_seq: statement_seq statement");
    
    ruleArray[139] =  STATEMENTS_SEL  ;
    ruleStr[139] = strdup(
     "selection_statement: IF LP condition RP statement");
    
    ruleArray[140] =  STATEMENTS_SEL2 ;
    ruleStr[140] = strdup(
     "selection_statement: IF LP condition RP statement ELSE statement");
    
    ruleArray[141] =  STATEMENTS_SEL3 ;
    ruleStr[141] = strdup(
     "selection_statement: SWITCH LP condition RP statement");
    
    //ruleArray[142] =  STATEMENTS_COND  ;
    ruleArray[143] =  STATEMENTS_COND2 ;
    ruleStr[143] = strdup(
     "condition: type_specifier_seq declarator ASN assignment_expression");
    
    ruleArray[144] =  STATEMENTS_ITER  ;
    ruleStr[144] = strdup(
     "iteration_statement: WHILE LP condition RP statement");
    
    ruleArray[145] =  STATEMENTS_ITER2;
    ruleStr[145] = strdup(
     "iteration_statement: DO statement WHILE LP expression RP SM");
    
    ruleArray[146] =  STATEMENTS_ITER3 ;
    ruleStr[146] = strdup(
"iteration_statement: FOR LP for_init_statement condition_opt SM expression_opt RP statement"
    );
    
    ruleArray[147] =  STATEMENTS_FOR   ;
    ruleStr[147] = strdup("for_init_statement: expression_statement");
    
    ruleArray[148] =  STATEMENTS_FOR2  ;
    ruleStr[148] = strdup("for_init_statement: simple_declaration");
    
    ruleArray[149] =  STATEMENTS_JUMP   ;
    ruleStr[149] = strdup("jump_statement: BREAK SM");
    
    ruleArray[150] =  STATEMENTS_JUMP2  ;
    ruleStr[150] = strdup("jump_statement: CONTINUE SM");
    
    ruleArray[151] =  STATEMENTS_JUMP3  ;
    ruleStr[151] = strdup("jump_statement: RETURN expression_opt SM");
    
    //ruleArray[152] =  STATEMENTS_JUMP4  ;
    //ruleArray[153] =  STATEMENTS_DECL  ;
    ruleArray[154] =  DECLARATIONS_SEQ   ;
    ruleStr[154] = strdup("declaration_seq: declaration_seq declaration");

    //ruleArray[155] =  DECLARATIONS_DECL   ;
    //ruleArray[156] =  DECLARATIONS_DECL2  ;
    //ruleArray[157] =  DECLARATIONS_DECL3  ;
    //ruleArray[158] =  DECLARATIONS_DECL4  ;
    //ruleArray[159] =  DECLARATIONS_DECL5  ;
    //ruleArray[160] =  DECLARATIONS_DECL6  ;
    //ruleArray[161] =  DECLARATIONS_BLOCK   ;
    //ruleArray[162] =  DECLARATIONS_BLOCK2  ;
    //ruleArray[163] =  DECLARATIONS_BLOCK3  ;
    //ruleArray[164] =  DECLARATIONS_BLOCK4  ;
    ruleArray[165] =  DECLARATIONS_SIMPLE  ;
    ruleStr[165] = strdup(
     "simple_declaration: decl_specifier_seq init_declarator_list SM");
       
    //ruleArray[166] =  DECLARATIONS_SIMPLE2  ;
    //ruleArray[167] =  DECLARATIONS_DECLSP   ;
    //ruleArray[168] =  DECLARATIONS_DECLSP2  ;
    //ruleArray[169] =  DECLARATIONS_DECLSP3  ;
    //ruleArray[170] =  DECLARATIONS_DECLSP4  ;
    //ruleStr[170] = strdup("decl_specifier: TYPEDEF");
    
    ruleArray[171] =  DECLARATIONS_DECLSEQ  ;
    ruleStr[171] = strdup(
     "decl_specifier_seq: decl_specifier_seq decl_specifier");
    
    //ruleArray[172] =  DECLARATIONS_STCL   ;
    //ruleArray[173] =  DECLARATIONS_STCL2  ;
    //ruleArray[174] =  DECLARATIONS_STCL3  ;
    //ruleStr[174] = strdup("storage_class_specifier: STATIC");
    
    //ruleArray[175] =  DECLARATIONS_STCL4  ;
    //ruleStr[175] = strdup("storage_class_specifier: EXTERN");
    
    //ruleArray[176] =  DECLARATIONS_FUNSP   ;
    //ruleStr[176] = strdup("function_specifier: INLINE");
    
    //ruleArray[177] =  DECLARATIONS_FUNSP2  ;
    //ruleArray[178] =  DECLARATIONS_FUNSP3  ;
    //ruleStr[178] = strdup("function_specifier: EXPLICIT");
    
    //ruleArray[179] =  DECLARATIONS_TYPESP   ;
    //ruleArray[180] =  DECLARATIONS_TYPESP2  ;
    //ruleArray[181] =  DECLARATIONS_TYPESP3  ;
    //ruleArray[182] =  DECLARATIONS_TYPESP4  ;
    //ruleArray[183] =  DECLARATIONS_SIMPLETP   ;
    ruleArray[184] =  DECLARATIONS_SIMPLETP2 ;
    ruleStr[184] = strdup(
     "simple_type_specifier: nested_name_specifier type_name");
     
    //ruleArray[185] =  DECLARATIONS_SIMPLETP3  ;
    ruleArray[186] =  DECLARATIONS_SIMPLETP4  ;
    ruleStr[186] = strdup("simple_type_specifier: CHAR");
    
    //ruleArray[187] =  DECLARATIONS_SIMPLETP5  ;
    ruleArray[188] =  DECLARATIONS_SIMPLETP6  ;
    ruleStr[188] = strdup("simple_type_specifier: BOOL");
    
    ruleArray[189] =  DECLARATIONS_SIMPLETP7  ;
    ruleStr[189] = strdup("simple_type_specifier: SHORT");
    
    ruleArray[190] =  DECLARATIONS_SIMPLETP8  ;
    ruleStr[190] = strdup("simple_type_specifier: INT");
    
    ruleArray[191] =  DECLARATIONS_SIMPLETP9  ;
    ruleStr[191] = strdup("simple_type_specifier: LONG");
    
    //ruleArray[192] =  DECLARATIONS_SIMPLETP10  ;
    //ruleStr[192] = strdup("simple_type_specifier: SIGNED");
    
    //ruleArray[193] =  DECLARATIONS_SIMPLETP11  ;
    //ruleStr[193] = strdup("simple_type_specifier: UNSIGNED");
    
    ruleArray[194] =  DECLARATIONS_SIMPLETP12;
    ruleStr[194] = strdup("simple_type_specifier: FLOAT");
      
    ruleArray[195] =  DECLARATIONS_SIMPLETP13  ;
    ruleStr[195] = strdup("simple_type_specifier: DOUBLE");
    
    ruleArray[196] =  DECLARATIONS_SIMPLETP14  ;
    ruleStr[196] = strdup("simple_type_specifier: VOID");
    
    //ruleArray[197] =  DECLARATIONS_TYPENM   ;
    //ruleArray[198] =  DECLARATIONS_TYPENM2  ;
    //ruleArray[199] =  DECLARATIONS_ETYPE   ;
    ruleArray[200] =  DECLARATIONS_ETYPE2 ;
    ruleStr[200] = strdup(
     "elaborated_type_specifier: class_key COLONCOLON identifier"); 
    
    //ruleArray[201] =  DECLARATIONS_ETYPE3  ;
    //ruleArray[202] =  DECLARATIONS_NS   ;
    //ruleArray[203] =  DECLARATIONS_NS2  ;
    //ruleArray[204] =  DECLARATIONS_NAMEDNS   ;
    //ruleArray[205] =  DECLARATIONS_NAMEDNS2  ;
    //ruleArray[206] =  DECLARATIONS_ORIGNS   ;
    //ruleArray[207] =  DECLARATIONS_EXNS   ;
    //ruleArray[208] =  DECLARATIONS_UNNS   ;
    //ruleArray[209] =  DECLARATIONS_NSBODY  ; 
    //ruleArray[210] =  DECLARATIONS_NSALIAS   ;
    //ruleArray[211] =  DECLARATIONS_QUALNS  ;
    //ruleArray[212] =  DECLARATIONS_QUALNS2  ;
    //ruleArray[213] =  DECLARATIONS_QUALNS3  ;
    //ruleStr[213] = strdup(
//"qualified_namespace_specifier: nested_name_specifier namespace_name"
   // );
    
    //ruleArray[214] =  DECLARATIONS_QUALNS4  ;
    //ruleArray[215] =  DECLARATIONS_USINGDEC   ;
    //ruleArray[216] =  DECLARATIONS_USINGDEC2 ;
    //ruleStr[216] = strdup(
//"using_declaration: USING TYPENAME nested_name_specifier unqualified_id SM"
  //   );
     
    //ruleArray[217] =  DECLARATIONS_USINGDEC3  ;
    //ruleArray[218] =  DECLARATIONS_USINGDEC4  ;
    //ruleStr[218] = strdup(
    // "using_declaration: USING nested_name_specifier unqualified_id SM");
    
    //ruleArray[219] =  DECLARATIONS_USINGDEC5  ;
    //ruleArray[220] =  DECLARATIONS_USINGDIR   ;
    //ruleArray[221] =  DECLARATIONS_ASM   ;
    //ruleArray[222] =  DECLARATIONS_LINK   ;
    //ruleStr[222] = strdup(
//"linkage_specification: EXTERN string_literal LC declaration_seq_opt RC"
  //   );
    
    //ruleArray[223] =  DECLARATIONS_LINK2  ;
    //ruleStr[223] = strdup(
    // "linkage_specification: EXTERN string_literal declaration");
    
    //ruleArray[224] =  DECLARATORS_INITLIST   ;
    ruleArray[225] =  DECLARATORS_INITLIST2  ;
    ruleStr[225] = strdup(
     "init_declarator_list: init_declarator_list COMMA init_declarator");
    
    ruleArray[226] =  DECLARATORS_INIT  ; 
    ruleStr[226] = strdup("init_declarator: declarator initializer_opt");
    
    
    //ruleArray[227] =  DECLARATORS_DECL   ;
    ruleArray[228] =  DECLARATORS_DECL2  ;
    ruleStr[228] = strdup("declarator: ptr_operator declarator");
    
    //ruleArray[229] =  DECLARATORS_DIRDEC  ;
    ruleArray[230] =  DECLARATORS_DIRDEC2  ;
    ruleStr[230] = strdup("CLASS_NAME COLONCOLON CLASS_NAME LP RP");
    
    //ruleArray[231] =  DECLARATORS_DIRDEC3  ;
    //ruleStr[231] = strdup(
//"direct_declarator: direct_declarator LP parameter_declaration_clause RP cv_qualifier_seq"
    // );
    
    //ruleArray[232] =  DECLARATORS_DIRDEC4  ;
    //ruleStr[232] = strdup(
//"direct_declarator: direct_declarator LP parameter_declaration_clause RP exception_specification"
  //   );
    
    ruleArray[233] =  DECLARATORS_DIRDEC5  ;
    ruleStr[233] = strdup(
     "direct_declarator: direct_declarator LP parameter_declaration_clause RP");
    
    ruleArray[234] =  DECLARATORS_DIRDEC6  ;
    ruleStr[234] = strdup(
     "direct_declarator: direct_declarator LB constant_expression_opt RB");
    
    //ruleArray[235] =  DECLARATORS_DIRDEC7  ;
    //ruleArray[236] =  DECLARATORS_DIRDEC8  ;
    //ruleStr[236] = strdup(
     //"direct_declarator: CLASS_NAME LP parameter_declaration_clause RP");
    
    ruleArray[237] =  DECLARATORS_DIRDEC9  ;
    ruleStr[237] = strdup(
"direct_declarator: CLASS_NAME COLONCOLON declarator_id LP parameter_declaration_clause RP"
     );
    
    //ruleArray[238] =  DECLARATORS_DIRDEC10  ;
    //ruleStr[238] = strdup(
//"direct_declarator: CLASS_NAME COLONCOLON CLASS_NAME LP parameter_declaration_clause RP"
     //);
    
    ruleArray[239] =  DECLARATORS_PTR   ;
    ruleStr[239] = strdup("ptr_operator: STAR");
    
    //ruleArray[240] =  DECLARATORS_PTR2  ;
    //ruleStr[240] = strdup("ptr_operator: STAR cv_qualifier_seq");
    
    ruleArray[241] =  DECLARATORS_PTR3  ;
    ruleStr[241] = strdup("ptr_operator: AND");
    
    //ruleArray[242] =  DECLARATORS_PTR4  ;
    //ruleStr[242] = strdup("ptr_operator: nested_name_specifier STAR");
    
    //ruleArray[243] =  DECLARATORS_PTR5  ;
    //ruleStr[243] = strdup(
     //"ptr_operator: nested_name_specifier STAR cv_qualifier_seq");
    
    //ruleArray[244] =  DECLARATORS_PTR6  ;
    //ruleArray[245] =  DECLARATORS_PTR7  ;
    //ruleArray[246] =  DECLARATORS_CVSEQ   ;
    //ruleArray[247] =  DECLARATORS_CVSEQ2  ;
    //ruleStr[247] = strdup(
     //"cv_qualifier_seq: cv_qualifier cv_qualifier_seq");
    
    ruleArray[248] =  DECLARATORS_CV   ;
    ruleStr[248] = strdup("cv_qualifier: CONST");
    
    //ruleArray[249] =  DECLARATORS_ID   ;
    //ruleArray[250] =  DECLARATORS_ID2  ;
    //ruleArray[251] =  DECLARATORS_ID3  ;
    //ruleArray[252] =  DECLARATORS_ID4  ;
    ruleArray[253] =  DECLARATORS_TYPE   ;
    ruleStr[253] = strdup(
     "type_id: type_specifier_seq abstract_declarator_opt");
    
    ruleArray[254] =  DECLARATORS_TYPESEQ  ;
    ruleStr[254] = strdup(
     "type_specifier_seq: type_specifier type_specifier_seq_opt");
     
    ruleArray[255] =  DECLARATORS_ABST   ;
    ruleStr[255] = strdup(
     "abstract_declarator: ptr_operator abstract_declarator_opt");
    
    //ruleArray[256] =  DECLARATORS_ABST2  ;
    //ruleArray[257] =  DECLARATORS_DIRAB   ;
    //ruleStr[257] = strdup(
//"direct_abstract_declarator: direct_abstract_declarator_opt LP parameter_declaration_clause RP cv_qualifier_seq exception_specification"
  //   );
      
    //ruleArray[258] =  DECLARATORS_DIRAB2  ;
    //ruleStr[258] = strdup(
//"direct_abstract_declarator: direct_abstract_declarator_opt LP parameter_declaration_clause RP cv_qualifier_seq"
  //   );
    
    //ruleArray[259] =  DECLARATORS_DIRAB3  ;
    //ruleStr[259] = strdup(
//"direct_abstract_declarator: direct_abstract_declarator_opt LP parameter_declaration_clause RP exception_specification"
  //   );
    
    ruleArray[260] =  DECLARATORS_DIRAB4  ;
    ruleStr[260] = strdup(
"direct_abstract_declarator: direct_abstract_declarator_opt LP parameter_declaration_clause RP"
     );
    
    ruleArray[261] =  DECLARATORS_DIRAB5  ;
    ruleStr[261] = strdup(
"direct_abstract_declarator: direct_abstract_declarator_opt LB constant_expression_opt RB"
     );
    
    //ruleArray[262] =  DECLARATORS_DIRAB6;
    //ruleStr[262] = strdup(
     //"direct_abstract_declarator: LP abstract_declarator RP");
      
    //ruleArray[263] =  DECLARATORS_PARAMCL  ;
    //ruleArray[264] =  DECLARATORS_PARAMLS   ;
    ruleArray[265] =  DECLARATORS_PARAMLS2  ;
    ruleStr[265] = strdup(
"parameter_declaration_list: parameter_declaration_list COMMA parameter_declaration"
     );
    
    ruleArray[266] =  DECLARATORS_PARAMDC   ;
    ruleStr[266] = strdup(
     "parameter_declaration: decl_specifier_seq declarator");

    //ruleArray[267] =  DECLARATORS_PARAMDC2  ;
    //ruleStr[267] = strdup(
//"parameter_declaration: decl_specifier_seq declarator ASN assignment_expression"
//     );
    
    ruleArray[268] =  DECLARATORS_PARAMDC3  ;
    ruleStr[268] = strdup(
     "parameter_declaration: decl_specifier_seq abstract_declarator_opt");
    
    //ruleArray[269] =  DECLARATORS_PARAMDC4  ;
    //ruleStr[269] = strdup(
//"parameters_declaration: decl_specifier_seq abstract_declarator_opt ASN assignment_expression"
//     );
    
    ruleArray[270] =  DECLARATORS_FUN   ;
    ruleStr[270] = strdup(
     "function_definition: declarator ctor_initializer_opt function_body");
      
    ruleArray[271] =  DECLARATORS_FUN2  ;
    ruleStr[271] = strdup(
"function_definition: decl_specifier_seq declarator function_body"
     );
    
    //ruleArray[272] =  DECLARATORS_FUNBOD   ;
    //ruleArray[273] =  DECLARATORS_INIZ   ;
    //ruleStr[273] = strdup("initializer: ASN initializer_clause");
    
    //ruleArray[274] =  DECLARATORS_INIZ2  ;
    //ruleStr[274] = strdup("initializer: LP expression_list RP");
    
    //ruleArray[275] =  DECLARATORS_INIZC   ;
    ruleArray[276] =  DECLARATORS_INIZC2  ;
    ruleStr[276] = strdup(
     "initializer_clause: LC initializer_list COMMA_opt RC");
    
    ruleArray[277] =  DECLARATORS_INIZL   ;
    ruleStr[277] = strdup(
     "initializer_list: initializer_list COMMA initializer_clause");
    
    //ruleArray[278] =  DECLARATORS_INIZL2  ;
    ruleArray[279] =  CLASSES_SPEC  ;
    ruleStr[279] = strdup(
     "class_specifier: class_head LC member_specification_opt RC");

    ruleArray[280] =  CLASSES_HEAD   ;
    ruleStr[280] = strdup("class_head: class_key identifier");

    ruleArray[281] =  CLASSES_HEAD2  ;
    ruleStr[281] = strdup("class_head: class_key identifier base_clause");
    
    ruleArray[282] =  CLASSES_HEAD3  ;
    ruleStr[282] = strdup(
     "class_head: class_key nested_name_specifier identifier");
    
    ruleArray[283] =  CLASSES_HEAD4  ;
    ruleStr[283] = strdup(
     "class_head: class_key nested_name_specifier identifier base_clause");
    
    ruleArray[284] =  CLASSES_KEY  ;
    ruleStr[284] = strdup("class_key: CLASS");
     
    //ruleArray[285] =  CLASSES_KEY2  ;
    //ruleStr[285] = strdup("class_key: STRUCT");
    
    ruleArray[286] =  CLASSES_MEMSPEC   ;
    ruleStr[286] = strdup(
     "member_specification: member_declaration member_specification_opt");

    ruleArray[287] =  CLASSES_MEMSPEC2  ;
    ruleStr[287] = strdup(
     "member_specification: access_specifier COLON member_specification_opt");
    
    ruleArray[288] =  CLASSES_MEMDEC   ;
    ruleStr[288] = strdup(
     "member_declaration: decl_specifier_seq member_declarator_list SM"); 

    //ruleArray[289] =  CLASSES_MEMDEC2  ;
    //ruleStr[289] = strdup("member_declaration: decl_specifier_seq SM");
    
    //ruleArray[290] =  CLASSES_MEMDEC3  ;
    //ruleStr[290] = strdup("member_declaration: member_declarator_list SM");
    
    //ruleArray[291] =  CLASSES_MEMDEC4  ;
    //ruleStr[291] = strdup("member_declaration: SM");
    
    ruleArray[292] =  CLASSES_MEMDEC5  ;
    ruleStr[292] = strdup(
     "member_declaration: function_definition SEMICOLON_opt");
    
    //ruleArray[293] =  CLASSES_MEMDEC6  ;
    //ruleStr[293] = strdup("member_declaration: qualified_id SM");
    
    //ruleArray[294] =  CLASSES_MEMDEC7  ;
    //ruleArray[295] =  CLASSES_MEMLS   ;
    ruleArray[296] =  CLASSES_MEMLS2  ;
    ruleStr[296] = strdup(
     "member_declarator_list: member_declarator_list COMMA member_declarator"
     );
    
    ruleArray[297] =  CLASSES_MEMDC   ;
    ruleStr[297] = strdup("member_declarator: declarator pure_specifier");
    
    ruleArray[298] =  CLASSES_MEMDC2  ;
    ruleStr[298] = strdup(
     "member_declarator: declarator constant_initializer");
    
    ruleArray[299] =  CLASSES_MEMDC3  ;
    ruleStr[299] = strdup(
     "member_declarator: identifier COLON constant_expression");
    
    //ruleArray[300] =  CLASSES_CONST   ;
    //ruleStr[300] = strdup("constant_initializer: ASN constant_expression");
    
    //ruleArray[301] =  DCLASSES_BASE   ;
    //ruleStr[301] = strdup("base_clause: COLON base_specifier_list");
    
    //ruleArray[302] =  DCLASSES_BASESPL   ;
    ruleArray[303] =  DCLASSES_BASESPL2  ;
    ruleStr[303] = strdup(
     "base_specifier_list: base_specifier_list COMMA base_specifier");
    
    //ruleArray[304] =  DCLASSES_BASESP   ;
    //ruleArray[305] =  DCLASSES_BASESP2  ;
    ruleArray[306] =  DCLASSES_BASESP3  ;
    ruleStr[306] = strdup(
     "base_specifier: nested_name_specifier class_name");
    
    //ruleArray[307] =  DCLASSES_BASESP4  ;
    ruleArray[308] =  DCLASSES_BASESP5  ;
    ruleStr[308] = strdup(
"base_specifier: access_specifier COLONCOLON nested_name_specifier_opt class_name"
     );
    
    ruleArray[309] =  DCLASSES_BASESP6  ;
    ruleStr[309] = strdup(
     "base_specifier: access_specifier nested_name_specifier_opt class_name");
    
    ruleArray[310] =  DCLASSES_ACCESS   ;
    ruleStr[310] = strdup("access_specifier: PRIVATE");
    
    ruleArray[311] =  DCLASSES_ACCESS2 ;
    ruleStr[311] = strdup("access_specifier: PUBLIC");
 
    ruleArray[312] =  SPCMEM_CONVFUN   ;
    ruleStr[312] = strdup(
     "conversion_function_id: OPERATOR conversion_type_id");
    
    ruleArray[313] =  SPCMEM_CONVID   ;
    ruleStr[313] = strdup(
     "conversion_type_id: type_specifier_seq conversion_declarator_opt");
    
    ruleArray[314] =  SPCMEM_CONVDC   ;
    ruleStr[314] = strdup(
     "conversion_declarator: ptr_operator conversion_declarator_opt");
    
    //ruleArray[315] =  SPCMEM_CTOR   ;
    //ruleStr[315] = strdup("ctor_initializer: COLON mem_initializer_list");
    
    //ruleArray[316] =  SPCMEM_MEMLIST   ;
    ruleArray[317] =  SPCMEM_MEMLIST2  ;
    ruleStr[317] = strdup(
     "mem_initializer_list: mem_initializer COMMA mem_initializer_list");
    
    ruleArray[318] =  SPCMEM_MEMINIT ;
    ruleStr[318] = strdup(
     "mem_initializer: mem_initializer_id LP expression_list_opt RP");
      
    //ruleArray[319] =  SPCMEM_MEMID   ;
    //ruleArray[320] =  SPCMEM_MEMID2  ;
    ruleArray[321] =  SPCMEM_MEMID3 ;
    ruleStr[321] = strdup(
     "mem_initializer_id: nested_name_specifier class_name"); 
    
    //ruleArray[322] =  SPCMEM_MEMID4  ;
    //ruleArray[323] =  SPCMEM_MEMID5  ;
    ruleArray[324] =  OVERLOAD_FUNID;
    ruleStr[324] = strdup("operator_function_id: OPERATOR operator");
       
    ruleArray[325] =  OVERLOAD_OP  ;
    ruleStr[325] = strdup("operator: NEW");

    ruleArray[326] =  OVERLOAD_OP2  ;
    ruleStr[326] = strdup("operator: DELETE");
    
    ruleArray[327] =  OVERLOAD_OP3  ;
    ruleStr[327] = strdup("operator: PLUS");
    
    ruleArray[328] =  OVERLOAD_OP4  ;
    ruleStr[328] = strdup("operator: STAR");
    
    ruleArray[329] =  OVERLOAD_OP5  ;
    ruleStr[329] = strdup("operator: DIV");
    
    ruleArray[330] =  OVERLOAD_OP6  ;
    ruleStr[330] = strdup("operator: MOD");
    
    ruleArray[331] =  OVERLOAD_OP7  ;
    ruleStr[331] = strdup("operator: ER"); 
    
    ruleArray[332] =  OVERLOAD_OP8  ;
    ruleStr[332] = strdup("operator: AND");
    
    ruleArray[333] =  OVERLOAD_OP9  ;
    ruleStr[333] = strdup("operator: OR");
    
    ruleArray[334] =  OVERLOAD_OP10  ;
    ruleStr[334] = strdup("operator: NOT");
    
    ruleArray[335] =  OVERLOAD_OP11  ;
    ruleStr[335] = strdup("operator: BANG");
    
    ruleArray[336] =  OVERLOAD_OP12  ;
    ruleStr[336] = strdup("operator: ASN");
    
    ruleArray[337] =  OVERLOAD_OP13  ;
    ruleStr[337] = strdup("operator: LT");
    
    ruleArray[338] =  OVERLOAD_OP14  ;
    ruleStr[338] = strdup("operator: GT");
    
    ruleArray[339] =  OVERLOAD_OP15  ;
    ruleStr[339] = strdup("operator: ADDEQ");
    
    ruleArray[340] =  OVERLOAD_OP16  ;
    ruleStr[340] = strdup("operator: SUBEQ");
    
    ruleArray[341] =  OVERLOAD_OP17  ;
    ruleStr[341] = strdup("operator: MULEQ");
    
    ruleArray[342] =  OVERLOAD_OP18  ;
    ruleStr[342] = strdup("operator: DIVEQ");
    
    ruleArray[343] =  OVERLOAD_OP19  ;
    ruleStr[343] = strdup("operator: MODEQ");
    
    //ruleArray[344] =  OVERLOAD_OP20  ;
    //ruleArray[345] =  OVERLOAD_OP21  ;
    //ruleArray[346] =  OVERLOAD_OP22  ;
    ruleArray[347] =  OVERLOAD_OP23 ;
    ruleStr[347] = strdup("operator: SL");
     
    ruleArray[348] =  OVERLOAD_OP24  ;
    ruleStr[348] = strdup("operator: SR");
    
    ruleArray[349] =  OVERLOAD_OP25  ;
    ruleStr[349] = strdup("operator: SREQ");
    
    ruleArray[350] =  OVERLOAD_OP26  ;
    ruleStr[350] = strdup("operator: SLEQ");
    
    ruleArray[351] =  OVERLOAD_OP27  ;
    ruleStr[351] = strdup("operator: EQ");
    
    ruleArray[352] =  OVERLOAD_OP28 ;
    ruleStr[352] = strdup("operator: NOTEQ");
     
    ruleArray[353] =  OVERLOAD_OP29 ;
    ruleStr[353] = strdup("operator: LTEQ");
    
    ruleArray[354] =  OVERLOAD_OP30  ;
    ruleStr[354] = strdup("operator: GTEQ");
    
    ruleArray[355] =  OVERLOAD_OP31  ;
    ruleStr[355] = strdup("operator: ANDAND");
    
    ruleArray[356] =  OVERLOAD_OP32  ;
    ruleStr[356] = strdup("operator: OROR");
    
    ruleArray[357] =  OVERLOAD_OP33  ;
    ruleStr[357] = strdup("operator: PLUSPLUS");
    
    ruleArray[358] =  OVERLOAD_OP34  ;
    ruleStr[358] = strdup("operator: MINUSMINUS");
    
    ruleArray[359] =  OVERLOAD_OP35  ;
    ruleStr[359] = strdup("operator: COMMA");
    
    ruleArray[360] =  OVERLOAD_OP36  ;
    ruleStr[360] = strdup("operator: ARROWSTAR");
    
    ruleArray[361] =  OVERLOAD_OP37  ;
    ruleStr[361] = strdup("operator: ARROW");
    
    //ruleArray[362] =  OVERLOAD_OP38  ;
    //ruleArray[363] =  OVERLOAD_OP39  ;
    //ruleArray[364] =  EXCEPT_HANDLERSQ   ;
    //ruleStr[364] = strdup("handler_seq: handler handler_seq_opt");
    
    ruleArray[365] =  EXCEPT_DECL   ;
    ruleStr[365] = strdup(
     "exception_declaration: type_specifier_seq declarator");
    
    ruleArray[366] =  EXCEPT_DECL2 ;
    ruleStr[366] = strdup(
     "exception_declaration: type_specifier_seq abstract_declarator");
    
    //ruleArray[367] =  EXCEPT_DECL3  ;
    //ruleArray[368] =  EXCEPT_TYPEID   ;
    ruleArray[369] =  EXCEPT_TYPEID2  ;
    ruleStr[369] = strdup("type_id_list: type_id_list COMMA type_id");
    
    //ruleArray[370] =  EPSILON_DECLSEQ   ;
    //ruleArray[371] =  EPSILON_NESTEDNM   ;
    //ruleArray[372] =  EPSILON_EXLIST   ;
    //ruleArray[373] =  EPSILON_DCOLON   ;
    //ruleStr[373] = strdup("COLONCOLON_opt: COLONCOLON");
    
    //ruleArray[374] =  EPSILON_NEWPLACE   ;
    //ruleArray[375] =  EPSILON_NEWINIT   ;
    //ruleArray[376] =  EPSILON_NEWDECL   ;
    //ruleArray[377] =  EPSILON_EXP   ;
    //ruleArray[378] =  EPSILON_STMTSEQ   ;
    //ruleArray[379] =  EPSILON_COND   ;
    //ruleArray[380] =  EPSILON_INIT   ;
    //ruleArray[381] =  EPSILON_CONSTEX   ;
    //ruleArray[382] =  EPSILON_ABDECL   ;
    //ruleArray[383] =  EPSILON_TYPESEQ   ;
    //ruleArray[384] =  EPSILON_DIRABDECL   ;
    //ruleArray[385] =  EPSILON_CTOR   ;
    //ruleArray[386] =  EPSILON_COMMA   ;
    //ruleStr[386] = strdup("COMMA_opt: COMMA");
    
    //ruleArray[387] =  EPSILON_MEMSPEC   ;
    //ruleArray[388] =  EPSILON_SM  ;
    //ruleStr[388] = strdup("SEMICOLON_opt: SM");
     
    //ruleArray[389] =  EPSILON_CONVDECL   ;
    //ruleArray[390] =  EPSILON_HANDSEQ   ;
    //ruleArray[391] =  EPSILON_ASSGNEX   ;
    //ruleArray[392] =  EPSILON_TYPEID   ;
    
    ruleArray[393] = DECLARATORS_FUN3;
    ruleStr[393] = strdup("decl_specifier_seq declarator ctor_initializer function_body");
    
    ruleArray[394] = ADDALLLIBS;
    ruleStr[394] = strdup("ALL LIBRARIES");
    
    ruleArray[395] = ADDSTRLIBS;
    ruleStr[395] = strdup("STRING ONLY LIBRARY");
    /* now check memory */
    for(i = 0; i < NUMOFRULES; i++)
    {
        if(ruleArray[i] > 0 && ruleStr[i] == NULL)
        {
            fprintf(stderr, "Error! Out of memory.");
            exit(10);   
        }   
    }
}

char *readable_rule(int rule)
{
   int i;
   char *retval = "no rule found!";
   if(parser_debug == 1)
   {
       for(i = 0; i < NUMOFRULES; i++)
       {
            if(ruleArray[i] == rule)
            {
                retval = ruleStr[i];
                break;   
            }
       }
   }
   return retval;
}
