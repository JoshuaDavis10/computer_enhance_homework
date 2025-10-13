#define ASSERT(x) \
if(!x) \
{ \
	printf("assertion: EXPR = '%s', FILE = %s, LINE = %d\n", #x, __FILE__, __LINE__);\
	__builtin_trap(); \
} 

#include "sim86_shared.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

typedef struct instruction_table instruction_table;
typedef struct instruction instruction;

static u32 registers[8];

void print_reg(u32 reg)
{
	switch(reg)
	{
		case 1:
		{
			printf("ax");
		} break;
		case 2:
		{
			printf("bx");
		} break;
		case 3:
		{
			printf("cx");
		} break;
		case 4:
		{
			printf("dx");
		} break;
		case 5:
		{
			printf("sp");
		} break;
		case 6:
		{
			printf("bp");
		} break;
		case 7:
		{
			printf("si");
		} break;
		case 8:
		{
			printf("di");
		} break;
		default:
		{
			ASSERT(0);
		} break;
	}
}

void execute(instruction execute)
{
	printf("mov ");
	u32 op_one_type = execute.Operands[0].Type;
	u32 op_two_type = execute.Operands[1].Type;
	u32 op_type = execute.Op;
	if(op_type != Op_mov)
	{
        printf("\e[1;33m[WARNING] (execute): op is not a mov\e[0;37m\n");
		return;
	}
	if(op_one_type != Operand_Register)
	{
        printf("\e[1;33m[WARNING] (execute): first operand is not a register\e[0;37m\n");
		return;
	}
	if(op_two_type == Operand_Register)
	{
		u32 reg_one = execute.Operands[0].Register.Index;
		u32 reg_two = execute.Operands[1].Register.Index;

		u32 temp = registers[reg_one - 1];
		registers[reg_one - 1] = registers[reg_two - 1];

		print_reg(reg_one);
		printf(", ");
		print_reg(reg_two);
		printf(" ; ");
		print_reg(reg_one);
		printf(":0x%x->0x%x", temp, registers[reg_one - 1]);
	}
	else if(op_two_type == Operand_Immediate)
	{

		u32 reg = execute.Operands[0].Register.Index;
		s32 imm = execute.Operands[1].Immediate.Value;

		u32 temp = registers[reg - 1] = registers[reg - 1];
		registers[reg - 1] = imm;

		print_reg(reg);
		printf(", %d ; ", imm);
		print_reg(reg);
		printf(":0x%x->0x%x", temp, registers[reg - 1]);
	}
	else
	{
		ASSERT(0);
	}
	printf("\n");
}

int main(void)
{

	printf("\n\n");

	/* setup sim86 lib */
    u32 version = Sim86_GetVersion();
    if(version != SIM86_VERSION)
    {
        printf("\e[1;31m[ERROR]: Header file version doesn't match DLL.\e[0;37m\n");
        return -1;
    }
    
    instruction_table table;
    Sim86_Get8086InstructionTable(&table);
    
	/* get disassembly */
	struct stat disassembly_stat;
	ASSERT(stat("listing_0044_register_movs", &disassembly_stat) == 0);

	unsigned long long disassembly_size = disassembly_stat.st_size;
	int disassembly_fd = open("listing_0044_register_movs", O_RDONLY);
	unsigned char *disassembly = malloc(disassembly_size);
	read(disassembly_fd, disassembly, disassembly_size);

	/* decode and execute loop */
    u32 offset = 0;
    while(offset < disassembly_size)
    {
        instruction decoded;
        Sim86_Decode8086Instruction(disassembly_size - offset, 
			disassembly + offset, &decoded);
        if(decoded.Op)
        {
            offset += decoded.Size;
			execute(decoded);
        }
        else
        {
            printf("\e[1;33m[WARNING]: Unrecognized instruction\e[0;37m\n");
            break;
        }
    }

	printf("\nFinal registers:\n");
	printf("    ax: 0x%.4x (%u)\n", registers[0], registers[0]);
	printf("    bx: 0x%.4x (%u)\n", registers[1], registers[1]);
	printf("    cx: 0x%.4x (%u)\n", registers[2], registers[2]);
	printf("    dx: 0x%.4x (%u)\n", registers[3], registers[3]);
	printf("    sp: 0x%.4x (%u)\n", registers[4], registers[4]);
	printf("    bp: 0x%.4x (%u)\n", registers[5], registers[5]);
	printf("    si: 0x%.4x (%u)\n", registers[6], registers[6]);
	printf("    di: 0x%.4x (%u)\n\n\n", registers[7], registers[7]);
    
	return (0);
}
