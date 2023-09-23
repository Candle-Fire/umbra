grammar test;

/*
 * Parser Rules
 */
operation  : WHITESPACE* NUMBER WHITESPACE* '+' WHITESPACE* NUMBER;
/*
 * Lexer Rules
 */
NUMBER     : [0-9]+ ;
WHITESPACE : ' ' -> skip ;