#ifndef REAML_DIR
#$(error STARPU_DIR variable is not defined, aborting build)
#endif

CC         = mpic++
DEBUG=1

ifeq ($(strip $(DEBUG)),0)
	CFLAGS     = -O3 -fPIC -std=c++11 -ldl -rdynamic -lrt
	LDFLAGS    = -Wall -O3 -std=c++11 -ldl -rdynamic -lrt
else
	CFLAGS     = -O0 -ggdb -fPIC -std=c++11 -ldl -rdynamic -lrt
	LDFLAGS    = -Wall -O0 -ggdb -std=c++11 -ldl -rdynamic -lrt
endif

REALM_DIR = /vast/home/wwu/legion-install


# Include directories
INC        = -I$(REALM_DIR)/include
INC_EXT    = 

# Location of the libraries.
LIB        = -L$(REALM_DIR)/lib64 -lrealm -llegion
LIB_EXT    = 

INC := $(INC) $(INC_EXT)
LIB := $(LIB) $(LIB_EXT)

CFLAGS += $(INC)

TARGET = hello_world hello_world_runtime pingpong heat_2d_future multiple_future
all: $(TARGET)
	
.PRECIOUS: %.cc %.o

task_runtime.o: task_runtime.cc task_runtime.h
	$(CC) -c $(CFLAGS) $<
	
ompi_future.o: ompi_future.cc mpi_future.h
	$(CC) -c $(CFLAGS) $<

hello_world_runtime.o: hello_world_runtime.cc
	$(CC) -c $(CFLAGS) $<

hello_world.o: hello_world.cc
	$(CC) -c $(CFLAGS) $<
	
pingpong.o: pingpong.cc
	$(CC) -c $(CFLAGS) $<
	
heat_2d_future.o: heat_2d_future.cc
	$(CC) -c $(CFLAGS) $<
	
multiple_future.o: multiple_future.cc
	$(CC) -c $(CFLAGS) $<
	
hello_world_runtime: hello_world_runtime.o task_runtime.o ompi_future.o
	$(CC) $^ $(LIB) $(LDFLAGS) -o $@ 

hello_world: hello_world.o task_runtime.o ompi_future.o
	$(CC) $^ $(LIB) $(LDFLAGS) -o $@ 
	
pingpong: pingpong.o task_runtime.o ompi_future.o
	$(CC) $^ $(LIB) $(LDFLAGS) -o $@ 
	
heat_2d_future: heat_2d_future.o task_runtime.o ompi_future.o
	$(CC) $^ $(LIB) $(LDFLAGS) -o $@ 
	
multiple_future: multiple_future.o task_runtime.o ompi_future.o
	$(CC) $^ $(LIB) $(LDFLAGS) -o $@ 


clean:
	rm -f *.o
	rm -f $(TARGET)

.PHONY: all clean
