#!/bin/bash


user=${user:-"LaiYang"}


if [ $user == "LaiYang" ]
then
	echo "Welcome! LaiYang."
	sed -i -e "s/sysC-2.3.0/systemc-2.3.0/g" ./Makefile
elif [ "$user" == "JHLin" ]
then
	echo "Welcome! J.H. Lin."
	sed -i -e "s/systemc-2.3.0/sysC-2.3.0/g" Makefile

fi


