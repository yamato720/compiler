#pragma once
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <Afxtempl.h >
#include <malloc.h>

#define FILENAMESIZE 512    //文件名的最大长度
#define NUMOFWORD 12       //保留字的个数
#define MAXNUMOFNAMETABLE 1000  //名字表的最大项数
#define MAXNUMOFBLOCKTABLE 100 //程序体表的最大项数
#define MAXNUMOFARRAYTABLE  100//数组信息表的最大项数
#define MAXSYMNAMESIZE 255  //符号的最大字符数
#define MAXVARNAMESIZE 255  //变量名的最大字符数
#define MAXLEVELDEPTH 100   //过程嵌套的最大深度
#define MAXNUMOFCODEADDRESS 10000  //最大的地址空间

typedef enum _SYMBOL  //token的定义
{
	NUL,//0
	IDENFR,//1
	INTCON, // IntConst
	STRCON, //FormatString
	MAINTK, // main
	CONSTTK, // const
	INTTK, //int
	BREAKTK, //break
	CONTINUETK, //continue
	IFTK, //if
	ELSETK, // else
	NOT, // !
	AND, // &&
	OR, // ||
	WHILETK, //while
	GETINTTK, //getint
	PRINTFTK, //printf
	RETURNTK, // return
	PLUS, // +
	MINU, // -
	VOIDTK, // void
	MULT, // *
	DIV, // /
	MOD, // %
	LSS, // <
	LEQ, // <=
	GRE, // >
	GEQ, // >=
	EQL, // ==
	NEQ, // !=
	ASSIGN, // =
	SEMICN, // ;
	COMMA, // ,
	LPARENT, // (
	RPARENT, // )
	LBRACK, // [
	RBRACK, // ]
	LBRACE, // {
	RBRACE, // }
	ARRAY
} SYMBOL;

char symbol_name[39][20] =  //token的定义
{
	"NUL",//0
	"IDENFR",//1
	"INTCON", // IntConst
	"STRCON", //FormatString
	"MAINTK", // main
	"CONSTTK", // const
	"INTTK", //int
	"BREAKTK", //break
	"CONTINUETK", //continue
	"IFTK", //if
	"ELSETK", // else
	"NOT", // !
	"AND", // &&
	"OR", // ||
	"WHILETK", //while
	"GETINTTK", //getint
	"PRINTFTK", //printf
	"RETURNTK", // return
	"PLUS", // +
	"MINU", // -
	"VOIDTK", // void
	"MULT", // *
	"DIV", // /
	"MOD", // %
	"LSS", // <
	"LEQ", // <=
	"GRE", // >
	"GEQ", // >=
	"EQL", // ==
	"NEQ", // !=
	"ASSIGN", // =
	"SEMICN", // ;
	"COMMA", // ,
	"LPARENT", // (
	"RPARENT", // )
	"LBRACK", // [
	"RBRACK", // ]
	"LBRACE", // {
	"RBRACE" // }
};

class SYMLIST :public CList<SYMBOL, SYMBOL>
{
};


SYMLIST DECLBEGSYS, STATBEGSYS, FACBEGSYS, CONSTBEGSYS, TYPEBEGSYS;

typedef enum NAMEKIND  //标识符的种类的定义
{
	KONSTANT,  //常量
	TYPEL,  //无类型
	VARIABLE,  //变量
	PROCEDURE  //过程名
} OBJECT;

typedef enum VARTYPES  //变量的类型的定义
{
	NOTYP,  //（暂时）没有类型
	INTS,  //整数类型
	CHARS,  //字符类型
	BOOLS,  //布尔类型
	ARRAYS  //数组类型
} TYPES;

typedef enum _OPCOD  //操作码的定义
{
	LIT, LIT1, LOD, ILOD, LODA, LODT, LODB, STO, CPYB, JMP, JPC, RED, WRT, _CAL, RETP, UDIS, OPAC, ENTP,
	ENDP, ANDS, ORS, NOTS, IMOD, MUS, ADD, ADD1, SUB, _MULT, IDIV, EQ, NE, LS, LE, GT, GE
} OPCOD;

typedef struct _TYPEITEM  //辅助节点定义
{
	TYPES typ;
	int ref;
} TYPEITEM;

typedef struct _CONSTREC
{
	TYPES type;
	int value;
} CONSTREC;

typedef struct SYMBOL_ITEM  //从源文件读出的符号链表里面的节点的定义
{
	int lineNumber;  //符号所处的行数
	SYMBOL type;  //符号的类型
	union SYMBOL_VALUE  //根据不同的符号类型符号可以有不同的类型的值
	{
		int iValue;  //如果是整数常量或者字符常量，则存放其值
		char* lpValue;  //如果是符号，则存放其首字符的地址
	}value;
	char* name;
	struct SYMBOL_ITEM* next;  //指向下一个节点
}SymbolItem;


