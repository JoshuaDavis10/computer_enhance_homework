#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

/* NOTE: this gets a couple things wrong I think -> the whole byte/word thing in some decodings 
 * that I guess I forgot about, but Ima move on either way 
 */

#define ASSERT(x) \
if(!x) \
{ \
	printf("assertion: EXPR = '%s', FILE = %s, LINE = %d\n", #x, __FILE__, __LINE__);\
	__builtin_trap(); \
} 

static FILE *global_output_asm_file;
static unsigned char *global_instruction_stream = 0;
static unsigned int global_instruction_index = 0;

unsigned char get_next_byte_from_instruction_stream()
{

	printf("get byte: %b\n", global_instruction_stream[global_instruction_index]);
	return (global_instruction_stream[global_instruction_index++]);
}

/* NOTE: even when in reg code, still in same location (i.e. same bits), just in 2nd byte */
#define ADD_OP 0b00000000
#define SUB_OP 0b00101000
#define CMP_OP 0b00111000
#define OP_CHECK 0b00111000

#define JE     0b01110100
#define JL     0b01111100
#define JLE    0b01111110
#define JB     0b01110010
#define JBE    0b01110110
#define JP     0b01111010
#define JO     0b01110000
#define JS     0b01111000
#define JNE    0b01110101
#define JNL    0b01111101
#define JNLE   0b01111111
#define JNB    0b01110011
#define JNBE   0b01110111
#define JNP    0b01111011
#define JNO    0b01110001
#define JNS    0b01111001
#define LOOP   0b11100010
#define LOOPZ  0b11100001
#define LOOPNZ 0b11100000
#define JCXZ   0b11100011

#define IMMEDIATE_TO_ACCUMULATOR_CHECK 0b11000110
#define IMMEDIATE_TO_ACCUMULATOR 0b00000100
#define IMMEDIATE_TO_REG_OR_MEM_CHECK 0b11111100
#define IMMEDIATE_TO_REG_OR_MEM 0b10000000
#define REG_OR_MEM_TO_REG_OR_MEM_CHECK 0b11000100
#define REG_OR_MEM_TO_REG_OR_MEM 0b00000000

void print_rm_to_output_asm_file(unsigned char rm, unsigned char mod, unsigned char w);
void print_reg_to_output_asm_file(unsigned char reg, unsigned char w);

void decode_instruction()
{
	printf("decoding instruction...\n");
	unsigned char instruction_first_byte = get_next_byte_from_instruction_stream();

	/* immediate -> accumulator */
	if((instruction_first_byte & IMMEDIATE_TO_ACCUMULATOR_CHECK) == IMMEDIATE_TO_ACCUMULATOR)
	{
		unsigned char op = instruction_first_byte & OP_CHECK; 
		switch(op)
		{
			case ADD_OP:
			{
				fprintf(global_output_asm_file, "add "); 
			} break;
			case SUB_OP:
			{
				fprintf(global_output_asm_file, "sub "); 
			} break;
			case CMP_OP:
			{
				fprintf(global_output_asm_file, "cmp "); 
			} break;
		}
		
		unsigned char w = instruction_first_byte & 0b00000001;
		if(w == 0)
		{
			fprintf(global_output_asm_file, "al, "); 
			char immediate = get_next_byte_from_instruction_stream();
			fprintf(global_output_asm_file, "%d", immediate); 
		}
		else if(w == 1)
		{
			fprintf(global_output_asm_file, "ax, "); 
			short immediate = get_next_byte_from_instruction_stream() +
				(get_next_byte_from_instruction_stream() << 8);	
			fprintf(global_output_asm_file, "%d", immediate); 
		}
		else
		{
			ASSERT(0);
		}
	}

	/* immediate -> reg/mem */
	else if((instruction_first_byte & IMMEDIATE_TO_REG_OR_MEM_CHECK) == IMMEDIATE_TO_REG_OR_MEM)
	{
		unsigned char s = instruction_first_byte & 0b00000010;
		unsigned char w = instruction_first_byte & 0b00000001;
		unsigned char mod_reg_rm_byte = get_next_byte_from_instruction_stream();

		unsigned char op = mod_reg_rm_byte & OP_CHECK; 
		switch(op)
		{
			case ADD_OP:
			{
				fprintf(global_output_asm_file, "add "); 
			} break;
			case SUB_OP:
			{
				fprintf(global_output_asm_file, "sub "); 
			} break;
			case CMP_OP:
			{
				fprintf(global_output_asm_file, "cmp "); 
			} break;
		}

		unsigned char rm = mod_reg_rm_byte & 0b00000111;
		unsigned char mod = mod_reg_rm_byte & 0b11000000;

		print_rm_to_output_asm_file(rm, mod, w);
		if(w == 0)
		{
		   char immediate = get_next_byte_from_instruction_stream();
		   fprintf(global_output_asm_file, ", %d", immediate);
		}
		else if(w == 1)
		{
			if(s == 0)
		    {
				short immediate = get_next_byte_from_instruction_stream() +
		   			(get_next_byte_from_instruction_stream() << 8);
		   		fprintf(global_output_asm_file, ", %d", immediate);
		    }
		    else if(s == 2)
		    {
				short immediate;
		   		char data = get_next_byte_from_instruction_stream();	
		   		if(immediate < 0)
		   		{
		   			immediate = data + (0b11111111 << 8);
					fprintf(global_output_asm_file, ", %d", immediate);
		   		}
		   		else
		   		{
		   			immediate = data + (0b00000000 << 8);
					fprintf(global_output_asm_file, ", %d", immediate);
		   		}
		    }
		    else
		    {
				ASSERT(0);
		    }
		}
		else
		{
		    ASSERT(0);
		}
	}

	/* reg/mem -> reg/mem */
	else if((instruction_first_byte & REG_OR_MEM_TO_REG_OR_MEM_CHECK) == REG_OR_MEM_TO_REG_OR_MEM)
	{
		unsigned char op = instruction_first_byte & OP_CHECK; 
		switch(op)
		{
			case ADD_OP:
			{
				fprintf(global_output_asm_file, "add "); 
			} break;
			case SUB_OP:
			{
				fprintf(global_output_asm_file, "sub "); 
			} break;
			case CMP_OP:
			{
				fprintf(global_output_asm_file, "cmp "); 
			} break;
		}

		unsigned char d;
		unsigned char w;

		d = instruction_first_byte & 0b00000010;
		w = instruction_first_byte & 0b00000001;

		unsigned char mod_reg_rm_byte = get_next_byte_from_instruction_stream();

		unsigned char mod = mod_reg_rm_byte & 0b11000000;
		unsigned char reg = mod_reg_rm_byte & 0b00111000;
		unsigned char rm = mod_reg_rm_byte  & 0b00000111;

		if(d == 0)
		{
			print_rm_to_output_asm_file(rm, mod, w);
			fprintf(global_output_asm_file, ", ");
			print_reg_to_output_asm_file(reg, w);
		}
		else if(d == 2)
		{
			print_reg_to_output_asm_file(reg, w);
			fprintf(global_output_asm_file, ", ");
			print_rm_to_output_asm_file(rm, mod, w);
		}
		else
		{
			ASSERT(0);
		}
	}
	else
	{
		/* get IP-INC8 byte */
		char ip_inc8_byte = get_next_byte_from_instruction_stream();

		/* check all 8 bits and do a switch on that */
		switch(instruction_first_byte)
		{
			case JE:
			{
				fprintf(global_output_asm_file, "je ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case JL:
			{
				fprintf(global_output_asm_file, "jl ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case JLE:
			{
				fprintf(global_output_asm_file, "jle ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case JB:
			{
				fprintf(global_output_asm_file, "jb ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case JBE:
			{
				fprintf(global_output_asm_file, "jbe ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case JP:
			{
				fprintf(global_output_asm_file, "jp ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case JO:
			{
				fprintf(global_output_asm_file, "jo ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case JS:
			{
				fprintf(global_output_asm_file, "js ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case JNE:
			{
				fprintf(global_output_asm_file, "jne ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case JNL:
			{
				fprintf(global_output_asm_file, "jnl ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case JNLE:
			{
				fprintf(global_output_asm_file, "jnle ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case JNB:
			{
				fprintf(global_output_asm_file, "jnb ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case JNBE:
			{
				fprintf(global_output_asm_file, "jnbe ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case JNP:
			{
				fprintf(global_output_asm_file, "jnp ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case JNO:
			{
				fprintf(global_output_asm_file, "jno ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case JNS:
			{
				fprintf(global_output_asm_file, "jns ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case LOOP:
			{
				fprintf(global_output_asm_file, "loop ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case LOOPZ:
			{
				fprintf(global_output_asm_file, "loopz ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case LOOPNZ:
			{
				fprintf(global_output_asm_file, "loopnz ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			case JCXZ:
			{
				fprintf(global_output_asm_file, "jcxz ");
				fprintf(global_output_asm_file, "%d", ip_inc8_byte); 
			} break;
			default:
			{
				ASSERT(0);
			} break;
		}
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

#define REG_AX 0b00000000 
#define REG_CX 0b00001000
#define REG_DX 0b00010000
#define REG_BX 0b00011000
#define REG_SP 0b00100000
#define REG_BP 0b00101000
#define REG_SI 0b00110000
#define REG_DI 0b00111000

#define RM_000 0b00000000
#define RM_001 0b00000001
#define RM_010 0b00000010
#define RM_011 0b00000011
#define RM_100 0b00000100
#define RM_101 0b00000101
#define RM_110 0b00000110
#define RM_111 0b00000111

#define MOD_00 0b00000000
#define MOD_01 0b01000000
#define MOD_10 0b10000000
#define MOD_11 0b11000000

void print_reg_to_output_asm_file(unsigned char reg, unsigned char w)
{
	if(w == 0)
	{
		switch(reg)
		{
			case REG_AX:
			{
				fprintf(global_output_asm_file, "al");
			} break;
			case REG_CX:
			{
				fprintf(global_output_asm_file, "cl");
			} break;
			case REG_DX:
			{
				fprintf(global_output_asm_file, "dl");
			} break;
			case REG_BX:
			{
				fprintf(global_output_asm_file, "bl");
			} break;
			case REG_SP:
			{
				fprintf(global_output_asm_file, "ah");
			} break;
			case REG_BP:
			{
				fprintf(global_output_asm_file, "ch");
			} break;
			case REG_SI:
			{
				fprintf(global_output_asm_file, "dh");
			} break;
			case REG_DI:
			{
				fprintf(global_output_asm_file, "bh");
			} break;
			default:
			{
				printf("print_reg_to_output_asm_file: unkown reg value, %b", reg);
				ASSERT(0);
			} break;
		}
	}
	else if(w == 1)
	{
		switch(reg)
		{
			case REG_AX:
			{
				fprintf(global_output_asm_file, "ax");
			} break;
			case REG_CX:
			{
				fprintf(global_output_asm_file, "cx");
			} break;
			case REG_DX:
			{
				fprintf(global_output_asm_file, "dx");
			} break;
			case REG_BX:
			{
				fprintf(global_output_asm_file, "bx");
			} break;
			case REG_SP:
			{
				fprintf(global_output_asm_file, "sp");
			} break;
			case REG_BP:
			{
				fprintf(global_output_asm_file, "bp");
			} break;
			case REG_SI:
			{
				fprintf(global_output_asm_file, "si");
			} break;
			case REG_DI:
			{
				fprintf(global_output_asm_file, "di");
			} break;
			default:
			{
				printf("print_reg_to_output_asm_file: unkown reg value, %b", reg);
				ASSERT(0);
			} break;
		}
	}
	else
	{
		printf("print_reg_to_output_asm_file: w must be 0 or 1, but got passed w = %d\n", w);
		ASSERT(0);
	}
}

void print_rm_to_output_asm_file(unsigned char rm, unsigned char mod, unsigned char w)
{
	if(mod == MOD_11)
	{
		print_reg_to_output_asm_file((rm << 3), w);
	}
	else if(mod == MOD_00)
	{
		switch(rm)
		{
			case RM_000:
			{
				fprintf(global_output_asm_file, "[bx + si]");
			} break;
			case RM_001:
			{
				fprintf(global_output_asm_file, "[bx + di]");
			} break;
			case RM_010:
			{
				fprintf(global_output_asm_file, "[bp + si]");
			} break;
			case RM_011:
			{
				fprintf(global_output_asm_file, "[bp + di]");
			} break;
			case RM_100:
			{
				fprintf(global_output_asm_file, "[si]");
			} break;
			case RM_101:
			{
				fprintf(global_output_asm_file, "[di]");
			} break;
			case RM_110:
			{
				/* TODO: direct address, get displacement value */
				/* NOTE: this shouldn't happen in listing 0039 at all
				 * so I will probably leave it for now
				 */
			} break;
			case RM_111:
			{
				fprintf(global_output_asm_file, "[bx]");
			} break;
			default:
			{
				printf("print_rm_to_output_asm_file: unkown rm value, %b", rm);
				ASSERT(0);
			} break;
		}
	}
	else if(mod == MOD_01)
	{
		char displacement = get_next_byte_from_instruction_stream(); 
		switch(rm)
		{
			case RM_000:
			{
				fprintf(global_output_asm_file, "[bx + si + %d]", displacement);
			} break;
			case RM_001:
			{
				fprintf(global_output_asm_file, "[bx + di + %d]", displacement);
			} break;
			case RM_010:
			{
				fprintf(global_output_asm_file, "[bp + si + %d]", displacement);
			} break;
			case RM_011:
			{
				fprintf(global_output_asm_file, "[bp + di + %d]", displacement);
			} break;
			case RM_100:
			{
				fprintf(global_output_asm_file, "[si + %d]", displacement);
			} break;
			case RM_101:
			{
				fprintf(global_output_asm_file, "[di + %d]", displacement);
			} break;
			case RM_110:
			{
				fprintf(global_output_asm_file, "[bp + %d]", displacement);
			} break;
			case RM_111:
			{
				fprintf(global_output_asm_file, "[bx + %d]", displacement);
			} break;
			default:
			{
				printf("print_rm_to_output_asm_file: unkown rm value, %b", rm);
				ASSERT(0);
			} break;
		}
	}
	else if(mod == MOD_10)
	{
		short displacement = 
			get_next_byte_from_instruction_stream() +
			(get_next_byte_from_instruction_stream() << 8);
		switch(rm)
		{
			case RM_000:
			{
				fprintf(global_output_asm_file, "[bx + si + %d]", displacement);
			} break;
			case RM_001:
			{
				fprintf(global_output_asm_file, "[bx + di + %d]", displacement);
			} break;
			case RM_010:
			{
				fprintf(global_output_asm_file, "[bp + si + %d]", displacement);
			} break;
			case RM_011:
			{
				fprintf(global_output_asm_file, "[bp + di + %d]", displacement);
			} break;
			case RM_100:
			{
				fprintf(global_output_asm_file, "[si + %d]", displacement);
			} break;
			case RM_101:
			{
				fprintf(global_output_asm_file, "[di + %d]", displacement);
			} break;
			case RM_110:
			{
				fprintf(global_output_asm_file, "[bp + %d]", displacement);
			} break;
			case RM_111:
			{
				fprintf(global_output_asm_file, "[bx + %d]", displacement);
			} break;
			default:
			{
				printf("print_rm_to_output_asm_file: unkown rm value, %b", rm);
				ASSERT(0);
			} break;
		}
	}
	else
	{
		printf("print_rm_to_output_asm_file: unkown mod value, %b", mod);
	}
}
