#include"XqDisassemblerEngine.h"
#include<stdio.h>

void Format(string &str, DWORD Num, DWORD size);
string ToHexstring(DWORD num, int width);

char *REG8_TABLE[8]=
{
	"AL","BL","CL","DL","???","???","???","???",
};

char *REG16_TABLE[8] =
{
	"AX","BX","CX","DX","SP","BP","SI","DI",
};

char *SREG_TABLE[8] = 
{
	"ES","CS","SS","DS","FS","GS","R7_???","R8_???"
};

char *REG32_TABLE[8] =
{
	"EAX","EBX","ECX","EDX","ESP","EBP","ESI","EDI",
};

//ModRM的Gv表
Analy_Modrm G_TABLE[] = { 
{ DIRECT, { "AL","AX","EAX","MM0","XMM0" } },
{ DIRECT, { "CL","CX","ECX","MM1","XMM1" } },
{ DIRECT, { "DL","DX","EDX","MM2","XMM2" } },
{ DIRECT, { "BL","BX","EBX","MM3","XMM3" } },
{ DIRECT, { "AH","SP","ESP","MM4","XMM4" } },
{ DIRECT, { "CH","BP","EBP","MM5","XMM5" } },
{ DIRECT, { "DH","SI","ESI","MM6","XMM6" } },
{ DIRECT, { "BH","DI","EDI","MM7","XMM7" } }
};

const char* G_TABLE_FPU[8][8] = {
	{"FADD","FMUL","FCOM","FCOMP","FSUB","FSUBR","FDIV","FDIVR"},
	{"FLD","???","FST","FSTP","FLDENV","FLDCW","FSTENV","FSTCW"},
	{"FIADD","FIMUL","FICOM","FICOMP","FISUB","FISUBR","FIDIV","FIDIVR"},
	{"FILD","FISTTP","FIST","FISTP","???","FLD","???","FSTP"},
	{"FADD","FMUL","FCOM","FCOMP","FSUB","FSUBR","FDIV","FDIVR"},
	{"FLD","FISTTP","FST","FSTP","FRSTOR","???","FSAVE","FSTSW"},
	{"FIADD","FIMUL","FICOM","FICOMP","FISUB","FISUBR","FIDIV","FIDIVR"},
	{"FILD","FISTTP","FIST","FISTP","FBLD","FILD","FBSTP","FISTP"}
};

//ModRM的Ev表-32位寻址
Analy_Modrm E_TABLE[] = {
{ DIRECT,{ "[EAX]","[EAX]","[EAX]","[EAX]","[EAX]" } },
{ DIRECT,{ "[ECX]","[ECX]","[ECX]","[ECX]","[ECX]" } },
{ DIRECT,{ "[EDX]","[EDX]","[EDX]","[EDX]","[EDX]" } },
{ DIRECT,{ "[EBX]","[EBX]","[EBX]","[EBX]","[EBX]" } },
{ SIM,   { "???" ,"???","???","???","???" }},
{ DISP32,{"[0x%08X]","[0x%08X]" ,"[0x%08X]" ,"[0x%08X]" ,"[0x%08X]" }},
{ DIRECT,{ "[ESI]","[ESI]","[ESI]","[ESI]","[ESI]" } },
{ DIRECT,{ "[EDI]","[EDI]","[EDI]","[EDI]","[EDI]" } },

{ DISP8,{"[EAX+0x%02X]","[EAX+0x%02X]" ,"[EAX+0x%02X]" ,"[EAX+0x%02X]" ,"[EAX+0x%02X]" } },
{ DISP8,{"[ECX+0x%02X]","[ECX+0x%02X]" ,"[ECX+0x%02X]" ,"[ECX+0x%02X]" ,"[ECX+0x%02X]" } },
{ DISP8,{"[EDX+0x%02X]","[EDX+0x%02X]" ,"[EDX+0x%02X]" ,"[EDX+0x%02X]" ,"[EDX+0x%02X]" } },
{ DISP8,{"[EBX+0x%02X]","[EBX+0x%02X]" ,"[EBX+0x%02X]" ,"[EBX+0x%02X]" ,"[EBX+0x%02X]" } },
{ SIM, {"???","???" ,"???" ,"???" ,"???" }},
{ DISP8,{"[EBP+0x%02X]","[EBP+0x%02X]" ,"[EBP+0x%02X]" ,"[EBP+0x%02X]" ,"[EBP+0x%02X]" } },
{ DISP8,{"[ESI+0x%02X]","[ESI+0x%02X]" ,"[ESI+0x%02X]" ,"[ESI+0x%02X]" ,"[ESI+0x%02X]" } },
{ DISP8,{"[EDI+0x%02X]","[EDI+0x%02X]" ,"[EDI+0x%02X]" ,"[EDI+0x%02X]" ,"[EDI+0x%02X]" } },

{ DISP32,{ "[EAX+0x%08X]","[EAX+0x%08X]","[EAX+0x%08X]","[EAX+0x%08X]","[EAX+0x%08X]" } },
{ DISP32,{ "[ECX+0x%08X]","[ECX+0x%08X]","[ECX+0x%08X]","[ECX+0x%08X]","[ECX+0x%08X]" } },
{ DISP32,{ "[EDX+0x%08X]","[EDX+0x%08X]","[EDX+0x%08X]","[EDX+0x%08X]","[EDX+0x%08X]" } },
{ DISP32,{ "[EBX+0x%08X]","[EBX+0x%08X]","[EBX+0x%08X]","[EBX+0x%08X]","[EBX+0x%08X]" } },
{ SIM, { "???","???","???","???","???" }},
{ DISP32,{ "[EBP+0x%08X]","[EBP+0x%08X]","[EBP+0x%08X]","[EBP+0x%08X]","[EBP+0x%08X]" } },
{ DISP32,{ "[ESI+0x%08X]","[ESI+0x%08X]","[ESI+0x%08X]","[ESI+0x%08X]","[ESI+0x%08X]" } },
{ DISP32,{ "[EDI+0x%08X]","[EDI+0x%08X]","[EDI+0x%08X]","[EDI+0x%08X]","[EDI+0x%08X]" } },

{ DIRECT,{ "AL","AX","EAX","MM0","XMM0" } },
{ DIRECT,{ "CL","CX","ECX","MM1","XMM1" } },
{ DIRECT,{ "DL","DX","EDX","MM2","XMM2" } },
{ DIRECT,{ "BL","BX","EBX","MM3","XMM3" } },
{ DIRECT,{ "AH","SP","ESP","MM4","XMM4" } },
{ DIRECT,{ "CH","BP","EBP","MM5","XMM5" } },
{ DIRECT,{ "DH","SI","ESI","MM6","XMM6" } },
{ DIRECT,{ "BH","DI","EDI","MM7","XMM7" } }
};

//ModRM的E表-FPU32，只是C0-FF不一样
Analy_Modrm E_TABLE_FPU32[] = {
	{ DIRECT,{ "[EAX]","[EAX]","[EAX]","[EAX]","[EAX]" } },
	{ DIRECT,{ "[ECX]","[ECX]","[ECX]","[ECX]","[ECX]" } },
	{ DIRECT,{ "[EDX]","[EDX]","[EDX]","[EDX]","[EDX]" } },
	{ DIRECT,{ "[EBX]","[EBX]","[EBX]","[EBX]","[EBX]" } },
	{ SIM,{ "???" ,"???","???","???","???" } },
	{ DISP32,{ "[0x%08X]","[0x%08X]" ,"[0x%08X]" ,"[0x%08X]" ,"[0x%08X]" } },
	{ DIRECT,{ "[ESI]","[ESI]","[ESI]","[ESI]","[ESI]" } },
	{ DIRECT,{ "[EDI]","[EDI]","[EDI]","[EDI]","[EDI]" } },

	{ DISP8,{ "[EAX+0x%02X]","[EAX+0x%02X]" ,"[EAX+0x%02X]" ,"[EAX+0x%02X]" ,"[EAX+0x%02X]" } },
	{ DISP8,{ "[ECX+0x%02X]","[ECX+0x%02X]" ,"[ECX+0x%02X]" ,"[ECX+0x%02X]" ,"[ECX+0x%02X]" } },
	{ DISP8,{ "[EDX+0x%02X]","[EDX+0x%02X]" ,"[EDX+0x%02X]" ,"[EDX+0x%02X]" ,"[EDX+0x%02X]" } },
	{ DISP8,{ "[EBX+0x%02X]","[EBX+0x%02X]" ,"[EBX+0x%02X]" ,"[EBX+0x%02X]" ,"[EBX+0x%02X]" } },
	{ SIM,{ "???","???" ,"???" ,"???" ,"???" } },
	{ DISP8,{ "[EBP+0x%02X]","[EBP+0x%02X]" ,"[EBP+0x%02X]" ,"[EBP+0x%02X]" ,"[EBP+0x%02X]" } },
	{ DISP8,{ "[ESI+0x%02X]","[ESI+0x%02X]" ,"[ESI+0x%02X]" ,"[ESI+0x%02X]" ,"[ESI+0x%02X]" } },
	{ DISP8,{ "[EDI+0x%02X]","[EDI+0x%02X]" ,"[EDI+0x%02X]" ,"[EDI+0x%02X]" ,"[EDI+0x%02X]" } },

	{ DISP32,{ "[EAX+0x%08X]","[EAX+0x%08X]","[EAX+0x%08X]","[EAX+0x%08X]","[EAX+0x%08X]" } },
	{ DISP32,{ "[ECX+0x%08X]","[ECX+0x%08X]","[ECX+0x%08X]","[ECX+0x%08X]","[ECX+0x%08X]" } },
	{ DISP32,{ "[EDX+0x%08X]","[EDX+0x%08X]","[EDX+0x%08X]","[EDX+0x%08X]","[EDX+0x%08X]" } },
	{ DISP32,{ "[EBX+0x%08X]","[EBX+0x%08X]","[EBX+0x%08X]","[EBX+0x%08X]","[EBX+0x%08X]" } },
	{ SIM,{ "???","???","???","???","???" } },
	{ DISP32,{ "[EBP+0x%08X]","[EBP+0x%08X]","[EBP+0x%08X]","[EBP+0x%08X]","[EBP+0x%08X]" } },
	{ DISP32,{ "[ESI+0x%08X]","[ESI+0x%08X]","[ESI+0x%08X]","[ESI+0x%08X]","[ESI+0x%08X]" } },
	{ DISP32,{ "[EDI+0x%08X]","[EDI+0x%08X]","[EDI+0x%08X]","[EDI+0x%08X]","[EDI+0x%08X]" } },

	{ DIRECT,{ "ST(0)","ST(0)","ST(0)","ST(0)","ST(0)" } },
	{ DIRECT,{ "ST(1)","ST(1)","ST(1)","ST(1)","ST(1)" } },
	{ DIRECT,{ "ST(2)","ST(2)","ST(2)","ST(2)","ST(2)" } },
	{ DIRECT,{ "ST(3)","ST(3)","ST(3)","ST(3)","ST(3)" } },
	{ DIRECT,{ "ST(4)","ST(4)","ST(4)","ST(4)","ST(4)" } },
	{ DIRECT,{ "ST(5)","ST(5)","ST(5)","ST(5)","ST(5)" } },
	{ DIRECT,{ "ST(6)","ST(6)","ST(6)","ST(6)","ST(6)" } },
	{ DIRECT,{ "ST(7)","ST(7)","ST(7)","ST(7)","ST(7)" } }
};

//ModRM的E表-FPU16
Analy_Modrm E_TABLE_FPU16[] = {
	{ DIRECT,{ "[BX+SI]","[BX+SI]","[BX+SI]","[BX+SI]","[BX+SI]" } },
	{ DIRECT,{ "[BX+DI]","[BX+DI]","[BX+DI]","[BX+DI]","[BX+DI]" } },
	{ DIRECT,{ "[BP+SI]","[BP+SI]","[BP+SI]","[BP+SI]","[BP+SI]" } },
	{ DIRECT,{ "[BP+DI]","[BP+DI]","[BP+DI]","[BP+DI]","[BP+DI]" } },
	{ DIRECT,{ "[SI]","[SI]","[SI]","[SI]","[SI]" } },
	{ DIRECT,{ "[DI]","[DI]","[DI]","[DI]","[DI]" } },
	{ DISP16,{ "[%04X]","[%04X]","[%04X]","[%04X]","[%04X]" } },
	{ DIRECT,{ "[BX]","[BX]","[BX]","[BX]","[BX]" } },

	{ DISP8,{ "[BX+SI+%02X]","[BX+SI+%02X]","[BX+SI+%02X]","[BX+SI+%02X]","[BX+SI+%02X]" } },
	{ DISP8,{ "[BX+DI+%02X]","[BX+DI+%02X]","[BX+DI+%02X]","[BX+DI+%02X]","[BX+DI+%02X]" } },
	{ DISP8,{ "[BP+SI+%02X]","[BP+SI+%02X]","[BP+SI+%02X]","[BP+SI+%02X]","[BP+SI+%02X]" } },
	{ DISP8,{ "[BP+DI+%02X]","[BP+DI+%02X]","[BP+DI+%02X]","[BP+DI+%02X]","[BP+DI+%02X]" } },
	{ DISP8,{ "[SI+%02X]","[SI+%02X]","[SI+%02X]","[SI+%02X]","[SI+%02X]" } },
	{ DISP8,{ "[DI+%02X]","[DI+%02X]","[DI+%02X]","[DI+%02X]","[DI+%02X]" } },
	{ DISP8,{ "[BP+%02X]","[BP+%02X]","[BP+%02X]","[BP+%02X]","[BP+%02X]" } },
	{ DISP8,{ "[BX+%02X]","[BX+%02X]","[BX+%02X]","[BX+%02X]","[BX+%02X]" } },

	{ DISP16,{ "[BX+SI+%04X]","[BX+SI+%04X]","[BX+SI+%04X]","[BX+SI+%04X]","[BX+SI+%04X]" } },
	{ DISP16,{ "[BX+DI+%04X]","[BX+DI+%04X]","[BX+DI+%04X]","[BX+DI+%04X]","[BX+DI+%04X]" } },
	{ DISP16,{ "[BP+SI+%04X]","[BP+SI+%04X]","[BP+SI+%04X]","[BP+SI+%04X]","[BP+SI+%04X]" } },
	{ DISP16,{ "[BP+DI+%04X]","[BP+DI+%04X]","[BP+DI+%04X]","[BP+DI+%04X]","[BP+DI+%04X]" } },
	{ DISP16,{ "[SI+%04X]","[SI+%04X]","[SI+%04X]","[SI+%04X]","[SI+%04X]" } },
	{ DISP16,{ "[DI+%04X]","[DI+%04X]","[DI+%04X]","[DI+%04X]","[DI+%04X]" } },
	{ DISP16,{ "[BP+%04X]","[BP+%04X]","[BP+%04X]","[BP+%04X]","[BP+%04X]" } },
	{ DISP16,{ "[BX+%04X]","[BX+%04X]","[BX+%04X]","[BX+%04X]","[BX+%04X]" } },

	{ DIRECT,{ "ST(0)","ST(0)","ST(0)","ST(0)","ST(0)" } },
	{ DIRECT,{ "ST(1)","ST(1)","ST(1)","ST(1)","ST(1)" } },
	{ DIRECT,{ "ST(2)","ST(2)","ST(2)","ST(2)","ST(2)" } },
	{ DIRECT,{ "ST(3)","ST(3)","ST(3)","ST(3)","ST(3)" } },
	{ DIRECT,{ "ST(4)","ST(4)","ST(4)","ST(4)","ST(4)" } },
	{ DIRECT,{ "ST(5)","ST(5)","ST(5)","ST(5)","ST(5)" } },
	{ DIRECT,{ "ST(6)","ST(6)","ST(6)","ST(6)","ST(6)" } },
	{ DIRECT,{ "ST(7)","ST(7)","ST(7)","ST(7)","ST(7)" } }
};

//0X67前缀下的ModRM的Ev表，16位寻址
Analy_Modrm E_TABLE16[] = {
	{ DIRECT,{ "[BX+SI]","[BX+SI]","[BX+SI]","[BX+SI]","[BX+SI]" } },
	{ DIRECT,{ "[BX+DI]","[BX+DI]","[BX+DI]","[BX+DI]","[BX+DI]" } },
	{ DIRECT,{ "[BP+SI]","[BP+SI]","[BP+SI]","[BP+SI]","[BP+SI]" } },
	{ DIRECT,{ "[BP+DI]","[BP+DI]","[BP+DI]","[BP+DI]","[BP+DI]" } },
	{ DIRECT,{ "[SI]","[SI]","[SI]","[SI]","[SI]" } },
	{ DIRECT,{ "[DI]","[DI]","[DI]","[DI]","[DI]" } },
	{ DISP16,{ "[%04X]","[%04X]","[%04X]","[%04X]","[%04X]" } },
	{ DIRECT,{ "[BX]","[BX]","[BX]","[BX]","[BX]" } },

	{ DISP8,{ "[BX+SI+%02X]","[BX+SI+%02X]","[BX+SI+%02X]","[BX+SI+%02X]","[BX+SI+%02X]" } },
	{ DISP8,{ "[BX+DI+%02X]","[BX+DI+%02X]","[BX+DI+%02X]","[BX+DI+%02X]","[BX+DI+%02X]" } },
	{ DISP8,{ "[BP+SI+%02X]","[BP+SI+%02X]","[BP+SI+%02X]","[BP+SI+%02X]","[BP+SI+%02X]" } },
	{ DISP8,{ "[BP+DI+%02X]","[BP+DI+%02X]","[BP+DI+%02X]","[BP+DI+%02X]","[BP+DI+%02X]" } },
	{ DISP8,{ "[SI+%02X]","[SI+%02X]","[SI+%02X]","[SI+%02X]","[SI+%02X]" } },
	{ DISP8,{ "[DI+%02X]","[DI+%02X]","[DI+%02X]","[DI+%02X]","[DI+%02X]" } },
	{ DISP8,{ "[BP+%02X]","[BP+%02X]","[BP+%02X]","[BP+%02X]","[BP+%02X]" } },
	{ DISP8,{ "[BX+%02X]","[BX+%02X]","[BX+%02X]","[BX+%02X]","[BX+%02X]" } },

	{ DISP16,{ "[BX+SI+%04X]","[BX+SI+%04X]","[BX+SI+%04X]","[BX+SI+%04X]","[BX+SI+%04X]" } },
	{ DISP16,{ "[BX+DI+%04X]","[BX+DI+%04X]","[BX+DI+%04X]","[BX+DI+%04X]","[BX+DI+%04X]" } },
	{ DISP16,{ "[BP+SI+%04X]","[BP+SI+%04X]","[BP+SI+%04X]","[BP+SI+%04X]","[BP+SI+%04X]" } },
	{ DISP16,{ "[BP+DI+%04X]","[BP+DI+%04X]","[BP+DI+%04X]","[BP+DI+%04X]","[BP+DI+%04X]" } },
	{ DISP16,{ "[SI+%04X]","[SI+%04X]","[SI+%04X]","[SI+%04X]","[SI+%04X]" } },
	{ DISP16,{ "[DI+%04X]","[DI+%04X]","[DI+%04X]","[DI+%04X]","[DI+%04X]" } },
	{ DISP16,{ "[BP+%04X]","[BP+%04X]","[BP+%04X]","[BP+%04X]","[BP+%04X]" } },
	{ DISP16,{ "[BX+%04X]","[BX+%04X]","[BX+%04X]","[BX+%04X]","[BX+%04X]" } },

	{ DIRECT,{ "AL","AX","EAX","MM0","XMM0" } },
	{ DIRECT,{ "CL","CX","ECX","MM1","XMM1" } },
	{ DIRECT,{ "DL","DX","EDX","MM2","XMM2" } },
	{ DIRECT,{ "BL","BX","EBX","MM3","XMM3" } },
	{ DIRECT,{ "AH","SP","ESP","MM4","XMM4" } },
	{ DIRECT,{ "CH","BP","EBP","MM5","XMM5" } },
	{ DIRECT,{ "DH","SI","ESI","MM6","XMM6" } },
	{ DIRECT,{ "BH","DI","EDI","MM7","XMM7" } }
};

