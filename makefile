:set ft=make

os_info: os_info_enumeration.c
	gcc -o os_info os_info_enumeration.c
