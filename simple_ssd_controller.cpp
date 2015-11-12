#include "simple_ssd_controller.h"

#include <algorithm>
#include <cstdlib>
#include <map>
#include <utility>
#include <vector>
#include <cmath>

#include "systemc.h"

#include "ssd_typedef.h"
#include "gyc_bus.h"

#include "simple_die_scheduler.h"
#include "simple_die_ftl.h"

#define jiawei_die

using namespace std;
using namespace sc_core;

// LaiYang
extern bool all_req_completed;
// end LaiYang

double geometric_mean(const std::vector<long double> &data) {
	long double m = 1.0;
	long long ex = 0;
	long double invN = static_cast<long double>(1.0) / data.size();
	for (int k = 0; k < data.size(); k++) {
		int i;
		double f1 = std::frexp(data[k],&i);
		m*=f1;
		ex+=i;
	}
	return std::pow(std::numeric_limits<long double>::radix,ex * invN) * std::pow(m,invN);
}


// LaiYang
void simple_ssd_controller::spiner(){
	size_t max_io = 0;
	ppn_t  last_io = 0;
	ppn_t  this_io;
	ppn_t  diff_io;

	
	wait(SC_ZERO_TIME);
	while( !all_req_completed ){
		wait(el);
		this_io = x_total_io(); diff_io = this_io - last_io; last_io = this_io;
		if( diff_io > max_io ) max_io = diff_io;
	}

	printf("Epoch -----------\n" );
	printf("Finish with total IO: %d, epoch length: %f sec\n", x_total_io(), el.to_seconds() );
	printf("Peak IOPS: %f\n", (double)max_io / el.to_seconds() );
	printf("Epoch -----------\n" );
}

ppn_t simple_ssd_controller::x_total_io(){
	ppn_t total_number_of_io_page_read = 0;
	ppn_t total_number_of_io_page_write = 0;

	for (int die_idx = 0; die_idx < die_schedulers.size(); die_idx++){
		total_number_of_io_page_read += die_schedulers[die_idx]->number_of_io_page_read;
		total_number_of_io_page_write += die_schedulers[die_idx]->number_of_io_page_write;
	}

	return total_number_of_io_page_write + total_number_of_io_page_read;
}
// end LaiYang

simple_ssd_controller::simple_ssd_controller (
		sc_module_name module_name,
		gyc_bus_pkt_mm* global_mm,
		int total_number_of_dice,
		int total_number_of_channels,
		int min_free_blocks_percentage,
		slc_policy_t slc_policy,
		double last,
		std::string trace_path) :
	sc_module(module_name),
	download_ch_port("download_ch_port"),
	upload_ch_port("upload_ch_port")
{

	// LaiYang
	req_complete_cnt=0;
	epoch_length = sc_time(10,SC_MS);
	epoch_time_now = sc_time(0,SC_MS);
	epoch_time_next = epoch_time_now + epoch_length;
	
	el=sc_time(100,SC_MS);
	//SC_THREAD(spiner)
	// end LaiYang
#ifdef GYC_PAPER_D_MONITOR
	m_mlc_clean_amount = 0.0;
	m_slc_clean_amount = 0.0;
#endif
	assert(global_mm);
	m_mm = global_mm;

	m_queue_size = 0;
	m_recv_buf = NULL;
	m_download_ch_id = -1;
	m_upload_ch_id = -1;
	m_download_ch_busy = false;
	m_upload_ch_busy = false;
#ifdef GYC_PAPER_SLC_THROTTLING
	m_slc_throttle_rate = 1.0;
#endif
	m_slc_policy = slc_policy;
	die_schedulers.resize(total_number_of_dice, NULL);
	die_controllers.resize(total_number_of_dice, NULL);
	
	first = 0;
	ch_array.resize(total_number_of_channels, NULL);
	for (int ch_idx = 0; ch_idx < ch_array.size(); ch_idx++) {
		ch_array[ch_idx] = new gyc_bus(sc_gen_unique_name("ch"));
		ch_array[ch_idx]->set_bus_id(ch_idx);
	}

	m_number_of_die = total_number_of_dice;
	m_die_idx = 0;
	/* create and initialize all die scheduler & FTL */
	/* configure each die FTL */
	die_ftls = new simple_die_ftl(sc_gen_unique_name("die_ftl"), total_number_of_dice);
	die_ftls->min_free_blocks_percentage = min_free_blocks_percentage;
	die_ftls->slc_policy = slc_policy;
	die_ftls->reset_data_layout();
	for (int die_idx = 0; die_idx < total_number_of_dice; die_idx++) {

		/* create each die scheduler */
		die_schedulers[die_idx] = new simple_die_scheduler(sc_gen_unique_name("die_sched"), m_mm, die_idx);

		/* create each die FTL */

		/* create each die controller */
		die_controllers[die_idx] = new simple_die_controller(sc_gen_unique_name("die_ctrl"));

		/* bind each die FTL to each die scheduler */
		die_schedulers[die_idx]->set_ftl(die_ftls);
		die_ftls->set_ftl_callback(die_schedulers[die_idx], die_idx);

		/* bind each die scheduler to this SSD controller */
		die_schedulers[die_idx]->set_callback(this);

		int ch_idx = die_idx % ch_array.size();
		int sched_dev_idx = (die_idx / ch_array.size()) * 2 + 0;
		int ctrl_dev_idx = (die_idx / ch_array.size()) * 2 + 1;

		/* bind each die scheduler to each die channel */
		die_schedulers[die_idx]->ch_port(*ch_array[ch_idx]);
		ch_array[ch_idx]->dev(*die_schedulers[die_idx]);
		die_schedulers[die_idx]->set_ch_id(ch_idx);
		die_schedulers[die_idx]->set_ch_dev_id(sched_dev_idx);

		/* bind each die controller to each die channel */
		die_controllers[die_idx]->belonged_channel(*ch_array[ch_idx]);
		ch_array[ch_idx]->dev(*die_controllers[die_idx]);
		die_controllers[die_idx]->set_ch_id(ch_idx);
		die_controllers[die_idx]->set_ch_dev_id(ctrl_dev_idx);
	}

	set_number_of_dice(total_number_of_dice);
	set_last_path(trace_path);
	set_last_info(last);
	if(last == 0){ //load from which line
		set_last_time_for_save(sc_time(0, SC_MS));
	}
	else if(last > 0) { //save how many request
		set_last_time_for_save(sc_time(last, SC_MS));
	}
	else{
		set_last_time_for_save(sc_time(0, SC_MS));
	}

#ifdef DEADLINE_AWARE_SLC_POLICY_SELF_TUNING
	m_tla_next_targ = DEADLINE_AWARE_SLC_POLICY_DEADLINE;
	m_tla_next_pr99 = NULL_MSEC;
	m_tla_prev_1_targ = 0;
	m_tla_prev_1_pr99 = 0;
	m_tla_prev_2_targ = 999999;
	m_tla_prev_2_pr99 = 999999;
	m_tla_log.open("tla_log.txt");
#endif

	_update_deadline_for_all_die();

#if 0
	/* print debug information */
	cout << "Finish creating and initializing all die scheduler & FTL" << endl;
	cout << "Each die has been applied the same setting as follow:" << endl;
	cout << "\tmin_free_blocks_percentage=" << min_free_blocks_percentage << endl;
	cout << "\tslc_policy=";
	switch (slc_policy) {
		case NO_SLC_POLICY:
			cout << "NO_SLC_POLICY";
			break;
		case QUEUE_AWARE_SLC_POLICY:
			cout << "QUEUE_AWARE_SLC_POLICY";
			break;
		case ALWAYS_SLC_POLICY:
			cout << "ALWAYS_SLC_POLICY";
			break;
		default:
			assert(0);
			break;
	}
	cout << endl;
#endif

}

