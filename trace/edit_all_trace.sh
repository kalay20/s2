#!/bin/bash -v


wd_start=${wd_start:-0}
wd_end=${wd_end:-6}

workload_burst=( mse_b msn_b msl_b usr1_b src10_b synf_b )
workload_dst=( mse msn msl usr1 src10 synf )


g++ trace_edit.c
for (( wn="$wd_start"; wn<"$wd_end"; wn++ ))
do
	./a.out "${workload_dst["$wn"]}" "${workload_burst["$wn"]}"
done






