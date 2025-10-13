#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define REG_AX 0b0000000100000000
#define REG_BX 0b0000000100011000
#define REG_CX 0b0000000100001000
#define REG_DX 0b0000000100010000
#define REG_SP 0b0000000100100000
#define REG_BP 0b0000000100101000
#define REG_SI 0b0000000100110000
#define REG_DI 0b0000000100111000
#define REG_AL 0b0000000000000000
#define REG_BL 0b0000000000011000
#define REG_CL 0b0000000000001000
#define REG_DL 0b0000000000010000
#define REG_AH 0b0000000000100000
#define REG_BH 0b0000000000111000
#define REG_CH 0b0000000000101000
#define REG_DH 0b0000000000110000
#define REG 0b0000000100111000

#define RM_AX 0b0000000100000000
#define RM_BX 0b0000000100000011
#define RM_CX 0b0000000100000001
#define RM_DX 0b0000000100000010
#define RM_SP 0b0000000100000100
#define RM_BP 0b0000000100000101
#define RM_SI 0b0000000100000110
#define RM_DI 0b0000000100000111
#define RM_AL 0b0000000000000000
#define RM_BL 0b0000000000000011
#define RM_CL 0b0000000000000001
#define RM_DL 0b0000000000000010
#define RM_AH 0b0000000000000100
#define RM_BH 0b0000000000000111
#define RM_CH 0b0000000000000101
#define RM_DH 0b0000000000000110
#define RM 0b0000000100000111

#define MOD_00 0b0000000000000000
#define MOD_01 0b0000000001000000
#define MOD_10 0b0000000010000000
#define MOD_11 0b0000000011000000
#define MOD 0b0000000011000000

#define D_0 0b0000000000000000
#define D_1 0b0000001000000000
#define D 0b0000001000000000
#define W_0 0b0000000000000000
#define W_1 0b0000000100000000
#define W 0b0000000100000000