Analy_SIB SIB_REGA[] =
{
	{ DIRECT,"EAX" },
	{ DIRECT,"ECX" },
	{ DIRECT,"EDX" },
	{ DIRECT,"EBX" },
	{ DIRECT,"ESP" },
	{ DISP32,"EBP" },
	{ DIRECT,"ESI" },
	{ DIRECT,"EDI" },
};

Analy_SIB SIB_REGB[] =
{
	{ DIRECT,"EAX" },
	{ DIRECT,"ECX" },
	{ DIRECT,"EDX" },
	{ DIRECT,"EBX" },
	{ T_NONE,"???" },
	{ DIRECT,"EBP" },
	{ DIRECT,"ESI" },
	{ DIRECT,"EDI" },

	{ DIRECT,"EAX*2" },
	{ DIRECT,"ECX*2" },
	{ DIRECT,"EDX*2" },
	{ DIRECT,"EBX*2" },
	{ T_NONE,"???" },
	{ DIRECT,"EBP*2" },
	{ DIRECT,"ESI*2" },
	{ DIRECT,"EDI*2" },

	{ DIRECT,"EAX*4" },
	{ DIRECT,"ECX*4" },
	{ DIRECT,"EDX*4" },
	{ DIRECT,"EBX*4" },
	{ T_NONE,"???" },
	{ DIRECT,"EBP*4" },
	{ DIRECT,"ESI*4" },
	{ DIRECT,"EDI*4" },

	{ DIRECT,"EAX*8" },
	{ DIRECT,"ECX*8" },
	{ DIRECT,"EDX*8" },
	{ DIRECT,"EBX*8" },
	{ T_NONE,"???" },
	{ DIRECT,"EBP*8" },
	{ DIRECT,"ESI*8" },
	{ DIRECT,"EDI*8" },
};


DWORD ONE_OPCODE_TABLE[256] =
{
		/*0			1			2			3			4			5			6			7			8			9			A			B			C			D			E			F*/
	/*0*/MODRM,		MODRM,		MODRM,		MODRM,		IMM8,		IMM66,		ONEBYTE,	ONEBYTE,	MODRM,		MODRM,		MODRM,		MODRM,		IMM8,		IMM66,		ONEBYTE,	TWOBYTE,
	/*1*/MODRM,		MODRM,		MODRM,		MODRM,		IMM8,		IMM66,		ONEBYTE,	ONEBYTE,	MODRM,		MODRM,		MODRM,		MODRM,		IMM8,		IMM66,		ONEBYTE,	ONEBYTE,
	/*2*/MODRM,		MODRM,		MODRM,		MODRM,		IMM8,		IMM66,		PREFIX,		ONEBYTE,	MODRM,		MODRM,		MODRM,		MODRM,		IMM8,		IMM66,		PREFIX,		ONEBYTE,
	/*3*/MODRM,		MODRM,		MODRM,		MODRM,		IMM8,		IMM66,		PREFIX,		ONEBYTE,	MODRM,		MODRM,		MODRM,		MODRM,		IMM8,		IMM66,		PREFIX,		ONEBYTE,
	/*4*/ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,
	/*5*/ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,
	/*6*/ONEBYTE,	ONEBYTE,	MODRM+MXX,	MODRM,		PREFIX,		PREFIX,		PREFIX,		PREFIX,		IMM66,		MODRM+IMM66,IMM8,		MODRM+IMM8,	ONEBYTE+StringInstruction,	ONEBYTE+ StringInstruction,	ONEBYTE+ StringInstruction,	ONEBYTE+ StringInstruction,
	/*7*/IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM8,
	/*8*/GROUP,		GROUP,		GROUP,		GROUP,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM+MXX,	MODRM,		MODRM,
	/*9*/ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	IMM66+IMM16,ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,
	/*A*/ADDR67,	ADDR67,		ADDR67,		ADDR67,		ONEBYTE+StringInstruction,ONEBYTE+StringInstruction,ONEBYTE+StringInstruction,ONEBYTE+StringInstruction,	IMM8,		IMM66,		ONEBYTE + StringInstruction,	ONEBYTE + StringInstruction,	ONEBYTE + StringInstruction,	ONEBYTE + StringInstruction,	ONEBYTE + StringInstruction,	ONEBYTE + StringInstruction,
	/*B*/IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM66,		IMM66,		IMM66,		IMM66,		IMM66,		IMM66,		IMM66,		IMM66,
	/*C*/GROUP,		GROUP,		IMM16,		ONEBYTE,	MODRM+MXX,	MODRM+MXX,	GROUP,		GROUP,		IMM16+IMM8,	ONEBYTE,	IMM16,		ONEBYTE,	ONEBYTE,	IMM8,		ONEBYTE,	ONEBYTE,
	/*D*/GROUP,		GROUP,		GROUP,		GROUP,		IMM8,		IMM8,		ONEBYTE,	ONEBYTE,	FPU,		FPU,		FPU,		FPU,		FPU,		FPU,		FPU,		FPU,
	/*E*/IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM8,		IMM66,		IMM66,		IMM66+IMM16,IMM8,		ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,
	/*F*/PREFIX,	ONEBYTE,	PREFIX,		PREFIX,		ONEBYTE,	ONEBYTE,	GROUP,		GROUP,		ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	GROUP,		GROUP,
};

DWORD TWO_OPCODE_TABLE[256] =
{
	/*		0			1			2			3			4			5			6			7			8				9			A			B			C			D			E			F		*/
	/*0*/	GROUP,GROUP,MODRM,		MODRM,		RESERVED,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	RESERVED,	ONEBYTE,	RESERVED,	MODRM,		RESERVED,	RESERVED,
	/*1*/	MODRM,		MODRM,		MODRM + MXX + Uxx,		MODRM + MXX + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + Uxx + MXX + MustNoF2,		MODRM + MXX + MustNoF2 + MustNoF3,		GROUP,RESERVED,	RESERVED,	RESERVED,	RESERVED,	RESERVED,	RESERVED,	MODRM,
	/*2*/	MODRM + REG,		MODRM + REG,		MODRM + REG,		MODRM + REG,		RESERVED,	RESERVED,	RESERVED,	RESERVED,	MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM,		MODRM + MXX,		MODRM,		MODRM,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,
	/*3*/	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	ONEBYTE,	RESERVED,	RESERVED,	ThreeOpCode0F38,RESERVED,ThreeOpCode0F3A,MODRM,	MODRM,		MODRM,		MODRM,		MODRM,
	/*4*/	MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,
	/*5*/	MODRM + Uxx + MustNoF2 + MustNoF3,	MODRM,		MODRM + MustNo66 + MustNoF2,		MODRM + MustNo66 + MustNoF2,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM,		MODRM,		MODRM,		MODRM + MustNoF2,		MODRM,		MODRM,		MODRM,		MODRM,
	/*6*/	MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3 + MustHave66,		MODRM + MustNoF2 + MustNoF3 + MustHave66,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2,
	/*7*/	MODRM + IMM8,	GROUP,		GROUP,		GROUP,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNo66 + MustNoF2 + MustNoF3,MODRM + MustNo66 + MustNoF2 + MustNoF3,		MODRM + MustNo66 + MustNoF2 + MustNoF3,		RESERVED,	RESERVED,	MODRM + MustNoF3 + MustHavePrefix,		MODRM + MustNoF3 + MustHavePrefix,		MODRM + MustNoF2,		MODRM + MustNoF2,
	/*8*/	IMM66,		IMM66,		IMM66,		IMM66,		IMM66,		IMM66,		IMM66,		IMM66,		IMM66,		IMM66,		IMM66,		IMM66,		IMM66,		IMM66,		IMM66,		IMM66,
	/*9*/	MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,		MODRM,
	/*A*/	ONEBYTE,	ONEBYTE,	ONEBYTE,	MODRM,		MODRM + IMM8,	MODRM,		RESERVED,	RESERVED,	ONEBYTE,	ONEBYTE,	ONEBYTE,	MODRM,		MODRM + IMM8,	MODRM,		GROUP,MODRM,
	/*B*/	MODRM,		MODRM,		MODRM + MXX,		MODRM,		MODRM + MXX,		MODRM + MXX,		MODRM,		MODRM,		RESERVED + MustNo66,	GROUP,GROUP,MODRM,MODRM,		MODRM,		MODRM,		MODRM,
	/*C*/	MODRM + MustNo66 + MustNoF2 + MustNoF3,		MODRM + MustNo66 + MustNoF2 + MustNoF3,		MODRM + IMM8,	MODRM + MXX + MustNo66 + MustNoF2 + MustNoF3,		MODRM + REG + MXX + IMM8 + MustNoF2 + MustNoF3,	MODRM + IMM8 + Uxx + Nxx + MustNoF2 + MustNoF3,	MODRM + IMM8 + MustNoF2 + MustNoF3,	MODRM + GROUP,ONEBYTE + MustNo66,	ONEBYTE + MustNo66,	ONEBYTE + MustNo66,	ONEBYTE + MustNo66,	ONEBYTE + MustNo66,	ONEBYTE + MustNo66,	ONEBYTE + MustNo66,	ONEBYTE + MustNo66,
	/*D*/	MODRM + MustNoF3 + MustHavePrefix,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustHavePrefix,		MODRM + Uxx + Nxx + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,
	/*E*/	MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustHavePrefix,		MODRM + MXX + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,
	/*F*/	MODRM + MXX + MustHaveF2 + MustNo66 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + Uxx + Nxx + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		MODRM + MustNoF2 + MustNoF3,		RESERVED,
};

#define REG_A(M) ((M)&(0xFF))		//解析REGA索引，0x0102表示在Ev[1]以及Gv[2]
#define REG_B(M) ((M)>>(8))		//解析REGB索引
WORD SIB_TABLE[256] =
{
			/*0		1		2		3		4		5		6		7*/
	/*00*/  0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
	/*01*/  0x0100, 0x0101, 0x0102, 0x0103, 0x0104, 0x0105, 0x0106, 0x0107,
	/*02*/  0x0200, 0x0201, 0x0202, 0x0203, 0x0204, 0x0205, 0x0206, 0x0207,
	/*03*/  0x0300, 0x0301, 0x0302, 0x0303, 0x0304, 0x0305, 0x0306, 0x0307,
	/*04*/  0x0400, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407,
	/*05*/  0x0500, 0x0501, 0x0502, 0x0503, 0x0504, 0x0505, 0x0506, 0x0507,
	/*06*/  0x0600, 0x0601, 0x0602, 0x0603, 0x0604, 0x0605, 0x0606, 0x0607,
	/*07*/  0x0700, 0x0701, 0x0702, 0x0703, 0x0704, 0x0705, 0x0706, 0x0707,
	/*08*/  0x0800, 0x0801, 0x0802, 0x0803, 0x0804, 0x0805, 0x0806, 0x0807,
	/*09*/  0x0900, 0x0901, 0x0902, 0x0903, 0x0904, 0x0905, 0x0906, 0x0907,
	/*0A*/  0x0A00, 0x0A01, 0x0A02, 0x0A03, 0x0A04, 0x0A05, 0x0A06, 0x0A07,
	/*0B*/  0x0B00, 0x0B01, 0x0B02, 0x0B03, 0x0B04, 0x0B05, 0x0B06, 0x0B07,
	/*0C*/  0x0C00, 0x0C01, 0x0C02, 0x0C03, 0x0C04, 0x0C05, 0x0C06, 0x0C07,
	/*0D*/  0x0D00, 0x0D01, 0x0D02, 0x0D03, 0x0D04, 0x0D05, 0x0D06, 0x0D07,
	/*0E*/  0x0E00, 0x0E01, 0x0E02, 0x0E03, 0x0E04, 0x0E05, 0x0E06, 0x0E07,
	/*0F*/  0x0F00, 0x0F01, 0x0F02, 0x0F03, 0x0F04, 0x0F05, 0x0F06, 0x0F07,
	/*10*/  0x1000, 0x1001, 0x1002, 0x1003, 0x1004, 0x1005, 0x1006, 0x1007,
	/*11*/  0x1100, 0x1101, 0x1102, 0x1103, 0x1104, 0x1105, 0x1106, 0x1107,
	/*12*/  0x1200, 0x1201, 0x1202, 0x1203, 0x1204, 0x1205, 0x1206, 0x1207,
	/*13*/  0x1300, 0x1301, 0x1302, 0x1303, 0x1304, 0x1305, 0x1306, 0x1307,
	/*14*/  0x1400, 0x1401, 0x1402, 0x1403, 0x1404, 0x1405, 0x1406, 0x1407,
	/*15*/  0x1500, 0x1501, 0x1502, 0x1503, 0x1504, 0x1505, 0x1506, 0x1507,
	/*16*/  0x1600, 0x1601, 0x1602, 0x1603, 0x1604, 0x1605, 0x1606, 0x1607,
	/*17*/  0x1700, 0x1701, 0x1702, 0x1703, 0x1704, 0x1705, 0x1706, 0x1707,
	/*18*/  0x1800, 0x1801, 0x1802, 0x1803, 0x1804, 0x1805, 0x1806, 0x1807,
	/*19*/  0x1900, 0x1901, 0x1902, 0x1903, 0x1904, 0x1905, 0x1906, 0x1907,
	/*1A*/  0x1A00, 0x1A01, 0x1A02, 0x1A03, 0x1A04, 0x1A05, 0x1A06, 0x1A07,
	/*1B*/  0x1B00, 0x1B01, 0x1B02, 0x1B03, 0x1B04, 0x1B05, 0x1B06, 0x1B07,
	/*1C*/  0x1C00, 0x1C01, 0x1C02, 0x1C03, 0x1C04, 0x1C05, 0x1C06, 0x1C07,
	/*1D*/  0x1D00, 0x1D01, 0x1D02, 0x1D03, 0x1D04, 0x1D05, 0x1D06, 0x1D07,
	/*1E*/  0x1E00, 0x1E01, 0x1E02, 0x1E03, 0x1E04, 0x1E05, 0x1E06, 0x1E07,
	/*1F*/  0x1F00, 0x1F01, 0x1F02, 0x1F03, 0x1F04, 0x1F05, 0x1F06, 0x1F07,
};

#define MODRM_G(M) ((M)&(0xFF))		//解析Gv索引，0x0102表示在Ev[1]以及Gv[2]
#define MODRM_E(M) ((M)>>(8))		//解析Ev索引
WORD ModRM_TABLE[256] =
{
			/*0		1		2		3		4		5		6		7*/
	/*00*/  0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
	/*01*/  0x0001, 0x0101, 0x0201, 0x0301, 0x0401, 0x0501, 0x0601, 0x0701,
	/*02*/  0x0002, 0x0102, 0x0202, 0x0302, 0x0402, 0x0502, 0x0602, 0x0702,
	/*03*/  0x0003, 0x0103, 0x0203, 0x0303, 0x0403, 0x0503, 0x0603, 0x0703,
	/*04*/  0x0004, 0x0104, 0x0204, 0x0304, 0x0404, 0x0504, 0x0604, 0x0704,
	/*05*/  0x0005, 0x0105, 0x0205, 0x0305, 0x0405, 0x0505, 0x0605, 0x0705,
	/*06*/  0x0006, 0x0106, 0x0206, 0x0306, 0x0406, 0x0506, 0x0606, 0x0706,
	/*07*/  0x0007, 0x0107, 0x0207, 0x0307, 0x0407, 0x0507, 0x0607, 0x0707,

	/*08*/  0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00,
	/*09*/  0x0801, 0x0901, 0x0A01, 0x0B01, 0x0C01, 0x0D01, 0x0E01, 0x0F01,
	/*0A*/  0x0802, 0x0902, 0x0A02, 0x0B02, 0x0C02, 0x0D02, 0x0E02, 0x0F02,
	/*0B*/  0x0803, 0x0903, 0x0A03, 0x0B03, 0x0C03, 0x0D03, 0x0E03, 0x0F03,
	/*0C*/  0x0804, 0x0904, 0x0A04, 0x0B04, 0x0C04, 0x0D04, 0x0E04, 0x0F04,
	/*0D*/  0x0805, 0x0905, 0x0A05, 0x0B05, 0x0C05, 0x0D05, 0x0E05, 0x0F05,
	/*0E*/  0x0806, 0x0906, 0x0A06, 0x0B06, 0x0C06, 0x0D06, 0x0E06, 0x0F06,
	/*0F*/  0x0807, 0x0907, 0x0A07, 0x0B07, 0x0C07, 0x0D07, 0x0E07, 0x0F07,

	/*10*/  0x1000, 0x1100, 0x1200, 0x1300, 0x1400, 0x1500, 0x1600, 0x1700,
	/*11*/  0x1001, 0x1101, 0x1201, 0x1301, 0x1401, 0x1501, 0x1601, 0x1701,
	/*12*/  0x1002, 0x1102, 0x1202, 0x1302, 0x1402, 0x1502, 0x1602, 0x1702,
	/*13*/  0x1003, 0x1103, 0x1203, 0x1303, 0x1403, 0x1503, 0x1603, 0x1703,
	/*14*/  0x1004, 0x1104, 0x1204, 0x1304, 0x1404, 0x1504, 0x1604, 0x1704,
	/*15*/  0x1005, 0x1105, 0x1205, 0x1305, 0x1405, 0x1505, 0x1605, 0x1705,
	/*16*/  0x1006, 0x1106, 0x1206, 0x1306, 0x1406, 0x1506, 0x1606, 0x1706,
	/*17*/  0x1007, 0x1107, 0x1207, 0x1307, 0x1407, 0x1507, 0x1607, 0x1707,

	/*18*/  0x1800, 0x1900, 0x1A00, 0x1B00, 0x1C00, 0x1D00, 0x1E00, 0x1F00,
	/*19*/  0x1801, 0x1901, 0x1A01, 0x1B01, 0x1C01, 0x1D01, 0x1E01, 0x1F01,
	/*1A*/  0x1802, 0x1902, 0x1A02, 0x1B02, 0x1C02, 0x1D02, 0x1E02, 0x1F02,
	/*1B*/  0x1803, 0x1903, 0x1A03, 0x1B03, 0x1C03, 0x1D03, 0x1E03, 0x1F03,
	/*1C*/  0x1804, 0x1904, 0x1A04, 0x1B04, 0x1C04, 0x1D04, 0x1E04, 0x1F04,
	/*1D*/  0x1805, 0x1905, 0x1A05, 0x1B05, 0x1C05, 0x1D05, 0x1E05, 0x1F05,
	/*1E*/  0x1806, 0x1906, 0x1A06, 0x1B06, 0x1C06, 0x1D06, 0x1E06, 0x1F06,
	/*1F*/  0x1807, 0x1907, 0x1A07, 0x1B07, 0x1C07, 0x1D07, 0x1E07, 0x1F07,
};

