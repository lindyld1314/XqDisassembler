#pragma once
#include<Windows.h>
#include<iostream>
#include<vector>
#include<map>
#include<string>
#include<sstream>
#include <iomanip>
#include<algorithm>
using namespace std;

typedef struct _MODR_M
{
	union
	{
		struct
		{
			BYTE R_M : 3;	
			BYTE Reg : 3;
			BYTE Mod : 2;
		};
		BYTE B_MOD;
	};
}_MODR_M;

typedef enum TYPE
{
	T_NONE = 0x0,
	DIRECT = 0x100,
	SIM	= 0x200,
	DISP8 = 0x400,
	DISP32 = 0x800,
	DISP16 = 0x1000,
	SIM_DIP8 = (SIM | DISP8),
	SIM_DISP32 = (SIM | DISP32)
};

typedef enum ADDR_MODE
{
	ADDR_NONE = 0,
	ADDR_BYTE = 1,
	ADDR_WORD = 2,
	ADDR_DWORD = 4
};

typedef struct Analy_SIB
{
	TYPE type;
	const char* pInfo;
}Analy_SIB;

typedef struct Analy_Opcode
{
	DWORD Operand[3];
	string Description;
};

typedef struct
{
	TYPE type;
	const char* pInfo[5];
}Analy_Modrm;

typedef struct Instruction
{
	BYTE	RepeatPrefix;	//重复指令前缀Repe..
	BYTE	SegmentPrefix;	//段前缀FS,SS..
	BYTE	OperandPrefix;	//操作数大小前缀0x66，16->32，32->16
	BYTE	AddressPrefix;	//地址大小前缀0x67，16->32，32->16

	BYTE	Opcode[3];		//最多3个Opcode

	BYTE	ModR_M;			//ModR/M
	BYTE	SIB;			//SIB

	Analy_Opcode GroupOpcode;
	string  instruct;
	string  operand[3];

	string	DisassemblerStr;			//反汇编指令的文本形式
	string	OpcodeStr;					//反汇编指令的OPCODE形式
	DWORD  Address;
	DWORD	DisassemblerLen;			//反汇编OPCODE的长度
}Instruction, *PInstruction;


class XqDisassemblerEngine
{
private:
	vector<Instruction> m_VecInstr;
	DWORD m_OpIndex;
	PBYTE m_pOpcode;
	DWORD m_StartIndex;

public:
	DWORD XqDisassembler_Start(IN BYTE *pOpcode,IN const DWORD DisLen,IN const DWORD ImageBase = 0);
	VOID ShowDissembler_Str();

private:
	VOID DecodeOneByte(IN DWORD Mark, OUT PInstruction pInstr);
	VOID DecodeImm8(IN DWORD Mark,OUT PInstruction pInstr);
	VOID DecodeImm16(IN DWORD Mark, OUT PInstruction pInstr);
	VOID DecodeImm66(IN DWORD Mark, OUT PInstruction pInstr);
	VOID DecodeFPU(IN DWORD Mark, IN BYTE mod, OUT PInstruction pInstr);
	VOID DecodeModRM(IN DWORD Mark,IN BYTE MorRM,OUT PInstruction pInstr);
	VOID DecodeSIB(IN BYTE mod,IN DWORD i,OUT PInstruction pInstr);
	VOID DecodeDISPx(IN _MODR_M m,IN Analy_Modrm ModAanlyE, IN int i, OUT PInstruction pInstr,BYTE size);
	VOID DecodeGroup(IN DWORD &Mark,IN DWORD opcode,IN _MODR_M m,OUT PInstruction pInstr);
	VOID DecodeAddr67(IN DWORD Mark, OUT PInstruction pInstr);
	VOID DecodeRegX(PInstruction pInstr, int i, DWORD op);
	VOID Decode2Bytes(IN DWORD &Mark,OUT PInstruction pInstr);
	string GetPrefixString(PInstruction pInstr);
	VOID ComposeInstr(OUT PInstruction pInstr);
};

enum
{
	ES=1,
	CS,
	SS,
	DS,
	FS,
	GS,
	OperandSize66,
	AddressSize67,
	LOCK,
	REPNE,
	REPE
};


