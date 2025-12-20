grammar aloe;

options
{
    exportMacro='';
}


////////////////////////////////////////////////////////////////////
//
// PARSER
//
////////////////////////////////////////////////////////////////////
prog
    : (statement ';'? )* EOF
    ;

statement
    : objectDeclaration 
    | varDeclaration
    | funDeclaration 
    ;

/* OBJECT DECLARATION */

objectDeclaration  
    : 'object' (identifier)? (inheritanceChain)? Begin statement* End
    ;  

inheritanceChain
    :   ('>' (type))+
    ;

/* TYPES */

int 
    : IntType
    ;

void 
    : VoidType
    ;

char 
    : CharType
    ;

builtinType 
    : int
    | char
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
    : PointerArrow type
    ;


/* Var Decalaration */

varDeclaration
    : 'var' identifier? ':' type
    ;

/* Fun Decalaration */

funDeclaration  
    : 'fun' identifier? ':' funType Begin End
    ; 

funType
    :  type '(' varList ')' (replaces identifier)?
    ;

varList
    : varDeclaration?  
    | varDeclaration (',' varDeclaration)+
    ;
    
replaces
    : Replaces
    ;

/* General Decalaration */

identifier
    : Identifier
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

fragment Nondigit
    : [a-zA-Z_]
    ;

fragment Digit
    : [0-9]
    ;

PointerArrow
    : '^'
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

Begin
    : '{'
    ;

End
    : '}'
    ;
