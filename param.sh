#!/bin/bash

change=${change:-1}
is_config=${is_config:-0}
is_exec=${is_exec:=0}
is_run=${is_run:=0}

#declare -A pA

if [ "$change" -eq 1 ]; then
	prefix=wl_die
	pt=2
	p_start[0]=0; p_end[0]=5; pA0=( 100 150 200 250 300 )
	p_start[1]=0; p_end[1]=5; pA1=( 100 125 150 175 200 )


	Param_Small_Shell()
	{
		local p=("${!2}")
		
		wl=$(awk "BEGIN {printf \"%.3f\",$wl_base*${p[0]}/100}")
		dn=$(awk "BEGIN {printf \"%.0f\",$dn_base*${p[1]}/100}")
		#dn=${p[1]}
		block_per_die=$(($block_per_die_base*$dn_base/$dn))

		#echo "wl=" $wl ", dn=" $dn
	}

	if [ "$is_config" -eq 1 ]
	then
		#use_standard_config=1
		use_standard_config=0
		
		CF(){
			local name=$1
			local p=("${!2}")

			#echo "wl=" $wl ", dn=" $dn

			#R=$(awk "BEGIN {printf \"%.3f\",$wl_base*$wl/100}")
			cp "$config_origin" "$config_dir"/ssd_typedef$name.h
			sed -i -e "s/BLOCK_PER_DIE ($block_per_die_base \/ PLANE_PER_DIE)/BLOCK_PER_DIE ($block_per_die \/ PLANE_PER_DIE)/g" "$config_dir"/ssd_typedef$name.h
			sed -i -e "s/MLC_PROGRAM_LATENCY ((msec_t) $wl_base)/MLC_PROGRAM_LATENCY ((msec_t) $wl)/g" "$config_dir"/ssd_typedef$name.h
		}
	elif [ "$is_exec" -eq 1 ]
	then
		use_standard_exec=0
		EF(){
			local name=$1
			local p=("${!2}")
			
			make -f make_script config_file="$config_dir"/ssd_typedef$name.h
			echo "cp ./run.x ./"$exec_dir"/run$name.x"
			cp ./run.x ./"$exec_dir"/run$name.x
		}
	elif [ "$is_run" -eq 1 ]
	then
		use_standard_run=0
		RF(){
			local name=$1
			local p=("${!2}")
			local trace_info=("${!3}")
			local workload_short=${trace_info[0]}
			local workload=${trace_info[1]}
			local trace_device_number=${trace_info[2]}

			echo "./"$exec_dir"/run$name.x $workload $trace_device_number $dn > "$out_dir"/$workload_short$name 2> "$out_dir"/stderr/$workload_short$name &"
	        ./"$exec_dir"/run$name.x $workload $trace_device_number $dn > "$out_dir"/$workload_short$name 2> "$out_dir"/stderr/$workload_short$name &
		}
	fi
elif [ "$change" -eq 4 ]; then
	prefix=el_die
	pt=2
	p_start[0]=0; p_end[0]=5; pA0=( 100 150 200 250 300 )
	p_start[1]=0; p_end[1]=5; pA1=( 100 125 150 175 200 )


	Param_Small_Shell()
	{
		local p=("${!2}")
		
		el=$(awk "BEGIN {printf \"%.3f\",$el_base*${p[0]}/100}")
		dn=$(awk "BEGIN {printf \"%.0f\",$dn_base*${p[1]}/100}")
		block_per_die=$(($block_per_die_base*$dn_base/$dn))

		#echo "wl=" $wl ", dn=" $dn
	}

	if [ "$is_config" -eq 1 ]
	then
		use_standard_config=1
	elif [ "$is_exec" -eq 1 ]
	then
		use_standard_exec=1
	elif [ "$is_run" -eq 1 ]
	then
		use_standard_run=1
	fi
elif [ "$change" -eq 5 ]; then
	prefix=bs_die
	pt=2
	p_start[0]=0; p_end[0]=4; pA0=( 100 200 400 800 )
	p_start[1]=0; p_end[1]=5; pA1=( 100 125 150 175 200 )


	Param_Small_Shell()
	{
		local p=("${!2}")
		
		bs=$(awk "BEGIN {printf \"%.0f\",$bs_base*${p[0]}/100}")
		dn=$(awk "BEGIN {printf \"%.0f\",$dn_base*${p[1]}/100}")
		block_per_die=$(($block_per_die_base*$dn_base*$bs_base/$dn/$bs))

		#echo "wl=" $wl ", dn=" $dn
	}

	if [ "$is_config" -eq 1 ]
	then
		use_standard_config=1
	elif [ "$is_exec" -eq 1 ]
	then
		use_standard_exec=1
	elif [ "$is_run" -eq 1 ]
	then
		use_standard_run=1
	fi
