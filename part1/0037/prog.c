#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define REG_AX 0b0000000000000000
#define REG_BX 0b0000000000011000
#define REG_CX 0b0000000000001000
#define REG_DX 0b0000000000010000
#define REG 0b0000000000111000

#define RM_AX 0b0000000000000000
#define RM_BX 0b0000000000000011
#define RM_CX 0b0000000000000001
#define RM_DX 0b0000000000000010
#define RM 0b0000000000000111

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
	if(instruction & 0b1000100000000000 == 0b1000100000000000)
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
				default:
				{
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
				default:
				{
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
				default:
				{
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
				default:
				{
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
