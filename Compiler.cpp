// Compiler.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#pragma warning(disable : 4996)
#define _AFXDLL
#include <iostream>
#include <string>
using namespace std;
#include "common.h"

SYMLIST * listsAdd(SYMLIST * list1,SYMLIST * list2);
SYMLIST * listAddSym(SYMLIST * list,SYMBOL sym);
int SYMINLIST(SYMBOL sym,SYMLIST * list);
void COPYLIST(SYMLIST * list1,SYMLIST * list2);

void error(int);
int nError;

void INITIAL();
void ENTERID(const char* name, OBJECT kind, TYPES type, int value);
void ENTERPREID();

void getSymbols(FILE *);
void getASymbol();
void destroySymbols();
SymbolItem *Symbols=NULL;
SymbolItem *CurSymbol=NULL;

void GEN(OPCOD func,int level,int address);
void WriteObjCode(char *);
void WriteCodeList(char *);

void ENTERARRAY(SYMLIST * list,TYPES type,int low,int high);
void ENTERBLOCK();
void ENTER(OBJECT object);
int GETPOSITION(char * id);
void CONSTANT(SYMLIST * list,CONSTREC & constRec);
void ARRAYTYP(SYMLIST * list,int & aref,int & arsz);
void TYP(SYMLIST * list,TYPES & tp,int & rf,int & sz);
void PARAMENTERLIST(SYMLIST * list);

void CONSTDECLARATION(SYMLIST * list);
void TYPEDECLARATION(SYMLIST * list);
void VARDECLARATION(SYMLIST * list);
void PROCDECLARATION(SYMLIST * list);

void FACTOR(SYMLIST * list,TYPEITEM & typeItem);
void TERM(SYMLIST * list,TYPEITEM & typeItem);
void SIMPLEEXPRESSION(SYMLIST * list,TYPEITEM & typeItem);
void EXPRESSION(SYMLIST * list,TYPEITEM & typeItem);
void ARRAYELEMENT(SYMLIST * list,TYPEITEM & typeItem);

void ASSIGNMENT(SYMLIST * list);
void IFSTATEMENT(SYMLIST * list);
void WHILESTATEMENT(SYMLIST * list);
void COMPOUND(SYMLIST * list);
void STANDPROC(SYMLIST * list,int i);
void CALL(SYMLIST * list);
void STATEMENT(SYMLIST * list);
void BLOCK(SYMLIST * list,int level);

void error(int errCode)   //负责显示出错信息的函数
{
	char errorScript[][100] =
	{
		"应该是\':\'",//0
		"应该是\';\'",//1
		"应该是\')\'",//2
		"应该是\'(\'",//3
		"应该是\'[\'",//4
		"应该是\']\'",//5
		"应该是\'=\'",//6
		"应该是\':=\'",//7
		"应该是\'.\'",//8
		"应该是\'do\'",//9
		"应该是\'of\'",//10
		"应该是\'then\'",//11
		"应该是\'end\'",//12
		"应该是\'program\'",//13
		"应该是标识符",//14
		"应该是类型标识符",//15
		"应该是变量",//16
		"应该是常量或常量标识符",//17
		"应该是过程名",//18
		"数组上下界大小关系错误",//19
		"数组定义时，下标元素类型错误",//20
		"数组定义时，上下界类型不一致",//21
		"引用时，数组元素下标元素类型出错",//22
		"下标个数不正确",//23
		"数组表溢出",//24
		"名字表溢出",//25
		"程序体表溢出",//26
		"系统为本编译程序分配的堆不够用",//27
		"实参与形参个数不等",//28
		"实参与形参类型不一致",//29
		"实参个数不够",//30
		"程序体内符号重定义",//31
		"if或while后面表达式必须为布尔类型",//32
		"标识符没有定义",//33
		"过程名或类型名不能出现在表达式中",//34
		"操作数类型错误",//35
		"类型定义出错",//36
		"类型不一致",//37
		"不认识的字符出现在源程序",//38
		"不能打开.lst文件",//39
		"不能打开.pld文件",//40
		"不能打开.lab文件",//41
		"应该是\'..\'",//42
		"分析时缺少标识符",//43
	};
}
int Feof(FILE* fp)//判断是否到了源文件尾
{
	int getChar;
	getChar = fgetc(fp);
	if (getChar == -1)
	{
		if (feof(fp))
			return 1;//如果是，返回“真”
	}
	else
		fseek(fp, -1, SEEK_CUR);//否则，将指向文件流的指针向后移动一个字符
	return 0;
}

