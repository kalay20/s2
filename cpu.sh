#!/bin/bash



cpu_or_job=${2:-1}
user=09021


if [ $cpu_or_job -eq 1 ]
then
	top -b -n1 | grep "09021" | awk '{print $9}' > cpu.dat
	#top -b -n1 | grep "Cpu(s)" | awk '{print $2 + $4}'
	./cpu.out "$1" "$cpu_or_job"
	if [ $? -eq 0 ]
	then
		exit 0
	else
		exit -1
	fi
elif [ $cpu_or_job -eq 2 ]
then
	top -b -n1 | grep -E "09021.*run_" | awk '{print $9}' > cpu.dat
	#top -b -n1 | grep "Cpu(s)" | awk '{print $2 + $4}'
	./cpu.out "$1" "$cpu_or_job"
	if [ $? -eq 0 ]
	then
		exit 0
	else
		exit -1
	fi
fi


