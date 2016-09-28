# Repast HPC Manual Build Makefile

# VARIABLES (Supply values for these; for definitions and examples, see INSTALL)
CXX=mpicxx
CXXLD=mpicxx
BOOST_INCLUDE_DIR=/usr/local/include
BOOST_LIB_DIR=/usr/local/lib
BOOST_INFIX=-mt
NETCDF_INCLUDE_DIR=/usr/local/include
NETCDF_LIB_DIR=/usr/local/lib
NETCDF_CXX_INCLUDE_DIR=/usr/local/include
NETCDF_CXX_LIB_DIR=/usr/local/lib
CURL_INCLUDE_DIR=./
CURL_LIB_DIR=./
INSTALL_DIR=$HOME/sfw/repast_hpc-2.2.0

# change to true to build static libraries
STATIC=false

# DIRECTORIES
SRC_DIR=../src
REPAST_HPC_DIR=../src/repast_hpc
RELOGO_DIR=../src/relogo
RUMOR_DIR=../src/rumor
ZOMBIE_DIR=../src/zombie

# DERIVED VARIABLES (Do not modify)
I_REPAST=-I$(REPAST_HPC_DIR)
I_BOOST=-I$(BOOST_INCLUDE_DIR)
L_BOOST=-L$(BOOST_LIB_DIR)
l_BOOST=-lboost_mpi$(BOOST_INFIX) -lboost_serialization$(BOOST_INFIX) -lboost_system$(BOOST_INFIX) -lboost_filesystem$(BOOST_INFIX)
I_NETCDF=-I$(NETCDF_INCLUDE_DIR) -I$(NETCDF_CXX_INCLUDE_DIR)
L_NETCDF=-L$(NETCDF_LIB_DIR) -L$(NETCDF_CXX_LIB_DIR)
l_NETCDF=-lnetcdf_c++ -lnetcdf
I_CURL=-I$(CURL_INCLUDE_DIR)
L_CURL=-L$(CURL_LIB_DIR)
l_CURL=-lcurl

RPATHS :=
ifneq ($(NETCDF_LIB_DIR),)
  RPATHS += -Wl,-rpath -Wl,$(NETCDF_LIB_DIR)
endif

ifneq ($(NETCDF_CXX_LIB_DIR),)
  RPATHS += -Wl,-rpath -Wl,$(NETCDF_CXX_LIB_DIR)
endif

ifneq ($(BOOST_LIB_DIR),)
  RPATHS += -Wl,-rpath -Wl,$(BOOST_LIB_DIR)
endif
RELOGO_RPATHS = $(RPATHS) -Wl,-rpath -Wl,$(INSTALL_DIR)/lib

# These are are set in the modules included below
REPAST_HPC_SRC :=
RELOGO_SRC :=
RUMOR_SRC :=
ZOMBIE_SRC :=

include ../src/repast_hpc/module.mk
include ../src/relogo/module.mk
include ../src/rumor/module.mk
include ../src/zombie/module.mk

# SOURCE FILES
# Repast HPC
REPAST_HPC_SOURCES=$(addprefix ../, $(REPAST_HPC_SRC))
# ReLogo
RELOGO_SOURCES=$(addprefix ../, $(RELOGO_SRC))
# Rumor Model
RUMOR_SOURCES=$(addprefix ../, $(RUMOR_SRC))
# Zombie Model
ZOMBIE_SOURCES=$(addprefix ../, $(ZOMBIE_SRC))

# OBJECT FILES
REPAST_HPC_OBJECTS=$(patsubst %.cpp,%.o,$(REPAST_HPC_SOURCES))
RELOGO_OBJECTS=$(patsubst %.cpp,%.o,$(RELOGO_SOURCES))
RUMOR_OBJECTS=$(patsubst %.cpp,%.o,$(RUMOR_SOURCES))
ZOMBIE_OBJECTS=$(patsubst %.cpp,%.o,$(ZOMBIE_SOURCES))

REPAST_VERSION=2.2
REPAST_HPC_NAME=repast_hpc-$(REPAST_VERSION)
l_REPAST_HPC=-l$(REPAST_HPC_NAME)

DYN_FLAG = -shared -fPIC
LIB_EXT = so
LIB_CXXLD = $(CXXLD)
LIB_L_FLAGS = $(L_BOOST) $(L_NETCDF) $(L_CURL)
LIB_l_FLAGS = $(l_BOOST) $(l_NETCDF) $(l_CURL)
RELOGO_L_FLAGS = $(LIB_L_FLAGS) -L$(INSTALL_DIR)/lib
RELOGO_l_FLAGS = $(LIB_l_FLAGS) $(l_REPAST_HPC)
# need to able to set this to empty for the static compile
OUTPUT_FLAG = -o

EXE_FLAG =
ifeq ($(STATIC), true)
	DYN_FLAG =
	LIB_EXT = a
	LIB_CXXLD = ar rcs
	LIB_L_FLAGS =
	LIB_l_FLAGS =
	
	RELOGO_L_FLAGS =
	RELOGO_l_FLAGS =
	OUTPUT_FLAG =
	RPATHS =
	RELOGO_RPATHS =
	EXE_FLAG = -static
