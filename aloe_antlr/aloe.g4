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
    : (declaration)* EOF
    ;

declaration
    : objectDeclaration 
    | varDeclaration
    | funDeclaration 
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

pointerType
    : PointerArrow (builtinType | identifier | pointerType)
    ;

type 
    : builtinType
    | pointerType
    ;

/* Object Decalaration */

objectDeclaration  
    : 'object' inheritanceChain '{' declaration* '}'
    ;  

inheritanceChain
    :  identifier ('>' inheritedType)*
    ;

inheritedType
    : inheritedObjectType
    | inheritedVirtualType
    ;

inheritedObjectType 
    : identifier;

inheritedVirtualType :
    | PointerArrow identifier
    ;

/* Var Decalaration */

varDeclaration
    : 'var' identifier ':' type
    ;

/* Fun Decalaration */

funDeclaration  
    : 'fun' identifier ':' funType '{''}'
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

