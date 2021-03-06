#ifndef DATA_H
#define DATA_H
#include <stdarg.h>


enum{
	CHAR=300,
	SHORT,
	INT,
	LONG,
	UNSIGNED,
	UCHAR,
	USHORT,
	UINT,
	ULONG,
	FLOAT,
	DOUBLE,
	VOID,
	STRING,
	POINTER,
	ARRAY,
	ADDRESS,
	FUNCTION,
	STRUCT,
	UNION,
	TYPEDEF,
	BASIC_TYPE,
	ID,
	STRUCT_UNION,
	CONSTANT,
	TYPE,
	TERMINAL,
	GLOBAL,
	LOCAL,
	PARAMETER,
	SEQ,
	ASSIGN	,
	OR,
	AND,
	TOP_OR,
	TOP_AND,
	TOP_NOT,
	BOR,
	XOR,
	BAND,
	EQ	,
	NEQ,
	SHL	,
	SHR,
	LT	,
	GT	,
	LE	,
	GE,
	ADD	,
	SUB,
	MUL	,
	DIV,	
	MOD,
	NOT	,
	BNEQ,	
	PLUS	,
	MINUS	,
	INDIR	,
	DEREF	,
	INCR	,
	DECR	,
	SIZEOF	,
	FIELD	,
	FIELD_INDIR	,
	CAST,
	CALL,
	RIGHT,
	COND,
	//leaf
	ADDR,
	CONST,
	//storage
	MEM,
	IMM,
	IND,
	LAB,
	
	
	NONE=0,
};

#endif

