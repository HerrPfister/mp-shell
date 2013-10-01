***********************************
	Author: Matthew Pfister
	Project: Shell
	class: CS 385
	Date: 02/19/13
************************************


mpShell - Matthew Pfister's Shell
=================================


Run instructions
-----------------
 1. make
 2. ./mpShell

commands
--------
>mpShell works the same as the UNIX shell: 

	All standard commands that work in UNIX, work in mpShell.
		i.e. ls, ps, who, etc.

	Pipelining works the same as well, 
		i.e. cat makefile | grep c | grep g
			who | wc -l

	Redirecting works the same as well, 
		i.e. ls > test
			cat < test

	Complex commands work as well,
		i.e. ps -a | grep mpShell > log 