void simple_ssd_controller::debug_check_all_req_completed() const {

	ppn_t total_number_of_io_page_read = 0;
	ppn_t total_number_of_io_page_write = 0;
	ppn_t total_number_of_io_slc_page_write = 0;
	ppn_t total_number_of_benefited_page_write = 0;
	ppn_t total_number_of_truely_benefited_page_write = 0;
	ppn_t total_number_of_io_mlc_page_write = 0;
	ppn_t total_number_of_gc_page_read = 0;
	ppn_t total_number_of_gc_page_write = 0;
	pbn_t total_number_of_gc_block_erase = 0;
	pbn_t total_number_of_gc_slc_block_erase = 0;
	pbn_t total_number_of_gc_mlc_block_erase = 0;

	pbn_t total_number_of_physical_blocks = 0;
	
	// LaiYang
	size_t total_number_of_gc=0;
	int total_number_live_copy_gc=0;

	ppn_t total_number_of_gc_page_read_f = 0;
	ppn_t total_number_of_gc_page_read_b = 0;
	ppn_t total_number_of_gc_page_erase_f = 0;
	ppn_t total_number_of_gc_page_erase_b = 0;
	ppn_t total_number_of_gc_page_write_f = 0;
	ppn_t total_number_of_gc_page_write_b = 0;
	// end LaiYang
	// Ana
	cout << "**************************** Request per Die ****************************"<<endl;
	// end Ana
	for (int die_idx = 0; die_idx < die_schedulers.size(); die_idx++) {

#if 0
		cout << "For die " << die_idx << ":" << endl;
/*
		cout << "\tnumber_of_io_page_read: " << die_schedulers[die_idx]->number_of_io_page_read << endl;
		cout << "\tnumber_of_io_page_write: " << die_schedulers[die_idx]->number_of_io_page_write << endl;
		cout << "\tnumber_of_gc_page_read: " << die_schedulers[die_idx]->number_of_gc_page_read << endl;
		cout << "\tnumber_of_gc_page_write: " << die_schedulers[die_idx]->number_of_gc_page_write << endl;
	 	cout << "\tnumber_of_gc_page_erase: " << die_schedulers[die_idx]->number_of_gc_block_erase << endl;
*/
	 	cout << "\tnumber_of_gc_slc_block_erase: " << die_schedulers[die_idx]->number_of_gc_slc_block_erase << endl;
	 	cout << "\tnumber_of_gc_mlc_block_erase: " << die_schedulers[die_idx]->number_of_gc_mlc_block_erase << endl;
#endif

		total_number_of_io_page_read += die_schedulers[die_idx]->number_of_io_page_read;
		total_number_of_io_page_write += die_schedulers[die_idx]->number_of_io_page_write;
		total_number_of_io_slc_page_write += die_schedulers[die_idx]->number_of_io_slc_page_write;
		total_number_of_benefited_page_write += die_schedulers[die_idx]->number_of_benefited_page_write;
		total_number_of_truely_benefited_page_write += die_schedulers[die_idx]->number_of_truely_benefited_page_write;
		total_number_of_io_mlc_page_write += die_schedulers[die_idx]->number_of_io_mlc_page_write;
		total_number_of_gc_page_read += die_schedulers[die_idx]->number_of_gc_page_read;
		total_number_of_gc_page_write += die_schedulers[die_idx]->number_of_gc_page_write;
		total_number_of_gc_block_erase += die_schedulers[die_idx]->number_of_gc_block_erase;
		total_number_of_gc_slc_block_erase += die_schedulers[die_idx]->number_of_gc_slc_block_erase;
		total_number_of_gc_mlc_block_erase += die_schedulers[die_idx]->number_of_gc_mlc_block_erase;

		total_number_of_physical_blocks += BLOCK_PER_DIE;
                // LaiYang
                total_number_of_gc += die_ftls->number_of_gc_trigger[die_idx];
                for( int i=0; i< die_ftls->number_of_gc_trigger[die_idx]; i++ ){
                        total_number_live_copy_gc += die_ftls->live_page_copy_per_gc[die_idx][i];
                }
                total_number_of_gc_page_read_f += die_schedulers[die_idx]->number_of_gc_page_read_f;
                total_number_of_gc_page_read_b += die_schedulers[die_idx]->number_of_gc_page_read_b;
                total_number_of_gc_page_write_f += die_schedulers[die_idx]->number_of_gc_page_write_f;
                total_number_of_gc_page_write_b += die_schedulers[die_idx]->number_of_gc_page_write_b;
                total_number_of_gc_page_erase_f += die_schedulers[die_idx]->number_of_gc_page_erase_f;	
		total_number_of_gc_page_erase_b += die_schedulers[die_idx]->number_of_gc_page_erase_b;
		// end LaiYang
		// Ana
		cout << "Die: " << die_idx << "\t# of f_gc: " << die_schedulers[die_idx]->number_of_gc_page_erase_f;                
                cout << "\t# of b_gc: " << die_schedulers[die_idx]->number_of_gc_page_erase_b << "\t\tPage_r: ";
		cout << die_schedulers[die_idx]->number_of_io_page_read << "\tPage_w: " << die_schedulers[die_idx]->number_of_io_page_write << endl;
		// end Ana

	}
	// Ana
	cout << "************************** end of request per die ************************" << endl;
	// end Ana

	// LaiYang
        cout << " total gc: " << total_number_of_gc << endl;
        cout << " avg live page copy per gc: " << (double)total_number_live_copy_gc / (double)total_number_of_gc << endl;
        // end LaiYang

	cout << "For entire SSD:" << endl;
	cout << "\t# of User Page Read: " << total_number_of_io_page_read << endl;
	cout << "\t# of User Page Write: " << total_number_of_io_page_write << endl;
	cout << "\t\t# of User SLC Page Write: " << total_number_of_io_slc_page_write << " (" << static_cast<long double>(total_number_of_io_slc_page_write) * 100 / total_number_of_io_page_write << "%)" << endl;
	cout << "\t\t# of User MLC Page Write: " << total_number_of_io_mlc_page_write << " (" << static_cast<long double>(total_number_of_io_mlc_page_write) * 100 / total_number_of_io_page_write << "%)" << endl;
	cout << "\t# of GC Page Read: " << total_number_of_gc_page_read << endl;
        cout << "\t\t# of Foreground: " << total_number_of_gc_page_read_f << ",\t\t# of Background: " << total_number_of_gc_page_read_b << endl;
	cout << "\t# of GC Page Write: " << total_number_of_gc_page_write << endl;
        cout << "\t\t# of Foreground: " << total_number_of_gc_page_write_f << ",\t\t# of Background: " << total_number_of_gc_page_write_b << endl;
	cout << "\t# of GC Block Erase: " << total_number_of_gc_block_erase << endl;
        cout << "\t\t# of Foreground: " << total_number_of_gc_page_erase_f << ",\t\t# of Background: " << total_number_of_gc_page_erase_b << endl;
	cout << "\t\t# of GC SLC Block Erase: " << total_number_of_gc_slc_block_erase << " (" << static_cast<long double>(total_number_of_gc_slc_block_erase) * 100 / total_number_of_gc_block_erase << "%)" << endl;
	cout << "\t\t# of GC MLC Block Erase: " << total_number_of_gc_mlc_block_erase << " (" << static_cast<long double>(total_number_of_gc_mlc_block_erase) * 100 / total_number_of_gc_block_erase << "%)" << endl;

	/* For my master thesis, report some easily understandable statistic */
	cout << "For my master thesis: " << endl;
	cout << dec << fixed;
	cout << "\tPolicy: ";
	switch (m_slc_policy) {
		case NO_SLC_POLICY:
			cout << "NO_SLC_POLICY";
			break;
		case QUEUE_AWARE_SLC_POLICY:
			cout << "QUEUE_AWARE_SLC_POLICY";
			break;
		case ALWAYS_SLC_POLICY:
			cout << "ALWAYS_SLC_POLICY";
			break;
		default:
			assert(0);
			break;
	}
#ifdef DEADLINE_AWARE_SLC_POLICY_SELF_TUNING
		cout << " + SELF_TUNING";
#endif
	cout << endl;
	cout << "\tDeadline: " << DEADLINE_AWARE_SLC_POLICY_DEADLINE << endl;
	cout << "\tDie Number: " << die_schedulers.size() << endl;
	cout << "\tBlock Number: " << total_number_of_physical_blocks << endl;
	cout << "\tUser Write Count: " << total_number_of_io_page_write << endl;
	cout << "\tUser SLC Write Count: " << total_number_of_io_slc_page_write << endl;
	cout << "\tUser SLC Percentage: " << static_cast<long double>(total_number_of_io_slc_page_write) * 100 / total_number_of_io_page_write << endl;
	cout << "\tUser SLC Overall Speedup Efficiency: " << ((total_number_of_io_slc_page_write > 0) ? (static_cast<long double>(total_number_of_benefited_page_write) / total_number_of_io_slc_page_write) : 0) << endl;
	cout << "\tUser SLC SLO-targeted Speedup Efficiency: " << ((total_number_of_io_slc_page_write > 0) ? (static_cast<long double>(total_number_of_truely_benefited_page_write) / total_number_of_io_slc_page_write) : 0) << endl;
	cout << "\tTotal Write Count: " << total_number_of_io_page_write + total_number_of_gc_page_write << endl;
	cout << "\tWrite Amplification: " << static_cast<long double>(total_number_of_io_page_write + total_number_of_gc_page_write) /  total_number_of_io_page_write << endl;
	cout << "\tTotal Erase Count: " << total_number_of_gc_block_erase << endl;
	cout << "\tAverage Erase Count: " << static_cast<long double>(total_number_of_gc_block_erase) / total_number_of_physical_blocks << endl;

#ifdef GYC_PAPER_D_MONITOR
	cout << "\tMLC clean amount=" << m_mlc_clean_amount << endl;
	cout << "\tSLC clean amount=" << m_slc_clean_amount << endl;
#endif

/*
	cout << "For read resposne time:" << endl;
	cout << "\tread_response_time.avg()=" << read_response_time.avg() << endl;
	cout << "\tread_response_time.percentile(0.50)=" << read_response_time.percentile(0.50) << endl;
	cout << "\tread_response_time.percentile(0.90)=" << read_response_time.percentile(0.90) << endl;
	cout << "\tread_response_time.percentile(0.95)=" << read_response_time.percentile(0.95) << endl;
	cout << "\tread_response_time.percentile(0.99)=" << read_response_time.percentile(0.99) << endl;
	cout << "\tread_response_time.percentile(0.999)=" << read_response_time.percentile(0.999) << endl;
	cout << "\tread_response_time.percentile(0.9999)=" << read_response_time.percentile(0.9999) << endl;

	cout << "For write resposne time:" << endl;
	cout << "\twrite_response_time.avg()=" << write_response_time.avg() << endl;
	cout << "\twrite_response_time.percentile(0.50)=" << write_response_time.percentile(0.50) << endl;
	cout << "\twrite_response_time.percentile(0.90)=" << write_response_time.percentile(0.90) << endl;
	cout << "\twrite_response_time.percentile(0.95)=" << write_response_time.percentile(0.95) << endl;
	cout << "\twrite_response_time.percentile(0.99)=" << write_response_time.percentile(0.99) << endl;
	cout << "\twrite_response_time.percentile(0.999)=" << write_response_time.percentile(0.999) << endl;
	cout << "\twrite_response_time.percentile(0.9999)=" << write_response_time.percentile(0.9999) << endl;
*/
}

