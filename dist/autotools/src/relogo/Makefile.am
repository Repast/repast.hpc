lib_LTLIBRARIES = librelogo-2.0.0.la
librelogo_2_0_0_la_SOURCES = AbstractRelogoAgent.cpp \
	AgentSet.cpp \
	Observer.cpp \
	Patch.cpp \
	RandomMove.cpp \
	RelogoAgent.cpp \
	RelogoContinuousSpaceAdder.cpp \
	RelogoDiscreteSpaceAdder.cpp \
	RelogoLink.cpp \
	Turtle.cpp \
	WorldCreator.cpp \
	WorldDefinition.cpp \
	creators.cpp \
	utility.cpp

headers = AbstractRelogoAgent.h \
	AgentSet.h \
	Observer.h \
	Patch.h \
	RandomMove.h \
	RelogoAgent.h \
	RelogoContinuousSpaceAdder.h \
	RelogoDiscreteSpaceAdder.h \
	RelogoErrors.h \
	RelogoLink.h \
	RelogoSharedContinuousSpace.h \
	RelogoSharedDiscreteSpace.h \
	SimulationRunner.h \
	Turtle.h \
	WorldCreator.h \
	WorldDefinition.h \
	agent_set_functions.h \
	creators.h \
	grid_types.h \
	relogo.h \
	utility.h

relogodir = $(includedir)/relogo

relogo_HEADERS = $(headers)
			  
librelogo_2_0_0_la_CPPFLAGS = $(BOOST_CPPFLAGS) -I ..
librelogo_2_0_0_la_LDFLAGS = $(BOOST_MPI_LDFLAGS)
librelogo_2_0_0_la_LIBS = $(BOOST_MPI_LIBS)
librelogo_2_0_0_la_LIBADD = ../repast_hpc/librepast_hpc-2.0.0.la
