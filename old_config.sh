#!/bin/bash


el_start=${el_start:-0}
el_end=${el_end:-5}
wl_start=${wl_start:-0}
wl_end=${wl_end:-5}
dn_start=${dn_start:-0}
dn_end=${dn_end:-5}
bs_start=${bs_start:-0}
bs_end=${bs_end:-4}
ch_start=${ch_start:-0}
ch_end=${ch_end:-5}


config_origin=ssd_typedef_normal.h
#config_dir=block_size_config

el=( 100 150 200 250 300 )
wl=( 100 150 200 250 300 )
dn=( 64 80 96 112 128 )
bs=( 128 256 512 1024 )
ch=( 1 2 4 8 16 )

change=${change:-1}

prefix=(wl_die block_size_config 3_factor el_die bs_die ch_die )
config_dir="config/${prefix["$change"-1]}"
exec_dir="exec/${prefix["$change"-1]}"
out_dir="out/${prefix["$change"-1]}"


if [ $change -eq 1 ]; then

	for (( i="$wl_start"; i<"$wl_end"; i++ ))
	do
		for (( j="$dn_start"; j<"$dn_end"; j++ ))
		do
			R=$(awk "BEGIN {printf \"%.2f\",0.6*${wl[$i]}/100}")
			cp "$config_origin" "$config_dir"/ssd_typedef_${wl["$i"]}_${dn["$j"]}.h
			sed -i -e "s/BLOCK_PER_DIE (8192 \/ PLANE_PER_DIE)/BLOCK_PER_DIE (1+$((8192*64/${dn[$j]})) \/ PLANE_PER_DIE)/g" "$config_dir"/ssd_typedef_${wl["$i"]}_${dn["$j"]}.h
			sed -i -e "s/MLC_PROGRAM_LATENCY ((msec_t) 0.600)/MLC_PROGRAM_LATENCY ((msec_t) $R)/g" "$config_dir"/ssd_typedef_${wl["$i"]}_${dn["$j"]}.h
			#./run.x "$workload" 0 ${dn["$j"]} > out/"$workload_short"_${wl["$i"]}_${dn["$j"]} 2> out/stderr/"$workload_short"_${wl["$i"]}_${dn["$j"]} &
		done
	done

elif [ $change -eq 2 ] ; then
	for (( i="$bs_start"; i<"$bs_end"; i++ ))
	do
		#R=$(awk "BEGIN {printf \"%.2f\",11.8*${wl[$i]}}")
		cp "$config_origin" "$config_dir"/ssd_typedef_${bs["$i"]}.h
		sed -i -e "s/PAGE_PER_BLOCK 516/PAGE_PER_BLOCK ${bs[$i]}/g" "$config_dir"/ssd_typedef_${bs["$i"]}.h
		sed -i -e "s/BLOCK_PER_DIE (2048 \/ PLANE_PER_DIE)/BLOCK_PER_DIE ($((2048*512/${bs[$i]})) \/ PLANE_PER_DIE)/g" "$config_dir"/ssd_typedef_${bs["$i"]}.h
	done
elif [ $change -eq 3 ] ; then
	for (( i="$el_start"; i<"$el_end"; i++ ))
	do
		for (( j="$wl_start"; j<"$wl_end"; j++ ))
		do
			for (( k="$bs_start"; k<"$bs_end"; k++ ))
			do
				R=$(awk "BEGIN {printf \"%.2f\",11.8*${wl[$j]}}")
				R2=$(awk "BEGIN {printf \"%.2f\",5.0*${el[$i]}}")
				cp "$config_origin" "$config_dir"/ssd_typedef_${el["$i"]}_${wl["$j"]}_${bs["$k"]}.h
				sed -i -e "s/MLC_ERASE_LATENCY ((msec_t) 5.000)/MLC_ERASE_LATENCY ((msec_t) $R2)/g" "$config_dir"/ssd_typedef_${el["$i"]}_${wl["$j"]}_${bs["$k"]}.h
				sed -i -e "s/(0.800 + 3.400 + 7.600)/($R)/g" "$config_dir"/ssd_typedef_${el["$i"]}_${wl["$j"]}_${bs["$k"]}.h
				sed -i -e "s/PAGE_PER_BLOCK 516/PAGE_PER_BLOCK ${bs[$k]}/g" "$config_dir"/ssd_typedef_${el["$i"]}_${wl["$j"]}_${bs["$k"]}.h
				sed -i -e "s/BLOCK_PER_DIE (2048 \/ PLANE_PER_DIE)/BLOCK_PER_DIE ($((2048*512/${bs[$k]})) \/ PLANE_PER_DIE)/g" "$config_dir"/ssd_typedef_${el["$i"]}_${wl["$j"]}_${bs["$k"]}.h
			done
		done
	done

elif [ $change -eq 4 ] ; then
	for (( i="$el_start"; i<"$el_end"; i++ ))
	do
		for (( j="$dn_start"; j<"$dn_end"; j++ ))
		do
			R2=$(awk "BEGIN {printf \"%.2f\",3.0*${el[$i]}/100}")
			cp "$config_origin" "$config_dir"/ssd_typedef_${el["$i"]}_${dn["$j"]}.h
			sed -i -e "s/BLOCK_PER_DIE (8192 \/ PLANE_PER_DIE)/BLOCK_PER_DIE (1+$((8192*64/${dn[$j]})) \/ PLANE_PER_DIE)/g" "$config_dir"/ssd_typedef_${el["$i"]}_${dn["$j"]}.h
			sed -i -e "s/MLC_ERASE_LATENCY ((msec_t) 3.000)/MLC_ERASE_LATENCY ((msec_t) $R2)/g" "$config_dir"/ssd_typedef_${el["$i"]}_${dn["$j"]}.h
		done
	done
elif [ $change -eq 5 ] ; then
	for (( i="$bs_start"; i<"$bs_end"; i++ ))
	do
		for (( j="$dn_start"; j<"$dn_end"; j++ ))
		do
			cp "$config_origin" "$config_dir"/ssd_typedef_${bs["$i"]}_${dn["$j"]}.h
			sed -i -e "s/PAGE_PER_BLOCK 128/PAGE_PER_BLOCK ${bs[$i]}/g" "$config_dir"/ssd_typedef_${bs["$i"]}_${dn["$j"]}.h
			sed -i -e "s/BLOCK_PER_DIE (8192 \/ PLANE_PER_DIE)/BLOCK_PER_DIE (1+$((8192*128*64/${bs[$i]}/${dn[$j]})) \/ PLANE_PER_DIE)/g" "$config_dir"/ssd_typedef_${bs["$i"]}_${dn["$j"]}.h
		done
	done
elif [ $change -eq 6 ] ; then
	for (( i="$ch_start"; i<"$ch_end"; i++ ))
	do
		for (( j="$dn_start"; j<"$dn_end"; j++ ))
		do
			cp "$config_origin" "$config_dir"/ssd_typedef_${ch["$i"]}_${dn["$j"]}.h
			sed -i -e "s/SSD_CHANNEL_NUMBER 8/SSD_CHANNEL_NUMBER ${ch[$i]}/g" "$config_dir"/ssd_typedef_${ch["$i"]}_${dn["$j"]}.h
			sed -i -e "s/BLOCK_PER_DIE (8192 \/ PLANE_PER_DIE)/BLOCK_PER_DIE (1+$((8192*64/${dn[$j]})) \/ PLANE_PER_DIE)/g" "$config_dir"/ssd_typedef_${ch["$i"]}_${dn["$j"]}.h
		done
	done
fi




