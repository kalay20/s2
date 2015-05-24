#ifndef __SIMPLE_DIE_FTL_H__
#define __SIMPLE_DIE_FTL_H__

#include <cassert>
#include <cstdlib>
#include <cmath>
#include <new>
#include <iostream>
// LaiYang
#include <queue>
// end LaiYang

#include "systemc.h"

#include "ssd_typedef.h"
#include "ssd_die_ftl_callback.h"
#include "ssd_die_ftl_interface.h"

#include "gyc_ftl_typedef.h"
#include "gyc_ftl_assert.h"
#include "gyc_ftl_foreach.h"
#include "gyc_ftl_p_meta_table.h"
#include "gyc_ftl_wr_ptr.h"

class simple_die_ftl : public sc_module, public ssd_die_ftl_interface {
	public:
		//std::vector<lp_meta_t> lp_meta_table;
		lp_meta_t* lp_meta_table;
	private:
		p_meta_table_t** p_meta_table;	

		std::vector<wr_ptr_t* > wr_ptr_for_short_io_queue;
		std::vector<wr_ptr_t* > wr_ptr_for_long_io_queue;
		std::vector<wr_ptr_t* > wr_ptr_for_gc;
		std::vector<ssd_die_ftl_callback* > ftl_callback;

		void schedule_gc_if_needed(int die_idx);
		msec_t _determine_write_latency(const wr_ptr_t *wr_ptr) const;

	public:
		SC_HAS_PROCESS(simple_die_ftl);
		simple_die_ftl(sc_core::sc_module_name module_name, int number_of_dice);

		/* setting */
		int min_free_blocks_percentage;
		slc_policy_t slc_policy;

		ssd_die_ftl_callback* get_callback() const;
		void set_ftl_callback(ssd_die_ftl_callback* callback, int die_idx);
		void reset_data_layout();

		bool check_mapping_validity(lpn_t logical_page, ppn_t physical_page, int die_idx) const;

		msec_t issue_page_read(lpn_t logical_page, bool generated_by_gc, int die_idx);
		msec_t issue_page_write(lpn_t logical_page, bool generated_by_gc, int die_idx);
		msec_t issue_block_erase(pbn_t physical_block, int die_idx);

		void print_status(int die_idx);
		int m_number_of_dice;

		p_meta_table_t* get_p_meta_table(int i){ return p_meta_table[i]; }
		lp_meta_t* get_lp_meta_table(){ return lp_meta_table; }
		wr_ptr_t* get_wr_ptr_for_short(int i){ return wr_ptr_for_short_io_queue[i]; }
		wr_ptr_t* get_wr_ptr_for_long(int i){ return wr_ptr_for_long_io_queue[i]; }
		wr_ptr_t* get_wr_ptr_for_gc(int i){ return wr_ptr_for_gc[i]; }
		
		// LaiYang
		std::deque<int>* live_page_copy_per_gc;
		int* number_of_gc_trigger;
		// end LaiYang

#ifdef GYC_PAPER_OVERLOAD_PROTECTION
		ppn_t m_current_busy_period_slc_write_count;
		ppn_t m_current_busy_period_mlc_write_count;
		bool m_current_busy_period_has_record_slc_usage;
		bool m_after_foreground_gc;
		long double m_max_slc_usage;
		long double m_ready_slc_usage;
		long double get_current_busy_period_slc_write_utilization() const;
		bool should_enable_slc(int die_idx) const;
#endif

};

#endif