typedef DWORD Type;
typedef DWORD CodeIndex;
map<Type, map<CodeIndex, Analy_Opcode>> INSTR_TABLE =
{
	{ MODRM,
	{
		{0x00,
			{Eb,Gb,NONE,"ADD "}
		},
		{0x01,
			{Ev,Gv,NONE,"ADD "}
		},
		{0x02,
			{Gb,Eb,NONE,"ADD "}
		},
		{0x03,
			{Gv,Ev,NONE,"ADD "}
		},
		{0x10,
			{Eb,Gb,NONE,"ADC "}
		},
		{0x11,
			{Ev,Gv,NONE,"ADC "}
		},
		{0x12,
			{Gb,Eb,NONE,"ADC "}
		},
		{0x13,
			{Gv,Ev,NONE,"ADC "}
		},
		{0x20,
			{Eb,Gb,NONE,"AND "}
		},
		{0x21,
			{Ev,Gv,NONE,"AND "}
		},
		{0x22,
			{Gb,Eb,NONE,"AND "}
		},
		{0x23,
			{Gv,Ev,NONE,"AND "}
		},
		{0x30,
			{ Eb,Gb,NONE,"XOR " }
		},
		{0x31,
			{ Ev,Gv,NONE,"XOR " }
		},
		{0x32,
			{ Gb,Eb,NONE,"XOR " }
		},
		{0x33,
			{ Gv,Ev,NONE,"XOR " }
		},
		{ 0x08,
			{ Eb,Gb,NONE,"OR " }
		},
		{ 0x09,
			{ Ev,Gv,NONE,"OR " }
		},
		{ 0x0A,
			{ Gb,Eb,NONE,"OR " }
		},
		{ 0x0B,
			{ Gv,Ev,NONE,"OR " }
		},
		{ 0x18,
			{ Eb,Gb,NONE,"SBB " }
		},
		{ 0x19,
			{ Ev,Gv,NONE,"SBB " }
		},
		{ 0x1A,
			{ Gb,Eb,NONE,"SBB " }
		},
		{ 0x1B,
			{ Gv,Ev,NONE,"SBB " }
		},
		{ 0x18,
			{ Eb,Gb,NONE,"SUB " }
		},
		{ 0x19,
			{ Ev,Gv,NONE,"SUB " }
		},
		{ 0x1A,
			{ Gb,Eb,NONE,"SUB " }
		},
		{ 0x1B,
			{ Gv,Ev,NONE,"SUB " }
		},
		{ 0x18,
			{ Eb,Gb,NONE,"CMP " }
		},
		{ 0x19,
			{ Ev,Gv,NONE,"CMP " }
		},
		{ 0x1A,
			{ Gb,Eb,NONE,"CMP " }
		},
		{ 0x1B,
			{ Gv,Ev,NONE,"CMP " }
		},
		{ 0x62,
			{ Gv,MXX,NONE,"BOUND " }
		},
		{ 0x63,
			{ Ew,Gw,NONE,"ARPL " }
		},
		{ 0x69,
			{ Gv,Ev,Iz,"IMUL " }
		},
		{ 0x6B,
			{ Gv,Ev,Ib,"IMUL " }
		},
		{ 0x84,
			{ Eb,Gb,NONE,"TEST " }
		},
		{ 0x85,
			{ Ev,Gv,NONE,"TEST " }
		},
		{ 0x86,
			{ Eb,Gb,NONE,"XCHG " }
		},
		{ 0x87,
			{ Ev,Gv,NONE,"XCHG " }
		},
		{ 0x88,
			{ Eb,Gb,NONE,"MOV " }
		},
		{ 0x89,
			{ Ev,Gv,NONE,"MOV " }
		},
		{ 0x8A,
			{ Gb,Eb,NONE,"MOV " }
		},
		{ 0x8B,
			{ Gv,Ev,NONE,"MOV " }
		},
		{ 0x8C,
			{ Ev,Sw,NONE,"MOV " }
		},
		{ 0x8D,
			{ Gv,MXX,NONE,"LEA " }
		},
		{ 0x8E,
			{ Sw,Ew,NONE,"MOV " }
		},
		{ 0xC4,
			{ Gv,MXX,NONE,"LES " }
		},
		{ 0xC5,
			{ Gv,MXX,NONE,"LDS " }
		},
	}
	},

	{IMM8,
	{
		{ 0x04,
			{ AL,Ib,NONE,"ADD AL," }		//AL直接确定下来，在IMM8处理中就不用再判断了
		},
		{ 0x14,
			{ AL,Ib,NONE,"ADC AL," }
		},
		{ 0x24,
			{ AL,Ib,NONE,"AND AL," }
		},
		{ 0x34,
			{ AL,Ib,NONE,"XOR AL," }
		},
		{ 0x0C,
			{ AL,Ib,NONE,"OR AL," }
		},
		{ 0x1C,
			{ AL,Ib,NONE,"SBB AL," }
		},
		{ 0x2C,
			{ AL,Ib,NONE,"SUB AL," }
		},
		{ 0x3C,
			{ AL,Ib,NONE,"CMP AL," }
		},
		{0x6A,
			{Ib,NONE,NONE,"PUSH "}
		},
		{ 0x70,
			{ Jb,NONE,NONE,"JO " }
		},
		{ 0x71,
			{ Jb,NONE,NONE,"JNO " }
		},
		{ 0x72,
			{ Jb,NONE,NONE,"JB " }
		},
		{ 0x73,
			{ Jb,NONE,NONE,"JNB " }
		},
		{ 0x74,
			{ Jb,NONE,NONE,"JE " }
		},
		{ 0x75,
			{ Jb,NONE,NONE,"JNE " }
		},
		{ 0x76,
			{ Jb,NONE,NONE,"JBE " }
		},
		{ 0x77,
			{ Jb,NONE,NONE,"JNBE " }
		},
		{ 0x78,
			{ Jb,NONE,NONE,"JS " }
		},
		{ 0x79,
			{ Jb,NONE,NONE,"JNS " }
		},
		{ 0x7A,
			{ Jb,NONE,NONE,"JP " }
		},
		{ 0x7B,
			{ Jb,NONE,NONE,"JNP " }
		},
		{ 0x7C,
			{ Jb,NONE,NONE,"JL " }
		},
		{ 0x7D,
			{ Jb,NONE,NONE,"JNL " }
		},
		{ 0x7E,
			{ Jb,NONE,NONE,"JNG " }
		},
		{ 0x7F,
			{ Jb,NONE,NONE,"JG " }
		},
		{ 0x8A,
			{ Ib,NONE,NONE,"PUSH" }
		},
		{ 0xB0,
			{ Ib,NONE,NONE,"MOV AL," }
		},
		{ 0xB1,
			{ Ib,NONE,NONE,"MOV CL," }
		},
		{ 0xB2,
			{ Ib,NONE,NONE,"MOV DL," }
		},
		{ 0xB3,
			{ Ib,NONE,NONE,"MOV BL," }
		},
		{ 0xB4,
			{ Ib,NONE,NONE,"MOV AH," }
		},
		{ 0xB5,
			{ Ib,NONE,NONE,"MOV CH," }
		},
		{ 0xB6,
			{ Ib,NONE,NONE,"MOV DH," }
		},
		{ 0xB7,
			{ Ib,NONE,NONE,"MOV BH," }
		},
		{ 0xA8,
			{ Ib,NONE,NONE,"TEST AL," }
		},
		{ 0xE0,
			{ Jb,NONE,NONE,"LOOPNE " }
		},
		{ 0xE1,
			{ Jb,NONE,NONE,"LOOPE " }
		},
		{ 0xE2,
			{ Jb,NONE,NONE,"LOOP " }
		},
		{ 0xE3,
			{ Jb,NONE,NONE,"JECXZ " }
		},
		{ 0xD4,
			{ Ib,NONE,NONE,"AAM " }
		},
		{ 0xD5,
			{ Ib,NONE,NONE,"AAD " }
		},
		{ 0xE4,
			{ AL,Ib,NONE,"IN " }
		},
		{ 0xE5,
			{ EAX,Ib,NONE,"IN " }
		},
		{ 0xE6,
			{ Ib,AL,NONE,"OUT " }
		},
		{ 0xE7,
			{ Ib,EAX,NONE,"OUT " }
		},
		{ 0xEB,
			{ Jb,NONE,NONE,"JMP SHORT " }
		},
		{ 0xCD,
			{ Ib,NONE,NONE,"INT " }
		},
	}
	},

	{ IMM16,
	{
		{ 0xC8,
			{ Iw,Ib,NONE,"ENTER " }
		},
		{0x9A,
			{Iz,Iw,NONE,"CALL FAR "}
		},
		{ 0xC2,
			{ Iw,NONE,NONE,"RET " }
		},
		{ 0xCA,
			{ Iw,NONE,NONE,"RET FAR " }
		},
		{ 0xEA,
			{ Iz,Iw,NONE,"JMP FAR " }
		},
	}
	},

	{ IMM66,
	{
		{ 0x05,
			{ EAX,Iz,NONE,"ADD " }
		},
		{ 0x15,
			{ EAX,Iz,NONE,"ADC " }
		},
		{ 0x25,
			{ EAX,Iz,NONE,"AND " }
		},
		{ 0x35,
			{ EAX,Iz,NONE,"XOR " }
		},
		{ 0x0D,
			{ EAX,Iz,NONE,"OR " }
		},
		{ 0x1D,
			{ EAX,Iz,NONE,"SBB " }
		},
		{ 0x2D,
			{ EAX,Iz,NONE,"SUB " }
		},
		{ 0x3D,
			{ EAX,Iz,NONE,"CMP " }
		},
		{ 0x68,
			{ Iz,NONE,NONE,"PUSH " }
		},
		{ 0xA9,
			{ EAX,Iz,NONE,"TEST " }
		},

		{ 0xB8,
			{ EAX,Iz,NONE,"MOV " }
		},
		{ 0xB9,
			{ ECX,Iz,NONE,"MOV " }
		},
		{ 0xBA,
			{ EDX,Iz,NONE,"MOV " }
		},
		{ 0xBB,
			{ EBX,Iz,NONE,"MOV " }
		},
		{ 0xBC,
			{ ESP,Iz,NONE,"MOV " }
		},
		{ 0xBD,
			{ EBP,Iz,NONE,"MOV " }
		},
		{ 0xBE,
			{ ESI,Iz,NONE,"MOV " }
		},
		{ 0xBF,
			{ EDI,Iz,NONE,"MOV " }
		},
		{ 0xE8,
			{ Jz,NONE,NONE,"CALL " }
		},
		{ 0xE9,
			{ Jz,NONE,NONE,"JMP " }
		},
	}
	},

	{ ONEBYTE,
	{
		{ 0x06,
			{ NONE,NONE,NONE,"PUSH ES " }
		},
		{ 0x16,
			{ NONE,NONE,NONE,"PUSH SS " }
		},
		{ 0x07,
			{ NONE,NONE,NONE,"POP ES " }
		},
		{ 0x17,
			{ NONE,NONE,NONE,"POP SS " }
		},
		{ 0x27,
			{ NONE,NONE,NONE,"DAA " }
		},
		{ 0x37,
			{ NONE,NONE,NONE,"AAA " }
		},
		{ 0x0E,
			{ NONE,NONE,NONE,"PUSH CS " }
		},
		{ 0x1E,
			{ NONE,NONE,NONE,"PUSH DS " }
		},
		{ 0x1F,
			{ NONE,NONE,NONE,"POP DS " }
		},
		{ 0x2F,
			{ NONE,NONE,NONE,"DAS " }
		},
		{ 0x3F,
			{ NONE,NONE,NONE,"AAS " }
		},
		{ 0x40,
			{ EAX,NONE,NONE,"INC " }
		},
		{ 0x50,
			{ EAX,NONE,NONE,"PUSH " }
		},
		{ 0x60,
			{ NONE,NONE,NONE,"PUSH" }
		},
		{ 0x41,
			{ ECX,NONE,NONE,"INC " }
		},
		{ 0x51,
			{ ECX,NONE,NONE,"PUSH " }
		},
		{ 0x61,
			{ NONE,NONE,NONE,"POP" }
		},
		{ 0x42,
			{ EDX,NONE,NONE,"INC " }
		},
		{ 0x52,
			{ EDX,NONE,NONE,"PUSH " }
		},
		{ 0x43,
			{ EBX,NONE,NONE,"INC " }
		},
		{ 0x53,
			{ EBX,NONE,NONE,"PUSH " }
		},
		{ 0x44,
			{ ESP,NONE,NONE,"INC " }
		},
		{ 0x54,
			{ ESP,NONE,NONE,"PUSH " }
		},
		{ 0x45,
			{ EBP,NONE,NONE,"INC " }
		},
		{ 0x55,
			{ EBP,NONE,NONE,"PUSH " }
		},
		{ 0x46,
			{ ESI,NONE,NONE,"INC " }
		},
		{ 0x56,
			{ ESI,NONE,NONE,"PUSH " }
		},
		{ 0x47,
			{ EDI,NONE,NONE,"INC " }
		},
		{ 0x57,
			{ EDI,NONE,NONE,"PUSH " }
		},
		{ 0x48,
			{ EAX,NONE,NONE,"DEC " }
		},
		{ 0x58,
			{ EAX,NONE,NONE,"POP " }
		},
		{ 0x49,
			{ ECX,NONE,NONE,"DEC " }
		},
		{ 0x59,
			{ ECX,NONE,NONE,"POP " }
		},
		{ 0x4A,
			{ EDX,NONE,NONE,"DEC " }
		},
		{ 0x5A,
			{ EDX,NONE,NONE,"POP " }
		},
		{ 0x4B,
			{ EBX,NONE,NONE,"DEC " }
		},
		{ 0x5B,
			{ EBX,NONE,NONE,"POP " }
		},
		{ 0x4C,
			{ ESP,NONE,NONE,"DEC " }
		},
		{ 0x5C,
			{ ESP,NONE,NONE,"POP " }
		},
		{ 0x4D,
			{ EBP,NONE,NONE,"DEC " }
		},
		{ 0x5D,
			{ EBP,NONE,NONE,"POP " }
		},
		{ 0x4E,
			{ ESI,NONE,NONE,"DEC " }
		},
		{ 0x5E,
			{ ESI,NONE,NONE,"POP " }
		},
		{ 0x4F,
			{ EDI,NONE,NONE,"DEC " }
		},
		{ 0x5F,
			{ EDI,NONE,NONE,"POP " }
		},
		{ 0x6C,
			{ NONE,NONE,NONE,"INSB " }
		},
		{ 0x6D,
			{ NONE,NONE,NONE,"INS " }
		},
		{ 0x6E,
			{ NONE,NONE,NONE,"OUTSB " }
		},
		{ 0x6E,
			{ NONE,NONE,NONE,"OUTS " }
		},
		{ 0x90,
			{ NONE,NONE,NONE,"NOP " }
		},
		{ 0x91,
			{ ECX,EAX,NONE,"XCHG " }
		},
		{ 0x92,
			{ EDX,EAX,NONE,"XCHG " }
		},
		{ 0x93,
			{ EBX,EAX,NONE,"XCHG " }
		},
		{ 0x94,
			{ ESP,NONE,NONE,"XCHG " }
		},
		{ 0x95,
			{ EBP,EAX,NONE,"XCHG " }
		},
		{ 0x96,
			{ ESI,EAX,NONE,"XCHG " }
		},
		{ 0x97,
			{ EDI,EAX,NONE,"XCHG " }
		},
		{ 0xA4,
			{ NONE,NONE,NONE,"MOVSB " }
		},
		{ 0xA5,
			{ NONE,NONE,NONE,"MOVS" }
		},
		{ 0xA6,
			{ NONE,NONE,NONE,"CMPSB " }
		},
		{ 0xA7,
			{ NONE,NONE,NONE,"CMPS" }
		},
		{ 0x98,
			{ NONE,NONE,NONE,"C" }
		},
		{ 0x99,
			{ NONE,NONE,NONE,"C" }
		},
		{ 0x9B,
			{ NONE,NONE,NONE,"FWAIT" }
		},
		{ 0x9C,
			{ NONE,NONE,NONE,"PUSHF" }
		},
		{ 0x9D,
			{ NONE,NONE,NONE,"POPF" }
		},
		{ 0x9E,
			{ NONE,NONE,NONE,"SAHF" }
		},
		{ 0x9F,
			{ NONE,NONE,NONE,"LAHF" }
		},
		{ 0xAA,
			{ NONE,NONE,NONE,"STOSB" }
		},
		{ 0xAB,
			{ NONE,NONE,NONE,"STOS" }
		},
		{ 0xAC,
			{ NONE,NONE,NONE,"LODSB" }
		},
		{ 0xAD,
			{ NONE,NONE,NONE,"LODS" }
		},
		{ 0xAE,
			{ NONE,NONE,NONE,"SCASB" }
		},
		{ 0xAF,
			{ NONE,NONE,NONE,"SCAS" }
		},
		{ 0xC3,
			{ NONE,NONE,NONE,"RETN" }
		},
		{ 0xF1,
			{ NONE,NONE,NONE,"INT1" }
		},
		{ 0xD6,
			{ NONE,NONE,NONE,"SALC" }
		},
		{ 0xD7,
			{ NONE,NONE,NONE,"XLAT" }
		},
		{ 0xF4,
			{ NONE,NONE,NONE,"HLT" }
		},
		{ 0xF5,
			{ NONE,NONE,NONE,"CMC" }
		},
		{ 0xC9,
			{ NONE,NONE,NONE,"LEAVE" }
		},
		{ 0xCB,
			{ NONE,NONE,NONE,"RET FAR" }
		},
		{ 0xF8,
			{ NONE,NONE,NONE,"CLS" }
		},
		{ 0xF9,
			{ NONE,NONE,NONE,"STC" }
		},
		{ 0xFA,
			{ NONE,NONE,NONE,"CLI" }
		},
		{ 0xFB,
			{ NONE,NONE,NONE,"STI" }
		},
		{ 0xFC,
			{ NONE,NONE,NONE,"CLD" }
		},
		{ 0xFD,
			{ NONE,NONE,NONE,"STD" }
		},
		{ 0xEC,
			{ AL,DX,NONE,"IN " }
		},
		{ 0xED,
			{ EAX,DX,NONE,"IN " }
		},
		{ 0xEE,
			{ DX,AL,NONE,"OUT " }
		},
		{ 0xEF,
			{ DX,EAX,NONE,"OUT " }
		},
		{ 0xCC,
			{ NONE,NONE,NONE,"INT3" }
		},
		{ 0xCE,
			{ NONE,NONE,NONE,"INTO" }
		},
		{ 0xCF,
			{ NONE,NONE,NONE,"IRET" }
		},
	}
	},

	{ PREFIX,
	{
		{ 0x26,
			{ NONE,NONE,NONE,"ES:" }
		},
		{ 0x37,
			{ NONE,NONE,NONE,"SS:" }
		},
		{ 0x2E,
			{ NONE,NONE,NONE,"CS:" }
		},
		{ 0x3E,
			{ NONE,NONE,NONE,"DS:" }
		},
		{ 0x64,
			{ NONE,NONE,NONE,"FS:" }
		},
		{ 0x65,
			{ NONE,NONE,NONE,"GS:" }
		},
		{ 0xF0,
			{ NONE,NONE,NONE,"LOCK" }
		},
		{ 0xF2,
			{ NONE,NONE,NONE,"REPNE" }
		},
		{ 0xF3,
			{ NONE,NONE,NONE,"REPE" }
		},
	}
	},

	{ GROUP,
	{
		{ 0x80,
			{ Eb,Ib,NONE,"Group" }
		},
		{ 0x81,
			{ Ev,Iz,NONE,"Group" }
		},
		{ 0x82,
			{ Eb,Ib,NONE,"Group" }
		},
		{ 0x83,
			{ Ev,Ib,NONE,"Group" }
		},
		{ 0x8F,
			{ Ev,NONE,NONE,"Group" }
		},
		{ 0xC0,
			{ Eb,Ib,NONE,"Group" }
		},
		{ 0xC1,
			{ Ev,Ib,NONE,"Group" }
		},
		{ 0xD0,
			{ Eb,NONE,NONE,"Group" }	//带一个1立即数
		},
		{ 0xD1,
			{ Ev,NONE,NONE,"Group" }
		},
		{ 0xD2,
			{ Eb,CL,NONE,"Group" }
		},
		{ 0xD3,
			{ Ev,CL,NONE,"Group" }
		},
		{ 0xF6,
			{ Eb,NONE,NONE,"Group" }
		},
		{ 0xF7,
			{ Ev,NONE,NONE,"Group" }
		},
		{ 0xFE,
			{ Eb,NONE,NONE,"Group" }
		},
		{ 0xFF,
			{ Ev,NONE,NONE,"Group" }
		},
		{ 0xC6,
			{ Eb,Ib,NONE,"Group" }
		},
		{ 0xC7,
			{ Ev,Iz,NONE,"Group" }
		},
		{0x8F,
			{ Ev,NONE,NONE,"Group" }
		},
	}
	},

	{ ADDR67,
	{
		{ 0xA0,
		{ AL,Ob,NONE,"MOV " }
		},
		{ 0xA1,
		{ EAX,Ov,NONE,"MOV " }
		},
		{ 0xA2,
		{ Ob,AL,NONE,"MOV " }
		},
		{ 0xA3,
		{ Ov,EAX,NONE,"MOV " }
		},
	}
	},

	{ TWOBYTE,
	{
		{ 0x05,
		{ NONE,NONE,NONE,"SYSCALL" }
		},
		{ 0x07,
		{ NONE,NONE,NONE,"SYSRET" }
		},
		{ 0x1F,
		{ Ev,Gv,NONE,"NOP" }
		},
		{ 0x1C,
		{ Ev,Gv,NONE,"NOP" }
		},
		{ 0x1D,
		{ Ev,Gv,NONE,"NOP" }
		},
		{ 0x1E,
		{ Ev,Gv,NONE,"NOP" }
		},
		{ 0x30,
		{ NONE,NONE,NONE,"WRMSR" }
		},
		{ 0x31,
		{ NONE,NONE,NONE,"RDTSC" }
		},
		{ 0x32,
		{ NONE,NONE,NONE,"RDMSR" }
		},
		{ 0x33,
		{ NONE,NONE,NONE,"RDPMC" }
		},
		{ 0x34,
		{ NONE,NONE,NONE,"SYSENTER" }
		},
		{ 0x35,
		{ NONE,NONE,NONE,"SYSEXIT" }
		},
		{ 0x36,
		{ NONE,NONE,NONE,"GETSEC" }
		},
		{ 0x40,
		{ Gv,Ev,NONE,"CMOVO" }
		},
		{ 0x41,
		{ Gv,Ev,NONE,"CMOVNO" }
		},
		{ 0x42,
		{ Gv,Ev,NONE,"CMOVB" }
		},
		{ 0x43,
		{ Gv,Ev,NONE,"CMOVNB" }
		},
		{ 0x44,
		{ Gv,Ev,NONE,"CMOVE" }
		},
		{ 0x45,
		{ Gv,Ev,NONE,"CMOVNE" }
		},
		{ 0x46,
		{ Gv,Ev,NONE,"CMOVBE" }
		},
		{ 0x47,
		{ Gv,Ev,NONE,"CMOVA" }
		},
		{ 0x48,
		{ Gv,Ev,NONE,"CMOVS" }
		},
		{ 0x49,
		{ Gv,Ev,NONE,"CMOVNS" }
		},
		{ 0x4A,
		{ Gv,Ev,NONE,"CMOVP" }
		},
		{ 0x4B,
		{ Gv,Ev,NONE,"CMOVNP" }
		},
		{ 0x4C,
		{ Gv,Ev,NONE,"CMOVL" }
		},
		{ 0x4D,
		{ Gv,Ev,NONE,"CMOVNL" }
		},
		{ 0x4E,
		{ Gv,Ev,NONE,"CMOVNG" }
		},
		{ 0x4F,
		{ Gv,Ev,NONE,"CMOVG" }
		},
		{ 0x80,
		{ Jz,NONE,NONE,"JO" }
		},
		{ 0x81,
		{ Jz,NONE,NONE,"JNO" }
		},
		{ 0x82,
		{ Jz,NONE,NONE,"JB" }
		},
		{ 0x83,
		{ Jz,NONE,NONE,"JNB" }
		},
		{ 0x84,
		{ Jz,NONE,NONE,"JE" }
		},
		{ 0x85,
		{ Jz,NONE,NONE,"JNE" }
		},
		{ 0x86,
		{ Jz,NONE,NONE,"JNA" }
		},
		{ 0x87,
		{ Jz,NONE,NONE,"JA" }
		},
		{ 0x88,
		{ Jz,NONE,NONE,"JS" }
		},
		{ 0x89,
		{ Jz,NONE,NONE,"JNS" }
		},
		{ 0x8A,
		{ Jz,NONE,NONE,"JP" }
		},
		{ 0x8B,
		{ Jz,NONE,NONE,"JNP" }
		},
		{ 0x8C,
		{ Jz,NONE,NONE,"JL" }
		},
		{ 0x8D,
		{ Jz,NONE,NONE,"JNL" }
		},
		{ 0x8E,
		{ Jz,NONE,NONE,"JNG" }
		},
		{ 0x8F,
		{ Jz,NONE,NONE,"JG" }
		},
		{ 0x90,
		{ Eb,NONE,NONE,"SETO" }
		},
		{ 0x91,
		{ Eb,NONE,NONE,"SETNO" }
		},
		{ 0x92,
		{ Eb,NONE,NONE,"SETB" }
		},
		{ 0x93,
		{ Eb,NONE,NONE,"SETNB" }
		},
		{ 0x94,
		{ Eb,NONE,NONE,"SETE" }
		},
		{ 0x95,
		{ Eb,NONE,NONE,"SETNE" }
		},
		{ 0x96,
		{ Eb,NONE,NONE,"SETNA" }
		},
		{ 0x97,
		{ Eb,NONE,NONE,"SETA" }
		},
		{ 0x98,
		{ Eb,NONE,NONE,"SETS" }
		},
		{ 0x99,
		{ Eb,NONE,NONE,"SETNS" }
		},
		{ 0x9A,
		{ Eb,NONE,NONE,"SETP" }
		},
		{ 0x9B,
		{ Eb,NONE,NONE,"SETNP" }
		},
		{ 0x9C,
		{ Eb,NONE,NONE,"SETL" }
		},
		{ 0x9D,
		{ Eb,NONE,NONE,"SETNL" }
		},
		{ 0x9E,
		{ Eb,NONE,NONE,"SETNG" }
		},
		{ 0x9F,
		{ Eb,NONE,NONE,"SETG" }
		},
		{ 0xA0,
		{ NONE,NONE,NONE,"PUSH FS" }
		},
		{ 0xA1,
		{ NONE,NONE,NONE,"POP FS" }
		},
		{ 0xA2,
		{ NONE,NONE,NONE,"CPUID" }
		},
		{ 0xA3,
		{ Ev,Gv,NONE,"BT" }
		},
		{ 0xA4,
		{ Ev,Gv,Ib,"SHLD" }
		},
		{ 0xA5,
		{ Ev,Gv,CL,"SHLD" }
		},
		{ 0xA8,
		{ NONE,NONE,NONE,"PUSH GS" }
		},
		{ 0xA9,
		{ NONE,NONE,NONE,"POP GS" }
		},
		{ 0xAA,
		{ NONE,NONE,NONE,"RSM" }
		},
		{ 0xAB,
		{ Ev,Gv,NONE,"BTS" }
		},
		{ 0xAC,
		{ Ev,Gv,Ib,"SHRD" }
		},
		{ 0xAD,
		{ Ev,Gv,CL,"SHRD" }
		},
		{ 0xAF,
		{ Gv,Ev,NONE,"IMUL" }
		},
		{ 0xB0,
		{ Eb,Gb,NONE,"CMPXCHG" }
		},
		{ 0xB1,
		{ Ev,Gv,NONE,"CMPXCHG" }
		},
		{ 0xB2,
		{ Gv,MXX,NONE,"LSS" }
		},
		{ 0xB3,
		{ Ev,Gv,NONE,"BTR" }
		},
		{ 0xB4,
		{ Gv,MXX,NONE,"LFS" }
		},
		{ 0xB5,
		{ Gv,MXX,NONE,"LGS" }
		},
		{ 0xB6,
		{ Gv,Eb,NONE,"MOVZX" }
		},
		{ 0xB7,
		{ Gv,Ew,NONE,"MOVZX" }
		},
		{ 0xBB,
		{ Ev,Gv,NONE,"BTC" }
		},
		{ 0xBC,
		{ Gv,Ev,NONE,"BSF" }
		},
		{ 0xBD,
		{ Gv,Ev,NONE,"BSR" }
		},
		{ 0xBE,
		{ Gv,Eb,NONE,"MOVSX" }
		},
		{ 0xBF,
		{ Gv,Ew,NONE,"MOVSX" }
		},
	}
	},

};

