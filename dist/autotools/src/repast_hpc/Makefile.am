lib_LTLIBRARIES = librepast_hpc-1.0.1.la
librepast_hpc_1_0_1_la_SOURCES = AgentExporter.cpp \
          AgentId.cpp \
          AgentImporter.cpp \
          AgentRequest.cpp \
          AgentStatus.cpp \
          Graph.cpp \
          GridComponents.cpp \
          initialize_random.cpp \
          io.cpp \
          logger.cpp \
          NCDataSet.cpp \
          NCDataSetBuilder.cpp \
          NetworkBuilder.cpp \
          Point.cpp \
          Properties.cpp \
          Random.cpp \
          RepastProcess.cpp \
          RequestManager.cpp \
          Schedule.cpp \
          SharedContext.cpp \
          SharedGrid.cpp \
          SharedNetwork.cpp \
          spatial_math.cpp \
          SRManager.cpp \
          SVDataSetBuilder.cpp \
          SVDataSet.cpp \
          Utilities.cpp \
          ValueLayer.cpp \
          Variable.cpp
          
headers = AgentExporter.h \
		AgentId.h \
		AgentImporter.h \
		AgentRequest.h \
		AgentStatus.h \
		BaseGrid.h \
		Context.h \
		DataSet.h \
		SVDataSetBuilder.h \
		SVDataSource.h \
		SVDataSet.h \
		Edge.h \
		Graph.h \
		Grid.h \
		Grid2DQuery.h \
		GridComponents.h \
		GridMovePackets.h \
		Moore2DGridQuery.h \
		MultipleOccupancy.h \
		NCDataSet.h \
		NCDataSetBuilder.h \
		NCDataSource.h \
		NCReducibleDataSource.h \
		NetworkBuilder.h \
		Point.h \
		Projection.h \
		Properties.h \
		Random.h \
		ReducibleDataSource.h \
		RepastProcess.h \
		RequestManager.h \
		Schedule.h \
		SharedContext.h \
		SharedGrid.h \
		SharedNetwork.h \
		SharedSpace.h \
                SRManager.h \
		SingleOccupancy.h \
		Space.h \
		TDataSource.h \
		Utilities.h \
		VN2DGridQuery.h \
		ValueLayer.h \
		Variable.h \
		initialize_random.h \
		io.h \
		logger.h \
		matrix.h \
		mpi_constants.h \
		spatial_math.h \
		Vertex.h \
		DirectedVertex.h \
		UndirectedVertex.h 

repast_hpcdir = $(includedir)/repast_hpc

repast_hpc_HEADERS = $(headers)
			  
librepast_hpc_1_0_1_la_CPPFLAGS = $(BOOST_CPPFLAGS)
librepast_hpc_1_0_1_la_LDFLAGS = $(BOOST_MPI_LDFLAGS) $(BOOST_SERIALIZATION_LDFLAGS) \
		$(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_SYSTEM_LDFLAGS)
librepast_hpc_1_0_1_la_LIBS = $(BOOST_MPI_LIBS) $(BOOST_SERIALIZATION_LIBS) \
		$(BOOST_FILESYSTEM_LIBS) $(BOOST_SYSTEM_LIBS)
librepast_hpc_1_0_1_la_LIBADD = $(BOOST_MPI_LIBS) $(BOOST_SERIALIZATION_LIBS) \
		$(BOOST_FILESYSTEM_LIBS) $(BOOST_SYSTEM_LIBS)
