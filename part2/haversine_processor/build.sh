#!/usr/bin/bash

echo -e "\e[1;33mBuilding..."
echo -e "\e[1;37mgcc -std=c89 main.c -o haversine_processor -lm\e[0;31m"
if gcc -std=c89 main.c -fsanitize=address -o haversine_processor -lm; then
	echo -e "\e[1;32mBuild succeeded.\e[0;30m"
else
	echo -e "\e[1;31mBuild failed.\e[0;30m"
fi