map<Type, map<CodeIndex, Analy_Opcode>> FPU_INSTR_TABLE00_BF = 
{
	{ 0xD8,
	{
		{ 0,
		{ Freg+0,Fd,NONE,"FADD" }
		},
		{ 1,
		{ Freg + 0,Fd,NONE,"FMUL" }
		},
		{ 2,
		{ Freg + 0,Fd,NONE,"FCOM" }
		},
		{ 3,
		{ Freg + 0,Fd,NONE,"FCOMP" }
		},
		{ 4,
		{ Freg + 0,Fd,NONE,"FSUB" }
		},
		{ 5,
		{ Freg + 0,Fd,NONE,"FSUBR" }
		},
		{ 6,
		{ Freg + 0,Fd,NONE,"FDIV" }
		},
		{ 7,
		{ Freg + 0,Fd,NONE,"FDIVR" }
		},
	}
	},

	{ 0xD9,
	{
		{ 0,
		{ Freg + 0,Fd,NONE,"FLD" }
		},
		{ 1,
		{ NONE,NONE,NONE,"???" }
		},
		{ 2,
		{Fd, Freg + 0,NONE,"FST" }
		},
		{ 3,
		{ Fd, Freg + 0,NONE,"FSTP" }
		},
		{ 4,
		{ F28,NONE,NONE,"FLDENV" }
		},
		{ 5,
		{ Fw,NONE,NONE,"FLDCW" }
		},
		{ 6,
		{ F28,NONE,NONE,"FSTENV" }
		},
		{ 7,
		{ Fw,NONE,NONE,"FSTCW" }
		},
	}
	},

	{ 0xDA,
	{
		{ 0,
		{ Freg + 0,Fd,NONE,"FIADD" }
		},
		{ 1,
		{ Freg + 0,Fd,NONE,"FIMUL" }
		},
		{ 2,
		{ Freg + 0,Fd,NONE,"FICOM" }
		},
		{ 3,
		{ Freg + 0,Fd,NONE,"FICOMP" }
		},
		{ 4,
		{ Freg + 0,Fd,NONE,"FISUB" }
		},
		{ 5,
		{ Freg + 0,Fd,NONE,"FISUBR" }
		},
		{ 6,
		{ Freg + 0,Fd,NONE,"FIDIV" }
		},
		{ 7,
		{ Freg + 0,Fd,NONE,"FIDIVR" }
		},
	}
	},

	{ 0xDB,
	{
		{ 0,
		{ Freg + 0,Fd,NONE,"FILD" }
		},
		{ 1,
		{ Fd,Freg + 0,NONE,"FISTTP" }
		},
		{ 2,
		{ Fd,Freg + 0,NONE,"FIST" }
		},
		{ 3,
		{ Fd,Freg + 0,NONE,"FISTP" }
		},
		{ 4,
		{ NONE,NONE,NONE,"???" }
		},
		{ 5,
		{ Freg + 0,Ft,NONE,"FLD" }
		},
		{ 6,
		{ NONE,NONE,NONE,"???" }
		},
		{ 7,
		{ Ft,Freg + 0,NONE,"FSTP" }
		},
	}
	},

	{ 0xDC,
	{
		{ 0,
		{ Freg + 0,Fq,NONE,"FADD" }
		},
		{ 1,
		{ Freg + 0,Fq,NONE,"FMUL" }
		},
		{ 2,
		{ Freg + 0,Fq,NONE,"FCOM" }
		},
		{ 3,
		{ Freg + 0,Fq,NONE,"FCOMP" }
		},
		{ 4,
		{ Freg + 0,Fq,NONE,"FSUB" }
		},
		{ 5,
		{ Freg + 0,Fq,NONE,"FSUBR" }
		},
		{ 6,
		{ Freg + 0,Fq,NONE,"FDIV" }
		},
		{ 7,
		{ Freg + 0,Fq,NONE,"FDIVR" }
		},
	}
	},

	{ 0xDD,
	{
		{ 0,
		{ Freg + 0,Fq,NONE,"FLD" }
		},
		{ 1,
		{ Fq,Freg + 0,NONE,"FISTTP" }
		},
		{ 2,
		{ Fq,Freg + 0,NONE,"FST" }
		},
		{ 3,
		{ Fq,Freg + 0,NONE,"FSTP" }
		},
		{ 4,
		{ F98,NONE,NONE,"FRSTOR" }
		},
		{ 5,
		{ NONE,NONE,NONE,"???" }
		},
		{ 6,
		{ F98,NONE,NONE,"FSAVE" }
		},
		{ 7,
		{ Fw,NONE,NONE,"FSTSW" }
		},
	}
	},

	{ 0xDE,
	{
		{ 0,
		{ Freg + 0,Fw,NONE,"FIADD" }
		},
		{ 1,
		{ Freg + 0,Fw,NONE,"FIMUL" }
		},
		{ 2,
		{ Freg + 0,Fw,NONE,"FICOM" }
		},
		{ 3,
		{ Freg + 0,Fw,NONE,"FICOMP" }
		},
		{ 4,
		{ Freg + 0,Fw,NONE,"FISUB" }
		},
		{ 5,
		{ Freg + 0,Fw,NONE,"FISUBR" }
		},
		{ 6,
		{ Freg + 0,Fw,NONE,"FIDIV" }
		},
		{ 7,
		{ Freg + 0,Fw,NONE,"FIDIVR" }
		},
	}
	},

	{ 0xDF,
	{
		{ 0,
		{ Freg + 0,Fw,NONE,"FILD" }
		},
		{ 1,
		{ Fw,Freg + 0,NONE,"FISTTP" }
		},
		{ 2,
		{ Fw,Freg + 0,NONE,"FIST" }
		},
		{ 3,
		{ Fw,Freg + 0,NONE,"FISTP" }
		},
		{ 4,
		{ Freg + 0,Ft,NONE,"FBLD" }
		},
		{ 5,
		{ Freg + 0,Fq,NONE,"FILD" }
		},
		{ 6,
		{ Ft,Freg + 0,NONE,"FBSTP" }
		},
		{ 7,
		{ Fq,Freg + 0,NONE,"FISTP" }
		},
	}
	},
};