SYMBOL GetReserveWord(char* nameValue)//判断得到的符号是否是保留字
{
	int i;

	char reserveWord[NUMOFWORD][20] =
	{
		"main", "const", "int", "break", "continue", "if", "else", "while", "getint", "printf", "return", "void"
	};
	SYMBOL reserveType[NUMOFWORD] =
	{
		MAINTK, CONSTTK, INTTK, BREAKTK, CONTINUETK, IFTK, ELSETK, WHILETK, GETINTTK, PRINTFTK, RETURNTK, VOIDTK
	};

	for (i = 0; i < NUMOFWORD; i++)  //这里采用了遍历的做法，但是效率不高，可以考虑采用二叉查找法
		if (!stricmp(reserveWord[i], nameValue))
		{
			// printf("%d\n", reserveType[i]);
			return reserveType[i];
		} //查询到了是保留字，返回对应保留字
	return (SYMBOL)0;
}

void AddSymbolNode(SymbolItem** current, int lineNumber, SYMBOL type, int iValue)  //在词法分析的时候向符号列表中加入一个分析出来的符号
{
	(*current)->next = new SymbolItem;
	if (!(*current)->next)
	{
		error(27);//系统为本编译程序分配的堆不够用
		exit(4);
	}
	(*current) = (*current)->next;
	(*current)->lineNumber = lineNumber;
	(*current)->type = type;
	(*current)->value.iValue = iValue;
	(*current)->next = NULL;
}



