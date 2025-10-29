#!/usr/bin/bash

echo -e "\e[1;33mBuilding..."
echo -e "\e[1;37mgcc -ggdb -std=c89 test.c -o test -lm\e[0;31m"
if gcc -std=c89 -ggdb test.c -o test -lm; then
	echo -e "\e[1;32mBuild succeeded.\e[0;30m"
else
	echo -e "\e[1;31mBuild failed.\e[0;30m"
fi