map<Type, map<CodeIndex, Analy_Opcode>> FPU_INSTR_TABLEC0_FF =
{
	{ 0xD8,
	{
		{ 0x1800,
		{ Freg + 0,Freg + 0,NONE,"FADD" }
		},
		{ 0x1900,
		{ Freg + 0,Freg + 1,NONE,"FADD" }
		},
		{ 0x1A00,
		{ Freg + 0,Freg + 2,NONE,"FADD" }
		},
		{ 0x1B00,
		{ Freg + 0,Freg + 3,NONE,"FADD" }
		},
		{ 0x1C00,
		{ Freg + 0,Freg + 4,NONE,"FADD" }
		},
		{ 0x1D00,
		{ Freg + 0,Freg + 5,NONE,"FADD" }
		},
		{ 0x1E00,
		{ Freg + 0,Freg + 6,NONE,"FADD" }
		},
		{ 0x1F00,
		{ Freg + 0,Freg + 7,NONE,"FADD" }
		},

		{ 0x1801,
		{ Freg + 0,Freg + 0,NONE,"FMUL" }
		},
		{ 0x1901,
		{ Freg + 0,Freg + 1,NONE,"FMUL" }
		},
		{ 0x1A01,
		{ Freg + 0,Freg + 2,NONE,"FMUL" }
		},
		{ 0x1B01,
		{ Freg + 0,Freg + 3,NONE,"FMUL" }
		},
		{ 0x1C01,
		{ Freg + 0,Freg + 4,NONE,"FMUL" }
		},
		{ 0x1D01,
		{ Freg + 0,Freg + 5,NONE,"FMUL" }
		},
		{ 0x1E01,
		{ Freg + 0,Freg + 6,NONE,"FMUL" }
		},
		{ 0x1F01,
		{ Freg + 0,Freg + 7,NONE,"FMUL" }
		},

		{ 0x1802,
		{ Freg + 0,Freg + 0,NONE,"FCOM" }
		},
		{ 0x1902,
		{ Freg + 0,Freg + 1,NONE,"FCOM" }
		},
		{ 0x1A02,
		{ Freg + 0,Freg + 2,NONE,"FCOM" }
		},
		{ 0x1B02,
		{ Freg + 0,Freg + 3,NONE,"FCOM" }
		},
		{ 0x1C02,
		{ Freg + 0,Freg + 4,NONE,"FCOM" }
		},
		{ 0x1D02,
		{ Freg + 0,Freg + 5,NONE,"FCOM" }
		},
		{ 0x1E02,
		{ Freg + 0,Freg + 6,NONE,"FCOM" }
		},
		{ 0x1F02,
		{ Freg + 0,Freg + 7,NONE,"FCOM" }
		},


		{ 0x1803,
		{ Freg + 0,Freg + 0,NONE,"FCOMP" }
		},
		{ 0x1903,
		{ Freg + 0,Freg + 1,NONE,"FCOMP" }
		},
		{ 0x1A03,
		{ Freg + 0,Freg + 2,NONE,"FCOMP" }
		},
		{ 0x1B03,
		{ Freg + 0,Freg + 3,NONE,"FCOMP" }
		},
		{ 0x1C03,
		{ Freg + 0,Freg + 4,NONE,"FCOMP" }
		},
		{ 0x1D03,
		{ Freg + 0,Freg + 5,NONE,"FCOMP" }
		},
		{ 0x1E03,
		{ Freg + 0,Freg + 6,NONE,"FCOMP" }
		},
		{ 0x1F03,
		{ Freg + 0,Freg + 7,NONE,"FCOMP" }
		},

		{ 0x1804,
		{ Freg + 0,Freg + 0,NONE,"FSUB" }
		},
		{ 0x1904,
		{ Freg + 0,Freg + 1,NONE,"FSUB" }
		},
		{ 0x1A04,
		{ Freg + 0,Freg + 2,NONE,"FSUB" }
		},
		{ 0x1B04,
		{ Freg + 0,Freg + 3,NONE,"FSUB" }
		},
		{ 0x1C04,
		{ Freg + 0,Freg + 4,NONE,"FSUB" }
		},
		{ 0x1D04,
		{ Freg + 0,Freg + 5,NONE,"FSUB" }
		},
		{ 0x1E04,
		{ Freg + 0,Freg + 6,NONE,"FSUB" }
		},
		{ 0x1F04,
		{ Freg + 0,Freg + 7,NONE,"FSUB" }
		},

		{ 0x1805,
		{ Freg + 0,Freg + 0,NONE,"FSUBR" }
		},
		{ 0x1905,
		{ Freg + 0,Freg + 1,NONE,"FSUBR" }
		},
		{ 0x1A05,
		{ Freg + 0,Freg + 2,NONE,"FSUBR" }
		},
		{ 0x1B05,
		{ Freg + 0,Freg + 3,NONE,"FSUBR" }
		},
		{ 0x1C05,
		{ Freg + 0,Freg + 4,NONE,"FSUBR" }
		},
		{ 0x1D05,
		{ Freg + 0,Freg + 5,NONE,"FSUBR" }
		},
		{ 0x1E05,
		{ Freg + 0,Freg + 6,NONE,"FSUBR" }
		},
		{ 0x1F05,
		{ Freg + 0,Freg + 7,NONE,"FSUBR" }
		},

		{ 0x1806,
		{ Freg + 0,Freg + 0,NONE,"FDIV" }
		},
		{ 0x1906,
		{ Freg + 0,Freg + 1,NONE,"FDIV" }
		},
		{ 0x1A06,
		{ Freg + 0,Freg + 2,NONE,"FDIV" }
		},
		{ 0x1B06,
		{ Freg + 0,Freg + 3,NONE,"FDIV" }
		},
		{ 0x1C06,
		{ Freg + 0,Freg + 4,NONE,"FDIV" }
		},
		{ 0x1D06,
		{ Freg + 0,Freg + 5,NONE,"FDIV" }
		},
		{ 0x1E06,
		{ Freg + 0,Freg + 6,NONE,"FDIV" }
		},
		{ 0x1F06,
		{ Freg + 0,Freg + 7,NONE,"FDIV" }
		},

		{ 0x1807,
		{ Freg + 0,Freg + 0,NONE,"FDIVR" }
		},
		{ 0x1907,
		{ Freg + 0,Freg + 1,NONE,"FDIVR" }
		},
		{ 0x1A07,
		{ Freg + 0,Freg + 2,NONE,"FDIVR" }
		},
		{ 0x1B07,
		{ Freg + 0,Freg + 3,NONE,"FDIVR" }
		},
		{ 0x1C07,
		{ Freg + 0,Freg + 4,NONE,"FDIVR" }
		},
		{ 0x1D07,
		{ Freg + 0,Freg + 5,NONE,"FDIVR" }
		},
		{ 0x1E07,
		{ Freg + 0,Freg + 6,NONE,"FDIVR" }
		},
		{ 0x1F07,
		{ Freg + 0,Freg + 7,NONE,"FDIVR" }
		},
	}
	}, 

	{ 0xD9,
	{
		{ 0x1800,
		{ Freg + 0,Freg + 0,NONE,"FLD" }
		},
		{ 0x1900,
		{ Freg + 0,Freg + 1,NONE,"FLD" }
		},
		{ 0x1A00,
		{ Freg + 0,Freg + 2,NONE,"FLD" }
		},
		{ 0x1B00,
		{ Freg + 0,Freg + 3,NONE,"FLD" }
		},
		{ 0x1C00,
		{ Freg + 0,Freg + 4,NONE,"FLD" }
		},
		{ 0x1D00,
		{ Freg + 0,Freg + 5,NONE,"FLD" }
		},
		{ 0x1E00,
		{ Freg + 0,Freg + 6,NONE,"FLD" }
		},
		{ 0x1F00,
		{ Freg + 0,Freg + 7,NONE,"FLD" }
		},

		{ 0x1801,
		{ Freg + 0,Freg + 0,NONE,"FXCH" }
		},
		{ 0x1901,
		{ Freg + 0,Freg + 1,NONE,"FXCH" }
		},
		{ 0x1A01,
		{ Freg + 0,Freg + 2,NONE,"FXCH" }
		},
		{ 0x1B01,
		{ Freg + 0,Freg + 3,NONE,"FXCH" }
		},
		{ 0x1C01,
		{ Freg + 0,Freg + 4,NONE,"FXCH" }
		},
		{ 0x1D01,
		{ Freg + 0,Freg + 5,NONE,"FXCH" }
		},
		{ 0x1E01,
		{ Freg + 0,Freg + 6,NONE,"FXCH" }
		},
		{ 0x1F01,
		{ Freg + 0,Freg + 7,NONE,"FXCH" }
		},

		{ 0x1802,
		{ NONE,NONE,NONE,"FNOP" }
		},
		{ 0x1902,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1A02,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1B02,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1C02,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1D02,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1E02,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1F02,
		{ NONE,NONE,NONE,"???" }
		},


		{ 0x1803,
		{ Freg + 0,Freg + 0,NONE,"FSTPNCE" }
		},
		{ 0x1903,
		{ Freg + 1,Freg + 0,NONE,"FSTPNCE" }
		},
		{ 0x1A03,
		{ Freg + 2,Freg + 0,NONE,"FSTPNCE" }
		},
		{ 0x1B03,
		{ Freg + 3,Freg + 0,NONE,"FSTPNCE" }
		},
		{ 0x1C03,
		{ Freg + 4,Freg + 0,NONE,"FSTPNCE" }
		},
		{ 0x1D03,
		{ Freg + 5,Freg + 0,NONE,"FSTPNCE" }
		},
		{ 0x1E03,
		{ Freg + 6,Freg + 0,NONE,"FSTPNCE" }
		},
		{ 0x1F03,
		{ Freg + 7,Freg + 0,NONE,"FSTPNCE" }
		},

		{ 0x1804,
		{ NONE,NONE,NONE,"FCHS" }
		},
		{ 0x1904,
		{ NONE,NONE,NONE,"FABS" }
		},
		{ 0x1A04,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1B04,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1C04,
		{ NONE,NONE,NONE,"FTST" }
		},
		{ 0x1D04,
		{ NONE,NONE,NONE,"FXAM" }
		},
		{ 0x1E04,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1F04,
		{ NONE,NONE,NONE,"???" }
		},

		{ 0x1805,
		{ NONE,NONE,NONE,"FLD1" }
		},
		{ 0x1905,
		{ NONE,NONE,NONE,"FLDL2T" }
		},
		{ 0x1A05,
		{ NONE,NONE,NONE,"FLDL2E" }
		},
		{ 0x1B05,
		{ NONE,NONE,NONE,"FLDP1" }
		},
		{ 0x1C05,
		{ NONE,NONE,NONE,"FLDLG2" }
		},
		{ 0x1D05,
		{ NONE,NONE,NONE,"FLDLN2" }
		},
		{ 0x1E05,
		{ NONE,NONE,NONE,"FLDZ" }
		},
		{ 0x1F05,
		{ NONE,NONE,NONE,"???" }
		},

		{ 0x1806,
		{ NONE,NONE,NONE,"F2XM1" }
		},
		{ 0x1906,
		{ NONE,NONE,NONE,"FYL2X" }
		},
		{ 0x1A06,
		{ NONE,NONE,NONE,"FPTAN" }
		},
		{ 0x1B06,
		{ NONE,NONE,NONE,"FPATAN" }
		},
		{ 0x1C06,
		{ NONE,NONE,NONE,"FXTRACT" }
		},
		{ 0x1D06,
		{ NONE,NONE,NONE,"FPREM1" }
		},
		{ 0x1E06,
		{ NONE,NONE,NONE,"FDECSTP" }
		},
		{ 0x1F06,
		{ NONE,NONE,NONE,"FINCSTP" }
		},

		{ 0x1807,
		{ NONE,NONE,NONE,"FPREM" }
		},
		{ 0x1907,
		{ NONE,NONE,NONE,"FYL2XP1" }
		},
		{ 0x1A07,
		{ NONE,NONE,NONE,"FSQRT" }
		},
		{ 0x1B07,
		{ NONE,NONE,NONE,"FSINCOS" }
		},
		{ 0x1C07,
		{ NONE,NONE,NONE,"FRNDINT" }
		},
		{ 0x1D07,
		{ NONE,NONE,NONE,"FSCALE" }
		},
		{ 0x1E07,
		{ NONE,NONE,NONE,"FSIN" }
		},
		{ 0x1F07,
		{ NONE,NONE,NONE,"FCOS" }
		},
	}
	},

	{ 0xDA,
	{
		{ 0x1800,
		{ Freg + 0,Freg + 0,NONE,"FCMOVB" }
		},
		{ 0x1900,
		{ Freg + 0,Freg + 1,NONE,"FCMOVB" }
		},
		{ 0x1A00,
		{ Freg + 0,Freg + 2,NONE,"FCMOVB" }
		},
		{ 0x1B00,
		{ Freg + 0,Freg + 3,NONE,"FCMOVB" }
		},
		{ 0x1C00,
		{ Freg + 0,Freg + 4,NONE,"FCMOVB" }
		},
		{ 0x1D00,
		{ Freg + 0,Freg + 5,NONE,"FCMOVB" }
		},
		{ 0x1E00,
		{ Freg + 0,Freg + 6,NONE,"FCMOVB" }
		},
		{ 0x1F00,
		{ Freg + 0,Freg + 7,NONE,"FCMOVB" }
		},

		{ 0x1801,
		{ Freg + 0,Freg + 0,NONE,"FCMOVE" }
		},
		{ 0x1901,
		{ Freg + 0,Freg + 1,NONE,"FCMOVE" }
		},
		{ 0x1A01,
		{ Freg + 0,Freg + 2,NONE,"FCMOVE" }
		},
		{ 0x1B01,
		{ Freg + 0,Freg + 3,NONE,"FCMOVE" }
		},
		{ 0x1C01,
		{ Freg + 0,Freg + 4,NONE,"FCMOVE" }
		},
		{ 0x1D01,
		{ Freg + 0,Freg + 5,NONE,"FCMOVE" }
		},
		{ 0x1E01,
		{ Freg + 0,Freg + 6,NONE,"FCMOVE" }
		},
		{ 0x1F01,
		{ Freg + 0,Freg + 7,NONE,"FCMOVE" }
		},

		{ 0x1802,
		{ Freg + 0,Freg + 0,NONE,"FCMOVBE" }
		},
		{ 0x1902,
		{ Freg + 0,Freg + 1,NONE,"FCMOVBE" }
		},
		{ 0x1A02,
		{ Freg + 0,Freg + 2,NONE,"FCMOVBE" }
		},
		{ 0x1B02,
		{ Freg + 0,Freg + 3,NONE,"FCMOVBE" }
		},
		{ 0x1C02,
		{ Freg + 0,Freg + 4,NONE,"FCMOVBE" }
		},
		{ 0x1D02,
		{ Freg + 0,Freg + 5,NONE,"FCMOVBE" }
		},
		{ 0x1E02,
		{ Freg + 0,Freg + 6,NONE,"FCMOVBE" }
		},
		{ 0x1F02,
		{ Freg + 0,Freg + 7,NONE,"FCMOVBE" }
		},


		{ 0x1803,
		{ Freg + 0,Freg + 0,NONE,"FCMOVU" }
		},
		{ 0x1903,
		{ Freg + 0,Freg + 1,NONE,"FCMOVU" }
		},
		{ 0x1A03,
		{ Freg + 0,Freg + 2,NONE,"FCMOVU" }
		},
		{ 0x1B03,
		{ Freg + 0,Freg + 3,NONE,"FCMOVU" }
		},
		{ 0x1C03,
		{ Freg + 0,Freg + 4,NONE,"FCMOVU" }
		},
		{ 0x1D03,
		{ Freg + 0,Freg + 5,NONE,"FCMOVU" }
		},
		{ 0x1E03,
		{ Freg + 0,Freg + 6,NONE,"FCMOVU" }
		},
		{ 0x1F03,
		{ Freg + 0,Freg + 7,NONE,"FCMOVU" }
		},

		{ 0x1804,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1904,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1A04,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1B04,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1C04,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1D04,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1E04,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1F04,
		{ NONE,NONE,NONE,"???" }
		},

		{ 0x1805,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1905,
		{ NONE,NONE,NONE,"FUCOMPP" }
		},
		{ 0x1A05,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1B05,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1C05,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1D05,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1E05,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1F05,
		{ NONE,NONE,NONE,"???" }
		},

		{ 0x1806,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1906,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1A06,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1B06,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1C06,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1D06,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1E06,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1F06,
		{ NONE,NONE,NONE,"???" }
		},

		{ 0x1807,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1907,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1A07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1B07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1C07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1D07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1E07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1F07,
		{ NONE,NONE,NONE,"???" }
		},
	}
	},

	{ 0xDB,
	{
		{ 0x1800,
		{ Freg + 0,Freg + 0,NONE,"FCMOVNB" }
		},
		{ 0x1900,
		{ Freg + 0,Freg + 1,NONE,"FCMOVNB" }
		},
		{ 0x1A00,
		{ Freg + 0,Freg + 2,NONE,"FCMOVNB" }
		},
		{ 0x1B00,
		{ Freg + 0,Freg + 3,NONE,"FCMOVNB" }
		},
		{ 0x1C00,
		{ Freg + 0,Freg + 4,NONE,"FCMOVNB" }
		},
		{ 0x1D00,
		{ Freg + 0,Freg + 5,NONE,"FCMOVNB" }
		},
		{ 0x1E00,
		{ Freg + 0,Freg + 6,NONE,"FCMOVNB" }
		},
		{ 0x1F00,
		{ Freg + 0,Freg + 7,NONE,"FCMOVNB" }
		},

		{ 0x1801,
		{ Freg + 0,Freg + 0,NONE,"FCMOVNE" }
		},
		{ 0x1901,
		{ Freg + 0,Freg + 1,NONE,"FCMOVNE" }
		},
		{ 0x1A01,
		{ Freg + 0,Freg + 2,NONE,"FCMOVNE" }
		},
		{ 0x1B01,
		{ Freg + 0,Freg + 3,NONE,"FCMOVNE" }
		},
		{ 0x1C01,
		{ Freg + 0,Freg + 4,NONE,"FCMOVNE" }
		},
		{ 0x1D01,
		{ Freg + 0,Freg + 5,NONE,"FCMOVNE" }
		},
		{ 0x1E01,
		{ Freg + 0,Freg + 6,NONE,"FCMOVNE" }
		},
		{ 0x1F01,
		{ Freg + 0,Freg + 7,NONE,"FCMOVNE" }
		},

		{ 0x1802,
		{ Freg + 0,Freg + 0,NONE,"FCMOVNBE" }
		},
		{ 0x1902,
		{ Freg + 0,Freg + 1,NONE,"FCMOVNBE" }
		},
		{ 0x1A02,
		{ Freg + 0,Freg + 2,NONE,"FCMOVNBE" }
		},
		{ 0x1B02,
		{ Freg + 0,Freg + 3,NONE,"FCMOVNBE" }
		},
		{ 0x1C02,
		{ Freg + 0,Freg + 4,NONE,"FCMOVNBE" }
		},
		{ 0x1D02,
		{ Freg + 0,Freg + 5,NONE,"FCMOVNBE" }
		},
		{ 0x1E02,
		{ Freg + 0,Freg + 6,NONE,"FCMOVNBE" }
		},
		{ 0x1F02,
		{ Freg + 0,Freg + 7,NONE,"FCMOVNBE" }
		},


		{ 0x1803,
		{ Freg + 0,Freg + 0,NONE,"FCMOVNU" }
		},
		{ 0x1903,
		{ Freg + 0,Freg + 1,NONE,"FCMOVNU" }
		},
		{ 0x1A03,
		{ Freg + 0,Freg + 2,NONE,"FCMOVNU" }
		},
		{ 0x1B03,
		{ Freg + 0,Freg + 3,NONE,"FCMOVNU" }
		},
		{ 0x1C03,
		{ Freg + 0,Freg + 4,NONE,"FCMOVNU" }
		},
		{ 0x1D03,
		{ Freg + 0,Freg + 5,NONE,"FCMOVNU" }
		},
		{ 0x1E03,
		{ Freg + 0,Freg + 6,NONE,"FCMOVNU" }
		},
		{ 0x1F03,
		{ Freg + 0,Freg + 7,NONE,"FCMOVNU" }
		},

		{ 0x1804,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1904,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1A04,
		{ NONE,NONE,NONE,"FCLEX" }
		},
		{ 0x1B04,
		{ NONE,NONE,NONE,"FINIT" }
		},
		{ 0x1C04,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1D04,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1E04,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1F04,
		{ NONE,NONE,NONE,"???" }
		},

		{ 0x1805,
		{ Freg + 0,Freg + 0,NONE,"FUCOM1" }
		},
		{ 0x1905,
		{ Freg + 0,Freg + 1,NONE,"FUCOM1" }
		},
		{ 0x1A05,
		{ Freg + 0,Freg + 2,NONE,"FUCOM1" }
		},
		{ 0x1B05,
		{ Freg + 0,Freg + 3,NONE,"FUCOM1" }
		},
		{ 0x1C05,
		{ Freg + 0,Freg + 4,NONE,"FUCOM1" }
		},
		{ 0x1D05,
		{ Freg + 0,Freg + 5,NONE,"FUCOM1" }
		},
		{ 0x1E05,
		{ Freg + 0,Freg + 6,NONE,"FUCOM1" }
		},
		{ 0x1F05,
		{ Freg + 0,Freg + 7,NONE,"FUCOM1" }
		},

		{ 0x1806,
		{ Freg + 0,Freg + 0,NONE,"FCOM1" }
		},
		{ 0x1906,
		{ Freg + 0,Freg + 1,NONE,"FCOM1" }
		},
		{ 0x1A06,
		{ Freg + 0,Freg + 2,NONE,"FCOM1" }
		},
		{ 0x1B06,
		{ Freg + 0,Freg + 3,NONE,"FCOM1" }
		},
		{ 0x1C06,
		{ Freg + 0,Freg + 4,NONE,"FCOM1" }
		},
		{ 0x1D06,
		{ Freg + 0,Freg + 5,NONE,"FCOM1" }
		},
		{ 0x1E06,
		{ Freg + 0,Freg + 6,NONE,"FCOM1" }
		},
		{ 0x1F06,
		{ Freg + 0,Freg + 7,NONE,"FCOM1" }
		},

		{ 0x1807,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1907,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1A07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1B07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1C07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1D07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1E07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1F07,
		{ NONE,NONE,NONE,"???" }
		},
	}
	},

	{ 0xDC,
	{
		{ 0x1800,
		{ Freg + 0,Freg + 0,NONE,"FADD" }
		},
		{ 0x1900,
		{ Freg + 1,Freg + 0,NONE,"FADD" }
		},
		{ 0x1A00,
		{ Freg + 2,Freg + 0,NONE,"FADD" }
		},
		{ 0x1B00,
		{ Freg + 3,Freg + 0,NONE,"FADD" }
		},
		{ 0x1C00,
		{ Freg + 4,Freg + 0,NONE,"FADD" }
		},
		{ 0x1D00,
		{ Freg + 5,Freg + 0,NONE,"FADD" }
		},
		{ 0x1E00,
		{ Freg + 6,Freg + 0,NONE,"FADD" }
		},
		{ 0x1F00,
		{ Freg + 7,Freg + 0,NONE,"FADD" }
		},

		{ 0x1801,
		{ Freg + 0,Freg + 0,NONE,"FMUL" }
		},
		{ 0x1901,
		{ Freg + 1,Freg + 0,NONE,"FMUL" }
		},
		{ 0x1A01,
		{ Freg + 2,Freg + 0,NONE,"FMUL" }
		},
		{ 0x1B01,
		{ Freg + 3,Freg + 0,NONE,"FMUL" }
		},
		{ 0x1C01,
		{ Freg + 4,Freg + 0,NONE,"FMUL" }
		},
		{ 0x1D01,
		{ Freg + 5,Freg + 0,NONE,"FMUL" }
		},
		{ 0x1E01,
		{ Freg + 6,Freg + 0,NONE,"FMUL" }
		},
		{ 0x1F01,
		{ Freg + 7,Freg + 0,NONE,"FMUL" }
		},

		{ 0x1802,
		{ Freg + 0,Freg + 0,NONE,"FCOM" }
		},
		{ 0x1902,
		{ Freg + 0,Freg + 1,NONE,"FCOM" }
		},
		{ 0x1A02,
		{ Freg + 0,Freg + 2,NONE,"FCOM" }
		},
		{ 0x1B02,
		{ Freg + 0,Freg + 3,NONE,"FCOM" }
		},
		{ 0x1C02,
		{ Freg + 0,Freg + 4,NONE,"FCOM" }
		},
		{ 0x1D02,
		{ Freg + 0,Freg + 5,NONE,"FCOM" }
		},
		{ 0x1E02,
		{ Freg + 0,Freg + 6,NONE,"FCOM" }
		},
		{ 0x1F02,
		{ Freg + 0,Freg + 7,NONE,"FCOM" }
		},


		{ 0x1803,
		{ Freg + 0,Freg + 0,NONE,"FCOMP" }
		},
		{ 0x1903,
		{ Freg + 0,Freg + 1,NONE,"FCOMP" }
		},
		{ 0x1A03,
		{ Freg + 0,Freg + 2,NONE,"FCOMP" }
		},
		{ 0x1B03,
		{ Freg + 0,Freg + 3,NONE,"FCOMP" }
		},
		{ 0x1C03,
		{ Freg + 0,Freg + 4,NONE,"FCOMP" }
		},
		{ 0x1D03,
		{ Freg + 0,Freg + 5,NONE,"FCOMP" }
		},
		{ 0x1E03,
		{ Freg + 0,Freg + 6,NONE,"FCOMP" }
		},
		{ 0x1F03,
		{ Freg + 0,Freg + 7,NONE,"FCOMP" }
		},

		{ 0x1804,
		{ Freg + 0,Freg + 0,NONE,"FSUBR" }
		},
		{ 0x1904,
		{ Freg + 1,Freg + 0,NONE,"FSUBR" }
		},
		{ 0x1A04,
		{ Freg + 2,Freg + 0,NONE,"FSUBR" }
		},
		{ 0x1B04,
		{ Freg + 3,Freg + 0,NONE,"FSUBR" }
		},
		{ 0x1C04,
		{ Freg + 4,Freg + 0,NONE,"FSUBR" }
		},
		{ 0x1D04,
		{ Freg + 5,Freg + 0,NONE,"FSUBR" }
		},
		{ 0x1E04,
		{ Freg + 6,Freg + 0,NONE,"FSUBR" }
		},
		{ 0x1F04,
		{ Freg + 7,Freg + 0,NONE,"FSUBR" }
		},

		{ 0x1805,
		{ Freg + 0,Freg + 0,NONE,"FSUB" }
		},
		{ 0x1905,
		{ Freg + 1,Freg + 0,NONE,"FSUB" }
		},
		{ 0x1A05,
		{ Freg + 2,Freg + 0,NONE,"FSUB" }
		},
		{ 0x1B05,
		{ Freg + 3,Freg + 0,NONE,"FSUB" }
		},
		{ 0x1C05,
		{ Freg + 4,Freg + 0,NONE,"FSUB" }
		},
		{ 0x1D05,
		{ Freg + 5,Freg + 0,NONE,"FSUB" }
		},
		{ 0x1E05,
		{ Freg + 6,Freg + 0,NONE,"FSUB" }
		},
		{ 0x1F05,
		{ Freg + 7,Freg + 0,NONE,"FSUB" }
		},

		{ 0x1806,
		{ Freg + 0,Freg + 0,NONE,"FDIVR" }
		},
		{ 0x1906,
		{ Freg + 1,Freg + 0,NONE,"FDIVR" }
		},
		{ 0x1A06,
		{ Freg + 2,Freg + 0,NONE,"FDIVR" }
		},
		{ 0x1B06,
		{ Freg + 3,Freg + 0,NONE,"FDIVR" }
		},
		{ 0x1C06,
		{ Freg + 4,Freg + 0,NONE,"FDIVR" }
		},
		{ 0x1D06,
		{ Freg + 5,Freg + 0,NONE,"FDIVR" }
		},
		{ 0x1E06,
		{ Freg + 6,Freg + 0,NONE,"FDIVR" }
		},
		{ 0x1F06,
		{ Freg + 7,Freg + 0,NONE,"FDIVR" }
		},

		{ 0x1807,
		{ Freg + 0,Freg + 0,NONE,"FDIV" }
		},
		{ 0x1907,
		{ Freg + 1,Freg + 0,NONE,"FDIV" }
		},
		{ 0x1A07,
		{ Freg + 2,Freg + 0,NONE,"FDIV" }
		},
		{ 0x1B07,
		{ Freg + 3,Freg + 0,NONE,"FDIV" }
		},
		{ 0x1C07,
		{ Freg + 4,Freg + 0,NONE,"FDIV" }
		},
		{ 0x1D07,
		{ Freg + 5,Freg + 0,NONE,"FDIV" }
		},
		{ 0x1E07,
		{ Freg + 6,Freg + 0,NONE,"FDIV" }
		},
		{ 0x1F07,
		{ Freg + 7,Freg + 0,NONE,"FDIV" }
		},
	}
	},

	{ 0xDD,
	{
		{ 0x1800,
		{ Freg + 0,NONE,NONE,"FFREE" }
		},
		{ 0x1900,
		{ Freg + 1,NONE,NONE,"FFREE" }
		},
		{ 0x1A00,
		{ Freg + 2,NONE,NONE,"FFREE" }
		},
		{ 0x1B00,
		{ Freg + 3,NONE,NONE,"FFREE" }
		},
		{ 0x1C00,
		{ Freg + 4,NONE,NONE,"FFREE" }
		},
		{ 0x1D00,
		{ Freg + 5,NONE,NONE,"FFREE" }
		},
		{ 0x1E00,
		{ Freg + 6,NONE,NONE,"FFREE" }
		},
		{ 0x1F00,
		{ Freg + 7,NONE,NONE,"FFREE" }
		},

		{ 0x1801,
		{ Freg + 0,Freg + 0,NONE,"FXCH" }
		},
		{ 0x1901,
		{ Freg + 0,Freg + 1,NONE,"FXCH" }
		},
		{ 0x1A01,
		{ Freg + 0,Freg + 2,NONE,"FXCH" }
		},
		{ 0x1B01,
		{ Freg + 0,Freg + 3,NONE,"FXCH" }
		},
		{ 0x1C01,
		{ Freg + 0,Freg + 4,NONE,"FXCH" }
		},
		{ 0x1D01,
		{ Freg + 0,Freg + 5,NONE,"FXCH" }
		},
		{ 0x1E01,
		{ Freg + 0,Freg + 6,NONE,"FXCH" }
		},
		{ 0x1F01,
		{ Freg + 0,Freg + 7,NONE,"FXCH" }
		},

		{ 0x1802,
		{ Freg + 0,NONE,NONE,"FST" }
		},
		{ 0x1902,
		{ Freg + 1,NONE,NONE,"FST" }
		},
		{ 0x1A02,
		{ Freg + 2,NONE,NONE,"FST" }
		},
		{ 0x1B02,
		{ Freg + 3,NONE,NONE,"FST" }
		},
		{ 0x1C02,
		{ Freg + 4,NONE,NONE,"FST" }
		},
		{ 0x1D02,
		{ Freg + 5,NONE,NONE,"FST" }
		},
		{ 0x1E02,
		{ Freg + 6,NONE,NONE,"FST" }
		},
		{ 0x1F02,
		{ Freg + 7,NONE,NONE,"FST" }
		},

		{ 0x1803,
		{ Freg + 0,NONE,NONE,"FSTP" }
		},
		{ 0x1903,
		{ Freg + 1,NONE,NONE,"FSTP" }
		},
		{ 0x1A03,
		{ Freg + 2,NONE,NONE,"FSTP" }
		},
		{ 0x1B03,
		{ Freg + 3,NONE,NONE,"FSTP" }
		},
		{ 0x1C03,
		{ Freg + 4,NONE,NONE,"FSTP" }
		},
		{ 0x1D03,
		{ Freg + 5,NONE,NONE,"FSTP" }
		},
		{ 0x1E03,
		{ Freg + 6,NONE,NONE,"FSTP" }
		},
		{ 0x1F03,
		{ Freg + 7,NONE,NONE,"FSTP" }
		},

		{ 0x1804,
		{ Freg + 0,Freg + 0,NONE,"FUCOM" }
		},
		{ 0x1904,
		{ Freg + 1,Freg + 0,NONE,"FUCOM" }
		},
		{ 0x1A04,
		{ Freg + 2,Freg + 0,NONE,"FUCOM" }
		},
		{ 0x1B04,
		{ Freg + 3,Freg + 0,NONE,"FUCOM" }
		},
		{ 0x1C04,
		{ Freg + 4,Freg + 0,NONE,"FUCOM" }
		},
		{ 0x1D04,
		{ Freg + 5,Freg + 0,NONE,"FUCOM" }
		},
		{ 0x1E04,
		{ Freg + 6,Freg + 0,NONE,"FUCOM" }
		},
		{ 0x1F04,
		{ Freg + 7,Freg + 0,NONE,"FUCOM" }
		},

		{ 0x1805,
		{ Freg + 0,NONE,NONE,"FUCOMP" }
		},
		{ 0x1905,
		{ Freg + 1,NONE,NONE,"FUCOMP" }
		},
		{ 0x1A05,
		{ Freg + 2,NONE,NONE,"FUCOMP" }
		},
		{ 0x1B05,
		{ Freg + 3,NONE,NONE,"FUCOMP" }
		},
		{ 0x1C05,
		{ Freg + 4,NONE,NONE,"FUCOMP" }
		},
		{ 0x1D05,
		{ Freg + 5,NONE,NONE,"FUCOMP" }
		},
		{ 0x1E05,
		{ Freg + 6,NONE,NONE,"FUCOMP" }
		},
		{ 0x1F05,
		{ Freg + 7,NONE,NONE,"FUCOMP" }
		},

		{ 0x1806,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1906,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1A06,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1B06,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1C06,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1D06,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1E06,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1F06,
		{ NONE,NONE,NONE,"???" }
		},

		{ 0x1807,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1907,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1A07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1B07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1C07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1D07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1E07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1F07,
		{ NONE,NONE,NONE,"???" }
		},
	}
	},

	{ 0xDE,
	{
		{ 0x1800,
		{ Freg + 0,Freg + 0,NONE,"FADDP" }
		},
		{ 0x1900,
		{ Freg + 1,Freg + 0,NONE,"FADDP" }
		},
		{ 0x1A00,
		{ Freg + 2,Freg + 0,NONE,"FUCOM" }
		},
		{ 0x1B00,
		{ Freg + 3,Freg + 0,NONE,"FADDP" }
		},
		{ 0x1C00,
		{ Freg + 4,Freg + 0,NONE,"FADDP" }
		},
		{ 0x1D00,
		{ Freg + 5,Freg + 0,NONE,"FADDP" }
		},
		{ 0x1E00,
		{ Freg + 6,Freg + 0,NONE,"FADDP" }
		},
		{ 0x1F00,
		{ Freg + 7,Freg + 0,NONE,"FADDP" }
		},

		{ 0x1801,
		{ Freg + 0,Freg + 0,NONE,"FMULP" }
		},
		{ 0x1901,
		{ Freg + 1,Freg + 0,NONE,"FMULP" }
		},
		{ 0x1A01,
		{ Freg + 2,Freg + 0,NONE,"FMULP" }
		},
		{ 0x1B01,
		{ Freg + 3,Freg + 0,NONE,"FMULP" }
		},
		{ 0x1C01,
		{ Freg + 4,Freg + 0,NONE,"FMULP" }
		},
		{ 0x1D01,
		{ Freg + 5,Freg + 0,NONE,"FMULP" }
		},
		{ 0x1E01,
		{ Freg + 6,Freg + 0,NONE,"FMULP" }
		},
		{ 0x1F01,
		{ Freg + 7,Freg + 0,NONE,"FMULP" }
		},

		{ 0x1802,
		{ Freg + 0,Freg + 0,NONE,"FCOMP" }
		},
		{ 0x1902,
		{ Freg + 0,Freg + 1,NONE,"FCOMP" }
		},
		{ 0x1A02,
		{ Freg + 0,Freg + 2,NONE,"FCOMP" }
		},
		{ 0x1B02,
		{ Freg + 0,Freg + 3,NONE,"FCOMP" }
		},
		{ 0x1C02,
		{ Freg + 0,Freg + 4,NONE,"FCOMP" }
		},
		{ 0x1D02,
		{ Freg + 0,Freg + 5,NONE,"FCOMP" }
		},
		{ 0x1E02,
		{ Freg + 0,Freg + 6,NONE,"FCOMP" }
		},
		{ 0x1F02,
		{ Freg + 0,Freg + 7,NONE,"FCOMP" }
		},


		{ 0x1803,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1903,
		{ NONE,NONE,NONE,"FCOMPP" }
		},
		{ 0x1A03,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1B03,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1C03,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1D03,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1E03,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1F03,
		{ NONE,NONE,NONE,"???" }
		},

		{ 0x1804,
		{ Freg + 0,Freg + 0,NONE,"FSUBRP" }
		},
		{ 0x1904,
		{ Freg + 1,Freg + 0,NONE,"FSUBRP" }
		},
		{ 0x1A04,
		{ Freg + 2,Freg + 0,NONE,"FSUBRP" }
		},
		{ 0x1B04,
		{ Freg + 3,Freg + 0,NONE,"FSUBRP" }
		},
		{ 0x1C04,
		{ Freg + 4,Freg + 0,NONE,"FSUBRP" }
		},
		{ 0x1D04,
		{ Freg + 5,Freg + 0,NONE,"FSUBRP" }
		},
		{ 0x1E04,
		{ Freg + 6,Freg + 0,NONE,"FSUBRP" }
		},
		{ 0x1F04,
		{ Freg + 7,Freg + 0,NONE,"FSUBRP" }
		},

		{ 0x1805,
		{ Freg + 0,Freg + 0,NONE,"FSUBR" }
		},
		{ 0x1905,
		{ Freg + 1,Freg + 0,NONE,"FSUBR" }
		},
		{ 0x1A05,
		{ Freg + 2,Freg + 0,NONE,"FSUBR" }
		},
		{ 0x1B05,
		{ Freg + 3,Freg + 0,NONE,"FSUBR" }
		},
		{ 0x1C05,
		{ Freg + 4,Freg + 0,NONE,"FSUBR" }
		},
		{ 0x1D05,
		{ Freg + 5,Freg + 0,NONE,"FSUBR" }
		},
		{ 0x1E05,
		{ Freg + 6,Freg + 0,NONE,"FSUBR" }
		},
		{ 0x1F05,
		{ Freg + 7,Freg + 0,NONE,"FSUBR" }
		},

		{ 0x1806,
		{ Freg + 0,Freg + 0,NONE,"FDIVRP" }
		},
		{ 0x1906,
		{ Freg + 1,Freg + 0,NONE,"FDIVRP" }
		},
		{ 0x1A06,
		{ Freg + 2,Freg + 0,NONE,"FDIVRP" }
		},
		{ 0x1B06,
		{ Freg + 3,Freg + 0,NONE,"FDIVRP" }
		},
		{ 0x1C06,
		{ Freg + 4,Freg + 0,NONE,"FDIVRP" }
		},
		{ 0x1D06,
		{ Freg + 5,Freg + 0,NONE,"FDIVRP" }
		},
		{ 0x1E06,
		{ Freg + 6,Freg + 0,NONE,"FDIVRP" }
		},
		{ 0x1F06,
		{ Freg + 7,Freg + 0,NONE,"FDIVRP" }
		},

		{ 0x1807,
		{ Freg + 0,Freg + 0,NONE,"FDIVP" }
		},
		{ 0x1907,
		{ Freg + 1,Freg + 0,NONE,"FDIVP" }
		},
		{ 0x1A07,
		{ Freg + 2,Freg + 0,NONE,"FDIVP" }
		},
		{ 0x1B07,
		{ Freg + 3,Freg + 0,NONE,"FDIVP" }
		},
		{ 0x1C07,
		{ Freg + 4,Freg + 0,NONE,"FDIVP" }
		},
		{ 0x1D07,
		{ Freg + 5,Freg + 0,NONE,"FDIVP" }
		},
		{ 0x1E07,
		{ Freg + 6,Freg + 0,NONE,"FDIVP" }
		},
		{ 0x1F07,
		{ Freg + 7,Freg + 0,NONE,"FDIVP" }
		},
	}
	},

	{ 0xDF,
	{
		{ 0x1800,
		{ Freg + 0,NONE,NONE,"FFREEP" }
		},
		{ 0x1900,
		{ Freg + 1,NONE,NONE,"FFREEP" }
		},
		{ 0x1A00,
		{ Freg + 2,NONE,NONE,"FFREEP" }
		},
		{ 0x1B00,
		{ Freg + 3,NONE,NONE,"FFREEP" }
		},
		{ 0x1C00,
		{ Freg + 4,NONE,NONE,"FFREEP" }
		},
		{ 0x1D00,
		{ Freg + 5,NONE,NONE,"FFREEP" }
		},
		{ 0x1E00,
		{ Freg + 6,NONE,NONE,"FFREEP" }
		},
		{ 0x1F00,
		{ Freg + 7,NONE,NONE,"FFREEP" }
		},

		{ 0x1801,
		{ Freg + 0,Freg + 0,NONE,"FXCH" }
		},
		{ 0x1901,
		{ Freg + 0,Freg + 1,NONE,"FXCH" }
		},
		{ 0x1A01,
		{ Freg + 0,Freg + 2,NONE,"FXCH" }
		},
		{ 0x1B01,
		{ Freg + 0,Freg + 3,NONE,"FXCH" }
		},
		{ 0x1C01,
		{ Freg + 0,Freg + 4,NONE,"FXCH" }
		},
		{ 0x1D01,
		{ Freg + 0,Freg + 5,NONE,"FXCH" }
		},
		{ 0x1E01,
		{ Freg + 0,Freg + 6,NONE,"FXCH" }
		},
		{ 0x1F01,
		{ Freg + 0,Freg + 7,NONE,"FXCH" }
		},

		{ 0x1802,
		{ Freg + 0,Freg + 0,NONE,"FSTP" }
		},
		{ 0x1902,
		{ Freg + 1,Freg + 0,NONE,"FSTP" }
		},
		{ 0x1A02,
		{ Freg + 2,Freg + 0,NONE,"FSTP" }
		},
		{ 0x1B02,
		{ Freg + 3,Freg + 0,NONE,"FSTP" }
		},
		{ 0x1C02,
		{ Freg + 4,Freg + 0,NONE,"FSTP" }
		},
		{ 0x1D02,
		{ Freg + 5,Freg + 0,NONE,"FSTP" }
		},
		{ 0x1E02,
		{ Freg + 6,Freg + 0,NONE,"FSTP" }
		},
		{ 0x1F02,
		{ Freg + 7,Freg + 0,NONE,"FSTP" }
		},

		{ 0x1803,
		{ Freg + 0,Freg + 0,NONE,"FSTP" }
		},
		{ 0x1903,
		{ Freg + 1,Freg + 0,NONE,"FSTP" }
		},
		{ 0x1A03,
		{ Freg + 2,Freg + 0,NONE,"FSTP" }
		},
		{ 0x1B03,
		{ Freg + 3,Freg + 0,NONE,"FSTP" }
		},
		{ 0x1C03,
		{ Freg + 4,Freg + 0,NONE,"FSTP" }
		},
		{ 0x1D03,
		{ Freg + 5,Freg + 0,NONE,"FSTP" }
		},
		{ 0x1E03,
		{ Freg + 6,Freg + 0,NONE,"FSTP" }
		},
		{ 0x1F03,
		{ Freg + 7,Freg + 0,NONE,"FSTP" }
		},

		{ 0x1804,
		{ NONE,NONE,NONE,"FNSTSW AX" }
		},
		{ 0x1904,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1A04,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1B04,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1C04,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1D04,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1E04,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1F04,
		{ NONE,NONE,NONE,"???" }
		},

		{ 0x1805,
		{ Freg + 0,Freg + 0,NONE,"FUCOMIP" }
		},
		{ 0x1905,
		{ Freg + 0,Freg + 1,NONE,"FUCOMIP" }
		},
		{ 0x1A05,
		{ Freg + 0,Freg + 2,NONE,"FUCOMIP" }
		},
		{ 0x1B05,
		{ Freg + 0,Freg + 3,NONE,"FUCOMIP" }
		},
		{ 0x1C05,
		{ Freg + 0,Freg + 4,NONE,"FUCOMIP" }
		},
		{ 0x1D05,
		{ Freg + 0,Freg + 5,NONE,"FUCOMIP" }
		},
		{ 0x1E05,
		{ Freg + 0,Freg + 6,NONE,"FUCOMIP" }
		},
		{ 0x1F05,
		{ Freg + 0,Freg + 7,NONE,"FUCOMIP" }
		},

		{ 0x1806,
		{ Freg + 0,Freg + 0,NONE,"FCOMIP" }
		},
		{ 0x1906,
		{ Freg + 0,Freg + 1,NONE,"FCOMIP" }
		},
		{ 0x1A06,
		{ Freg + 0,Freg + 2,NONE,"FCOMIP" }
		},
		{ 0x1B06,
		{ Freg + 0,Freg + 3,NONE,"FCOMIP" }
		},
		{ 0x1C06,
		{ Freg + 0,Freg + 4,NONE,"FCOMIP" }
		},
		{ 0x1D06,
		{ Freg + 0,Freg + 5,NONE,"FCOMIP" }
		},
		{ 0x1E06,
		{ Freg + 0,Freg + 6,NONE,"FCOMIP" }
		},
		{ 0x1F06,
		{ Freg + 0,Freg + 7,NONE,"FCOMIP" }
		},

		{ 0x1807,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1907,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1A07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1B07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1C07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1D07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1E07,
		{ NONE,NONE,NONE,"???" }
		},
		{ 0x1F07,
		{ NONE,NONE,NONE,"???" }
		},
	}
	},
};

