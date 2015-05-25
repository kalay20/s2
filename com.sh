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



if [ "$change" -eq 1 ]; then
		for (( i="$wl_start"; i<"$wl_end"; i++ ))
		do
			for (( j="$dn_start"; j<"$dn_end"; j++ ))
			do
				make -f make_script config_file="$config_dir"/ssd_typedef_${wl["$i"]}_${dn["$j"]}.h
				echo "cp ./run.x ./"$exec_dir"/run_${wl["$i"]}_${dn["$j"]}.x"
				cp ./run.x ./"$exec_dir"/run_${wl["$i"]}_${dn["$j"]}.x
			done
		done
elif [ $change -eq 4 ] ; then
		for (( i="$el_start"; i<"$el_end"; i++ ))
		do
			for (( j="$dn_start"; j<"$dn_end"; j++ ))
			do
				make -f make_script config_file="$config_dir"/ssd_typedef_${el["$i"]}_${dn["$j"]}.h
				echo "cp ./run.x ./"$exec_dir"/run_${el["$i"]}_${dn["$j"]}.x"
				cp ./run.x ./"$exec_dir"/run_${el["$i"]}_${dn["$j"]}.x

				done
		done
elif [ $change -eq 5 ] ; then
		for (( i="$bs_start"; i<"$bs_end"; i++ ))
		do
			for (( j="$dn_start"; j<"$dn_end"; j++ ))
			do
				make -f make_script config_file="$config_dir"/ssd_typedef_${bs["$i"]}_${dn["$j"]}.h
				echo "cp ./run.x ./"$exec_dir"/run_${bs["$i"]}_${dn["$j"]}.x"
				cp ./run.x ./"$exec_dir"/run_${bs["$i"]}_${dn["$j"]}.x
			done
		done
elif [ $change -eq 6 ] ; then
	for (( i="$ch_start"; i<"$ch_end"; i++ ))
	do
		for (( j="$dn_start"; j<"$dn_end"; j++ ))
		do
			make -f make_script config_file="$config_dir"/ssd_typedef_${ch["$i"]}_${dn["$j"]}.h
			echo "cp ./run.x ./"$exec_dir"/run_${ch["$i"]}_${dn["$j"]}.x"
			cp ./run.x ./"$exec_dir"/run_${ch["$i"]}_${dn["$j"]}.x
		done
	done
fi