/* ==================================== */
/* implement ssd_die_scheduler_callback */
/* ==================================== */

void simple_ssd_controller::complete_parent(gyc_bus_pkt* parent_req) {

	assert(parent_req);
	assert(parent_req->get_req_remain_size() > 0);
	parent_req->set_req_remain_size(parent_req->get_req_remain_size() - 1);

	if (parent_req->get_req_remain_size() == 0) {


		assert(sc_time_stamp() > parent_req->get_req_arrival_time());
		msec_t response_time = (sc_time_stamp() - parent_req->get_req_arrival_time()).to_seconds() * 1000;
		io_latency_monitor.record_value(response_time);
		// LaiYang
		//add_req_complete_time(se_time(response_time,SC_MS));
		// end LaiYang

		if (io_latency_monitor.count() % 100000 == 0) {

#ifdef DEADLINE_AWARE_SLC_POLICY
#ifdef DEADLINE_AWARE_SLC_POLICY_SELF_TUNING
			assert(m_tla_next_pr99 == NULL_MSEC);
			m_tla_next_pr99 = io_latency_monitor.percentile(0.99);

			assert(m_tla_log.is_open());
			m_tla_log << dec << fixed << sc_time_stamp().to_seconds();
			m_tla_log << '\t' << m_tla_next_targ;
			m_tla_log << '\t' << m_tla_next_pr99;
			m_tla_log << endl;

			cout << "At t = " << sc_time_stamp().to_seconds() << " sec";
			cout << ", next(" << m_tla_next_targ << ", " << m_tla_next_pr99 << ")";
			tla();
			assert(m_tla_next_pr99 == NULL_MSEC);
			cout << ", next(" << m_tla_next_targ << ", ?)";
			_update_deadline_for_all_die();

#endif

#ifdef GYC_PAPER_SLC_THROTTLING
			lpn_t last_epoch_number_of_io_page_write = 0;
			lpn_t last_epoch_number_of_io_slc_page_write = 0;
			for (int die_idx = 0; die_idx < die_schedulers.size(); die_idx++) {
				last_epoch_number_of_io_page_write += die_schedulers[die_idx]->last_epoch_number_of_io_page_write;
				last_epoch_number_of_io_slc_page_write += die_schedulers[die_idx]->last_epoch_number_of_io_slc_page_write;

				die_schedulers[die_idx]->last_epoch_number_of_io_page_read = 0;
				die_schedulers[die_idx]->last_epoch_number_of_io_page_write = 0;
				die_schedulers[die_idx]->last_epoch_number_of_io_slc_page_write = 0;
				die_schedulers[die_idx]->last_epoch_number_of_io_mlc_page_write = 0;
				die_schedulers[die_idx]->last_epoch_number_of_gc_page_read = 0;
				die_schedulers[die_idx]->last_epoch_number_of_gc_page_write = 0;
				die_schedulers[die_idx]->last_epoch_number_of_gc_block_erase = 0;
			}

			long double last_epoch_measured_slc_utilization;
			if (last_epoch_number_of_io_page_write > 0) {
				last_epoch_measured_slc_utilization = static_cast<long double>(last_epoch_number_of_io_slc_page_write)
					/ last_epoch_number_of_io_page_write;
			} else if (last_epoch_number_of_io_page_write == 0) {
				last_epoch_measured_slc_utilization = 0;
			} else {
				assert(0);
			}
			cout << ", measured SLC utilization = " << last_epoch_measured_slc_utilization * 100 << "%";

			assert(GYC_PAPER_SLC_THROTTLING__TARGETED_SLC_UTILIZATION > 0.0);
			assert(GYC_PAPER_SLC_THROTTLING__TARGETED_SLC_UTILIZATION < 1.0);
			assert(GYC_PAPER_SLC_THROTTLING__THROTTLING_RATE_MULTIPLIER_FOR_ZERO_SLC_UTILIZATION > 1.0);
			assert(GYC_PAPER_SLC_THROTTLING__THROTTLING_RATE_INITIAL_VALUE_FOR_ZERO_SLC_UTILIZATION > 0.0);
			assert(GYC_PAPER_SLC_THROTTLING__THROTTLING_RATE_INITIAL_VALUE_FOR_ZERO_SLC_UTILIZATION <= 1.0);

			if (last_epoch_measured_slc_utilization > GYC_PAPER_SLC_THROTTLING__TARGETED_SLC_UTILIZATION) {
				/* need slow down */
				assert(last_epoch_measured_slc_utilization > 0);
				m_slc_throttle_rate *= GYC_PAPER_SLC_THROTTLING__TARGETED_SLC_UTILIZATION
					/ last_epoch_measured_slc_utilization;
				assert(m_slc_throttle_rate >= 0);
			} else if (last_epoch_measured_slc_utilization < GYC_PAPER_SLC_THROTTLING__TARGETED_SLC_UTILIZATION) {
				/* need speed up */
				if (last_epoch_measured_slc_utilization > 0) {
					if (m_slc_throttle_rate > 0) {
						m_slc_throttle_rate *= GYC_PAPER_SLC_THROTTLING__TARGETED_SLC_UTILIZATION
							/ last_epoch_measured_slc_utilization;
						if (m_slc_throttle_rate > 1.0) {
							m_slc_throttle_rate = 1.0;
						}
					} else if (m_slc_throttle_rate == 0) {
						/* if you completely turn off slc, last_epoch_measured_slc_utilization should be zero */
						assert(0);
					} else {
						/* m_slc_throttle_rate should not be non-negative */
						assert(0);
					}
				} else if (last_epoch_measured_slc_utilization == 0) {
					if (m_slc_throttle_rate > 0) {
						m_slc_throttle_rate *= GYC_PAPER_SLC_THROTTLING__THROTTLING_RATE_MULTIPLIER_FOR_ZERO_SLC_UTILIZATION;
						if (m_slc_throttle_rate > 1.0) {
							m_slc_throttle_rate = 1.0;
						}
					} else if (m_slc_throttle_rate == 0) {
						m_slc_throttle_rate = GYC_PAPER_SLC_THROTTLING__THROTTLING_RATE_INITIAL_VALUE_FOR_ZERO_SLC_UTILIZATION;
					} else {
						assert(0);
					}
				} else {
					/* last_epoch_measured_slc_utilization should not be non-negative */
					assert(0);
				}
			}

			cout << ", SLC throttling rate = " << m_slc_throttle_rate * 100 << "%";
#endif

#endif
			cout << endl;
			io_latency_monitor.reset();
		}

		m_complete_req_queue.push_back(parent_req);
		if (peek_next_pkt_to_send(m_upload_ch_id)) {
			upload_ch_port->require_send_next_pkt();
		}
	}
}

