rumordir = $(abs_top_srcdir)/examples_bin/rumor
rumor_PROGRAMS = rumor_model

rumor_model_SOURCES = RumorModel.cpp \
		RumorModel.h \
		main.cpp

rumor_model_CPPFLAGS = $(BOOST_CPPFLAGS) -I ..
rumor_model_LDFLAGS = $(BOOST_MPI_LDFLAGS) $(BOOST_SERIALIZATION_LDFLAGS)
rumor_model_LDADD = ../repast_hpc/librepast_hpc-1.0.1.la ../relogo/librelogo-1.0.1.la $(BOOST_MPI_LIBS) $(BOOST_SERIALIZATION_LIBS) -lnetcdf_c++

rumor_DATA = model.props config.props README.txt

EXTRA_DIST = model.props config.props README.txt
