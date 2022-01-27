ifndef LG_RT_DIR
$(error LG_RT_DIR variable is not defined, aborting build)
endif

# Flags for directing the runtime makefile what to include
DEBUG           ?= 1		# Include debugging symbols
OUTPUT_LEVEL    ?= LEVEL_DEBUG	# Compile time logging level
USE_CUDA        ?= 0		# Include CUDA support (requires CUDA)
USE_GASNET      ?= 0		# Include GASNet support (requires GASNet)
USE_HDF         ?= 0		# Include HDF5 support (requires HDF5)
ALT_MAPPERS     ?= 0		# Include alternative mappers (not recommended)

# Put the binary file name here
OUTFILE		?= hello_world_runtime 
# List all the application source files here
GEN_SRC		?= hello_world_runtime.cc task_runtime.cc		# .cc files
GEN_GPU_SRC	?=				# .cu files

# You can modify these variables, some will be appended to by the runtime makefile
INC_FLAGS	?= -I/projects/opt/centos8/x86_64/openmpi/3.1.6-gcc_8.5.0/include
CC_FLAGS	?=
NVCC_FLAGS	?=
GASNET_FLAGS	?=
LD_FLAGS	?= -L/projects/opt/centos8/x86_64/openmpi/3.1.6-gcc_8.5.0/lib -lmpi

###########################################################################
#
#   Don't change anything below here
#   
###########################################################################

include $(LG_RT_DIR)/runtime.mk