#define MODRM	0x80000000
#define IMM8	0x40000000
#define IMM66	0x20000000
#define PREFIX	0x10000000
#define IMM16	0x8000000
#define ONEBYTE	0x4000000
#define ADDR67	0x2000000
#define GROUP	0x1000000
#define MXX		0x800000		//在ModRM的M部分
#define IMMX	(IMM8|IMM16|IMM66)
#define StringInstruction	0x400000		//指令重复前缀0xF2 0xF3(REPNE REP/REPE)在Opcode1表中只能与下面7组字符串指令组合，
											// 		0xA4: 0xA5:		MOVS
											// 		0xA6: 0xA7:		CMPS
											// 		0xAE: 0xAF:		SCAS
											// 		0xAC: 0xAD:		LODS
											// 		0xAA: 0xAB:		STOS
											// 		0x6C: 0x6D:		INS
											// 		0x6E: 0x6F:		OUTS
#define FPU		0x200000		//D8开始的FPU_CODE

#define TWOBYTE 0x100000
#define RESERVED			0x00000400		//保留
#define MustHave66			0x00000800		//必须有0x66前缀,只在opcode38/3A表中有这样的指令
#define MustHaveF2			0x00001000		//目前只有一个指令是必须有0xF2前缀的：0FF0
#define MustHavePrefix		0x00002000		//必须有前缀
#define MustNo66			0x00004000		//必须没有0x66前缀,在扫描指令时出现66则取标志指令看是否有此标志，有则直接返回1，说明此66前缀是多余的
#define MustNoF2			0x00008000		//意义同上
#define MustNoF3			0x00010000		//意义同上
#define ThreeOpCode0F38		0x00000080		//0x0F38，3个opcode
#define ThreeOpCode0F3A		0x00000100		//0x0F3A，3个opcode
#define Uxx					0x00040000		//rm用于寻XMM，只能在mod==11时才可以解码，可能的opcode: 66 0F 50/C5/D7/F7	F2 OF D6
#define Nxx					0x00080000		//rm用于寻MMX，只能在mod==11时才可以解码，可能的opcode: OF C5/D7/F7			F3 OF D6


#define Fw		(E+FPU+S_W)
#define Fd		(E+FPU+S_D)
#define Fq		(E+FPU+S_Q)
#define Ft		(E+FPU+S_T)
#define F28		(E+FPU+S_E)			//m14/m28
#define F98		(E+FPU)
#define Freg	(FPU+REG)			//st(x)

//需要ModR/M表部分
#define	NONE	0x0

#define S		0x100 
#define Sw		(S+S_W)

#define OX		0x200
#define Ob		(OX+S_B)
#define Ov		(OX+S_V)

#define E		0x0400 
#define Ev		(E+S_V)
#define Eb		(E+S_B)
#define Ew		(E+S_W)

#define G		0x0800 
#define Gv		(G+S_V)
#define Gb		(G+S_B)
#define Gw		(G+S_W)

#define Ib		(IMM8+S_B) 
#define Iw		(IMM16+S_W) 
#define Iz		(IMM66+S_D) 

#define Jxx		0x1000	
#define Jz		(Iz + Jxx)
#define Jb		(Ib + Jxx)

//寄存器部分
#define EXX		0x80
#define XX		0x40
#define XL		0x20
#define REG		(EXX+XX+XL)

#define EAX		(EXX+R_A)	
#define EBX		(EXX+R_B)
#define ECX		(EXX+R_C)
#define	EDX		(EXX+R_D)
#define ESP		(EXX+R_SP)
#define EBP		(EXX+R_BP)
#define ESI		(EXX+R_SI)
#define EDI		(EXX+R_DI)

#define AX		(XX+R_A)
#define BX		(XX+R_B)
#define CX		(XX+R_C)
#define	DX		(XX+R_D)
#define SP		(XX+R_SP)
#define BP		(XX+R_BP)
#define SI		(XX+R_SI)
#define DI		(XX+R_DI)

#define AL		(XL+R_A)
#define BL		(XL+R_B)
#define CL		(XL+R_C)
#define	DL		(XL+R_D)

#define THREEBYTE 0x04

enum OP_SIZE
{
	S_B,	//BYTE
	S_W,	//WORD
	S_D,	//DWORD
	S_Q,	//QWORD
	S_T,	//TWORD
	S_E,	//16 BYTES
	S_V		//看是否有改写操作数大小的opcode->66或67前缀
};

enum REGISTER
{
	R_A,
	R_B,
	R_C,
	R_D,
	R_SP,
	R_BP,
	R_SI,
	R_DI
};








