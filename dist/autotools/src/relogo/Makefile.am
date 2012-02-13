lib_LTLIBRARIES = librelogo-1.0.1.la
librelogo_1_0_1_la_SOURCES = AbstractRelogoAgent.cpp \
                AgentSet.cpp \
		Observer.cpp \
		Patch.cpp \
		RandomMove.cpp \
		RelogoAgent.cpp \
		RelogoGridAdder.cpp \
		RelogoLink.cpp \
		RelogoSpaceAdder.cpp \
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
		RelogoGridAdder.h \
		RelogoLink.h \
		RelogoSharedGrid.h \
		RelogoSharedSpace.h \
		RelogoSpaceAdder.h \
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
			  
librelogo_1_0_1_la_CPPFLAGS = $(BOOST_CPPFLAGS) -I ..
librelogo_1_0_1_la_LDFLAGS = $(BOOST_MPI_LDFLAGS)
librelogo_1_0_1_la_LIBS = $(BOOST_MPI_LIBS)
librelogo_1_0_1_la_LIBADD = ../repast_hpc/librepast_hpc-1.0.1.la