#ifdef DEADLINE_AWARE_SLC_POLICY
void simple_ssd_controller::_update_deadline_for_all_die() {
	for (int i = 0; i < die_schedulers.size(); ++i) {
#ifdef DEADLINE_AWARE_SLC_POLICY_SELF_TUNING
		assert(m_tla_next_targ != NULL_MSEC);
		assert(m_tla_next_pr99 == NULL_MSEC);
		die_schedulers[i]->set_deadline(m_tla_next_targ);
#else
		die_schedulers[i]->set_deadline(DEADLINE_AWARE_SLC_POLICY_DEADLINE);
#endif

	}
}
#endif

/* ======================== */
/* implement gyc_bus_dev_if */
/* ======================== */

gyc_bus_pkt* simple_ssd_controller::peek_next_pkt_to_send(int bus_id) const {
	if (bus_id == m_download_ch_id) {
		return NULL;
	} else if (bus_id == m_upload_ch_id) {
		if (!m_upload_ch_busy && !m_complete_req_queue.empty()) {
			return m_complete_req_queue.front();
		}
		return NULL;
	} else {
		assert(0);
		return NULL;
	}
}

void simple_ssd_controller::on_send_started(int bus_id) {
	if (bus_id == m_download_ch_id) {
		assert(0);
	} else if (bus_id == m_upload_ch_id) {
		assert(!m_upload_ch_busy);
		assert(!m_complete_req_queue.empty());
		m_upload_ch_busy = true;
		m_complete_req_queue.pop_front();
	} else {
		assert(0);
	}
}