else
	UNAME := $(shell uname)
	ifeq ($(UNAME), Darwin)
		DYN_FLAG = -dynamiclib
		LIB_EXT = dylib
	endif
endif

# TARGETS
REPAST_HPC_LIB=$(INSTALL_DIR)/lib/lib$(REPAST_HPC_NAME).$(LIB_EXT)
RELOGO_NAME=relogo-$(REPAST_VERSION)
RELOGO_LIB=$(INSTALL_DIR)/lib/lib$(RELOGO_NAME).$(LIB_EXT)
RUMOR_EXE=$(INSTALL_DIR)/bin/rumor/rumor_model
ZOMBIE_EXE=$(INSTALL_DIR)/bin/zombie/zombie_model

l_RELOGO=-l$(RELOGO_NAME)

# Default rule makes everything
.PHONY : all
all: copy_includes repast_hpc relogo rumor_model zombie_model

.PHONY : copy_includes
copy_includes:
	mkdir -p $(INSTALL_DIR)/include/repast_hpc
	mkdir -p $(INSTALL_DIR)/include/relogo
	cp $(REPAST_HPC_DIR)/*.h $(INSTALL_DIR)/include/repast_hpc/
	cp $(RELOGO_DIR)/*.h $(INSTALL_DIR)/include/relogo/

$(REPAST_HPC_LIB): $(REPAST_HPC_OBJECTS)
	mkdir -p $(INSTALL_DIR)/lib
	rm -f $(REPAST_HPC_LIB)
	$(LIB_CXXLD) $(DYN_FLAG) $(LIB_L_FLAGS) $(OUTPUT_FLAG) $(REPAST_HPC_LIB) $(REPAST_HPC_OBJECTS) $(LIB_l_FLAGS) $(RPATHS)

$(RELOGO_LIB): $(RELOGO_OBJECTS)
	mkdir -p $(INSTALL_DIR)/lib
	rm -f $(RELOGO_LIB)
	$(LIB_CXXLD) $(DYN_FLAG) $(RELOGO_L_FLAGS) $(OUTPUT_FLAG) $(RELOGO_LIB) $(RELOGO_OBJECTS) $(RELOGO_l_FLAGS) $(RELOGO_RPATHS)


$(RUMOR_EXE): $(REPAST_HPC_LIB) $(RUMOR_OBJECTS)
	mkdir -p $(INSTALL_DIR)/bin/rumor
	cp $(RUMOR_DIR)/config.props $(INSTALL_DIR)/bin/rumor/
	cp $(RUMOR_DIR)/model.props $(INSTALL_DIR)/bin/rumor/
	$(CXXLD) $(EXE_FLAG) -L$(INSTALL_DIR)/lib/ $(LIB_L_FLAGS) $(RUMOR_OBJECTS) -o $(RUMOR_EXE) $(l_REPAST_HPC) $(LIB_l_FLAGS) $(RELOGO_RPATHS)

$(ZOMBIE_EXE): $(REPAST_HPC_LIB) $(RELOGO_LIB) $(ZOMBIE_OBJECTS)
	mkdir -p $(INSTALL_DIR)/bin/zombie
	cp $(ZOMBIE_DIR)/config.props $(INSTALL_DIR)/bin/zombie/
	cp $(ZOMBIE_DIR)/model.props $(INSTALL_DIR)/bin/zombie/
	$(CXXLD) $(EXE_FLAG) -L$(INSTALL_DIR)/lib $(RELOGO_L_FLAGS) $(ZOMBIE_OBJECTS) -o $(ZOMBIE_EXE) $(l_RELOGO) $(RELOGO_l_FLAGS) $(RELOGO_RPATHS)

# User-friendly names
.PHONY : repast_hpc
repast_hpc: $(REPAST_HPC_LIB)

.PHONY : relogo
relogo: $(RELOGO_LIB)

.PHONY : rumor_model
rumor_model: $(RUMOR_EXE)

.PHONY : zombie_model
zombie_model: $(ZOMBIE_EXE)


# Cleanup
.PHONY : repast_hpc_clean
repast_hpc_clean:
	rm -f $(REPAST_HPC_LIB)
	rm -f $(REPAST_HPC_DIR)/*.o

.PHONY : relogo_clean
relogo_clean :
	rm -f $(RELOGO_LIB)
	rm -f $(RELOGO_DIR)/*.o

.PHONY : rumor_clean
rumor_clean :
	rm -f $(RUMOR_EXE)
	rm -f $(RUMOR_DIR)/*.o

.PHONY : zombie_clean
zombie_clean :
	rm -f $(ZOMBIE_EXE)
	rm -f $(ZOMBIE_DIR)/*.o

.PHONY : clean
clean: repast_hpc_clean relogo_clean rumor_clean zombie_clean

.PHONY : distclean
distclean: clean
	rm -fr ../include
	rm -fr ../lib
	rm -fr ../bin


# Object files are built from cpp files
%.o:
	$(CXX) -fPIC -std=c++11 -c $(I_BOOST) $(I_NETCDF) $(I_CURL) -I$(SRC_DIR) -g0 -O3 -o $@ $*.cpp

# Cancel implicit rule for name with no suffix, to avoid confusing output:
%: %.o
