#!/bin/bash

prefix_end=7
prefix=(wl_die block_size_config 3_factor el_die bs_die ch_die rwd)
d1=( config exec out )


mkdir trace

for (( i=0; i<3; i++ ))
do
	mkdir "${d1["$i"]}"
	for (( j=0; j<$prefix_end; j++ ))
	do
		mkdir "${d1["$i"]}"/"${prefix["$j"]}"
		if [ "${d1["$i"]}" == "out" ]
		then
			mkdir "${d1["$i"]}"/"${prefix["$j"]}"/stderr
		fi
	done
done










