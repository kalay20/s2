

workload_src=(/home/r01/gengyouchen/ssd_trace/ms_exchange_server.trace /home/r01/gengyouchen/ssd_trace/msn_fs.trace /home/r01/gengyouchen/ssd_trace/ms_live_maps.trace /home/r01/gengyouchen/ssd_trace/MSRC-io-traces-ascii/usr_1.trace /home/r01/gengyouchen/ssd_trace/MSRC-io-traces-ascii/src1_0.trace /home/r01/gengyouchen/ssd_trace/synthetic_overload_full.trace)
workload_burst=(/home/r03/sin19682004/test/simulator2/trace/mse_burst.trace /home/r03/sin19682004/test/simulator2/trace/msn_burst.trace /home/r03/sin19682004/test/simulator2/trace/synf_burst.trace /home/r01/gengyouchen/ssd_trace/ms_live_maps.trace /home/r01/gengyouchen/ssd_trace/MSRC-io-traces-ascii/usr_1.trace /home/r01/gengyouchen/ssd_trace/MSRC-io-traces-ascii/src1_0.trace)
#workload_short=(mse msn msl usr1 src10 synf_b)
workload_short=(mse_b msn_b synf_b msl usr1 src10)
workload_dst=( mse msn msl usr1 src10 synf )
workload_dir=

for (( i=0; i<6; i++ ))
do
	rsync -avP "${workload_src["$i"]}" "$workload_dir""${workload_dst["$i"]}"
done