void simple_ssd_controller::on_send_completed(int bus_id) {
	if (bus_id == m_download_ch_id) {
		assert(0);
	} else if (bus_id == m_upload_ch_id) {
		assert(m_upload_ch_busy);
		m_upload_ch_busy = false;
		assert(m_queue_size > 0);
		m_queue_size--;
		if (!m_complete_req_queue.empty()) {
			upload_ch_port->require_send_next_pkt();
		}
		if (m_queue_size < SSD_QUEUE_DEPTH && !m_download_ch_busy && !m_recv_buf) {
			download_ch_port->require_recv_next_pkt();
		}
	} else {
		assert(0);
	}
}

bool simple_ssd_controller::can_recv_next_pkt(int bus_id, const gyc_bus_pkt* next_pkt_to_recv) const {
	if (bus_id == m_download_ch_id) {
		if (m_queue_size < SSD_QUEUE_DEPTH && !m_download_ch_busy && !m_recv_buf) {
			return true;
		}
		return false;
	} else if (bus_id == m_upload_ch_id) {
		return false;
	} else {
		assert(0);
		return false;
	}
}

void simple_ssd_controller::recv_next_pkt(int bus_id, gyc_bus_pkt* next_pkt_to_recv) {
	if(get_last_time_for_save() > sc_time(0, SC_MS) && next_pkt_to_recv->get_req_arrival_time() == get_last_time_for_save()){
		//cout << mytest << " par @@ " << next_pkt_to_recv->get_req_arrival_time() << endl;
		save_last_use(get_last_path(), get_last_info(), get_number_of_dice());

		set_last_time_for_save(sc_time(-1, SC_MS));

		//exit(-1);
	}
	if (bus_id == m_download_ch_id) {
		assert(m_queue_size < SSD_QUEUE_DEPTH);
		assert(!m_download_ch_busy);
		assert(!m_recv_buf);
		m_download_ch_busy = true;
		m_recv_buf = next_pkt_to_recv;
	} else if (bus_id == m_upload_ch_id) {
		assert(0);
	} else {
		assert(0);
	}
}

