#!/bin/bash


user=${user:-"LaiYang"}


if [ $user == "LaiYang" ]
then
	echo "Welcome! LaiYang."
	sed -i -e "s/sysC-2.3.0/systemc-2.3.0/g" ./Makefile
elif [ "$user" == "" ]
then
	sed -i -e "s/systemc-2.3.0/sysC-2.3.0/g" Makefile

fi


