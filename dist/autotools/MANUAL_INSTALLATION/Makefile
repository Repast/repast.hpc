# Repast HPC Manual Build Makefile

# VARIABLES (Supply values for these; for definitions and examples, see INSTALL)
CXX=/usr/local/bin/mpicxx
CXXLD=/usr/local/bin/mpicxx
BOOST_INCLUDE_DIR=/Users/murphy/ext/boost_1_46_1/
BOOST_LIB_DIR=/Users/murphy/ext/boost_1_46_1/stage/lib/
BOOST_INFIX=-xgcc42-1_46_1
NETCDF_INCLUDE_DIR=/usr/local/include/
NETCDF_LIB_DIR=/usr/local/lib/
CURL_INCLUDE_DIR=/usr/local/include/
CURL_LIB_DIR=/usr/local/lib/

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
I_NETCDF=-I$(NETCDF_INCLUDE_DIR)
L_NETCDF=-L$(NETCDF_LIB_DIR)
l_NETCDF=-lnetcdf_c++ -lnetcdf
I_CURL=-I$(CURL_INCLUDE_DIR)
L_CURL=-L$(CURL_LIB_DIR)
l_CURL=-lcurl


# SOURCE FILES
# Repast HPC
REPAST_HPC_SOURCES=$(addprefix $(REPAST_HPC_DIR)/, \
AgentExporter.cpp    AgentId.cpp           AgentImporter.cpp   AgentRequest.cpp \
AgentStatus.cpp      Graph.cpp             GridComponents.cpp  NCDataSet.cpp \
NCDataSetBuilder.cpp NetworkBuilder.cpp    Point.cpp           Properties.cpp \
Random.cpp           RepastProcess.cpp     RequestManager.cpp  SRManager.cpp \
SVDataSet.cpp        SVDataSetBuilder.cpp  Schedule.cpp        SharedContext.cpp \
SharedGrid.cpp       SharedNetwork.cpp     Utilities.cpp       ValueLayer.cpp \
Variable.cpp         initialize_random.cpp io.cpp              logger.cpp \
spatial_math.cpp )

# ReLogo
RELOGO_SOURCES=$(addprefix $(RELOGO_DIR)/, \
AbstractRelogoAgent.cpp   AgentSet.cpp    Observer.cpp        Patch.cpp \
RandomMove.cpp            RelogoAgent.cpp RelogoGridAdder.cpp RelogoLink.cpp \
RelogoSpaceAdder.cpp      Turtle.cpp      WorldCreator.cpp    WorldDefinition.cpp \
creators.cpp utility.cpp )

# Rumor Model
RUMOR_SOURCES=$(addprefix $(RUMOR_DIR)/, \
RumorModel.cpp  main.cpp)

# Zombie Model
ZOMBIE_SOURCES=$(addprefix $(ZOMBIE_DIR)/, \
Human.cpp   InfectionSum.cpp   Zombie.cpp   ZombieObserver.cpp \
main.cpp)


# OBJECT FILES
REPAST_HPC_OBJECTS=$(patsubst %.cpp,%.o,$(REPAST_HPC_SOURCES))
RELOGO_OBJECTS=$(patsubst %.cpp,%.o,$(RELOGO_SOURCES))
RUMOR_OBJECTS=$(patsubst %.cpp,%.o,$(RUMOR_SOURCES))
ZOMBIE_OBJECTS=$(patsubst %.cpp,%.o,$(ZOMBIE_SOURCES))


# TARGETS
REPAST_VERSION=1.0.1
REPAST_HPC_NAME=repast_hpc-$(REPAST_VERSION)
REPAST_HPC_LIB=./lib/lib$(REPAST_HPC_NAME).a
RELOGO_NAME=relogo-$(REPAST_VERSION)
RELOGO_LIB=./lib/lib$(RELOGO_NAME).a
RUMOR_EXE=./bin/rumor_model
ZOMBIE_EXE=./bin/zombie_model

l_REPAST_HPC=-l$(REPAST_HPC_NAME)
l_RELOGO=-l$(RELOGO_NAME)


# Default rule makes everything
.PHONY : all
all: copy_includes repast_hpc relogo rumor_model zombie_model

.PHONY : copy_includes
copy_includes:
	mkdir -p ./include/repast_hpc
	mkdir -p ./include/relogo
	cp $(REPAST_HPC_DIR)/*.h ./include/repast_hpc/
	cp $(RELOGO_DIR)/*.h ./include/relogo/

$(REPAST_HPC_LIB): $(REPAST_HPC_OBJECTS)
	mkdir -p ./lib
	rm -f $(REPAST_HPC_LIB)
	ar rc $(REPAST_HPC_LIB) $(REPAST_HPC_OBJECTS)
	ranlib $(REPAST_HPC_LIB)

$(RELOGO_LIB): $(RELOGO_OBJECTS)
	mkdir -p ./lib
	rm -f $(RELOGO_LIB)
	ar rc $(RELOGO_LIB) $(RELOGO_OBJECTS)
	ranlib $(RELOGO_LIB)

$(RUMOR_EXE): $(REPAST_HPC_LIB) $(RUMOR_OBJECTS)
	mkdir -p ./bin
	cp $(RUMOR_DIR)/config.props ./bin/rumor_config.props
	cp $(RUMOR_DIR)/model.props ./bin/rumor_model.props
	$(CXXLD) -L./lib/ $(L_BOOST) $(L_NETCDF) $(RUMOR_OBJECTS) $(l_NETCDF) $(l_REPAST_HPC) $(l_BOOST) -o $(RUMOR_EXE)

$(ZOMBIE_EXE): $(REPAST_HPC_LIB) $(RELOGO_LIB) $(ZOMBIE_OBJECTS)
	mkdir -p ./bin
	cp $(ZOMBIE_DIR)/config.props ./bin/zombie_config.props
	cp $(ZOMBIE_DIR)/model.props ./bin/zombie_model.props
	$(CXXLD) -L./lib/ $(L_BOOST) $(L_NETCDF) $(L_CURL) $(ZOMBIE_OBJECTS) $(l_NETCDF) $(l_CURL) $(l_RELOGO) $(l_REPAST_HPC) $(l_BOOST) -o $(ZOMBIE_EXE)

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
	rm -fr ./include
	rm -fr ./lib
	rm -fr ./bin


# Object files are built from cpp files
%.o:
	$(CXX) -c $(I_BOOST) $(I_NETCDF) $(I_CURL) -I$(SRC_DIR) -o $@ $*.cpp
	
# Cancel implicit rule for name with no suffix, to avoid confusing output:
%: %.o