void simple_ssd_controller::on_recv_completed(int bus_id) {
	if (bus_id == m_download_ch_id) {
		assert(m_download_ch_busy);
		m_download_ch_busy = false;

		/* prepare reversed bus path for the future time when this operation is finished processing */
		assert(m_recv_buf);
		assert(m_recv_buf->has_ssd_req_ext());
		assert(m_recv_buf->get_bus_pkt_dest() == 1);
		m_recv_buf->set_bus_pkt_dest(0);
		switch (m_recv_buf->get_req_type()) {
			case SSD_REQ_TYP_RD:
				m_recv_buf->set_bus_pkt_prio(1);
				m_recv_buf->set_bus_pkt_delay(sc_time(HOST_BUS_IO_CYCLE * (HOST_BUS_HEADER_SIZE + m_recv_buf->get_req_size() * SECTOR_PER_PAGE * BYTE_PER_SECTOR), SC_MS));
				break;
			case SSD_REQ_TYP_WR:
				m_recv_buf->set_bus_pkt_prio(0);
				m_recv_buf->set_bus_pkt_delay(sc_time(HOST_BUS_IO_CYCLE * HOST_BUS_HEADER_SIZE, SC_MS));
				break;
			default:
				assert(0);
				break;
		}

		/* creating sub-request */
		for (lpn_t op_lpn = m_recv_buf->get_req_lpn();
				op_lpn < m_recv_buf->get_req_lpn() + m_recv_buf->get_req_size();
				op_lpn++) {
			
			int die_index;
			lpn_t die_op_lpn = op_lpn;
	
			//die_index = op_lpn % die_schedulers.size();
			//die_op_lpn = op_lpn / die_schedulers.size() + die_index*USER_CAPACITY_IN_PAGE_PER_DIE;}


			switch (m_recv_buf->get_req_type()) {
				case SSD_REQ_TYP_RD:
					die_index = die_ftls->lp_meta_table[die_op_lpn].die_idx;
					//cout << "lp: " << die_ftls->lp_meta_table[op_lpn].die_idx << " " << op_lpn % die_schedulers.size() << endl;
					die_schedulers[die_index]->schedule_io_page_read(m_recv_buf, die_op_lpn);
					break;
				case SSD_REQ_TYP_WR:
#ifdef jiawei_die
					die_index = m_die_idx;
					m_die_idx += ch_array.size();
					if (m_die_idx > m_number_of_die -1){
						first = (first+1) % ch_array.size();
						m_die_idx = first;
					}
					die_index = m_die_idx;
					die_schedulers[die_index]->schedule_io_page_write(m_recv_buf, die_op_lpn);
					break;
#else 					//LaiYang_die
					die_index = m_die_idx;
					m_die_idx++;
					if (m_die_idx > m_number_of_die -1){
						first = (first+1) % ch_array.size();
						m_die_idx = first;
					}
					//die_index = m_die_idx;
					die_schedulers[die_index]->schedule_io_page_write(m_recv_buf, die_op_lpn);
					break;
					//LaiYang_die end
#endif
				default:
					assert(0);
					break;
			}
		}
		m_recv_buf = NULL;
		assert(m_queue_size < SSD_QUEUE_DEPTH);
		m_queue_size++;
		if (m_queue_size < SSD_QUEUE_DEPTH) {
			download_ch_port->require_recv_next_pkt();
		}
	} else if (bus_id == m_upload_ch_id) {
		assert(0);
	} else {
		assert(0);
	}
}

#ifdef DEADLINE_AWARE_SLC_POLICY_SELF_TUNING
void simple_ssd_controller::tla() {
	assert(m_tla_next_pr99 != NULL_MSEC);
	if (m_tla_next_pr99 > DEADLINE_AWARE_SLC_POLICY_DEADLINE__MAX) {
		if ((m_tla_next_pr99 - m_tla_prev_1_pr99) * (m_tla_next_targ - m_tla_prev_1_targ) > 0) {
			m_tla_prev_2_targ = m_tla_prev_1_targ;
			m_tla_prev_2_pr99 = m_tla_prev_1_pr99;
		} else {
			m_tla_prev_2_targ = DEADLINE_AWARE_SLC_POLICY_NO_DEADLINE;
			m_tla_prev_2_pr99 = DEADLINE_AWARE_SLC_POLICY_NO_DEADLINE;
		}
		m_tla_prev_1_targ = m_tla_next_targ;
		m_tla_prev_1_pr99 = m_tla_next_pr99;
		if (m_tla_prev_2_targ != DEADLINE_AWARE_SLC_POLICY_NO_DEADLINE
				&& m_tla_prev_2_pr99 != DEADLINE_AWARE_SLC_POLICY_NO_DEADLINE) {
			m_tla_next_targ = m_tla_prev_1_targ + (DEADLINE_AWARE_SLC_POLICY_DEADLINE__MAX - m_tla_prev_1_pr99)
				* (m_tla_prev_1_targ - m_tla_prev_2_targ) / (m_tla_prev_1_pr99 - m_tla_prev_2_pr99);
		} else {
			m_tla_next_targ = m_tla_prev_1_targ + (DEADLINE_AWARE_SLC_POLICY_DEADLINE__MAX - m_tla_prev_1_pr99);
		}
		if (m_tla_next_targ > DEADLINE_AWARE_SLC_POLICY_DEADLINE) {
			m_tla_next_targ = DEADLINE_AWARE_SLC_POLICY_DEADLINE;
		} else if (m_tla_next_targ < 0) {
			m_tla_next_targ = 0;
		}
	} else if (m_tla_next_pr99 < DEADLINE_AWARE_SLC_POLICY_DEADLINE__MIN) {
		if ((m_tla_next_pr99 - m_tla_prev_1_pr99) * (m_tla_next_targ - m_tla_prev_1_targ) > 0) {
			m_tla_prev_2_targ = m_tla_prev_1_targ;
			m_tla_prev_2_pr99 = m_tla_prev_1_pr99;
		} else {
			m_tla_prev_2_targ = DEADLINE_AWARE_SLC_POLICY_NO_DEADLINE;
			m_tla_prev_2_pr99 = DEADLINE_AWARE_SLC_POLICY_NO_DEADLINE;
		}
		m_tla_prev_1_targ = m_tla_next_targ;
		m_tla_prev_1_pr99 = m_tla_next_pr99;
		if (m_tla_prev_2_targ != DEADLINE_AWARE_SLC_POLICY_NO_DEADLINE
				&& m_tla_prev_2_pr99 != DEADLINE_AWARE_SLC_POLICY_NO_DEADLINE) {
			m_tla_next_targ = m_tla_prev_1_targ + (DEADLINE_AWARE_SLC_POLICY_DEADLINE__MIN - m_tla_prev_1_pr99)
				* (m_tla_prev_1_targ - m_tla_prev_2_targ) / (m_tla_prev_1_pr99 - m_tla_prev_2_pr99);
		} else {
			m_tla_next_targ = m_tla_prev_1_targ + (DEADLINE_AWARE_SLC_POLICY_DEADLINE__MIN - m_tla_prev_1_pr99);
		}
		if (m_tla_next_targ > DEADLINE_AWARE_SLC_POLICY_DEADLINE) {
			m_tla_next_targ = DEADLINE_AWARE_SLC_POLICY_DEADLINE;
		} else if (m_tla_next_targ < 0) {
			m_tla_next_targ = 0;
		}
	}
	m_tla_next_pr99 = NULL_MSEC;
}