elif [ "$change" -eq 7 ]; then
	prefix=wl_die
	pt=2
	p_start[0]=0; p_end[0]=5; pA0=( 100 150 200 300 400 )
	p_start[1]=0; p_end[1]=5; pA1=( 100 125 150 175 200 )


	Param_Small_Shell()
	{
		local p=("${!2}")

		rl=0.003

		wl=$(awk "BEGIN {printf \"%.3f\",$wl_base*${p[0]}/60/100}")
		dn=$(awk "BEGIN {printf \"%.0f\",$dn_base*${p[1]}/100}")
		block_per_die=$(($block_per_die_base*$dn_base/$dn))

		#echo "wl=" $wl ", dn=" $dn
	}

	if [ "$is_config" -eq 1 ]
	then
		use_standard_config=1
	elif [ "$is_exec" -eq 1 ]
	then
		use_standard_exec=1
	elif [ "$is_run" -eq 1 ]
	then
		use_standard_run=1
	fi
elif [ "$change" -eq 8 ]; then
	prefix=rwd
	pt=3
	p_start[0]=0; p_end[0]=2; pA0=( 100 200 )
	p_start[1]=0; p_end[1]=2; pA1=( 100 200 )
	p_start[2]=0; p_end[2]=2; pA2=( 100 200 )


	Param_Small_Shell()
	{
		local p=("${!2}")


		wl=$(awk "BEGIN {printf \"%.3f\",$wl_base*${p[0]}/100}")
		rl=$(awk "BEGIN {printf \"%.3f\",$rl_base*${p[1]}/100}")
		dn=$(awk "BEGIN {printf \"%.0f\",$dn_base*${p[2]}/100}")
		block_per_die=$(($block_per_die_base*$dn_base/$dn))

		#echo "wl=" $wl ", dn=" $dn
	}

	if [ "$is_config" -eq 1 ]
	then
		use_standard_config=1
	elif [ "$is_exec" -eq 1 ]
	then
		use_standard_exec=1
	elif [ "$is_run" -eq 1 ]
	then
		use_standard_run=1
	fi
fi

Param_Shell()
{
	local wl_base=0.600
	local el_base=3.000
	local rl_base=0.090
	local dn_base=64
	local ch_base=8
	
	local bs_base=128
	local block_per_die_base=8192
	
	#local wl_div=100
	#local el_div=100
	#local bs_div=1
	#local dn_div=1
	#local ch_div=1

	local wl=$wl_base
	local el=$el_base
	local rl=$rl_base
	local dn=$dn_base
	local ch=$ch_base
	local bs=$bs_base
	local block_per_die=$block_per_die_base
	
	local _A=("${!2}")
	

	Param_Small_Shell $1 _A[@]


	if [ "$is_config" -eq 1 ]
	then
		if [ "$use_standard_config" -eq 1 ]
		then
			Standard_Config $1 _A[@] 
		else
			CF $1 _A[@]
		fi
	elif [ "$is_exec" -eq 1 ]
	then
		if [ "$use_standard_exec" -eq 1 ]
		then
			Standard_Exec $1 _A[@]
		else
			EF $1 _A[@]
		fi
	elif [ "$is_run" -eq 1 ]
	then
		local _trace_info=("${!3}")

		if [ "$use_standard_run" -eq 1 ]
		then
			Standard_Run $1 _A[@] _trace_info[@]
		else
			RF $1 _A[@] _trace_info[@]
		fi
	fi
}


Standard_Config()
{
	local name=$1
	local p=("${!2}")

	cp "$config_origin" "$config_dir"/ssd_typedef$name.h
	sed -i -e "s/BLOCK_PER_DIE ($block_per_die_base \/ PLANE_PER_DIE)/BLOCK_PER_DIE ($block_per_die \/ PLANE_PER_DIE)/g" "$config_dir"/ssd_typedef$name.h
	sed -i -e "s/MLC_PROGRAM_LATENCY ((msec_t) $wl_base)/MLC_PROGRAM_LATENCY ((msec_t) $wl)/g" "$config_dir"/ssd_typedef$name.h
	sed -i -e "s/#define MLC_READ_LATENCY ((msec_t) $rl_base)/#define MLC_READ_LATENCY ((msec_t) $rl)/g" "$config_dir"/ssd_typedef$name.h
	sed -i -e "s/MLC_ERASE_LATENCY ((msec_t) $el_base)/MLC_ERASE_LATENCY ((msec_t) $el)/g" "$config_dir"/ssd_typedef$name.h
	sed -i -e "s/PAGE_PER_BLOCK $bs_base/PAGE_PER_BLOCK $bs/g" "$config_dir"/ssd_typedef$name.h
	sed -i -e "s/#define SSD_CHANNEL_NUMBER $ch_base/#define SSD_CHANNEL_NUMBER $ch/g" "$config_dir"/ssd_typedef$name.h
}

Standard_Exec(){
	local name=$1
	local p=("${!2}")
	
	make -f make_script config_file="$config_dir"/ssd_typedef$name.h
	echo "cp ./run.x ./"$exec_dir"/run$name.x"
	cp ./run.x ./"$exec_dir"/run$name.x
}

Standard_Run(){
	local name=$1
	local p=("${!2}")
	local trace_info=("${!3}")
	local workload_short=${trace_info[0]}
	local workload=${trace_info[1]}
	local trace_device_number=${trace_info[2]}

	echo "./"$exec_dir"/run$name.x $workload $trace_device_number $dn > "$out_dir"/$workload_short$name 2> "$out_dir"/stderr/$workload_short$name &"
	./"$exec_dir"/run$name.x $workload $trace_device_number $dn > "$out_dir"/$workload_short$name 2> "$out_dir"/stderr/$workload_short$name &
}



