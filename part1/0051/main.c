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

static u8 memory[2000] = {0};
static u16 registers[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static b32 sign_flag = 0;
static b32 zero_flag = 0;
static u16 ip = 0;

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

void print_flags()
{
	if(zero_flag)
	{
		printf("Z");
	}
	if(sign_flag)
	{
		printf("S");
	}
}

void execute(instruction execute)
{
	u32 op_one_type = execute.Operands[0].Type;
	u32 op_two_type = execute.Operands[1].Type;
	u32 op_type = execute.Op;

	/* mov */
	if(op_type == Op_mov)
	{
		printf("mov ");
		if(op_one_type == Operand_Register)
		{
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
				printf(":0x%x->0x%x ", temp, registers[reg_one - 1]);
			}
			else if(op_two_type == Operand_Immediate)
			{

				u32 reg = execute.Operands[0].Register.Index;
				s32 imm = execute.Operands[1].Immediate.Value;

				u32 temp = registers[reg - 1];
				registers[reg - 1] = imm;

				print_reg(reg);
				printf(", %d ; ", imm);
				print_reg(reg);
				printf(":0x%x->0x%x ", temp, registers[reg - 1]);
			}
			else if(op_two_type == Operand_Memory)
			{

				u16 address_calc_reg_one_value;
				u16 address_calc_reg_two_value;
				u32 address_calc_reg_one_index = execute.Operands[1].Address.Terms[0].Register.Index;
				u32 address_calc_reg_two_index = execute.Operands[1].Address.Terms[1].Register.Index;
				s32 address_calc_displacement  = execute.Operands[1].Address.Displacement;

				if(address_calc_reg_one_index == 0)
				{
					address_calc_reg_one_value = 0;
				}
				else
				{
					address_calc_reg_one_value = 
						registers[address_calc_reg_one_index - 1];
				}
				if(address_calc_reg_two_index == 0)
				{
					address_calc_reg_two_value = 0;
				}
				else
				{
					address_calc_reg_two_value = 
						registers[address_calc_reg_two_index - 1];
				}

				u32 address_calc = 
					address_calc_reg_one_value + address_calc_reg_two_value + 
					address_calc_displacement;

				u32 reg = execute.Operands[0].Register.Index;
				u32 temp = registers[reg - 1];

				u8 high_bits = memory[address_calc];
				u8 low_bits = memory[address_calc + 1];
				registers[reg - 1] = high_bits << 8;
				registers[reg - 1] += low_bits;

				print_reg(reg);
				printf(", [ ");
				if(address_calc_reg_one_index)
				{
					print_reg(address_calc_reg_one_index);
					printf(" + ");
				}
				if(address_calc_reg_two_index)
				{
					print_reg(address_calc_reg_two_index);
					printf(" + ");
				}
				printf("%d ] ; ", address_calc_displacement);
				print_reg(reg);
				printf(":0x%x->0x%x ", temp, registers[reg - 1]);
			}
			else
			{
    		    printf("\e[1;33m[WARNING] (execute): second operand is not a register or"
				" immediate or memory\e[0;37m\n");
				return;
			}
		}
		if(op_one_type == Operand_Memory)
		{
			/* NOTE: don't need op two is register case cuz it never comes up in listing 0051/0052 */
			if(op_two_type == Operand_Immediate)
			{
				u16 address_calc_reg_one_value;
				u16 address_calc_reg_two_value;
				u32 address_calc_reg_one_index = execute.Operands[0].Address.Terms[0].Register.Index;
				u32 address_calc_reg_two_index = execute.Operands[0].Address.Terms[1].Register.Index;
				s32 address_calc_displacement  = execute.Operands[0].Address.Displacement;

				if(address_calc_reg_one_index == 0)
				{
					address_calc_reg_one_value = 0;
				}
				else
				{
					address_calc_reg_one_value = 
						registers[address_calc_reg_one_index - 1];
				}
				if(address_calc_reg_two_index == 0)
				{
					address_calc_reg_two_value = 0;
				}
				else
				{
					address_calc_reg_two_value = 
						registers[address_calc_reg_two_index - 1];
				}

				u32 address_calc = 
					address_calc_reg_one_value + address_calc_reg_two_value + 
					address_calc_displacement;

				s16 imm = execute.Operands[1].Immediate.Value;
				u16 high_bits = (imm & 0b1111111100000000);
				u16 low_bits  = (imm & 0b0000000011111111);
				memory[address_calc] = (u8)high_bits >> 8;
				memory[address_calc+1] = (u8)low_bits;

				printf("[ ");
				if(address_calc_reg_one_index)
				{
					print_reg(address_calc_reg_one_index);
					printf(" + ");
				}
				if(address_calc_reg_two_index)
				{
					print_reg(address_calc_reg_two_index);
					printf(" + ");
				}
				printf("%d ], %d ; ", address_calc_displacement, imm);
			}
			else
			{
    		    printf("\e[1;33m[WARNING] (execute): expected op two to be register or immediate" 
						" since op one is memory");
				return;
			}
		}
	}

	/* add */
	else if(op_type == Op_add)
	{
		printf("add ");
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
			registers[reg_one - 1] = registers[reg_one - 1] + registers[reg_two - 1];

			print_reg(reg_one);
			printf(", ");
			print_reg(reg_two);
			printf(" ; ");
			print_reg(reg_one);
			printf(":0x%x->0x%x ", temp, registers[reg_one - 1]);

			b32 zero_flag_switched = 0;
			b32 sign_flag_switched = 0;
			if((registers[reg_one - 1] == 0) && (!zero_flag))
			{
				printf("flags:");
				print_flags();
				zero_flag = 1;
				zero_flag_switched = 1;
			}
			if(((registers[reg_one - 1] & 0x8000) == 0x8000) && (!sign_flag))
			{
				if(!zero_flag_switched)
				{
					printf("flags:");
					print_flags();
				}
				sign_flag = 1;
				sign_flag_switched = 1;
			}
			if(zero_flag_switched || sign_flag_switched)
			{
				printf("->");
				print_flags();
			}
		}
		else if(op_two_type == Operand_Immediate)
		{

			u32 reg = execute.Operands[0].Register.Index;
			s32 imm = execute.Operands[1].Immediate.Value;

			u32 temp = registers[reg - 1] = registers[reg - 1];
			registers[reg - 1] = registers[reg - 1] + imm;

			print_reg(reg);
			printf(", %d ; ", imm);
			print_reg(reg);
			printf(":0x%x->0x%x ", temp, registers[reg - 1]);

			b32 zero_flag_switched = 0;
			b32 sign_flag_switched = 0;
			if((registers[reg - 1] == 0) && (!zero_flag))
			{
				printf("flags:");
				print_flags();
				zero_flag = 1;
				zero_flag_switched = 1;
			}
			if(((registers[reg - 1] & 0x8000) == 0x8000) && (!sign_flag))
			{
				if(!zero_flag_switched)
				{
					printf("flags:");
					print_flags();
				}
				sign_flag = 1;
				sign_flag_switched = 1;
			}
			if(zero_flag_switched || sign_flag_switched)
			{
				printf("->");
				print_flags();
			}
		}
		else
		{
			ASSERT(0);
		}
	}

	/* sub */
	else if(op_type == Op_sub)
	{
		printf("sub ");
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
			registers[reg_one - 1] = registers[reg_one - 1] - registers[reg_two - 1];

			print_reg(reg_one);
			printf(", ");
			print_reg(reg_two);
			printf(" ; ");
			print_reg(reg_one);
			printf(":0x%x->0x%x ", temp, registers[reg_one - 1]);

			b32 zero_flag_switched = 0;
			b32 sign_flag_switched = 0;
			if((registers[reg_one - 1] == 0) && (!zero_flag))
			{
				printf("flags:");
				print_flags();
				zero_flag = 1;
				zero_flag_switched = 1;
			}
			if(((registers[reg_one - 1] & 0x8000) == 0x8000) && (!sign_flag))
			{
				if(!zero_flag_switched)
				{
					printf("flags:");
					print_flags();
				}
				sign_flag = 1;
				sign_flag_switched = 1;
			}
			if(zero_flag_switched || sign_flag_switched)
			{
				printf("->");
				print_flags();
			}
		}
		else if(op_two_type == Operand_Immediate)
		{

			u32 reg = execute.Operands[0].Register.Index;
			s32 imm = execute.Operands[1].Immediate.Value;

			u32 temp = registers[reg - 1] = registers[reg - 1];
			registers[reg - 1] = registers[reg - 1] - imm;

			print_reg(reg);
			printf(", %d ; ", imm);
			print_reg(reg);
			printf(":0x%x->0x%x ", temp, registers[reg - 1]);

			b32 zero_flag_switched = 0;
			b32 sign_flag_switched = 0;
			if((registers[reg - 1] == 0) && (!zero_flag))
			{
				printf("flags:");
				print_flags();
				zero_flag = 1;
				zero_flag_switched = 1;
			}
			if(((registers[reg - 1] & 0x8000) == 0x8000) && (!sign_flag))
			{
				if(!zero_flag_switched)
				{
					printf("flags:");
					print_flags();
				}
				sign_flag = 1;
				sign_flag_switched = 1;
			}
			if(zero_flag_switched || sign_flag_switched)
			{
				printf("->");
				print_flags();
			}
		}
		else
		{
			ASSERT(0);
		}
	}

	/* cmp */
	else if(op_type == Op_cmp)
	{
		printf("cmp ");
		if(op_one_type != Operand_Register)
		{
	    printf("\e[1;33m[WARNING] (execute): first operand is not a register\e[0;37m\n");
			return;
		}
		if(op_two_type == Operand_Register)
		{
			u32 reg_one = execute.Operands[0].Register.Index;
			u32 reg_two = execute.Operands[1].Register.Index;

			u32 temp = registers[reg_one - 1] - registers[reg_two - 1] ;

			print_reg(reg_one);
			printf(", ");
			print_reg(reg_two);
			printf(" ; ");

			b32 zero_flag_switched = 0;
			b32 sign_flag_switched = 0;
			if((temp == 0) && (!zero_flag))
			{
				printf("flags:");
				print_flags();
				zero_flag = 1;
				zero_flag_switched = 1;
			}
			if((temp != 0) && (zero_flag))
			{
				printf("flags:");
				print_flags();
				zero_flag = 0;
				zero_flag_switched = 1;
			}
			if(((temp & 0x8000) == 0x8000) && (!sign_flag))
			{
				if(!zero_flag_switched)
				{
					printf("flags:");
					print_flags();
				}
				sign_flag = 1;
				sign_flag_switched = 1;
			}
			if(((temp & 0x8000) != 0x8000) && (sign_flag))
			{
				if(!zero_flag_switched)
				{
					printf("flags:");
					print_flags();
				}
				sign_flag = 0;
				sign_flag_switched = 1;
			}
			if(zero_flag_switched || sign_flag_switched)
			{
				printf("->");
				print_flags();
			}
		}
		else if(op_two_type == Operand_Immediate)
		{

			u32 reg = execute.Operands[0].Register.Index;
			s32 imm = execute.Operands[1].Immediate.Value;

			u32 temp = registers[reg - 1] - imm;

			print_reg(reg);
			printf(", %d ; ", imm);

			b32 zero_flag_switched = 0;
			b32 sign_flag_switched = 0;
			if((temp == 0) && (!zero_flag))
			{
				printf("flags:");
				print_flags();
				zero_flag = 1;
				zero_flag_switched = 1;
			}
			if((temp != 0) && (zero_flag))
			{
				printf("flags:");
				print_flags();
				zero_flag = 0;
				zero_flag_switched = 1;
			}
			if(((temp & 0x8000) == 0x8000) && (!sign_flag))
			{
				if(!zero_flag_switched)
				{
					printf("flags:");
					print_flags();
				}
				sign_flag = 1;
				sign_flag_switched = 1;
			}
			if(((temp & 0x8000) != 0x8000) && (sign_flag))
			{
				if(!zero_flag_switched)
				{
					printf("flags:");
					print_flags();
				}
				sign_flag = 0;
				sign_flag_switched = 1;
			}
			if(zero_flag_switched || sign_flag_switched)
			{
				printf("->");
				print_flags();
			}
		}
		else
		{
			ASSERT(0);
		}
	}

	/* jne */
	else if(op_type == Op_jne)
	{
		s32 imm = execute.Operands[0].Immediate.Value;
		printf("jne %d", imm);
		if(zero_flag == 0)
		{
			ip += imm;
		}
	}
	else
	{
		printf("\e[1;33m[WARNING] (execute): op type is not mov, sub, add or cmp\e[0;37m\n");
		return;
	}
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
	ASSERT(stat("listing_0051_memory_mov", &disassembly_stat) == 0);

	unsigned long long disassembly_size = disassembly_stat.st_size;
	int disassembly_fd = open("listing_0051_memory_mov", O_RDONLY);
	unsigned char *disassembly = malloc(disassembly_size);
	read(disassembly_fd, disassembly, disassembly_size);

	/* decode and execute loop */
    while(ip < disassembly_size)
    {
        instruction decoded;
        Sim86_Decode8086Instruction(disassembly_size - ip, 
			disassembly + ip, &decoded);
        if(decoded.Op)
        {
			u16 temp_ip = ip;
            ip += decoded.Size;
			execute(decoded);

			printf("ip:0x%x->0x%x\n", temp_ip, ip);
        }
        else
        {
            printf("\e[1;33m[WARNING]: Unrecognized instruction\e[0;37m\n");
            break;
        }
    }

	printf("\nFinal registers:\n");
	if(registers[0] != 0)
	{
		printf("    ax: 0x%.4x (%u)\n", registers[0], registers[0]);
	}
	if(registers[1] != 0)
	{
		printf("    bx: 0x%.4x (%u)\n", registers[1], registers[1]);
	}
	if(registers[2] != 0)
	{
		printf("    cx: 0x%.4x (%u)\n", registers[2], registers[2]);
	}
	if(registers[3] != 0)
	{
		printf("    dx: 0x%.4x (%u)\n", registers[3], registers[3]);
	}
	if(registers[4] != 0)
	{
		printf("    sp: 0x%.4x (%u)\n", registers[4], registers[4]);
	}
	if(registers[5] != 0)
	{
		printf("    bp: 0x%.4x (%u)\n", registers[5], registers[5]);
	}
	if(registers[6] != 0)
	{
		printf("    si: 0x%.4x (%u)\n", registers[6], registers[6]);
	}
	if(registers[7] != 0)
	{
		printf("    di: 0x%.4x (%u)\n", registers[7], registers[7]);
	}
	printf("    ip: 0x%.4x (%u)\n", ip, ip);

	printf("  flags: ");
	if(sign_flag)
	{
		printf("S");
	}
	if(zero_flag)
	{
		printf("Z");
	}
	printf("\n\n\n");
    
	return (0);
}