DWORD XqDisassemblerEngine::XqDisassembler_Start(IN BYTE *pOpcode,IN const DWORD DisLen,IN const DWORD ImageBase)
{
	DWORD StartIndex = 0;
	BYTE opcode;
	DWORD Mark;
	m_OpIndex = 0;
	m_pOpcode = pOpcode;

	while (m_OpIndex < DisLen)
	{
		Instruction instr;
		memset(&instr,0,sizeof(Instruction));
		stringstream ss;
		ss << std::hex <<(ImageBase + m_OpIndex);
		ss >> instr.Address;
		m_StartIndex = m_OpIndex;			//一条指令开始的索引，用于获取指令长度

Lab_GetPreFix:
		opcode = m_pOpcode[m_OpIndex++];				//取得opcode
		Mark = ONE_OPCODE_TABLE[opcode];	//取得在INSTR_TABLE中的Mark含义
		instr.Opcode[0] = opcode;
		if (Mark & PREFIX)
		{
			//指令前缀：
			switch (opcode)
			{
			case 0x26:
				instr.SegmentPrefix = ES;
				break;
			case 0x2E:
				instr.SegmentPrefix = CS;
				break;
			case 0x36:
				instr.SegmentPrefix = SS;
				break;
			case 0x3E:
				instr.SegmentPrefix = DS;
				break;
			case 0x64:
				instr.SegmentPrefix = FS;
				break;
			case 0x65:
				instr.SegmentPrefix = GS;
				break;
			case 0x66:
				instr.OperandPrefix = OperandSize66;
				break;
			case 0x67:
				instr.AddressPrefix = AddressSize67;
				break;
			case 0xF0:
				instr.RepeatPrefix = LOCK;
				break;
			case 0xF2:
				instr.RepeatPrefix = REPNE;
				break;
			case 0xF3:
				instr.RepeatPrefix = REPE;
				break;
			default:
				break;
			}
			goto Lab_GetPreFix;
		}

Lab_TwoOpcode:
		if (Mark & TWOBYTE)
		{
			Decode2Bytes(Mark,&instr);
		}

Lab_FPU:
		if (Mark & FPU)
		{
			BYTE mod = m_pOpcode[m_OpIndex++];
			DecodeFPU(Mark, mod, &instr);
		}

Lab_Group:
		if (Mark & GROUP)
		{
			_MODR_M mod;
			mod.B_MOD = m_pOpcode[m_OpIndex];	//不加m_OpIndex，后面Modrm解析再+
			DecodeGroup(Mark,opcode, mod,&instr);
		}

Lab_Imm16:
		if (Mark & IMM16)				//不管有没有66前缀，IMM16都是16位，处理了IMM66和IMM8一起的情况
		{
			DecodeImm16(Mark,&instr);
		}

Lab_ModRM:
		if (Mark & MODRM)				//处理了IMM66和IMM8一起的情况
		{
			BYTE mod = m_pOpcode[m_OpIndex++];
			DecodeModRM(Mark,mod, &instr);
		}

Lab_Imm8:
	if (!(Mark & MODRM) && !(Mark & IMM16))	//处理IMM8，排除MODRM和IMM16中处理过的
		{
			if (Mark & IMM8)
			{
				DecodeImm8(Mark, &instr);
			}
		}

Lab_Imm66:
		if (!(Mark & MODRM) && !(Mark & IMM16))	//处理IMM66，排除MODRM和IMM16中处理过的
		{
			if (Mark & IMM66)
			{
				DecodeImm66(Mark, &instr);
			}
		}

Lab_OneByte:
		if (Mark & ONEBYTE)
		{
			DecodeOneByte(Mark,&instr);
		}

Lab_Addr67:
		if (Mark & ADDR67)
		{
			DecodeAddr67(Mark,&instr);
		}

Lab_Rep:
		if (Mark & StringInstruction)		//REP前缀
		{
			if(REPE == instr.RepeatPrefix)
				instr.instruct = "REPE " + instr.instruct;
			else
				instr.instruct = "REPNE " + instr.instruct;
		}

		instr.OpcodeStr = "";
		for(;StartIndex < m_OpIndex;StartIndex++)
			instr.OpcodeStr += ToHexstring(m_pOpcode[StartIndex],2);
		ComposeInstr(&instr);
		m_VecInstr.push_back(instr);
		StartIndex = m_OpIndex;
	}
	return 0;
}

