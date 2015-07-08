#ifndef __SIMPLE_SSD_CONTROLLER_H__
#define __SIMPLE_SSD_CONTROLLER_H__

#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "systemc.h"

#include "pmf_builder.h"

#include "ssd_typedef.h"
#include "gyc_bus.h"
#include "ssd_die_scheduler_callback.h"

#include "simple_die_scheduler.h"
#include "simple_die_ftl.h"
#include "simple_die_controller.h"

class simple_ssd_controller :
	public sc_core::sc_module,
	public ssd_die_scheduler_callback,
	public gyc_bus_dev_if
{
	// LaiYang
	
	public:
		sc_time el;

		void spiner();
		ppn_t x_total_io();

		sc_time epoch_length;
		sc_time epoch_time_now;
		sc_time epoch_time_next;

		std::deque<unsigned int> req_complete_number;
		unsigned int req_complete_cnt;

		void add_req_complete_time(sc_time t){
			if( t >= epoch_time_next ){
				req_complete_cnt;
			}
			else{}
		}
	
	// end LaiYang
	private:
		gyc_bus_pkt_mm* m_mm;

		std::vector<simple_die_scheduler *> die_schedulers;
		simple_die_ftl* die_ftls;
		std::vector<simple_die_controller* > die_controllers;
		std::vector<gyc_bus* > ch_array;
		slc_policy_t m_slc_policy;
		int m_die_idx;
		int m_number_of_die;
		int first;

#ifdef DEADLINE_AWARE_SLC_POLICY
		void _update_deadline_for_all_die();
#ifdef DEADLINE_AWARE_SLC_POLICY_SELF_TUNING

		msec_t m_tla_next_targ;
		msec_t m_tla_next_pr99;
		msec_t m_tla_prev_1_targ;
		msec_t m_tla_prev_1_pr99;
		msec_t m_tla_prev_2_targ;
		msec_t m_tla_prev_2_pr99;
		void tla();
		std::ofstream m_tla_log;
#endif
#endif
		pmf_builder<msec_t> io_latency_monitor;

	public:
#ifdef GYC_PAPER_D_MONITOR
		pmf_builder<long double> m_big_d;
		//pmf_builder<long double> m_small_d;
		std::vector<long double> m_gc_eff;
#endif

		SC_HAS_PROCESS(simple_ssd_controller);
		simple_ssd_controller(
				sc_core::sc_module_name module_name,
				gyc_bus_pkt_mm* global_mm,
				int total_number_of_dice,
				int total_number_of_channels,
				int min_free_blocks_percentage,
				slc_policy_t slc_policy,
				double last,
				std::string trace_path);

		sc_port<gyc_bus_dev_cb> download_ch_port;
		sc_port<gyc_bus_dev_cb> upload_ch_port;

		void debug_check_all_req_completed() const;

		/* ==================================== */
		/* implement ssd_die_scheduler_callback */
		/* ==================================== */

		void complete_parent(gyc_bus_pkt* parent_req);
#ifdef GYC_PAPER_SLC_THROTTLING
		long double get_slc_throttle_rate() const { return m_slc_throttle_rate; }
#endif

#ifdef GYC_PAPER_D_MONITOR
		long double m_mlc_clean_amount;
		long double m_slc_clean_amount;
		void record_big_d(long double big_d) {
			m_mlc_clean_amount += big_d;
		}
		void record_small_d(long double small_d) {
			m_slc_clean_amount += small_d;
		}

#endif
	private:
		int m_queue_size;
		gyc_bus_pkt* m_recv_buf;
		std::deque<gyc_bus_pkt *> m_complete_req_queue;
		int m_download_ch_id;
		int m_upload_ch_id;
		bool m_download_ch_busy;
		bool m_upload_ch_busy;

		sc_time m_last_time_for_save;
		double m_last_info;
		std::string m_last_path;
		int m_number_of_dice;
#ifdef GYC_PAPER_SLC_THROTTLING
		long double m_slc_throttle_rate;
#endif

	public:
		void set_download_ch_id(int download_ch_id) { m_download_ch_id = download_ch_id; }
		int get_download_ch_id() const { return m_download_ch_id; }
		void set_upload_ch_id(int upload_ch_id) { m_upload_ch_id = upload_ch_id; }
		int get_upload_ch_id() const { return m_upload_ch_id; }

	public:
		/* ======================== */
		/* implement gyc_bus_dev_if */
		/* ======================== */
		gyc_bus_pkt* peek_next_pkt_to_send(int bus_id) const;
		void on_send_started(int bus_id);
		void on_send_completed(int bus_id);
		bool can_recv_next_pkt(int bus_id, const gyc_bus_pkt* next_pkt_to_recv) const;
		void recv_next_pkt(int bus_id, gyc_bus_pkt* next_pkt_to_recv);
		void on_recv_completed(int bus_id);

		void set_number_of_dice(const int number_of_dice){ m_number_of_dice = number_of_dice; }
		int get_number_of_dice() const { return m_number_of_dice; }
		void set_last_path(const std::string last_path){ m_last_path = last_path; }
		std::string get_last_path(){ return m_last_path; }
		void set_last_info(double last){ m_last_info = last; }
		double get_last_info(){ return m_last_info; }
		void set_last_time_for_save(const sc_time& req_time){ m_last_time_for_save = req_time; }
		sc_time get_last_time_for_save(){ return m_last_time_for_save; };
		void save_last_use(std::string trace_path, double last_info, int number_of_dice) const;
		void load_last_use(std::string trace_path, int number_of_dice) ;
};

#endif

