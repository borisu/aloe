grammar aloe;

options
{
    exportMacro='';
}

prog
    : declarationStatementList EOF
    ;

declarationStatementList
    : (declarationStatement ';'?)*
    ;

declarationStatement
    : objectDeclaration 
    | varDeclaration
    | funDeclaration 
    ;


executionStatement
    : objectDeclaration 
    | varDeclaration
    | funDeclaration 
    | expression
    ;

/********************/
/*      Object      */
/********************/

objectDeclaration  
    : 'object' (identifier)? (inheritanceChain)?  '('  varList ')' 
    ;  

inheritanceChain
    : ('>' (type))+
    ;

/********************/
/*      Types       */
/********************/

int 
    : IntType
    ;

void 
    : VoidType
    ;

char 
    : CharType
    ;

double
    : DoubleType
    ;

opaque
    : OpaqueType
    ;


builtinType 
    : int
    | char
    | double
    | opaque
    | void
    ;

type 
    : builtinType
    | objectDeclaration
    | funDeclaration
    | pointerType
    | identifier
    ;

pointerType
    : '^' type
    ;


/********************/
/* Var Decalaration */
/********************/

varDeclaration
    : 'var' identifier? ':' type
    ;

varList
    : varDeclaration?  
    | varDeclaration (',' varDeclaration)+
    ;


/********************/
/* Fun Decalaration */
/********************/

funDeclaration  
    : 'fun' identifier? ':' funType '{' (executionStatement ';'? )* '}'
    ; 

funType
    :  type '(' varList ')'
    ;

/*****************************/
/*   Identifiers & Literals  */
/*****************************/

/* General Decalaration */
identifier
    : Identifier
    ;

literal
    : DigitSequence
    | StringLiteral+
    | CharacterConstant
    ;
 
/********************/
/*   Expressions    */
/********************/

expression
    // -------- Precedence 0
    : identifier                                           #expr_identifier
    | literal                                              #expr_literal
    | '(' expression ')'                                   #expr_bracketed
    // -------- Precedence 1                                
    | expression '++'                                      #expr_sfxplusplus
    | expression '--'                                      #expr_sfxminmin
    | expression '(' argumentExpressionList? ')'           #expr_funcall
    | expression '[' expression ']'                        #expr_index
    | expression '.' identifier                            #expr_dot
    | expression '->' identifier                           #expr_arrow
    // -------- Precedence 2                                
    | '++' expression                                      #expr_preplusplus
    | '--' expression                                      #expr_preminmin
    | '+' expression                                       #expr_plus
    | '-' expression                                       #expr_min
    | '!' expression                                       #expr_not
    | '~' expression                                       #expr_bwsnot
    | '(' type ')' expression                              #expr_cast
    | '*' expression                                       #expr_deref
    | '&' expression                                       #expr_addressof
    | 'sizeof'  '(' expression ')'                         #expr_sizeofexpr
    | 'sizeof'  '(' type ')'                               #expr_sizeoftype
    // -------- Precedence 3                                
    | expression '*'  expression                           #expr_mult
    | expression '/'  expression                           #expr_div
    | expression '%'  expression                           #expr_mod
    // -------- Precedence 4                                
    | expression '+'  expression                           #expr_add
    | expression '-'  expression                           #expr_sub
    // -------- Precedence 5                                expr_
    | expression '<<'  expression                          #expr_shiftleft
    | expression '>>'  expression                          #expr_shiftright
    // -------- Precedence 6                                expr_
    | expression '<'  expression                           #expr_less
    | expression '<=' expression                           #expr_lesseeq
    | expression '>'  expression                           #expr_more
    | expression '>=' expression                           #expr_moreeq
    // -------- Precedence 7                                
    | expression '=='  expression                          #expr_logicaleq
    | expression '!='  expression                          #expr_noteq
    // -------- Precedence 8                                
    | expression '&'  expression                           #expr_and
    // -------- Precedence 9                                
    | expression '^'  expression                           #expr_xor
    // -------- Precedence 10                               
    | expression '|'  expression                           #expr_or
    // -------- Precedence 11                               
    | expression '&&'  expression                          #expr_logicaland
    // -------- Precedence 12                               
    | expression '||'  expression                          #expr_logicalor
    // -------- Precedence 13                               
    | expression '?' expression ':'  expression            #expr_ternary
    // -------- Precedence 14                               
    | expression '='  expression                           #expr_assign
    | expression '+='  expression                          #expr_addassign
    | expression '-='  expression                          #expr_subassign
    | expression '*='  expression                          #expr_multassign
    | expression '/='  expression                          #expr_divassign
    | expression '%='  expression                          #expr_modassign
    | expression '<<='  expression                         #expr_shiftleftassign
    | expression '>>='  expression                         #expr_shiftrightassign
    | expression '&='  expression                          #expr_andassign
    | expression '^='  expression                          #expr_xorassign
    | expression '|='  expression                          #expr_orassign
    // -------- Precedence 15                               
    | '(' argumentExpressionList? ')'                      #expr_comma
    ;

 
argumentExpressionList
    : expression (',' expression)*
    ;

////////////////////////////////////////////////////////////////////
//
// LEXER
//
////////////////////////////////////////////////////////////////////

/* Reserved Words */
Replaces 
    : 'replaces'
    ;

VoidType 
    : 'void'
    ;

OpaqueType 
    : 'opaque'
    ;

IntType 
    : 'int'
    ;

CharType 
    : 'char'
    ;

DoubleType 
    : 'double'
    ;

fragment Nondigit
    : [a-zA-Z_]
    ;

fragment Digit
    : [0-9]
    ;

Identifier
    : (Nondigit)(Nondigit|Digit)*
    ;

DigitSequence
    : Digit+
    ;

StringLiteral
    :  '"' CharSequence? '"'
    ;

CharacterConstant
    : '\'' Char '\''
    ;

fragment CharSequence
    : Char+
    ;

fragment Char   
    : ~["\\\r\n]
    | EscapeSequence
    ;

fragment EscapeSequence
    : SimpleEscapeSequence 
    ;

fragment SimpleEscapeSequence
    : '\\' ['"?abfnrtv\\]
    ;

Whitespace
    : [ \t]+ -> channel(HIDDEN)
    ;

Newline
    : ('\r' '\n'? | '\n') -> channel(HIDDEN)
    ;

BlockComment
    : '/*' .*? '*/' -> channel(HIDDEN)
    ;

LineComment
    : '//' ~[\r\n]* -> channel(HIDDEN)
    ;