void getSymbols(FILE *srcFile)  //从源文件读入字符，获得符号链表
{
	int lineNumber=1;
	char nameValue[MAXSYMNAMESIZE];
	int nameValueint;
	char readChar;
	SymbolItem head,*current=&head; // 标识符链表

	printf("\n进行词法分析  -->-->-->-->-->-->-->-->  ");

	while(!Feof(srcFile)) // 检测是否读到了尽头。如果没有，指针后移
	{
		readChar=fgetc(srcFile);

		if(iscsymf(readChar))
		{// 如果是标识符，即第一个字母是字母或下划线
			nameValueint=0;
			do
			{
				nameValue[nameValueint++]=readChar;
				readChar=fgetc(srcFile);
				if(Feof(srcFile)) 
					break;
			}while(iscsym(readChar) || isdigit(readChar)); //下一字符判断是否符合要求
			nameValue[nameValueint]='\0';
			fseek(srcFile,-1,SEEK_CUR);
			current->next=new SymbolItem;
			current=current->next;
			current->lineNumber=lineNumber; //位于第几行，便于报错
			if(!(current->type=GetReserveWord(nameValue)))// 如果不是保留字
			{
				current->type= IDENFR;
				current->value.lpValue = new char[nameValueint];
				
				strcpy(current->value.lpValue, nameValue);
				
			}
			current->name = new char[nameValueint];
			strcpy(current->name, nameValue);
			
			current->next=NULL; 
		}
		else if(isdigit(readChar))
		{// 是个数字字符
			nameValueint=0;
			do
			{
				nameValue[nameValueint++]=readChar;
				readChar=fgetc(srcFile);
				if(Feof(srcFile)) 
					break;
			}while(isdigit(readChar)); //下一字符判断是否符合要求
			nameValue[nameValueint]=0;
			fseek(srcFile,-1,SEEK_CUR);
			AddSymbolNode(&current,lineNumber,INTCON,atoi(nameValue));
			current->name = new char[nameValueint];
			strcpy(current->name, nameValue);

		}
		else if (readChar=='\"')
		{// 是个字符串
			nameValueint = 0;
			do
			{
				nameValue[nameValueint++] = readChar;
				readChar = fgetc(srcFile);
				if (Feof(srcFile))
					break;
			} while (readChar != '\"'); //下一字符判断是否符合要求
			nameValue[nameValueint++] = readChar;
			nameValue[nameValueint] = 0;
			current->next = new SymbolItem;
			current = current->next;
			current->lineNumber = lineNumber; //位于第几行，便于报错
			current->type = STRCON;
			current->value.lpValue = new char[nameValueint];
			strcpy(current->value.lpValue, nameValue);
			current->name = new char[nameValueint];
			strcpy(current->name, nameValue);
			current->next = NULL;
		}
		else switch(readChar)
		{
			case '	':				//字符 'tab'
			case ' ':	
				break;
			case '\n':
				break;
			case '\r':
				lineNumber++;
				break;
			case '=':
				if(Feof(srcFile))
					break;
				readChar=fgetc(srcFile);
				if(readChar=='=')
				{
					AddSymbolNode(&current,lineNumber,EQL,0); // 相等
					current->name = new char[2];
					strcpy(current->name, "==");
				}
				else
				{
					fseek(srcFile,-1,SEEK_CUR);
					AddSymbolNode(&current,lineNumber,ASSIGN,0);// 赋值
					current->name = new char[1];
					strcpy(current->name, "=");
				}
				break;
			case '<':
				if(Feof(srcFile))
					break;
				readChar=fgetc(srcFile);
				if (readChar == '=') {
					AddSymbolNode(&current,lineNumber,LEQ,0);
					current->name = new char[2];
					strcpy(current->name, "<=");
				}
					
				else
				{
					fseek(srcFile,-1,SEEK_CUR);
					AddSymbolNode(&current,lineNumber,LSS,0);
					current->name = new char[1];
					strcpy(current->name, "<");
				}
				break;
			case '>':
				if(Feof(srcFile))
					break;
				readChar=fgetc(srcFile);
				if (readChar == '=') {
					AddSymbolNode(&current,lineNumber,GEQ,0);
					current->name = new char[2];
					strcpy(current->name, ">=");
				}
					
				else
				{
					fseek(srcFile,-1,SEEK_CUR);
					AddSymbolNode(&current,lineNumber,GRE,0);
					current->name = new char[1];
					strcpy(current->name, ">");
				}
				break;
			case '+':
				AddSymbolNode(&current,lineNumber,PLUS,0);
				current->name = new char[1];
				strcpy(current->name, "+");
				break;
			case '-':
				AddSymbolNode(&current,lineNumber,MINU,0);
				current->name = new char[1];
				strcpy(current->name, "-");
				break;
			case '*':
				AddSymbolNode(&current,lineNumber,MULT,0);
				current->name = new char[1];
				strcpy(current->name, "*");
				break;
			case '/':
				if (Feof(srcFile))
					break;
				readChar = fgetc(srcFile);
				if (readChar == '/')
				{
					while (readChar = fgetc(srcFile)!='\r');
					break;
				}
				else if (readChar == '*')
				{
					while (readChar = fgetc(srcFile) != '*') {
						if (readChar = fgetc(srcFile) == '/')
						{
							break;
						}
						fseek(srcFile, -1, SEEK_CUR);
					}
					break;
				}
				else
				{
					fseek(srcFile, -1, SEEK_CUR);
					AddSymbolNode(&current, lineNumber, DIV, 0);
					current->name = new char[1];
					strcpy(current->name, "/");
				}
				break;
			case '(':
				AddSymbolNode(&current,lineNumber,LPARENT,0);
				current->name = new char[1];
				strcpy(current->name, "(");
				break;
			case ')':
				AddSymbolNode(&current,lineNumber,RPARENT,0);
				current->name = new char[1];
				strcpy(current->name, ")");
				break;
			case '[':
				AddSymbolNode(&current,lineNumber,LBRACK,0);
				current->name = new char[1];
				strcpy(current->name, "[");
				break;
			case ']':
				AddSymbolNode(&current,lineNumber,RBRACK,0);
				current->name = new char[1];
				strcpy(current->name, "]");
				break;
			case ',':
				AddSymbolNode(&current,lineNumber,COMMA,0);
				current->name = new char[1];
				strcpy(current->name, ",");
				break;
			case ';':
				AddSymbolNode(&current, lineNumber, SEMICN, 0);
				current->name = new char[1];
				strcpy(current->name, ";");
				break;
			case '{':
				AddSymbolNode(&current, lineNumber, LBRACE, 0);
				current->name = new char[1];
				strcpy(current->name, "{");
				break;
			case '}':
				AddSymbolNode(&current, lineNumber, RBRACE, 0);
				current->name = new char[1];
				strcpy(current->name, "}");
				break;
			default:
				error(38);
		}   //switch end
	}		//while end
	Symbols=head.next; 
	CurSymbol=Symbols; // 返回第一个获取到的symbol
	if(nError)
	{
		printf("\n%d errors found.",nError);
		exit(2);
	}
	else
		printf("词法分析成功！\n\n");
}

