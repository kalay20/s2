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
	p_start[1]=0; p_end[1]=5; pA1=( 64 80 96 112 128 )


	Param_Small_Shell()
	{
		local p=("${!2}")
		
		wl=$(awk "BEGIN {printf \"%.3f\",$wl_base*${p[0]}/100}")
		dn=${p[1]}
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

		#R=$(awk "BEGIN {printf \"%.2f\",0.6*${wl[$i]}/100}")
		#cp "$config_origin" "$config_dir"/ssd_typedef_${wl["$i"]}_${dn["$j"]}.h
		#sed -i -e "s/BLOCK_PER_DIE (8192 \/ PLANE_PER_DIE)/BLOCK_PER_DIE (1+$((8192*64/${dn[$j]})) \/ PLANE_PER_DIE)/g" "$config_dir"/ssd_typedef_${wl["$i"]}_${dn["$j"]}.h
		#sed -i -e "s/MLC_PROGRAM_LATENCY ((msec_t) 0.600)/MLC_PROGRAM_LATENCY ((msec_t) $R)/g" "$config_dir"/ssd_typedef_${wl["$i"]}_${dn["$j"]}.h
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
			Standard_Config $1
		else
			CF $1 _A[@]
		fi
	fi
}


Standard_Config()
{
	local name=$1

	cp "$config_origin" "$config_dir"/ssd_typedef$name.h
	sed -i -e "s/BLOCK_PER_DIE ($block_per_die_base \/ PLANE_PER_DIE)/BLOCK_PER_DIE ($block_per_die \/ PLANE_PER_DIE)/g" "$config_dir"/ssd_typedef$name.h
	sed -i -e "s/MLC_PROGRAM_LATENCY ((msec_t) $wl_base)/MLC_PROGRAM_LATENCY ((msec_t) $wl)/g" "$config_dir"/ssd_typedef$name.h
	sed -i -e "s/#define MLC_READ_LATENCY ((msec_t) $rl_base)/#define MLC_READ_LATENCY ((msec_t) $rl)/g" "$config_dir"/ssd_typedef$name.h
	sed -i -e "s/MLC_ERASE_LATENCY ((msec_t) $el_base)/MLC_ERASE_LATENCY ((msec_t) $el)/g" "$config_dir"/ssd_typedef$name.h
	sed -i -e "s/PAGE_PER_BLOCK $bs_base/PAGE_PER_BLOCK $bs/g" "$config_dir"/ssd_typedef$name.h
	sed -i -e "s/#define SSD_CHANNEL_NUMBER $ch_base/#define SSD_CHANNEL_NUMBER $ch/g" "$config_dir"/ssd_typedef$name.h
}




