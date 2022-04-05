#!/bin/bash

if [ $# -ne 2 ]
then
	{
		echo "Invalid arguments count! Correct count is 2."
		echo "First argument is source file name. "
		echo "Second argument is compiled file name."
	} >&2
	exit 1
fi

if [ ! -f $1 ]
then
	echo "First argument should be existing file name." >&2
	exit 1	
fi

gcc $1 -o "$2.exe" && "./$2.exe"
exit $?