void destroySymbols()  //编译完毕，将符号链表释放
{
	SymbolItem* current, * needDel;
	current = Symbols;
	while (current)
	{
		needDel = current;
		current = current->next;
		delete needDel;
	}
}

void INITIAL()
{
	nError = 0;
	displayLevel = 0;
	DISPLAY[0] = 0;

	DX = 0;
	CX = 0;
	BX = 1;
	TX = -1;
	JX = 0;

	// FIVE SYMLISTS' INITIALIZATION

	DECLBEGSYS.AddHead(CONSTTK);	DECLBEGSYS.AddHead(INTTK);
	DECLBEGSYS.AddHead(MAINTK);	  //包含标识符，表示变量或函数的声明的开始。

	STATBEGSYS.AddHead(LBRACE);	STATBEGSYS.AddHead(BREAKTK);
	STATBEGSYS.AddHead(IFTK);	STATBEGSYS.AddHead(WHILETK); //  可能包含标识符，表示语句的开始。

	FACBEGSYS.AddHead(IDENFR);	FACBEGSYS.AddHead(INTCON);	FACBEGSYS.AddHead(LPARENT); // 可能包含标识符，表示表达式的起始。
	FACBEGSYS.AddHead(NOT);		FACBEGSYS.AddHead(ARRAY);

	TYPEBEGSYS.AddHead(INTTK);	 // 可能包含标识符，表示数据类型的定义的开始。

	CONSTBEGSYS.AddHead(PLUS);	CONSTBEGSYS.AddHead(MINU);	CONSTBEGSYS.AddHead(INTCON);
	CONSTBEGSYS.AddHead(IDENFR);// 可能包含标识符，表示常量的定义的开始。
}
void ENTERID(const char* name, OBJECT kind, TYPES type, int value)  //向符号表中填入信息要用的函数
{
	TX++;
	strcpy(NAMETAB[TX].name, name);
	NAMETAB[TX].link = TX - 1;
	NAMETAB[TX].kind = kind;
	NAMETAB[TX].type = type;
	NAMETAB[TX].ref = 0;
	NAMETAB[TX].normal = 1;
	NAMETAB[TX].level = 0;
	switch (kind)
	{
	case VARIABLE:
	case PROCEDURE:
		NAMETAB[TX].unite.address = value; break;
	case KONSTANT:
		NAMETAB[TX].unite.value = value; break;
	case TYPEL:
		NAMETAB[TX].unite.size = value; break;
	}
	return;
}
void ENTERPREID()  //预先填入符号表的信息，将这些信息作为“过程零”里面的数据
{
	ENTERID("", VARIABLE, NOTYP, 0);
	ENTERID("char", TYPEL, CHARS, 1);
	ENTERID("integer", TYPEL, INTS, 1);
	ENTERID("boolean", TYPEL, BOOLS, 1);
	ENTERID("false", KONSTANT, BOOLS, 0);
	ENTERID("true", KONSTANT, BOOLS, 1);
	ENTERID("read", PROCEDURE, NOTYP, 1);
	ENTERID("write", PROCEDURE, NOTYP, 2);

	BTAB[0].last = TX;
	BTAB[0].lastPar = 1;
	BTAB[0].pSize = 0;
	BTAB[0].vSize = 0;
}

