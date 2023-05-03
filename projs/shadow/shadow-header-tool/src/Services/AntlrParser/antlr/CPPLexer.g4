grammar CPPLexer;

translationUnit: declarationseq? EOF;

declarationseq: declaration+;

declaration:
	  namespaceDefinition
	| classDeclaration
	| macroCall;
	
	
nestedNameSpecifier: (Identifier) (Doublecolon (Identifier))*;
	
namespaceDefinition: Inline? Namespace nestedNameSpecifier LeftBrace declarationseq? RightBrace;

/*Classes*/

classDeclaration: Class attributeSpecifierSeq? classIdentifier (Colon inheritanceSpecifierSeq)? LeftBrace (memberSpecification|.)*? RightBrace Semi;

classIdentifier: nestedNameSpecifier;

inheritanceSpecifierSeq: inheritanceSpecifier (',' inheritanceSpecifier)*;

inheritanceSpecifier: Public nestedNameSpecifier?;

/*members*/
memberSpecification: (memberdeclaration | accessSpecifier Colon)+;

accessSpecifier: Public | Private | Protected;

memberdeclaration: 
      macroCall
    | declSpecifierSeq;

declSpecifierSeq: attributeSpecifierSeq? Float Identifier Assign (.)? Semi;

/*Attributes*/

attributeSpecifierSeq: attributeSpecifier+;

attributeSpecifier: LeftBracket LeftBracket attributeList? RightBracket RightBracket;

attributeList: attribute (Comma attribute)* Ellipsis?;

attribute: nestedNameSpecifier attributeArgumentClause?;

attributeArgumentClause: LeftParen balancedTokenSeq? RightParen;

balancedTokenSeq: balancedtoken+;

balancedtoken:
	LeftParen balancedTokenSeq RightParen
	| LeftBracket balancedTokenSeq RightBracket
	| LeftBrace balancedTokenSeq RightBrace
	| ~(
		LeftParen
		| RightParen
		| LeftBrace
		| RightBrace
		| LeftBracket
		| RightBracket
	)+;


/*Mactros*/

macroCall: Identifier LeftParen .? RightParen Semi;



MultiLineMacro:
	'#' (~[\n]*? '\\' '\r'? '\n')+ ~ [\n]+ -> channel (HIDDEN);

Directive: '#' ~ [\n]* -> channel (HIDDEN);



/*Keywords*/
Inline: 'inline';

Namespace: 'namespace';

Class: 'class';

Public: 'public';

Private: 'private';

Protected: 'protected';


Void: 'void';
Bool: 'bool';
Char: 'char';
Char16: 'char16_t';
Char32: 'char32_t';
Int: 'int';
Long: 'long';
Float: 'float';


/*Operators*/
LeftParen: '(';

RightParen: ')';

LeftBracket: '[';

RightBracket: ']';

LeftBrace: '{';

RightBrace: '}';

Assign: '=';

Colon: ':';

Doublecolon: '::';

Semi: ';';

DotStar: '.*';

Ellipsis: '...';

Comma : ',';



Identifier: Identifiernondigit (Identifiernondigit | DIGIT)*;

fragment Identifiernondigit: NONDIGIT;

fragment NONDIGIT: [a-zA-Z_];

fragment DIGIT: [0-9];


Whitespace: [ \t]+ -> skip;

BlockComment: '/*' .*? '*/' -> skip;

LineComment: '//' ~ [\r\n]* -> skip;