typedef struct _INSTRUCTION  //指令结构定义
{
	int lineNumber;
	OPCOD func;
	int level;
	int address;
} INSTRUCTION;
INSTRUCTION CODE[MAXNUMOFCODEADDRESS];  //指令数组定义
int CX;  //指令数组索引


typedef struct _NAMETABITEM  //名字表里面的纪录的定义
{
	char name[MAXSYMNAMESIZE];
	OBJECT kind;
	TYPES type;
	int level;// 表示标识符所在的作用域层级
	int normal;// 用于标识是否为正常标识符（可能与错误或异常标识符区分）
	int ref; // 引用计数，用于跟踪标识符的引用次数
	union SYMBOL_VALUE  //根据不同的符号类型符号可以有不同的类型的值
	{
		int value;  //如果是整数常量或者字符常量，则存放其值
		int size;
		int address;
	}unite;
	int link;
	//struct NAMETAB_ITEM *Next;
}NAMETABITEM;
NAMETABITEM NAMETAB[MAXNUMOFNAMETABLE];  //名字表定义
int TX; //名字表索引

typedef struct PROGRAM_BLOCK_ITEM  //程序体表里的纪录的的定义
{
	int lastPar; // 指向程序块的上一个参数的索引
	int last;// 指向程序块的上一个局部变量的索引
	int pSize;// 程序块的参数大小
	int vSize;// 程序块的局部变量大小
	//struct PROGRAM_BLOCK_ITEM *Next;
}BLOCKITEM;
BLOCKITEM BTAB[MAXNUMOFBLOCKTABLE];  //程序表定义
int BX;  //程序体表索引

typedef struct ARRAY_INFORMATION_ITEM  //数组信息表定义
{
	TYPES intType;
	TYPES eleType;
	int low;
	int high;
	int elSize;
	int size;
	int elRef;
	//struct ARRAY_INFORMATION_ITEM *Next;
}ARRAYITEM;
ARRAYITEM  ATAB[MAXNUMOFARRAYTABLE];  //数组信息表定义
int AX;  //数组信息表索引

int JUMADRTAB[300];  //跳转表的定义
int JX;  //跳转表索引



int DISPLAY[MAXLEVELDEPTH];  //DISPLAY表定义
int displayLevel;  //DISPLAY表的索引

int DX;//数据空间分配的指示器,每个程序体都要用到一次

//#include "global.h"
//#include "symlist.h"
char ObjCodeScript[GE + 1][1000] =
{
	"LIT    %4d ,%4d      ------>  装入常量",
	"LIT1   %4d ,%4d      ------>  装入常量（用于计算数组元素地址）",
	"LOD    %4d ,%4d      ------>  装入变量值",
	"ILOD   %4d ,%4d      ------>  间接装入",
	"LODA   %4d ,%4d      ------>  装入变量地址",
	"LODT   %4d ,%4d      ------>  装入栈顶值为地址的内容",
	"LODB   %4d ,%4d      ------>  装入长度为A的块",
	"STO    %4d ,%4d      ------>  将栈顶值存入栈顶次值所指单元",
	"CPYB   %4d ,%4d      ------>  传送长度为A的块",
	"JMP    %4d ,%4d      ------>  无条件跳转",
	"JPC    %4d ,%4d      ------>  栈顶值为0时跳转",
	"READ   %4d ,%4d      ------>  读指令",
	"WRITE  %4d ,%4d      ------>  写指令",
	"CALL   %4d ,%4d      ------>  转子",
	"RETP   %4d ,%4d      ------>  过程返回",
	"UDIS   %4d ,%4d      ------>  调整Display",
	"OPAC   %4d ,%4d      ------>  打开活动记录",
	"ENTP   %4d ,%4d      ------>  进入过程",
	"ENDP   %4d ,%4d      ------>  程序结束",
	"AND    %4d ,%4d      ------>  与",
	"OR     %4d ,%4d      ------>  或",
	"NOT    %4d ,%4d      ------>  非",
	"IMOD   %4d ,%4d      ------>  模",
	"MUS    %4d ,%4d      ------>  求负",
	"ADD    %4d ,%4d      ------>  加",
	"ADD1   %4d ,%4d      ------>  加（用于计算数组元素地址）",
	"SUB    %4d ,%4d      ------>  减",
	"MULT   %4d ,%4d      ------>  乘",
	"IDIV   %4d ,%4d      ------>  除",
	"EQ     %4d ,%4d      ------>  ==",
	"NEQ    %4d ,%4d      ------>  !=",
	"LSS    %4d ,%4d      ------>  <",
	"LEQ    %4d ,%4d      ------>  <=",
	"GTR    %4d ,%4d      ------>  >",
	"GEQ    %4d ,%4d      ------>  >="
};
