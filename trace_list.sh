#!/bin/bash



wd_start=${wd_start:-0}
wd_end=${wd_end:-6}
wd_bool_default=(1 0 0 0 0 0)
wd_bool=("${wd_bool[@]:-${wd_bool_default[@]}}")
workload_prefix=( mse msn msl usr1 src10 synf )
workload_origin_path=( \
	/home/r01/gengyouchen/ssd_trace/ms_exchange_server.trace \
	/home/r01/gengyouchen/ssd_trace/msn_fs.trace \
	/home/r01/gengyouchen/ssd_trace/ms_live_maps.trace \
	/home/r01/gengyouchen/ssd_trace/MSRC-io-traces-ascii/usr_1.trace \
	/home/r01/gengyouchen/ssd_trace/MSRC-io-traces-ascii/src1_0.trace \
	/home/r01/gengyouchen/ssd_trace/synthetic_overload_full.trace \
)
#workload_burst_path_prefix=/home/r03/sin19682004/test/simulator2/trace



#wd_reg_cnt=0
#_Register_Trace()
#{
	
#}

#Register_Trace()
#{
#	_Register_Trace 1 
#}














workload=()
workload_short=()

for (( wn="$wd_start"; wn<"$wd_end"; wn++ ))
do
	if [ "${wd_bool["$wn"]}" -eq 1 ]
	then
		workload["$wn"]="${workload_origin_path["$wn"]}"
		workload_short["$wn"]="${workload_prefix["$wn"]}"
	fi
done





