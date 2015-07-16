
systemc_header = $(HOME)/local/systemc-2.3.0/include
systemc_library = $(HOME)/local/systemc-2.3.0/lib-linux64

CXXFLAGS += -I$(systemc_header)
LDFLAGS += -L$(systemc_library) -Wl,-rpath=$(systemc_library)
LIBS += -lsystemc

old:
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -o run.x gyc_ftl_p_meta_table.cpp gyc_ftl_wr_ptr.cpp simple_die_ftl.cpp simple_die_scheduler.cpp simple_die_controller.cpp simple_ssd_controller.cpp simple_io_scheduler.cpp main.cpp

all: lib lin

lin:
		$(CXX) $(CXXFLAGS)  $(LDFLAGS) $(LIBS) -L./ -lmain -pthread -o run.x caller.cpp
lib:
		$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -pthread -c gyc_ftl_p_meta_table.cpp gyc_ftl_wr_ptr.cpp simple_die_ftl.cpp simple_die_scheduler.cpp simple_die_controller.cpp simple_ssd_controller.cpp simple_io_scheduler.cpp main.cpp
		ar rcs libmain.a gyc_ftl_p_meta_table.o main.o simple_die_ftl.o simple_io_scheduler.o gyc_ftl_wr_ptr.o   simple_die_controller.o  simple_die_scheduler.o  simple_ssd_controller.o
		rm gyc_ftl_p_meta_table.o main.o simple_die_ftl.o simple_io_scheduler.o gyc_ftl_wr_ptr.o   simple_die_controller.o  simple_die_scheduler.o  simple_ssd_controller.o




clean:
	rm -f run.x

