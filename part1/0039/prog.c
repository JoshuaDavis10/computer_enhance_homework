#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define ASSERT(x) \
if(!x) \
{ \
	printf("assertion: EXPR = '%s', FILE = %s, LINE = %d\n", #x, __FILE__, __LINE__);\
	__builtin_trap(); \
} 

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

/* macros for decoding first byte of 8086 instructions*/
#define MOV_IMMEDIATE_TO_REGISTER 0b10110000
#define MOV_IMMEDIATE_TO_REGISTER_W 0b00001000
#define MOV_IMMEDIATE_TO_REGISTER_REG 0b00000111

/* NOTE: mov_standard is best name I got for it. it refers to the first mov decoding on page
 * 265 https://docs.google.com/file/d/0B9rh9tVI0J5mNWUxNjlmYmEtZDcxOS00MGQxLThlN2EtNjJmOTI0ZGUwNmI0/edit?pli=1&resourcekey=0-RZZt1hJMnjAIcud6eTf-pQ
 */
#define MOV_STANDARD 0b10001000 
#define MOV_STANDARD_W 0b00000001 
#define MOV_STANDARD_D 0b00000010 
#define MOV_STANDARD_MOD 0b11000000 
#define MOV_STANDARD_REG 0b00111000 
#define MOV_STANDARD_RM 0b00000111 

static FILE *global_output_asm_file;
static unsigned char *global_instruction_stream = 0;
static unsigned int global_instruction_index = 0;

void print_reg_immediate_to_register(unsigned char reg, char w);
void print_reg_standard(unsigned char reg, unsigned char w);
void print_rm_standard(unsigned char rm, unsigned char mod, unsigned char w);

unsigned char get_next_byte_from_instruction_stream()
{

	printf("get byte: %b\n", global_instruction_stream[global_instruction_index]);
	return (global_instruction_stream[global_instruction_index++]);
}

void decode_instruction()
{
	printf("decoding instruction...\n");
	unsigned char instruction_first_byte = get_next_byte_from_instruction_stream();
	if((instruction_first_byte & MOV_IMMEDIATE_TO_REGISTER) == MOV_IMMEDIATE_TO_REGISTER)
	{
		fprintf(global_output_asm_file, "mov ");

		if((instruction_first_byte & MOV_IMMEDIATE_TO_REGISTER_W) == MOV_IMMEDIATE_TO_REGISTER_W)
		{
			/* w is 1 */
			short immediate = 
				get_next_byte_from_instruction_stream() +
				(get_next_byte_from_instruction_stream() << 8);
			unsigned char reg = instruction_first_byte & MOV_IMMEDIATE_TO_REGISTER_REG;
			print_reg_immediate_to_register(reg, 1);
			fprintf(global_output_asm_file, "%d", immediate);
			return;
		}
		else
		{
			/* w is 0 */
			char immediate = get_next_byte_from_instruction_stream();
			unsigned char reg = instruction_first_byte & MOV_IMMEDIATE_TO_REGISTER_REG;
			print_reg_immediate_to_register(reg, 0);
			fprintf(global_output_asm_file, "%d", immediate);
			return;
		}
		/* NOTE: this should never happen, I'm just paranoid */
		ASSERT(0);
	}
	else if((instruction_first_byte & MOV_STANDARD) == MOV_STANDARD)
	{
		fprintf(global_output_asm_file, "mov ");

		unsigned char w;
		unsigned char d;
		unsigned char mod;
		unsigned char reg;
		unsigned char rm;

		unsigned char mod_reg_rm_byte = get_next_byte_from_instruction_stream();

		/* set w */
		if((instruction_first_byte & MOV_STANDARD_W) == MOV_STANDARD_W)
		{
			w = 1;
		}
		else
		{
			w = 0;
		}
		/* set d */
		if((instruction_first_byte & MOV_STANDARD_D) == MOV_STANDARD_D)
		{
			d = 1;
		}
		else
		{
			d = 0;
		}

		mod = mod_reg_rm_byte & MOV_STANDARD_MOD;
		reg = mod_reg_rm_byte & MOV_STANDARD_REG;
		rm  = mod_reg_rm_byte & MOV_STANDARD_RM;

		/* print reg/rm or rm/reg based on D */
		if(d == 0)
		{
			print_rm_standard(rm, mod, w);
			fprintf(global_output_asm_file, ", ");
			print_reg_standard(reg, w);
		}
		else if(d == 1)
		{
			print_reg_standard(reg, w);
			fprintf(global_output_asm_file, ", ");
			print_rm_standard(rm, mod, w);
		}
		else
		{
			ASSERT(0);
		}
	}
	else
	{
		printf("decode_instruction: got an instruction with first byte: %b"
			"\ndon't know how to decode\n", instruction_first_byte);
		ASSERT(0);
	}
}

int main()
{

	global_output_asm_file = fopen("decoded.asm", "w+");

	struct stat bin_file_stat;
	ASSERT(stat("bin", &bin_file_stat) == 0);

	unsigned long long bin_file_size = bin_file_stat.st_size;
	int bin_file_fd = open("bin", O_RDONLY);
	global_instruction_stream = malloc(bin_file_size);
	read(bin_file_fd, global_instruction_stream, bin_file_size);

	fprintf(global_output_asm_file, "bits 16\n\n");
	while(global_instruction_index < bin_file_size)
	{
		decode_instruction();
		fprintf(global_output_asm_file, "\n");
	}

	return 0;
}

#define MOV_IMMEDIATE_TO_REGISTER_AX 0b00000000 
#define MOV_IMMEDIATE_TO_REGISTER_CX 0b00000001 
#define MOV_IMMEDIATE_TO_REGISTER_DX 0b00000010 
#define MOV_IMMEDIATE_TO_REGISTER_BX 0b00000011 
#define MOV_IMMEDIATE_TO_REGISTER_SP 0b00000100 
#define MOV_IMMEDIATE_TO_REGISTER_BP 0b00000101 
#define MOV_IMMEDIATE_TO_REGISTER_SI 0b00000110 
#define MOV_IMMEDIATE_TO_REGISTER_DI 0b00000111 
void print_reg_immediate_to_register(unsigned char reg, char w)
{
	if(w == 0)
	{
		switch(reg)
		{
			case MOV_IMMEDIATE_TO_REGISTER_AX:
			{
				fprintf(global_output_asm_file, "al, ");
			} break;
			case MOV_IMMEDIATE_TO_REGISTER_CX:
			{
				fprintf(global_output_asm_file, "cl, ");
			} break;
			case MOV_IMMEDIATE_TO_REGISTER_DX:
			{
				fprintf(global_output_asm_file, "dl, ");
			} break;
			case MOV_IMMEDIATE_TO_REGISTER_BX:
			{
				fprintf(global_output_asm_file, "bl, ");
			} break;
			case MOV_IMMEDIATE_TO_REGISTER_SP:
			{
				fprintf(global_output_asm_file, "ah, ");
			} break;
			case MOV_IMMEDIATE_TO_REGISTER_BP:
			{
				fprintf(global_output_asm_file, "ch, ");
			} break;
			case MOV_IMMEDIATE_TO_REGISTER_SI:
			{
				fprintf(global_output_asm_file, "dh, ");
			} break;
			case MOV_IMMEDIATE_TO_REGISTER_DI:
			{
				fprintf(global_output_asm_file, "bh, ");
			} break;
			default:
			{
				printf("print_reg_immediate_to_register: unkown reg value, %b", reg);
				ASSERT(0);
			} break;
		}
	}
	else if(w == 1)
	{
		switch(reg)
		{
			case MOV_IMMEDIATE_TO_REGISTER_AX:
			{
				fprintf(global_output_asm_file, "ax, ");
			} break;
			case MOV_IMMEDIATE_TO_REGISTER_CX:
			{
				fprintf(global_output_asm_file, "cx, ");
			} break;
			case MOV_IMMEDIATE_TO_REGISTER_DX:
			{
				fprintf(global_output_asm_file, "dx, ");
			} break;
			case MOV_IMMEDIATE_TO_REGISTER_BX:
			{
				fprintf(global_output_asm_file, "bx, ");
			} break;
			case MOV_IMMEDIATE_TO_REGISTER_SP:
			{
				fprintf(global_output_asm_file, "sp ");
			} break;
			case MOV_IMMEDIATE_TO_REGISTER_BP:
			{
				fprintf(global_output_asm_file, "bp, ");
			} break;
			case MOV_IMMEDIATE_TO_REGISTER_SI:
			{
				fprintf(global_output_asm_file, "si, ");
			} break;
			case MOV_IMMEDIATE_TO_REGISTER_DI:
			{
				fprintf(global_output_asm_file, "di, ");
			} break;
			default:
			{
				printf("print_reg_immediate_to_register: unkown reg value, %b", reg);
				ASSERT(0);
			} break;
		}
	}
	else
	{
		printf("print_reg_immediate_to_register: w must be 0 or 1, but got passed w = %d\n", w);
		ASSERT(0);
	}
}

#define MOV_STANDARD_MOD_00 0b00000000 
#define MOV_STANDARD_MOD_01 0b01000000 
#define MOV_STANDARD_MOD_10 0b10000000 
#define MOV_STANDARD_MOD_11 0b11000000 

#define MOV_STANDARD_AX 0b00000000
#define MOV_STANDARD_CX 0b00001000
#define MOV_STANDARD_DX 0b00010000
#define MOV_STANDARD_BX 0b00011000
#define MOV_STANDARD_SP 0b00100000
#define MOV_STANDARD_BP 0b00101000
#define MOV_STANDARD_SI 0b00110000
#define MOV_STANDARD_DI 0b00111000

#define MOV_STANDARD_RM_000 0b00000000
#define MOV_STANDARD_RM_001 0b00000001
#define MOV_STANDARD_RM_010 0b00000010
#define MOV_STANDARD_RM_011 0b00000011
#define MOV_STANDARD_RM_100 0b00000100
#define MOV_STANDARD_RM_101 0b00000101
#define MOV_STANDARD_RM_110 0b00000110
#define MOV_STANDARD_RM_111 0b00000111

void print_reg_standard(unsigned char reg, unsigned char w)
{
	if(w == 0)
	{
		switch(reg)
		{
			case MOV_STANDARD_AX:
			{
				fprintf(global_output_asm_file, "al");
			} break;
			case MOV_STANDARD_CX:
			{
				fprintf(global_output_asm_file, "cl");
			} break;
			case MOV_STANDARD_DX:
			{
				fprintf(global_output_asm_file, "dl");
			} break;
			case MOV_STANDARD_BX:
			{
				fprintf(global_output_asm_file, "bl");
			} break;
			case MOV_STANDARD_SP:
			{
				fprintf(global_output_asm_file, "ah");
			} break;
			case MOV_STANDARD_BP:
			{
				fprintf(global_output_asm_file, "ch");
			} break;
			case MOV_STANDARD_SI:
			{
				fprintf(global_output_asm_file, "dh");
			} break;
			case MOV_STANDARD_DI:
			{
				fprintf(global_output_asm_file, "bh");
			} break;
			default:
			{
				printf("print_reg_standard: unkown reg value, %b", reg);
				ASSERT(0);
			} break;
		}
	}
	else if(w == 1)
	{
		switch(reg)
		{
			case MOV_STANDARD_AX:
			{
				fprintf(global_output_asm_file, "ax");
			} break;
			case MOV_STANDARD_CX:
			{
				fprintf(global_output_asm_file, "cx");
			} break;
			case MOV_STANDARD_DX:
			{
				fprintf(global_output_asm_file, "dx");
			} break;
			case MOV_STANDARD_BX:
			{
				fprintf(global_output_asm_file, "bx");
			} break;
			case MOV_STANDARD_SP:
			{
				fprintf(global_output_asm_file, "sp");
			} break;
			case MOV_STANDARD_BP:
			{
				fprintf(global_output_asm_file, "bp");
			} break;
			case MOV_STANDARD_SI:
			{
				fprintf(global_output_asm_file, "si");
			} break;
			case MOV_STANDARD_DI:
			{
				fprintf(global_output_asm_file, "di");
			} break;
			default:
			{
				printf("print_reg_standard: unkown reg value, %b", reg);
				ASSERT(0);
			} break;
		}
	}
	else
	{
		printf("print_reg_standard: w must be 0 or 1, but got passed w = %d\n", w);
		ASSERT(0);
	}
}

void print_rm_standard(unsigned char rm, unsigned char mod, unsigned char w)
{
	if(mod == MOV_STANDARD_MOD_11)
	{
		print_reg_standard((rm << 3), w);
	}
	else if(mod == MOV_STANDARD_MOD_00)
	{
		switch(rm)
		{
			case MOV_STANDARD_RM_000:
			{
				fprintf(global_output_asm_file, "[bx + si]");
			} break;
			case MOV_STANDARD_RM_001:
			{
				fprintf(global_output_asm_file, "[bx + di]");
			} break;
			case MOV_STANDARD_RM_010:
			{
				fprintf(global_output_asm_file, "[bp + si]");
			} break;
			case MOV_STANDARD_RM_011:
			{
				fprintf(global_output_asm_file, "[bp + di]");
			} break;
			case MOV_STANDARD_RM_100:
			{
				fprintf(global_output_asm_file, "[si]");
			} break;
			case MOV_STANDARD_RM_101:
			{
				fprintf(global_output_asm_file, "[di]");
			} break;
			case MOV_STANDARD_RM_110:
			{
				/* TODO: direct address, get displacement value */
				/* NOTE: this shouldn't happen in listing 0039 at all
				 * so I will probably leave it for now
				 */
			} break;
			case MOV_STANDARD_RM_111:
			{
				fprintf(global_output_asm_file, "[bx]");
			} break;
			default:
			{
				printf("print_rm_standard: unkown rm value, %b", rm);
				ASSERT(0);
			} break;
		}
	}
	else if(mod == MOV_STANDARD_MOD_01)
	{
		char displacement = get_next_byte_from_instruction_stream(); 
		switch(rm)
		{
			case MOV_STANDARD_RM_000:
			{
				fprintf(global_output_asm_file, "[bx + si + %d]", displacement);
			} break;
			case MOV_STANDARD_RM_001:
			{
				fprintf(global_output_asm_file, "[bx + di + %d]", displacement);
			} break;
			case MOV_STANDARD_RM_010:
			{
				fprintf(global_output_asm_file, "[bp + si + %d]", displacement);
			} break;
			case MOV_STANDARD_RM_011:
			{
				fprintf(global_output_asm_file, "[bp + di + %d]", displacement);
			} break;
			case MOV_STANDARD_RM_100:
			{
				fprintf(global_output_asm_file, "[si + %d]", displacement);
			} break;
			case MOV_STANDARD_RM_101:
			{
				fprintf(global_output_asm_file, "[di + %d]", displacement);
			} break;
			case MOV_STANDARD_RM_110:
			{
				fprintf(global_output_asm_file, "[bp + %d]", displacement);
			} break;
			case MOV_STANDARD_RM_111:
			{
				fprintf(global_output_asm_file, "[bx + %d]", displacement);
			} break;
			default:
			{
				printf("print_rm_standard: unkown rm value, %b", rm);
				ASSERT(0);
			} break;
		}
	}
	else if(mod == MOV_STANDARD_MOD_10)
	{
		short displacement = 
			get_next_byte_from_instruction_stream() +
			(get_next_byte_from_instruction_stream() << 8);
		switch(rm)
		{
			case MOV_STANDARD_RM_000:
			{
				fprintf(global_output_asm_file, "[bx + si + %d]", displacement);
			} break;
			case MOV_STANDARD_RM_001:
			{
				fprintf(global_output_asm_file, "[bx + di + %d]", displacement);
			} break;
			case MOV_STANDARD_RM_010:
			{
				fprintf(global_output_asm_file, "[bp + si + %d]", displacement);
			} break;
			case MOV_STANDARD_RM_011:
			{
				fprintf(global_output_asm_file, "[bp + di + %d]", displacement);
			} break;
			case MOV_STANDARD_RM_100:
			{
				fprintf(global_output_asm_file, "[si + %d]", displacement);
			} break;
			case MOV_STANDARD_RM_101:
			{
				fprintf(global_output_asm_file, "[di + %d]", displacement);
			} break;
			case MOV_STANDARD_RM_110:
			{
				fprintf(global_output_asm_file, "[bp + %d]", displacement);
			} break;
			case MOV_STANDARD_RM_111:
			{
				fprintf(global_output_asm_file, "[bx + %d]", displacement);
			} break;
			default:
			{
				printf("print_rm_standard: unkown rm value, %b", rm);
				ASSERT(0);
			} break;
		}
	}
	else
	{
		printf("print_rm_standard: unkown mod value, %b", mod);
	}
}
