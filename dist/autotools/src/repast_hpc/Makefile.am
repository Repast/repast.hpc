lib_LTLIBRARIES = librepast_hpc-2.0.0.la
librepast_hpc_2_0_0_la_SOURCES = AgentId.cpp \
	AgentImporterExporter.cpp \
	AgentRequest.cpp \
	AgentStatus.cpp \
	Graph.cpp \
	GridComponents.cpp \
	GridDimensions.cpp \
	NCDataSet.cpp \
	NCDataSetBuilder.cpp \
	NetworkBuilder.cpp \
	Properties.cpp \
	Random.cpp \
	RepastErrors.cpp \
	RepastProcess.cpp \
	SRManager.cpp \
	SVDataSet.cpp \
	SVDataSetBuilder.cpp \
	Schedule.cpp \
	SharedBaseGrid.cpp \
	SharedContext.cpp \
	SharedNetwork.cpp \
	Utilities.cpp \
	ValueLayer.cpp \
	Variable.cpp \
	initialize_random.cpp \
	io.cpp \
	logger.cpp \
	spatial_math.cpp

          
headers = AgentId.h \
	AgentImporterExporter.h \
	AgentRequest.h \
	AgentStatus.h \
	BaseGrid.h \
	Context.h \
	DataSet.h \
	DirectedVertex.h \
	Edge.h \
	Graph.h \
	Grid.h \
	Grid2DQuery.h \
	GridComponents.h \
	GridDimensions.h \
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
	RepastErrors.h \
	RepastProcess.h \
	SRManager.h \
	SVDataSet.h \
	SVDataSetBuilder.h \
	SVDataSource.h \
	Schedule.h \
	SharedBaseGrid.h \
	SharedContext.h \
	SharedContinuousSpace.h \
	SharedDiscreteSpace.h \
	SharedNetwork.h \
	SharedSpaces.h \
	SingleOccupancy.h \
	Spaces.h \
	TDataSource.h \
	UndirectedVertex.h \
	Utilities.h \
	VN2DGridQuery.h \
	ValueLayer.h \
	Variable.h \
	Vertex.h \
	initialize_random.h \
	io.h \
	logger.h \
	matrix.h \
	mpi_constants.h \
	spatial_math.h


repast_hpcdir = $(includedir)/repast_hpc

repast_hpc_HEADERS = $(headers)
			  
librepast_hpc_2_0_0_la_CPPFLAGS = $(BOOST_CPPFLAGS)
librepast_hpc_2_0_0_la_LDFLAGS = $(BOOST_MPI_LDFLAGS) $(BOOST_SERIALIZATION_LDFLAGS) \
		$(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_SYSTEM_LDFLAGS)
librepast_hpc_2_0_0_la_LIBS = $(BOOST_MPI_LIBS) $(BOOST_SERIALIZATION_LIBS) \
		$(BOOST_FILESYSTEM_LIBS) $(BOOST_SYSTEM_LIBS)
librepast_hpc_2_0_0_la_LIBADD = $(BOOST_MPI_LIBS) $(BOOST_SERIALIZATION_LIBS) \
		$(BOOST_FILESYSTEM_LIBS) $(BOOST_SYSTEM_LIBS)
