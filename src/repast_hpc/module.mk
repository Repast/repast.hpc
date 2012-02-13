SOURCES = AgentId.cpp \
          SVDataSet.cpp \
          SVDataSetBuilder.cpp \
          Graph.cpp \
          GridComponents.cpp \
          initialize_random.cpp \
          io.cpp \
          logger.cpp \
          mpiio.cpp \
          NetworkBuilder.cpp \
          Point.cpp \
          Properties.cpp \
          Random.cpp \
          RepastProcess.cpp \
          Schedule.cpp \
          SharedGrid.cpp \
          SharedNetwork.cpp \
          Utilities.cpp \
          ValueLayer.cpp \
          spatial_math.cpp \
          AgentRequest.cpp \
          AgentStatus.cpp \
          AgentExporter.cpp \
          AgentImporter.cpp \
          RequestManager.cpp \
          SharedContext.cpp \
          Variable.cpp \
          NCDataSet.cpp \
          NCDataSetBuilder.cpp

local_dir := src/repast_hpc
local_src := $(addprefix $(local_dir)/, $(SOURCES))

sources += $(local_src)