VOID XqDisassemblerEngine::ShowDissembler_Str()
{
	cout << "************************************ XqDisassemblerEngine **********************************\n\n";
	cout << "Address\t" << std::left << std::setw(0x20) << "Opcode";
	cout << "Instrution\n" << endl;
	for (auto &instr : m_VecInstr)
	{
		cout << std::uppercase << std::hex <<instr.Address << "\t";
		cout << std::left << std::setw(0x20) << instr.OpcodeStr;
		cout << instr.DisassemblerStr << endl;
	}
}

VOID XqDisassemblerEngine::DecodeOneByte(IN DWORD Mark,OUT PInstruction pInstr)
{
	Analy_Opcode OpAnaly;
	if (Mark & TWOBYTE)
		OpAnaly = INSTR_TABLE[TWOBYTE][pInstr->Opcode[1]];
	else
		OpAnaly = INSTR_TABLE[ONEBYTE][pInstr->Opcode[0]];
	
	pInstr->instruct = OpAnaly.Description;
	DWORD Operand_Type;
	
	for (int i = 0; i < 2; i++)
	{
		Operand_Type = OpAnaly.Operand[i];
		if(Operand_Type & REG)
			DecodeRegX(pInstr,i, Operand_Type);
	}
	//特殊处理部分，如PUSHAD PUSHA
	if (pInstr->OperandPrefix)
	{
		switch (pInstr->Opcode[0])
		{
		case 0x60:
		case 0x61:
			pInstr->instruct += "A";
			break;

		case 0x6D:
		case 0x6E:
		case 0xA5:
		case 0xA7:
		case 0xAB:
		case 0xAD:
		case 0xAF:
			pInstr->instruct += "W";
			break;

		case 0x98:
			pInstr->instruct += "BW";
			break;
		case 0x99:
			pInstr->instruct += "WD";
			break;

		default:
			break;
		}
	}
	else
	{
		switch (pInstr->Opcode[0])
		{
		case 0x60:
		case 0x61:
			pInstr->instruct += "AD";
			break;

		case 0x6D:
		case 0x6E:
		case 0xA5:
		case 0xA7:
		case 0x9C:
		case 0x9D:
		case 0xAB:
		case 0xAD:
		case 0xAF:
		case 0xCF:
			pInstr->instruct += "D";
			break;

		case 0x98:
			pInstr->instruct += "WDE";
			break;
		case 0x99:
			pInstr->instruct += "DQ";
			break;
		default:
			break;
		}
	}
}

VOID XqDisassemblerEngine::DecodeImm8(IN DWORD Mark, OUT PInstruction pInstr)
{
	Analy_Opcode OpAnaly;
	if(Mark & TWOBYTE)
		OpAnaly = INSTR_TABLE[TWOBYTE][pInstr->Opcode[0]];
	else
		OpAnaly = INSTR_TABLE[IMM8][pInstr->Opcode[0]];
	pInstr->instruct = OpAnaly.Description;							//获取指令
	DWORD Operand_Type;
	
	for (int i = 0; i < 2; i++)
	{
		Operand_Type = OpAnaly.Operand[i];
		if (Operand_Type & REG)
		{
			DecodeRegX(pInstr,i,Operand_Type);
		}
		else if(Operand_Type & IMM8)
		{
			DWORD imm8 = m_pOpcode[m_OpIndex++];
			if (Operand_Type & Jxx)
			{
				if (imm8 >= 0x80)
					imm8 -= 0x100;
				imm8 += pInstr->Address;
				imm8 += (m_OpIndex - m_StartIndex);				//jcc short2字节
				pInstr->operand[i] = "0x" + ToHexstring(imm8, 8);		//获取操作数
			}
			else
				pInstr->operand[i] = "0x" + ToHexstring(imm8, 2);		//获取操作数
		}
	}
}

VOID XqDisassemblerEngine::DecodeImm16(IN DWORD Mark, OUT PInstruction pInstr)
{
	Analy_Opcode OpAnaly = INSTR_TABLE[IMM16][pInstr->Opcode[0]];
	string DisStr;
	BYTE imm8;
	WORD imm16;
	DWORD imm66;
	imm16 = *(PWORD)(m_pOpcode + m_OpIndex);
	m_OpIndex += 2;

	pInstr->instruct = OpAnaly.Description;		//获取指令
	if (Mark & IMM8)
	{
		imm8 = m_pOpcode[m_OpIndex++];
		pInstr->operand[0] = "0x" + ToHexstring(imm16,4);
		pInstr->operand[1] = "0x" + ToHexstring(imm8,2);
	}
	else if (Mark & IMM66)
	{
		m_OpIndex -= 2;
		if (pInstr->OperandPrefix)
		{
			imm66 = *(PWORD)(m_pOpcode + m_OpIndex);
			m_OpIndex += 2;
		}
		else
		{
			imm66 = *(PDWORD)(m_pOpcode + m_OpIndex);
			m_OpIndex += 4;
		}
		imm16 = *(PWORD)(m_pOpcode + m_OpIndex);
		m_OpIndex += 2;
		pInstr->operand[0] = "0x" + ToHexstring(imm16,4) + ":" + "0x" + ToHexstring(imm66, 8);
	}
	else
	{
		pInstr->operand[0] = "0x" + ToHexstring(imm16,4);
	}
}

VOID XqDisassemblerEngine::DecodeImm66(IN DWORD Mark, OUT PInstruction pInstr)
{
	Analy_Opcode OpAnaly;
	if (Mark & TWOBYTE)
		OpAnaly = INSTR_TABLE[TWOBYTE][pInstr->Opcode[1]];
	else
		OpAnaly	= INSTR_TABLE[IMM66][pInstr->Opcode[0]];
	pInstr->instruct = OpAnaly.Description;		//获取指令
	DWORD imm66;

	for (int i = 0; i < 3; i++)
	{
		DWORD op = OpAnaly.Operand[i];
		if (op & REG)
		{
			DecodeRegX(pInstr,i,op);
		}
		if (op & IMMX)
		{
			if (pInstr->OperandPrefix)
			{
				imm66 = *(PWORD)(m_pOpcode + m_OpIndex);
				m_OpIndex += 2;
				if (op & Jxx)
				{
					imm66 += pInstr->Address;
					imm66 += (m_OpIndex - m_StartIndex);		//+2byte = 3字节
					pInstr->operand[i] = "0x" + ToHexstring(imm66, 8);		//获取操作数
				}
				else
					pInstr->operand[i] = "0x" + ToHexstring(imm66,4);
			}
			else
			{
				imm66 = *(PDWORD)(m_pOpcode + m_OpIndex);
				m_OpIndex += 4;
				if (op & Jxx)
				{
					imm66 += pInstr->Address;
					imm66 += (m_OpIndex - m_StartIndex);	//5字节jcc
					pInstr->operand[i] = "0x" + ToHexstring(imm66, 8);		//获取操作数
				}
				else
					pInstr->operand[i] = "0x" + ToHexstring(imm66,8);
			}
		}
	}
}

VOID XqDisassemblerEngine::DecodeFPU(IN DWORD Mark, IN BYTE mod, OUT PInstruction pInstr)
{
	WORD GE = ModRM_TABLE[mod];
	BYTE opcode1 = pInstr->Opcode[0];
	_MODR_M m;
	m.B_MOD = mod;

	Analy_Opcode OpAnaly;

	if(mod < 0xC0)
		OpAnaly = FPU_INSTR_TABLE00_BF[opcode1][m.Reg];
	else
		OpAnaly = FPU_INSTR_TABLEC0_FF[opcode1][GE];
	pInstr->instruct = OpAnaly.Description;


	Analy_Modrm ModAanlyE = E_TABLE_FPU32[MODRM_E(GE)];
	
	if (pInstr->AddressPrefix)								//有0x66前缀时候应该将32位E表换为16位的E表
		ModAanlyE = E_TABLE_FPU16[MODRM_E(GE)];


	for (int i = 0; i < 2; i++)
	{
		auto op = OpAnaly.Operand[i];
		BYTE size = op & 0x7;		//取得Size，111b = 7，对应AL,AX,EAX,MMM0这种索引
		if (op & NONE)
			continue;
		if (op & E)
		{
			if (ModAanlyE.type & SIM)	//处理SIM情况
			{
				DecodeSIB(mod, i, pInstr);
			}
			DecodeDISPx(m, ModAanlyE, i, pInstr, size);
			pInstr->operand[i] = pInstr->operand[i].substr(pInstr->operand[i].find_first_of('['));
			string szPrefix = GetPrefixString(pInstr);
			string szAddressmethod;
			switch (size)
			{
			case S_W:
				szAddressmethod = "WORD PTR ";
				break;
			case S_D:
				szAddressmethod = "DWORD PTR ";
				break;
			case S_Q:
				szAddressmethod = "QWORD PTR ";
				break;
			case S_T:
				szAddressmethod = "TWORD PTR ";
				break;
			case S_E:
				if(pInstr->OperandPrefix)
					szAddressmethod = "M14 PTR ";
				else
					szAddressmethod = "M28 PTR ";
				break;
			default:
				break;
			}
			pInstr->operand[i] = szAddressmethod + szPrefix + pInstr->operand[i];
		}
		if (op & REG)
		{
			char szTemp[] = "ST(%d)";
			snprintf(szTemp,sizeof(szTemp), szTemp, op - Freg);
			pInstr->operand[i] = szTemp;
		}
	}
}

