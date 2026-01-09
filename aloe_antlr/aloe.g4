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
    | funCall
    | funDeclaration 
    ;


/********************/
/*      Object       */
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

funCall
    : (identifier|funDeclaration) '(' argumentList ')'
    ;

argumentList
    : argument?
    | argument (',' argument)+
    ;

argument 
    : StringLiteral
    | DigitSequence
    | identifier
    | varDeclaration
    ;



/* General Decalaration */
identifier
    : Identifier
    ;

/* Function Call */

      

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

fragment CharSequence
    : Char+
    ;

fragment Char
    : ~["\\\r\n]
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