SYMLIST* listsAdd(SYMLIST* list1, SYMLIST* list2)  //两个“集合”相加，返回一个“集合”
{
	SYMLIST* temp = new SYMLIST;
	COPYLIST(temp, list1);
	temp->AddTail(list2);
	return temp;
}

SYMLIST* listAddSym(SYMLIST* list, SYMBOL sym)  //一个“集合”加上一个“元素”，返回一个“集合”
{
	SYMLIST* temp = new SYMLIST;
	COPYLIST(temp, list);
	temp->AddTail(sym);
	return temp;
}

int SYMINLIST(SYMBOL sym, SYMLIST* list)  //判断一个“元素”是否在“集合”里面
{
	for (POSITION pos = list->GetHeadPosition(); pos;)
	{
		SYMBOL temp;
		temp = list->GetNext(pos);
		if (temp == sym)
			return 1;  //如果在，返回非零
	}
	return 0;  //不在，则返回零
}

void COPYLIST(SYMLIST* list1, SYMLIST* list2)  //“集合”之间的拷贝，或者说把2的加到1后面
{
	for (POSITION pos = list2->GetHeadPosition(); pos;)
	{
		SYMBOL temp;
		temp = list2->GetNext(pos);
		list1->AddTail(temp);
	}
}
void ENTERBLOCK()           //每当编译一个过程开始时，保证调用该函数一次
{							//用来向程序体表填入一个表项以登记此过程体
	if (BX == MAXNUMOFBLOCKTABLE)
	{
		error(26);  //程序体表溢出
		printf("TOO MANY PROCEDURE IN PROGRAM!");
	}
	else
	{
		BX++;
		displayLevel++;  //编译到了一个过程，层次加一
		BTAB[BX].last = 0;
		BTAB[BX].lastPar = 0;
	}
}
//void BLOCK(SYMLIST* list, int level)  //过程体的分析
//{
//	int cx, tx, programBlock;
//	int dx;
//	dx = DX;//纪录静态上层局部数据区大小
//	DX = 3;//将本程序体的活动记录留出三个单元出来，用做连接数据
//	tx = TX;
//	NAMETAB[tx].unite.address = CX;
//
//	if (displayLevel > MAXLEVELDEPTH)
//		error(26);//程序体表溢出
//	ENTERBLOCK();  //登记过程体
//	programBlock = BX;
//	DISPLAY[displayLevel] = BX;
//	NAMETAB[tx].type = NOTYP;
//	NAMETAB[tx].ref = programBlock;
//
//	if (CurSymbol->type == LPAREN && displayLevel > 1)// 是左括号且层级比1大，即非主函数
//	{
//		PARAMENTERLIST(list);  //编译过程列表
//		if (CurSymbol->type == SEMICOLON)
//			getASymbol();
//		else
//			error(1);//应该是';'
//	}
//	else if (displayLevel > 1)
//	{
//		if (CurSymbol->type == SEMICOLON)
//			getASymbol();
//		else
//			error(1);//应该是';'
//	}
//	BTAB[programBlock].lastPar = TX;
//	BTAB[programBlock].pSize = DX;
//	GEN(JMP, 0, 0);
//	do
//	{
//		switch (CurSymbol->type)  //针对当前不同的不好进行不同的声明
//		{
//		case CONSTSYM:
//			getASymbol();
//			do
//			{
//				CONSTDECLARATION(list);  //常量声明，一次可声明多个
//			} while (CurSymbol->type == IDENT);
//			break;
//		case TYPESYM:
//			getASymbol();
//			do
//			{
//				TYPEDECLARATION(list);  //类型声明，一次可声明多个
//			} while (CurSymbol->type == IDENT);
//			break;
//		case VARSYM:
//			getASymbol();
//			do
//			{
//				VARDECLARATION(list);  //变量声明，一次可声明多个
//			} while (CurSymbol->type == IDENT);
//			break;
//		}
//		while (CurSymbol->type == PROCSYM)
//			PROCDECLARATION(list);    //过程声明，每次只能声明一个
//	} while (SYMINLIST(CurSymbol->type, &DECLBEGSYS));
//	CODE[NAMETAB[tx].unite.address].address = CX;//将执行语句的开始处地址回填
//	JUMADRTAB[JX] = CX;
//	JX++;
//	NAMETAB[tx].unite.address = CX;//代码开始地址
//	cx = CX;
//	GEN(ENTP, displayLevel, DX);
//
//	////////////////////////////////////////////////////
//	SYMLIST* tempList = new SYMLIST;
//	COPYLIST(tempList, listAddSym(listAddSym(list, ENDSYM), SEMICOLON));
//	STATEMENT(tempList);
//	delete tempList;
//	////////////////////////////////////////////////////
//	CODE[cx].address = DX;//回填数据区大小
//	if (displayLevel > 1)
//		GEN(RETP, 0, 0);//从程序体返回
//	else
//		GEN(ENDP, 0, 0);//程序结束
//	QUITBLOCK();
//	DX = dx;//恢复静态上层局部数据区大小
//}


