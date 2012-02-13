zombiesdir = $(abs_top_srcdir)/examples_bin/zombies
zombies_PROGRAMS = zombie_model

zombie_model_SOURCES = AgentPackage.h \
Human.cpp \
Human.h \
Zombie.cpp \
Zombie.h \
ZombieObserver.cpp \
ZombieObserver.h \
main.cpp \
InfectionSum.cpp \
InfectionSum.h

zombie_model_CPPFLAGS = $(BOOST_CPPFLAGS) -I ..
zombie_model_LDFLAGS = $(BOOST_MPI_LDFLAGS) $(BOOST_SERIALIZATION_LDFLAGS)
zombie_model_LDADD = ../repast_hpc/librepast_hpc-1.0.1.la ../relogo/librelogo-1.0.1.la $(BOOST_MPI_LIBS) $(BOOST_SERIALIZATION_LIBS) -lnetcdf_c++

zombies_DATA = model.props config.props

EXTRA_DIST = model.props config.props
