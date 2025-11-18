#!/usr/bin/bash
echo -e "\e[1;33mBuilding test..."
echo -e "\e[1;37mnasm -f elf64 write_loop.asm -o write_loop.o\e[0;31m"
if nasm -f elf64 write_loop.asm -o write_loop.o; then
	:
else
	echo -e "\e[1;31mBuild failed.\e[0;30m"
fi
echo -e "\e[1;37mgcc write_loop.o main.c -O1 -o test\e[0;31m"
if gcc write_loop.o main.c -O1 -o test; then
	echo -e "\e[1;32mBuild succeeded.\e[0;30m"
else
	echo -e "\e[1;31mBuild failed.\e[0;30m"
fi

echo -e "\e[1;33mBuilding NOP test..."
echo -e "\e[1;37mnasm -f elf64 write_loop.asm -o write_loop.o\e[0;31m"
if nasm -f elf64 write_loop.asm -o write_loop.o; then
	:
else
	echo -e "\e[1;31mBuild failed.\e[0;30m"
fi
echo -e "\e[1;37mgcc write_loop.o nop_test_main.c -O1 -o nop_test\e[0;31m"
if gcc write_loop.o nop_test_main.c -O1 -o nop_test; then
	echo -e "\e[1;32mBuild succeeded.\e[0;30m"
else
	echo -e "\e[1;31mBuild failed.\e[0;30m"
fi

echo -e "\e[1;33mBuilding Branch test..."
echo -e "\e[1;37mnasm -f elf64 branch.asm -o branch.o\e[0;31m"
if nasm -f elf64 branch.asm -o branch.o; then
	:
else
	echo -e "\e[1;31mBuild failed.\e[0;30m"
fi
echo -e "\e[1;37mgcc -fsanitize=address branch.o branch_test_main.c -o branch_test\e[0;31m"
if gcc -fsanitize=address branch.o branch_test_main.c -o branch_test; then
	echo -e "\e[1;32mBuild succeeded.\e[0;30m"
else
	echo -e "\e[1;31mBuild failed.\e[0;30m"
fi

echo -e "\e[1;33mBuilding multiple read test..."
echo -e "\e[1;37mnasm -f elf64 read_loop.asm -o read_loop.o\e[0;31m"
if nasm -f elf64 read_loop.asm -o read_loop.o; then
	:
else
	echo -e "\e[1;31mBuild failed.\e[0;30m"
fi
echo -e "\e[1;37mgcc -fsanitize=address read_loop.o multi_read_test_main.c -o multi_read_test\e[0;31m"
if gcc -fsanitize=address read_loop.o multi_read_test_main.c -o multi_read_test; then
	echo -e "\e[1;32mBuild succeeded.\e[0;30m"
else
	echo -e "\e[1;31mBuild failed.\e[0;30m"
fi

echo -e "\e[1;33mBuilding multiple write test..."
echo -e "\e[1;37mnasm -f elf64 multi_write.asm -o multi_write.o\e[0;31m"
if nasm -f elf64 multi_write.asm -o multi_write.o; then
	:
else
	echo -e "\e[1;31mBuild failed.\e[0;30m"
fi
echo -e "\e[1;37mgcc -fsanitize=address multi_write.o multi_write_test_main.c -o multi_write_test\e[0;31m"
if gcc -fsanitize=address multi_write.o multi_write_test_main.c -o multi_write_test; then
	echo -e "\e[1;32mBuild succeeded.\e[0;30m"
else
	echo -e "\e[1;31mBuild failed.\e[0;30m"
fi

echo -e "\e[1;32mDone.\e[0;30m"