VOID XqDisassemblerEngine::DecodeModRM(IN DWORD Mark, IN BYTE mod, OUT PInstruction pInstr)
{
	//map<Type, map<CodeIndex, Analy_Opcode>>
	WORD GE = ModRM_TABLE[mod];
	BYTE opcode1 = pInstr->Opcode[0];
	_MODR_M m;
	m.B_MOD =  mod ;
	Analy_Opcode OpAnaly;
	
	if (Mark & TWOBYTE)
	{
		opcode1 = pInstr->Opcode[1];
		OpAnaly = INSTR_TABLE[TWOBYTE][opcode1];
		pInstr->instruct = OpAnaly.Description;
	}
	else if (pInstr->GroupOpcode.Description == "")
	{
		OpAnaly = INSTR_TABLE[MODRM][opcode1];
		pInstr->instruct = OpAnaly.Description;
	}
	else
	{
		OpAnaly = pInstr->GroupOpcode;						//当Group中特殊处理了指令时候
		if(OpAnaly.Description != "Group")
			pInstr->instruct = OpAnaly.Description;
	}

	Analy_Modrm ModAanlyE = E_TABLE[MODRM_E(GE)];
	Analy_Modrm ModAnalyG = G_TABLE[MODRM_G(GE)];
	if (pInstr->AddressPrefix)								//有0x67前缀时候应该将32位E表换为16位的E表
		ModAanlyE = E_TABLE16[MODRM_E(GE)];

	if (pInstr->OperandPrefix == 0x66)
	{
		for (auto &op : OpAnaly.Operand)
		{
			op &= 0xFFFFFFF0;				//清空低位保存的Size，全部置为16位
			op += S_W;
		}
	}
	else
	{
		for (auto &op : OpAnaly.Operand)
		{
			if ((op & 0x7) == S_V)
			{
				op &= 0xFFFFFFF0;				//清空低位保存的Size，全部置为16位
				op += S_D;
			}
		}
	} 

Lab_DecodeOperand:
	for (int i = 0;i < 3;i++)
	{
		auto op = OpAnaly.Operand[i];
		BYTE size = op & 0x7;		//取得Size，111b = 7，对应AL,AX,EAX,MMM0这种索引
		if (op & NONE)
			continue;
		if (op & E)
		{		
			if (ModAanlyE.type & SIM)	//处理SIM情况
			{
				DecodeSIB(mod,i,pInstr);
			}
			DecodeDISPx(m,ModAanlyE,i,pInstr,size);
		}
		else if (op & G)
		{
			pInstr->operand[i] = ModAnalyG.pInfo[size];
		}
		else if (op & MXX)
		{
			if (mod >= 0xC0)						//M表的0xC0开始就不存在了，其他的就是E表部分
				pInstr->operand[i] = "???";
			else
			{
				if (ModAanlyE.type & SIM)	//处理SIM情况
				{
					DecodeSIB(mod, i, pInstr);
				}
				DecodeDISPx(m, ModAanlyE, i, pInstr, size);
			}
		}
		else if (op & S)
		{
			pInstr->operand[i] = SREG_TABLE[m.Reg];
		}
		else if (op & IMM8)
		{
			BYTE imm8 = m_pOpcode[m_OpIndex++];
			pInstr->operand[i] = "0x" + ToHexstring(imm8,2);
		}
		else if (op & IMM66)
		{
			DWORD imm66;
			if (pInstr->OperandPrefix)
			{
				imm66 = *(PWORD)(m_pOpcode + m_OpIndex);
				m_OpIndex += 2;
				pInstr->operand[i] = "0x" + ToHexstring(imm66,4);
			}
			else
			{
				imm66 = *(PDWORD)(m_pOpcode + m_OpIndex);
				m_OpIndex += 4;
				pInstr->operand[i] = "0x" + ToHexstring(imm66,8);
			}
		}
		else if (op & REG)
		{
			DecodeRegX(pInstr,i,op);
		}
	}
	
}

VOID XqDisassemblerEngine::DecodeSIB(IN BYTE mod, IN DWORD i, OUT PInstruction pInstr)
{
	BYTE bSib = m_pOpcode[m_OpIndex++];
	WORD SIB_AB = SIB_TABLE[bSib];
	Analy_SIB SIB_A = SIB_REGA[REG_A(SIB_AB)];
	Analy_SIB SIB_B = SIB_REGB[REG_B(SIB_AB)];
	_MODR_M m;
	m.B_MOD = mod;

	//寻址大小
	if (pInstr->OperandPrefix)
		pInstr->operand[i] = "WORD PTR ";
	else
		pInstr->operand[i] = "DWORD PTR ";

	//寻址方式DS,ES,FS
	if (SIB_A.type == DISP32)	//SIB的REG是EBP的情况
		pInstr->operand[i] += "SS:";
	else
		pInstr->operand[i] += GetPrefixString(pInstr);

	//RegA部分
	pInstr->operand[i] += "[";
	pInstr->operand[i] += SIB_A.pInfo;
	
	//RegB部分
	if (SIB_B.type != T_NONE)
	{
		pInstr->operand[i] += "+";
		pInstr->operand[i] += SIB_B.pInfo;
	}

	//Dipsx
	if (((SIB_A.type == DISP32) && (m.Mod == 0 || m.Mod == 2)) //SIB的[*]寻址32
		|| ((m.R_M == 4) && m.Mod == 2))						//ModRM为SIM+DISP32
	{
		DWORD Disp32 = *(PDWORD)(m_pOpcode + m_OpIndex);
		m_OpIndex += 4;
		pInstr->operand[i] += "+" + ToHexstring(Disp32, 8);
	}
	else if (((SIB_A.type == DISP32) && (m.Mod == 1))	//SIB的[*]寻址8
		|| ((m.R_M == 4) && m.Mod == 1))				//ModRM为SIM+DISP8
	{
		DWORD Dis8 = m_pOpcode[m_OpIndex++];
		pInstr->operand[i] += "+" + ToHexstring(Dis8, 2);
	}

	pInstr->operand[i] += "]";
}

VOID XqDisassemblerEngine::DecodeDISPx(IN _MODR_M m, IN Analy_Modrm ModAanlyE, IN int i, OUT PInstruction pInstr, BYTE size)
{
	ADDR_MODE AddrMode = ADDR_NONE;
	string szPrefix;

	if (ModAanlyE.type & DISP32)
	{
		AddrMode = ADDR_DWORD;
		pInstr->operand[i] = ModAanlyE.pInfo[size];
		DWORD Disp32 = *(PDWORD)(m_pOpcode + m_OpIndex);
		m_OpIndex += 4;
		Format(pInstr->operand[i], Disp32,8);
	}
	else if (ModAanlyE.type & DISP16)
	{
		AddrMode = ADDR_WORD;
		pInstr->operand[i] = ModAanlyE.pInfo[size];
		DWORD Disp16 = *(PWORD)(m_pOpcode + m_OpIndex);	
		m_OpIndex += 2;
		Format(pInstr->operand[i], Disp16, 4);
	}
	else if (ModAanlyE.type & DISP8)
	{
		AddrMode = ADDR_BYTE;
		pInstr->operand[i] = ModAanlyE.pInfo[size];
		DWORD Disp8 = m_pOpcode[m_OpIndex++];
		Format(pInstr->operand[i], Disp8,2);
	}
	else if (ModAanlyE.type & DIRECT)
	{
		if (m.Mod != 3)
		{
			if (size == S_B)
				AddrMode = ADDR_BYTE;
			else if (pInstr->OperandPrefix)
				AddrMode = ADDR_WORD;
			else
				AddrMode = ADDR_DWORD;
		}
		pInstr->operand[i] = ModAanlyE.pInfo[size];
	}

	szPrefix = GetPrefixString(pInstr);

	switch (AddrMode)
	{
	case ADDR_BYTE:
		pInstr->operand[i] = "BYTE PTR "+ szPrefix + pInstr->operand[i];
		break;
	case ADDR_WORD:
		pInstr->operand[i] = "WORD PTR "+ szPrefix + pInstr->operand[i];
		break;
	case ADDR_DWORD:
		pInstr->operand[i] = "DWORD PTR "+ szPrefix + pInstr->operand[i];
		break;
	default:
		break;
	}
}

VOID XqDisassemblerEngine::DecodeGroup(IN DWORD &Mark,IN DWORD opcode,IN _MODR_M m, OUT PInstruction pInstr)
{
	Mark |= MODRM;	//Group一定要经过ModRM
	pInstr->GroupOpcode = INSTR_TABLE[GROUP][opcode];
	pInstr->instruct = pInstr->GroupOpcode.Description;
	BYTE Reg = m.Reg;
	switch (opcode)
	{
	case 0x80:
	case 0x81:
	case 0x82:
	case 0x83:
		if (Reg == 0)
			pInstr->instruct = "ADD ";
		else if (Reg == 1)
			pInstr->instruct = "OR ";
		else if (Reg == 2)
			pInstr->instruct = "ADC ";
		else if (Reg == 3)
			pInstr->instruct = "SBB ";
		else if (Reg == 4)
			pInstr->instruct = "AND ";
		else if (Reg == 5)
			pInstr->instruct = "SUB ";
		else if (Reg == 6)
			pInstr->instruct = "XOR ";
		else if (Reg == 7)
			pInstr->instruct = "CMP ";
		break;
	
	case 0x8F:
		if(Reg == 0)
			pInstr->instruct = "POP ";
		break;
	
	case 0xC0:
	case 0xC1:
	case 0xD0:
		if (opcode == 0xD0)
			pInstr->operand[1] = "1";
	case 0xD1:
		if (opcode == 0xD1)
			pInstr->operand[1] = "1";
	case 0xD2:
	case 0xD3:
		if (Reg == 0)
			pInstr->instruct = "ROL ";
		else if (Reg == 1)
			pInstr->instruct = "ROR ";
		else if (Reg == 2)
			pInstr->instruct = "RCL ";
		else if (Reg == 3)
			pInstr->instruct = "RCR ";
		else if (Reg == 4)
			pInstr->instruct = "SHL ";	//SHL和SAL一样
		else if (Reg == 5)
			pInstr->instruct = "SHR ";
		else if (Reg == 7)
			pInstr->instruct = "SAR ";
		break;

	case 0xF6:
	case 0xF7:
		if (Reg == 0)
		{
			pInstr->GroupOpcode.Description = "TEST ";
			if (opcode == 0xF6)
			{
				pInstr->GroupOpcode.Operand[0] = Eb;
				pInstr->GroupOpcode.Operand[1] = Ib;
			}
			else
			{
				pInstr->GroupOpcode.Operand[0] = Ev;
				pInstr->GroupOpcode.Operand[1] = Iz;
			}
		}
		else if (Reg == 2)
			pInstr->instruct = "NOT ";
		else if (Reg == 3)
			pInstr->instruct = "NEG ";
		else if (Reg == 4)
			pInstr->instruct = "MUL ";
		else if (Reg == 5)
			pInstr->instruct = "IMUL ";
		else if (Reg == 6)
			pInstr->instruct = "DIV ";
		else if (Reg == 7)
			pInstr->instruct = "IDIV ";
		break;

	case 0xFE:
		if (Reg == 0)
			pInstr->instruct = "INC ";
		else if (Reg == 1)
			pInstr->instruct = "DEC ";
		break;

	case 0xFF:
		if (Reg == 0)
			pInstr->instruct = "INC ";
		else if (Reg == 1)
			pInstr->instruct = "DEC ";
		else if (Reg == 2)
			pInstr->instruct = "CALL ";
		else if (Reg == 3)
			pInstr->instruct = "CALL FAR ";
		else if (Reg == 4)
			pInstr->instruct = "JMP ";
		else if (Reg == 5)
			pInstr->instruct = "JMP FAR ";
		else if (Reg == 6)
			pInstr->instruct = "PUSH ";
		break;

	case 0xC6:
		if (Reg == 0)
			pInstr->instruct = "MOV ";
		else if (Reg == 7 && m.Mod == 3)
		{
			pInstr->instruct = "XABORT";
			pInstr->GroupOpcode.Operand[0] = Ib;
			pInstr->GroupOpcode.Operand[1] = NONE;
		}
		break;

	case 0xC7:
		if (Reg == 0)
			pInstr->instruct = "MOV ";
		else if (Reg == 7 && m.Mod == 3)
		{
			pInstr->instruct = "XBEGIN";
			pInstr->GroupOpcode.Operand[0] = Jz;
			pInstr->GroupOpcode.Operand[1] = NONE;
		}
	default:
		break;
	}
}

VOID XqDisassemblerEngine::DecodeAddr67(IN DWORD Mark,OUT PInstruction pInstr)
{
	Analy_Opcode OpAnaly;
	if (Mark & TWOBYTE)
		OpAnaly = INSTR_TABLE[TWOBYTE][pInstr->Opcode[1]];
	else
		OpAnaly = INSTR_TABLE[ADDR67][pInstr->Opcode[0]];
	pInstr->instruct = OpAnaly.Description;

	for (int i = 0; i < 2; i++)
	{
		DWORD OperandDes = OpAnaly.Operand[i];
		if (OperandDes & REG)
		{
			DecodeRegX(pInstr,i,OperandDes);
		}
		else if (OperandDes & OX)
		{
			BYTE size = OperandDes & 0x7;			//这里只是影响前缀是byte还是word还是dword
			if (size == S_B)
			{
				DWORD Disp32;
				if (pInstr->AddressPrefix) 
				{
					Disp32 = *(PWORD)(m_pOpcode + m_OpIndex);
					m_OpIndex += 2;
				}
				else
				{
					Disp32 = *(PDWORD)(m_pOpcode + m_OpIndex);
					m_OpIndex += 4;
				}
				pInstr->operand[i] = "BYTE PTR "+ GetPrefixString(pInstr) + "[" + ToHexstring(Disp32, 8) + "]";
			}
			else
			{
				if (pInstr->AddressPrefix)
				{
					DWORD Disp16 = *(PWORD)(m_pOpcode + m_OpIndex);
					m_OpIndex += 2;
					if(pInstr->OperandPrefix)
						pInstr->operand[i] = "WORD PTR "+ GetPrefixString(pInstr) + "[" + ToHexstring(Disp16, 4) + "]";
					else 
						pInstr->operand[i] = "DWORD PTR "+ GetPrefixString(pInstr) +"[" + ToHexstring(Disp16, 4) + "]";
				}
				else
				{
					DWORD Disp32 = *(PDWORD)(m_pOpcode + m_OpIndex);
					m_OpIndex += 4;
					if (pInstr->OperandPrefix)
						pInstr->operand[i] = "WORD PTR "+ GetPrefixString(pInstr) +"[" + ToHexstring(Disp32, 8) + "]";
					else
						pInstr->operand[i] = "DWORD PTR "+ GetPrefixString(pInstr) +"[" + ToHexstring(Disp32, 8) + "]";
				}
			}	
		}
	}

}

VOID XqDisassemblerEngine::DecodeRegX(PInstruction pInstr,int i,DWORD op)
{
	if (op & EXX)			//处理如push eax这种
	{
		if (pInstr->OperandPrefix)
			pInstr->operand[i] += REG16_TABLE[op - EXX];
		else
			pInstr->operand[i] += REG32_TABLE[op - EXX];
	}
	else if (op & XX)
		pInstr->operand[i] += REG16_TABLE[op - XX];
	else
		pInstr->operand[i] += REG8_TABLE[op - XL];
}

VOID XqDisassemblerEngine::Decode2Bytes(IN DWORD &Mark, OUT PInstruction pInstr)
{
	pInstr->Opcode[1] = m_pOpcode[m_OpIndex++];
	Mark |= TWO_OPCODE_TABLE[pInstr->Opcode[1]];
}

string XqDisassemblerEngine::GetPrefixString(PInstruction pInstr)
{
	string szPrefix = "DS:";
	if (pInstr->SegmentPrefix)
	{
		switch (pInstr->SegmentPrefix)
		{
		case ES:
			szPrefix = "ES:";
			break;
		case CS:
			szPrefix = "CS:";
			break;
		case SS:
			szPrefix = "SS:";
			break;
		case DS:
			szPrefix = "DS:";
			break;
		case FS:
			szPrefix = "FS:";
			break;
		case GS:
			szPrefix = "GS:";
			break;
		default:
			break;
		}
	}
	return szPrefix;
}

VOID XqDisassemblerEngine::ComposeInstr(OUT PInstruction pInstr)
{
	pInstr->DisassemblerStr = pInstr->instruct + " " + pInstr->operand[0];
	if (!pInstr->operand[1].empty())
	{
		pInstr->DisassemblerStr = pInstr->DisassemblerStr + "," + pInstr->operand[1];
	}
	if (!pInstr->operand[2].empty())
	{
		pInstr->DisassemblerStr = pInstr->DisassemblerStr + "," + pInstr->operand[2];
	}
}

void Format(string &str,DWORD Num,DWORD _size)
{
	char szTemp[0x40];
	DWORD max = pow(2, _size * 4 - 1);
	if (Num >= max)
	{
		Num = ~Num+1;
		__asm
		{
			mov eax, Num;
			cmp _size, 2;
			je _2;
			cmp _size, 4;
			je _4;
			jmp _end;
		_2:
			movzx eax, al;
			jmp _end;
		_4:
			movzx eax, ax;
		_end:
			mov Num, eax;
			//sub 
		}
		snprintf(szTemp, 0x40, const_cast<char*>(str.c_str()), Num);
		str = szTemp;
		/*str = str.substr(8-size,8);*/
		string::size_type pos = 0;
		if ((pos = str.find("+")) != string::npos)
			str.replace(pos,1,"-");
	}
	else
	{
		snprintf(szTemp, 0x40, const_cast<char*>(str.c_str()), Num);
		str = szTemp;
	}
}

string ToHexstring(DWORD num,int width)
{
	std::stringstream ioss; //定义字符串流
	std::string s_temp; //存放转化后字符
	ioss << std::hex << num; //以十六制形式输出
	ioss >> s_temp;
	std::string s(width - s_temp.size(), '0'); //补0
	s += s_temp; //合并
	transform(s.begin(),s.end(),s.begin(),::toupper);
	return s;
	
}