#endif

template <class T> T find_enum(int c) {
	return static_cast<T>(c);
}
void simple_ssd_controller::load_last_use(std::string trace_path, int number_of_dice){
	int i=0, tmp = -1;
	double tmp_info;

	string trace_last_path = trace_path + ".last";
	ifstream trace_last_ifs;
	trace_last_ifs.open(trace_last_path.c_str());
	if (!trace_last_ifs.is_open()) {
		cerr << "[ERROR] cannot open trace_last file (" << trace_last_path << ")" << endl;
		exit(-1);
	}

	trace_last_ifs >> tmp_info;
	cout << tmp_info << "load_last_use" << endl;
	for(int k=0; k < USER_CAPACITY_IN_PAGE_PER_DIE * get_number_of_dice(); k++){
		trace_last_ifs >> die_ftls->get_lp_meta_table()[k].ptr_to_physical_page;
	}
	while(1){
		if(i == number_of_dice)break;
		die_ftls->get_p_meta_table(i)->num_of_all_pps = BLOCK_PER_DIE * PAGE_PER_BLOCK;
		die_ftls->get_p_meta_table(i)->num_of_all_pbs = BLOCK_PER_DIE;
		die_ftls->get_p_meta_table(i)->num_of_pps_per_pb = PAGE_PER_BLOCK;
		
		trace_last_ifs >> die_ftls->get_wr_ptr_for_short(i)->ptr_to_head_pb;
		trace_last_ifs >> die_ftls->get_wr_ptr_for_short(i)->tot_num_of_wr_ops;
		trace_last_ifs >> tmp;
		die_ftls->get_wr_ptr_for_short(i)->mode = find_enum <wr_ptr_mode_t> (tmp);
		
		trace_last_ifs >> die_ftls->get_wr_ptr_for_long(i)->ptr_to_head_pb;
		trace_last_ifs >> die_ftls->get_wr_ptr_for_long(i)->tot_num_of_wr_ops;
		trace_last_ifs >> tmp;
		die_ftls->get_wr_ptr_for_long(i)->mode = find_enum <wr_ptr_mode_t> (tmp);

		trace_last_ifs >> die_ftls->get_wr_ptr_for_gc(i)->ptr_to_head_pb;
		trace_last_ifs >> die_ftls->get_wr_ptr_for_gc(i)->tot_num_of_wr_ops;
		trace_last_ifs >> tmp;
		die_ftls->get_wr_ptr_for_gc(i)->mode = find_enum <wr_ptr_mode_t> (tmp);
		

		for(pbn_t j=0; j < die_ftls->get_p_meta_table(i)->num_of_all_pbs; j++){
			trace_last_ifs >> tmp;
			die_ftls->get_p_meta_table(i)->pb_meta_array[j].state = find_enum <pb_state_t> (tmp);
			trace_last_ifs >> tmp;
			die_ftls->get_p_meta_table(i)->pb_meta_array[j].mode = find_enum <pb_mode_t> (tmp);

			trace_last_ifs >> die_ftls->get_p_meta_table(i)->pb_meta_array[j].ptr_to_free_page;
			trace_last_ifs >> die_ftls->get_p_meta_table(i)->pb_meta_array[j].ptr_to_first_page;
			trace_last_ifs >> die_ftls->get_p_meta_table(i)->pb_meta_array[j].ptr_to_last_page;
			trace_last_ifs >> die_ftls->get_p_meta_table(i)->pb_meta_array[j].num_of_free_pages;
			trace_last_ifs >> die_ftls->get_p_meta_table(i)->pb_meta_array[j].num_of_live_pages;
			trace_last_ifs >> die_ftls->get_p_meta_table(i)->pb_meta_array[j].num_of_dead_pages;
			trace_last_ifs >> die_ftls->get_p_meta_table(i)->pb_meta_array[j].tot_num_of_rd_ops;
			trace_last_ifs >> die_ftls->get_p_meta_table(i)->pb_meta_array[j].tot_num_of_wr_ops;
			trace_last_ifs >> die_ftls->get_p_meta_table(i)->pb_meta_array[j].tot_num_of_er_ops;
		}
		for(ppn_t j=0; j < die_ftls->get_p_meta_table(i)->num_of_all_pps; j++){
			trace_last_ifs >> tmp;
			die_ftls->get_p_meta_table(i)->pp_meta_array[j].state = find_enum <pp_state> (tmp);
			trace_last_ifs >> die_ftls->get_p_meta_table(i)->pp_meta_array[j].ptr_to_logical_page;
		}

		trace_last_ifs >> die_ftls->get_p_meta_table(i)->num_of_free_pbs;
		trace_last_ifs >> die_ftls->get_p_meta_table(i)->num_of_head_pbs;
		trace_last_ifs >> die_ftls->get_p_meta_table(i)->num_of_body_pbs;
		trace_last_ifs >> die_ftls->get_p_meta_table(i)->num_of_tail_pbs;
		i++;
	}
	trace_last_ifs.close();
}
void simple_ssd_controller::save_last_use(std::string trace_path, double last_info, int number_of_dice) const {
	string trace_last_path = trace_path + ".last";
	ofstream trace_last_ofs;
	trace_last_ofs.open(trace_last_path.c_str());
	trace_last_ofs.setf(ios::fixed);
	trace_last_ofs.precision(8);
	if(trace_last_ofs.is_open()){
		trace_last_ofs << last_info << endl;//last info
		// =============== 
		// save meta_table 
		// =============== 
		for(int k=0; k < USER_CAPACITY_IN_PAGE_PER_DIE * get_number_of_dice(); k++){
			trace_last_ofs << die_ftls->get_lp_meta_table()[k].ptr_to_physical_page << " ";
			//if(k % 500 == 0) cout << k <<" " << endl;
		}

		for(int i=0; i < number_of_dice; i++){
			//trace_last_ofs << die_ftls[i]->get_p_meta_table()->num_of_all_pps << endl;//num_pps 
			//trace_last_ofs << die_ftls[i]->get_p_meta_table()->num_of_all_pbs << endl;//num_pbs
			//trace_last_ofs << die_ftls[i]->get_p_meta_table()->num_of_pps_per_pb << endl;//num_pps_per_pb
			//trace_last_ofs << "pb_meta\n";
			
			trace_last_ofs << die_ftls->get_wr_ptr_for_short(i)->ptr_to_head_pb << " ";
			trace_last_ofs << die_ftls->get_wr_ptr_for_short(i)->tot_num_of_wr_ops << " ";
			trace_last_ofs << die_ftls->get_wr_ptr_for_short(i)->mode << " ";
	
			trace_last_ofs << die_ftls->get_wr_ptr_for_long(i)->ptr_to_head_pb << " ";
			trace_last_ofs << die_ftls->get_wr_ptr_for_long(i)->tot_num_of_wr_ops << " ";
			trace_last_ofs << die_ftls->get_wr_ptr_for_long(i)->mode << " ";

			trace_last_ofs << die_ftls->get_wr_ptr_for_gc(i)->ptr_to_head_pb << " ";
			trace_last_ofs << die_ftls->get_wr_ptr_for_gc(i)->tot_num_of_wr_ops << " ";
			trace_last_ofs << die_ftls->get_wr_ptr_for_gc(i)->mode << " ";
			

			for(pbn_t j=0; j < die_ftls->get_p_meta_table(i)->num_of_all_pbs; j++){
				trace_last_ofs << die_ftls->get_p_meta_table(i)->pb_meta_array[j].state << " ";
				trace_last_ofs << die_ftls->get_p_meta_table(i)->pb_meta_array[j].mode << " ";
				trace_last_ofs << die_ftls->get_p_meta_table(i)->pb_meta_array[j].ptr_to_free_page << " ";
				trace_last_ofs << die_ftls->get_p_meta_table(i)->pb_meta_array[j].ptr_to_first_page << " ";
				trace_last_ofs << die_ftls->get_p_meta_table(i)->pb_meta_array[j].ptr_to_last_page << " ";
				trace_last_ofs << die_ftls->get_p_meta_table(i)->pb_meta_array[j].num_of_free_pages << " ";
				trace_last_ofs << die_ftls->get_p_meta_table(i)->pb_meta_array[j].num_of_live_pages << " ";
				trace_last_ofs << die_ftls->get_p_meta_table(i)->pb_meta_array[j].num_of_dead_pages << " ";
				trace_last_ofs << die_ftls->get_p_meta_table(i)->pb_meta_array[j].tot_num_of_rd_ops << " ";
				trace_last_ofs << die_ftls->get_p_meta_table(i)->pb_meta_array[j].tot_num_of_wr_ops << " ";
				trace_last_ofs << die_ftls->get_p_meta_table(i)->pb_meta_array[j].tot_num_of_er_ops << " ";
			}
			//trace_last_ofs << "\npp_meta\n";
			for(ppn_t j=0; j < die_ftls->get_p_meta_table(i)->num_of_all_pps; j++){
				trace_last_ofs << die_ftls->get_p_meta_table(i)->pp_meta_array[j].state << " ";
				trace_last_ofs << die_ftls->get_p_meta_table(i)->pp_meta_array[j].ptr_to_logical_page << " ";
			}
			trace_last_ofs << endl;
			trace_last_ofs << die_ftls->get_p_meta_table(i)->num_of_free_pbs << endl;//num_free
			trace_last_ofs << die_ftls->get_p_meta_table(i)->num_of_head_pbs << endl;//num_head
			trace_last_ofs << die_ftls->get_p_meta_table(i)->num_of_body_pbs << endl;//num_body
			trace_last_ofs << die_ftls->get_p_meta_table(i)->num_of_tail_pbs << endl;//num_tail
		}
		// ===================
		// save die_controller
		// ===================
		trace_last_ofs.close();
	}
}
