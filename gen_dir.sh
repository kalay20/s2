#!/bin/bash


prefix=(wl_die block_size_config 3_factor el_die bs_die)
d1=( config exec out )


mkdir trace

for (( i=0; i<3; i++ ))
do
	mkdir "${d1["$i"]}"
	for (( j=0; j<5; j++ ))
	do
		mkdir "${d1["$i"]}"/"${prefix["$j"]}"
		if [ "${d1["$i"]}"==out ]
		then
			mkdir "${d1["$i"]}"/"${prefix["$j"]}"/stderr
		fi
	done
done










