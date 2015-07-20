#!/bin/bash

change=${change:-1}
is_config=${is_config:-0}

#declare -A pA

if [ "$change" -eq 1 ]; then
	prefix=wl_die
	pt=2
	p_start[0]=0; p_end[0]=5; pA0=( 100 150 200 250 300 )
	p_start[1]=0; p_end[1]=5; pA1=( 64 80 96 112 128 )


	use_standard_config=1
	if [ "$is_config" -eq 1 ]
	then
		ct=4
		
		c1="R=\$(awk \"BEGIN {printf \\\"%.2f\\\",0.6*\${p[0]}/100}\")"
		c2="cp \"\$config_origin\" \"$config_dir\"/ssd_typedef_\${p[0]}_\${p[1]}.h"
		c3="sed -i -e \"s/BLOCK_PER_DIE (8192 \\/ PLANE_PER_DIE)/BLOCK_PER_DIE (1+\$((8192*64/\${p[1]})) \\/ PLANE_PER_DIE)/g\" \"$config_dir\"/ssd_typedef_\${p[0]}_\${p[1]}.h"
		c4="sed -i -e \"s/MLC_PROGRAM_LATENCY ((msec_t) 0.600)/MLC_PROGRAM_LATENCY ((msec_t) \$R)/g\" \"$config_dir\"/ssd_typedef_\${p[0]}_\${p[1]}.h"

		#R=$(awk "BEGIN {printf \"%.2f\",0.6*${wl[$i]}/100}")
		#cp "$config_origin" "$config_dir"/ssd_typedef_${wl["$i"]}_${dn["$j"]}.h
		#sed -i -e "s/BLOCK_PER_DIE (8192 \/ PLANE_PER_DIE)/BLOCK_PER_DIE (1+$((8192*64/${dn[$j]})) \/ PLANE_PER_DIE)/g" "$config_dir"/ssd_typedef_${wl["$i"]}_${dn["$j"]}.h
		#sed -i -e "s/MLC_PROGRAM_LATENCY ((msec_t) 0.600)/MLC_PROGRAM_LATENCY ((msec_t) $R)/g" "$config_dir"/ssd_typedef_${wl["$i"]}_${dn["$j"]}.h
	fi

: << 'END'
	
	use_standard_config=0
END


fi





