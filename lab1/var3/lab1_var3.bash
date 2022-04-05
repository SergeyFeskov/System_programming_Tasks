#!/bin/bash

if [ $# -ne 2 ]; then
	echo "Скрипт принимает строго 2 параметра:"
	echo "1) строка, которую необходимо найти"
	echo "2) каталог, в файлах которого будет осуществляться поиск"
else
	if [ ! -d "$2" ]; then
		if [ ! -e "$2" ]; then
			echo "Ошибка: данного каталога не существует" >&2
		else
			echo "Ошибка: параметр 2 указывает не на каталог" >&2
		fi
	else
		touch for_sort
		for file in $(grep -F -l -r "$1" "$2") ; do
			wc -c "$file" >> for_sort
		done
		sort -n for_sort
		rm for_sort
	fi
fi