int main(int argc, char* argv[])
{
    //std::cout << "Hello World!\n";
	char srcFilename[FILENAMESIZE] = "F:\\Compiler\\test\\*.txt";
	char otpLFilename[FILENAMESIZE] = "F:\\Compiler\\Lexicon\\*.txt";
	char otpGFilename[FILENAMESIZE] = "F:\\Compiler\\Grammar\\*.txt";
	FILE* srcFile;
	char* srcFileNamePoint;

	//if (argc > 1)
	//	strcpy(srcFilename, argv[1]);
	//else
	//{
	//	printf("Please input the source file name : ");
	//	scanf("%s", srcFilename);
	//}
	//if (!(srcFile = fopen(srcFilename, "rb")))
	//{
	//	printf("Error : source file %s not found\n", srcFilename);
	//	exit(1);
	//}
	for (int num = 1; num <= 6; num++) {
		srcFilename[17] = '0' + num;
		otpLFilename[20] = '0' + num;
		otpGFilename[20] = '0' + num;
		srcFile = fopen(srcFilename, "rb");
		cout << srcFilename << "  success?" << (bool)srcFile << endl;
		cout << otpLFilename << endl;
		cout << otpGFilename << endl;
		cout << "start lexical analysis process!" << endl;
		getSymbols(srcFile);
		FILE *fp = fopen(otpLFilename, "w+");
		fclose(srcFile);
		while (CurSymbol)
		{
			fprintf_s(fp, "%-20s %-20s\n", symbol_name[CurSymbol->type], CurSymbol->name);
			
			
			CurSymbol = CurSymbol->next;
		}
		fclose(fp);
		INITIAL();  //初始化
		ENTERPREID();  //预填符号表
		destroySymbols();
	}


	return 0;
}

