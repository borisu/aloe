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

primaryExpression
    : identifier            # IdentifierExpr 
    | literal               # LiteralExpr 
    | '(' expression ')'    # ParenthesizedExpr
    ;
 
expression
    : primaryExpression (
        '[' expression ']'
        | '(' argumentExpressionList? ')'
        | ('.' | '->') Identifier
        | '++'
        | '--'
     )*
    | primaryExpression            
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

