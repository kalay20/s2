#!/bin/bash

change=${change:-1}
is_run=${is_run:-1}

# Setting CPU monitor
if_test=${if_test:-0}
if [ "$if_test" -eq 0 ]; then
	delay=${delay:-0}
elif [ "$if_test" -eq 1 ]; then
	delay=${delay:-0}
	delay_small=${delay_small:-0}
	cpu_max=${cpu_max:-0}
elif [ "$if_test" -eq 2 ]; then
	delay=${delay:-0}
	delay_small=${delay_small:-0}
	job_max=${job_max:-0}
fi

source ./trace_list.sh
source ./param.sh

#prefix=""
config_dir="config/$prefix"
exec_dir="exec/$prefix"
out_dir="out/$prefix"

IssueCommand()
{
	local pt=$1
	local index=("${!2}")
	
	local p=()
	local name
	local t
	local r
	local i
	local pA
	local s
	
	local wn=$3
	local workload_info=()
	workload_info[0]=${workload_short[$wn]}
	workload_info[1]=${workload[$wn]}
	trace_device_number=0
	workload_info[2]=$trace_device_number

	for (( i=0; i<$pt; i++ ))
	do
		t=${index[$i]}; s="r=\${pA$i[\$t]}"; eval $s
		p[$i]=$r
		name=$name"_"$r
	done
	
	#echo "name=" $name
	if [ $if_test -eq 1 ]
	then
		sleep "$delay_small"
		bash ./cpu.sh "$cpu_max" "$if_test"
		#echo "cpu.sh return= $?"
		while [ $? -eq 255 ]
		do
			sleep "$delay"
			bash ./cpu.sh "$cpu_max" "$if_test"
		done
	elif [ $if_test -eq 2 ]
	then
		sleep "$delay_small"
		bash ./cpu.sh "$job_max" "$if_test"
		#echo "cpu.sh return= $?"
		while [ $? -eq 255 ]
		do
			sleep "$delay"
			bash ./cpu.sh "$job_max" "$if_test"
		done
	fi
	
	Param_Shell $name p[@] workload_info[@]
	
	if [ $if_test -eq 0 ]
	then
		sleep "$delay"
	fi
}

index=()
Re()
{
	#local index=${2:-()}
	if (( $1==$pt ))
	then
		#t1=${index[0]}; t2=${pA0[$t1]}
		#printf "%d" "$t2"
		#t1=${index[1]}; t2=${pA1[$t1]}
		#printf "_%d\n" "$t2"
		IssueCommand $pt index[@] $2
		return
	fi
	#m=0
	#s="p[0]=\${pA$m[\$]}"
	#eval $s
	local p_i
	for (( p_i=${p_start[$1]}; p_i<${p_end[$1]}; p_i++ ))
	do
		index[$1]=$p_i
		Re $(($1+1)) $2
	done
}

for (( wn="$wd_start"; wn<"$wd_end"; wn++ ))
do
	if [ "${wd_bool[$wn]}" -eq 1 ]; then
		Re 0 $wn
	fi
done