#define ASSERT(x) \
if(!x) \
{ \
	printf("assertion: EXPR = '%s', FILE = %s, LINE = %d\n", #x, __FILE__, __LINE__);\
	__builtin_trap(); \
} 

FILE *output_asm_file;

void decode_instruction(short instruction)
{
	printf("instruction : %b | instruction & 0b1000100000000000 = %b\n", instruction, instruction & 0b1000100000000000);
	if((instruction & 0b1000100000000000) == 0b1000100000000000)
	{
		/* got a mov */
		short d = instruction & D;
		short w = instruction & W;
		short mod = instruction & MOD;
		short reg = instruction & REG;
		short rm = instruction & RM;
		fprintf(output_asm_file, "mov ");

		/* destination register is in RM field */
		if(d == D_0)
		{
			switch(rm)
			{
				case RM_AX:
				{
					fprintf(output_asm_file, "ax, ");
				} break;
				case RM_BX:
				{
					fprintf(output_asm_file, "bx, ");
				} break;
				case RM_CX:
				{
					fprintf(output_asm_file, "cx, ");
				} break;
				case RM_DX:
				{
					fprintf(output_asm_file, "dx, ");
				} break;
				case RM_SP:
				{
					fprintf(output_asm_file, "sp, ");
				} break;
				case RM_BP:
				{
					fprintf(output_asm_file, "bp, ");
				} break;
				case RM_SI:
				{
					fprintf(output_asm_file, "si, ");
				} break;
				case RM_DI:
				{
					fprintf(output_asm_file, "di, ");
				} break;
				case RM_AL:
				{
					fprintf(output_asm_file, "al, ");
				} break;
				case RM_BL:
				{
					fprintf(output_asm_file, "bl, ");
				} break;
				case RM_CL:
				{
					fprintf(output_asm_file, "cl, ");
				} break;
				case RM_DL:
				{
					fprintf(output_asm_file, "dl, ");
				} break;
				case RM_AH:
				{
					fprintf(output_asm_file, "ah, ");
				} break;
				case RM_BH:
				{
					fprintf(output_asm_file, "bh, ");
				} break;
				case RM_CH:
				{
					fprintf(output_asm_file, "ch, ");
				} break;
				case RM_DH:
				{
					fprintf(output_asm_file, "dh, ");
				} break;
				default:
				{
					printf("rm = %b\n", rm);
					ASSERT(0);
				} break;
			}
			switch(reg)
			{
				case REG_AX:
				{
					fprintf(output_asm_file, "ax");
				} break;
				case REG_BX:
				{
					fprintf(output_asm_file, "bx");
				} break;
				case REG_CX:
				{
					fprintf(output_asm_file, "cx");
				} break;
				case REG_DX:
				{
					fprintf(output_asm_file, "dx");
				} break;
				case REG_SP:
				{
					fprintf(output_asm_file, "sp, ");
				} break;
				case REG_BP:
				{
					fprintf(output_asm_file, "bp, ");
				} break;
				case REG_SI:
				{
					fprintf(output_asm_file, "si, ");
				} break;
				case REG_DI:
				{
					fprintf(output_asm_file, "di, ");
				} break;
				case REG_AL:
				{
					fprintf(output_asm_file, "al, ");
				} break;
				case REG_BL:
				{
					fprintf(output_asm_file, "bl, ");
				} break;
				case REG_CL:
				{
					fprintf(output_asm_file, "cl, ");
				} break;
				case REG_DL:
				{
					fprintf(output_asm_file, "dl, ");
				} break;
				case REG_AH:
				{
					fprintf(output_asm_file, "ah, ");
				} break;
				case REG_BH:
				{
					fprintf(output_asm_file, "bh, ");
				} break;
				case REG_CH:
				{
					fprintf(output_asm_file, "ch, ");
				} break;
				case REG_DH:
				{
					fprintf(output_asm_file, "dh, ");
				} break;
				default:
				{
					printf("reg = %b\n", reg);
					ASSERT(0);
				} break;
			}
		}
		/* destination register is in REG field */
		else if(d == D_1)
		{
			switch(reg)
			{
				case REG_AX:
				{
					fprintf(output_asm_file, "ax, ");
				} break;
				case REG_BX:
				{
					fprintf(output_asm_file, "bx, ");
				} break;
				case REG_CX:
				{
					fprintf(output_asm_file, "cx, ");
				} break;
				case REG_DX:
				{
					fprintf(output_asm_file, "dx, ");
				} break;
				case REG_SP:
				{
					fprintf(output_asm_file, "sp, ");
				} break;
				case REG_BP:
				{
					fprintf(output_asm_file, "bp, ");
				} break;
				case REG_SI:
				{
					fprintf(output_asm_file, "si, ");
				} break;
				case REG_DI:
				{
					fprintf(output_asm_file, "di, ");
				} break;
				case REG_AL:
				{
					fprintf(output_asm_file, "al, ");
				} break;
				case REG_BL:
				{
					fprintf(output_asm_file, "bl, ");
				} break;
				case REG_CL:
				{
					fprintf(output_asm_file, "cl, ");
				} break;
				case REG_DL:
				{
					fprintf(output_asm_file, "dl, ");
				} break;
				case REG_AH:
				{
					fprintf(output_asm_file, "ah, ");
				} break;
				case REG_BH:
				{
					fprintf(output_asm_file, "bh, ");
				} break;
				case REG_CH:
				{
					fprintf(output_asm_file, "ch, ");
				} break;
				case REG_DH:
				{
					fprintf(output_asm_file, "dh, ");
				} break;
				default:
				{
					printf("reg = %b\n", reg);
					ASSERT(0);
				} break;
			}
			switch(rm)
			{
				case RM_AX:
				{
					fprintf(output_asm_file, "ax");
				} break;
				case RM_BX:
				{
					fprintf(output_asm_file, "bx");
				} break;
				case RM_CX:
				{
					fprintf(output_asm_file, "cx");
				} break;
				case RM_DX:
				{
					fprintf(output_asm_file, "dx");
				} break;
				case RM_SP:
				{
					fprintf(output_asm_file, "sp, ");
				} break;
				case RM_BP:
				{
					fprintf(output_asm_file, "bp, ");
				} break;
				case RM_SI:
				{
					fprintf(output_asm_file, "si, ");
				} break;
				case RM_DI:
				{
					fprintf(output_asm_file, "di, ");
				} break;
				case RM_AL:
				{
					fprintf(output_asm_file, "al, ");
				} break;
				case RM_BL:
				{
					fprintf(output_asm_file, "bl, ");
				} break;
				case RM_CL:
				{
					fprintf(output_asm_file, "cl, ");
				} break;
				case RM_DL:
				{
					fprintf(output_asm_file, "dl, ");
				} break;
				case RM_AH:
				{
					fprintf(output_asm_file, "ah, ");
				} break;
				case RM_BH:
				{
					fprintf(output_asm_file, "bh, ");
				} break;
				case RM_CH:
				{
					fprintf(output_asm_file, "ch, ");
				} break;
				case RM_DH:
				{
					fprintf(output_asm_file, "dh, ");
				} break;
				default:
				{
					printf("rm = %b\n", rm);
					ASSERT(0);
				} break;
			}
		}
		else
		{
			ASSERT(0);
		}
	}
}

int main()
{

	output_asm_file = fopen("decoded.asm", "w+");

	struct stat bin_file_stat;
	ASSERT(stat("bin", &bin_file_stat) == 0);

	unsigned long long bin_file_size = bin_file_stat.st_size;
	int bin_file_fd = open("bin", O_RDONLY);
	unsigned char *bin_file_data_buffer = malloc(bin_file_size);
	read(bin_file_fd, bin_file_data_buffer, bin_file_size);

	short instruction;

	fprintf(output_asm_file, "bits 16\n\n");

	for(int i = 0; i < bin_file_size/2; i++)
	{
		instruction = (bin_file_data_buffer[2*i] << 8) | bin_file_data_buffer[2*i+1];
		decode_instruction(instruction);
		fprintf(output_asm_file, "\n");
	}

	return 0;
}
