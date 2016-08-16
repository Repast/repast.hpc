#************************************************************************************************************
#
# Repast HPC Tutorial Makefile
#
#************************************************************************************************************

include ./env

.PHONY: all
all : 


.PHONY: clean_output_files
clean_output_files:
	rm -f *.csv
	rm -f *.txt
	rm -f ./output/*.csv
	rm -f ./output/*.txt
	rm -f ./logs/*.*

.PHONY: clean_compiled_files
clean_compiled_files:
	rm -f *.exe
	rm -f ./bin/*.exe
	rm -f *.o
	rm -f ./object/*.o
	
.PHONY: clean
clean: clean_compiled_files clean_output_files remove_subdirectories
	rm -f *.cpp
	rm -f ./src/*.cpp
	rm -f *.props
	rm -f ./props/*.props
	rm -f ./include/*.h


.PHONY: RepastHPC_Demo_00
RepastHPC_Demo_00: clean_compiled_files
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -c Demo_00.cpp -o Demo_00.o
	$(MPICXX) $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) -o Demo_00.exe Demo_00.o $(REPAST_HPC_LIB) $(BOOST_LIBS)

.PHONY: RepastHPC_Demo_00_Step_01
RepastHPC_Demo_00_Step_01 : copy_RepastHPC_Demo_00_Step_01 RepastHPC_Demo_00

.PHONY: copy_RepastHPC_Demo_00_Step_01
copy_RepastHPC_Demo_00_Step_01 : clean
	cp ../RepastHPC/Demo_00/Step_01/* ./

.PHONY: compare_RepastHPC_Demo_00_Step_01
compare_RepastHPC_Demo_00_Step_01:
	@echo "Comparing Demo_00/Step_01/Demo_00.cpp…"
	-@diff ../RepastHPC/Demo_00/Step_01/Demo_00.cpp Demo_00.cpp


.PHONY: RepastHPC_Demo_00_Step_02
RepastHPC_Demo_00_Step_02 : copy_RepastHPC_Demo_00_Step_02 RepastHPC_Demo_00

.PHONY: copy_RepastHPC_Demo_00_Step_02
copy_RepastHPC_Demo_00_Step_02 : clean
	cp ../RepastHPC/Demo_00/Step_02/* ./

.PHONY: compare_RepastHPC_Demo_00_Step_02
compare_RepastHPC_Demo_00_Step_02:
	@echo "Comparing Demo_00/Step_02/Demo_00.cpp…"
	-@diff ../RepastHPC/Demo_00/Step_02/Demo_00.cpp Demo_00.cpp


.PHONY: RepastHPC_Demo_00_Step_03
RepastHPC_Demo_00_Step_03 : copy_RepastHPC_Demo_00_Step_03 RepastHPC_Demo_00

.PHONY: copy_RepastHPC_Demo_00_Step_03
copy_RepastHPC_Demo_00_Step_03 : clean
	cp ../RepastHPC/Demo_00/Step_03/* ./

.PHONY: compare_RepastHPC_Demo_00_Step_03
compare_RepastHPC_Demo_00_Step_03:
	@echo "Comparing Demo_00/Step_03/Demo_00.cpp…"
	-@diff ../RepastHPC/Demo_00/Step_03/Demo_00.cpp Demo_00.cpp

.PHONY: RepastHPC_Demo_00_Step_04
RepastHPC_Demo_00_Step_04 : copy_RepastHPC_Demo_00_Step_04 RepastHPC_Demo_00

.PHONY: copy_RepastHPC_Demo_00_Step_04
copy_RepastHPC_Demo_00_Step_04 : clean
	cp ../RepastHPC/Demo_00/Step_04/* ./

.PHONY: compare_RepastHPC_Demo_00_Step_04
compare_RepastHPC_Demo_00_Step_04:
	@echo "Comparing Demo_00/Step_04/Demo_00.cpp…"
	-@diff ../RepastHPC/Demo_00/Step_04/Demo_00.cpp Demo_00.cpp

.PHONY: RepastHPC_Demo_00_Step_05
RepastHPC_Demo_00_Step_05 : copy_RepastHPC_Demo_00_Step_05 RepastHPC_Demo_00

.PHONY: copy_RepastHPC_Demo_00_Step_05
copy_RepastHPC_Demo_00_Step_05 : clean
	cp ../RepastHPC/Demo_00/Step_05/* ./

.PHONY: compare_RepastHPC_Demo_00_Step_05
compare_RepastHPC_Demo_00_Step_05:
	@echo "Comparing Demo_00/Step_05/Demo_00.cpp…"
	-@diff ../RepastHPC/Demo_00/Step_05/Demo_00.cpp Demo_00.cpp


.PHONY: RepastHPC_Demo_00_Step_06
RepastHPC_Demo_00_Step_06 : copy_RepastHPC_Demo_00_Step_06 RepastHPC_Demo_00

.PHONY: copy_RepastHPC_Demo_00_Step_06
copy_RepastHPC_Demo_00_Step_06 : clean
	cp ../RepastHPC/Demo_00/Step_06/* ./

.PHONY: compare_RepastHPC_Demo_00_Step_06
compare_RepastHPC_Demo_00_Step_06:
	@echo "Comparing Demo_00/Step_06/Demo_00.cpp…"
	-@diff ../RepastHPC/Demo_00/Step_06/Demo_00.cpp Demo_00.cpp


.PHONY: RepastHPC_Demo_00_Step_07
RepastHPC_Demo_00_Step_07 : copy_RepastHPC_Demo_00_Step_07 RepastHPC_Demo_00

.PHONY: copy_RepastHPC_Demo_00_Step_07
copy_RepastHPC_Demo_00_Step_07 : clean
	cp ../RepastHPC/Demo_00/Step_07/* ./

.PHONY: compare_RepastHPC_Demo_00_Step_07
compare_RepastHPC_Demo_00_Step_07:
	@echo "Comparing Demo_00/Step_07/Demo_00.cpp…"
	-@diff ../RepastHPC/Demo_00/Step_07/Demo_00.cpp Demo_00.cpp


.PHONY: RepastHPC_Demo_00_Step_08
RepastHPC_Demo_00_Step_08 : copy_RepastHPC_Demo_00_Step_08 RepastHPC_Demo_00

.PHONY: copy_RepastHPC_Demo_00_Step_08
copy_RepastHPC_Demo_00_Step_08 : clean
	cp ../RepastHPC/Demo_00/Step_08/* ./

.PHONY: compare_RepastHPC_Demo_00_Step_08
compare_RepastHPC_Demo_00_Step_08:
	@echo "Comparing Demo_00/Step_08/Demo_00.cpp…"
	-@diff ../RepastHPC/Demo_00/Step_08/Demo_00.cpp Demo_00.cpp


.PHONY: RepastHPC_Demo_00_Step_09
RepastHPC_Demo_00_Step_09 : copy_RepastHPC_Demo_00_Step_09 RepastHPC_Demo_00

.PHONY: copy_RepastHPC_Demo_00_Step_09
copy_RepastHPC_Demo_00_Step_09 : clean
	cp ../RepastHPC/Demo_00/Step_09/* ./

.PHONY: compare_RepastHPC_Demo_00_Step_09
compare_RepastHPC_Demo_00_Step_09:
	@echo "Comparing Demo_00/Step_09/Demo_00.cpp…"
	-@diff ../RepastHPC/Demo_00/Step_09/Demo_00.cpp Demo_00.cpp

.PHONY: RepastHPC_Demo_00_Step_10
RepastHPC_Demo_00_Step_10 : copy_RepastHPC_Demo_00_Step_10 RepastHPC_Demo_00

.PHONY: copy_RepastHPC_Demo_00_Step_10
copy_RepastHPC_Demo_00_Step_10 : clean
	cp ../RepastHPC/Demo_00/Step_10/* ./

.PHONY: compare_RepastHPC_Demo_00_Step_10
compare_RepastHPC_Demo_00_Step_10:
	@echo "Comparing Demo_00/Step_10/Demo_00.cpp…"
	-@diff ../RepastHPC/Demo_00/Step_10/Demo_00.cpp Demo_00.cpp


.PHONY: RepastHPC_Demo_00_Step_11
RepastHPC_Demo_00_Step_11 : copy_RepastHPC_Demo_00_Step_11 RepastHPC_Demo_00

.PHONY: copy_RepastHPC_Demo_00_Step_11
copy_RepastHPC_Demo_00_Step_11 : clean
	cp ../RepastHPC/Demo_00/Step_11/* ./

.PHONY: compare_RepastHPC_Demo_00_Step_11
compare_RepastHPC_Demo_00_Step_11:
	@echo "Comparing Demo_00/Step_11/Demo_00.cpp…"
	-@diff ../RepastHPC/Demo_00/Step_11/Demo_00.cpp Demo_00.cpp


.PHONY: RepastHPC_Demo_00_Step_12
RepastHPC_Demo_00_Step_12 : copy_RepastHPC_Demo_00_Step_12 RepastHPC_Demo_00

.PHONY: copy_RepastHPC_Demo_00_Step_12
copy_RepastHPC_Demo_00_Step_12 : clean
	cp ../RepastHPC/Demo_00/Step_12/* ./

.PHONY: compare_RepastHPC_Demo_00_Step_12
compare_RepastHPC_Demo_00_Step_12:
	@echo "Comparing Demo_00/Step_12/Demo_00.cpp…"
	-@diff ../RepastHPC/Demo_00/Step_12/Demo_00.cpp Demo_00.cpp


.PHONY: RepastHPC_Demo_00_Step_13
RepastHPC_Demo_00_Step_13 : copy_RepastHPC_Demo_00_Step_13 RepastHPC_Demo_00

.PHONY: copy_RepastHPC_Demo_00_Step_13
copy_RepastHPC_Demo_00_Step_13 : clean
	cp ../RepastHPC/Demo_00/Step_13/* ./

.PHONY: compare_RepastHPC_Demo_00_Step_13
compare_RepastHPC_Demo_00_Step_13:
	@echo "Comparing Demo_00/Step_13/Demo_00.cpp…"
	-@diff ../RepastHPC/Demo_00/Step_13/Demo_00.cpp Demo_00.cpp


.PHONY: RepastHPC_Demo_00_Step_14
RepastHPC_Demo_00_Step_14 : copy_RepastHPC_Demo_00_Step_14 RepastHPC_Demo_00

.PHONY: copy_RepastHPC_Demo_00_Step_14
copy_RepastHPC_Demo_00_Step_14 : clean
	cp ../RepastHPC/Demo_00/Step_14/* ./

.PHONY: compare_RepastHPC_Demo_00_Step_14
compare_RepastHPC_Demo_00_Step_14:
	@echo "Comparing Demo_00/Step_14/Demo_00.cpp…"
	-@diff ../RepastHPC/Demo_00/Step_14/Demo_00.cpp Demo_00.cpp


.PHONY: RepastHPC_Demo_00_Step_15
RepastHPC_Demo_00_Step_15 : copy_RepastHPC_Demo_00_Step_15 RepastHPC_Demo_00

.PHONY: copy_RepastHPC_Demo_00_Step_15
copy_RepastHPC_Demo_00_Step_15 : clean
	cp ../RepastHPC/Demo_00/Step_15/* ./

.PHONY: compare_RepastHPC_Demo_00_Step_15
compare_RepastHPC_Demo_00_Step_15:
	@echo "Comparing Demo_00/Step_15/Demo_00.cpp…"
	-@diff ../RepastHPC/Demo_00/Step_15/Demo_00.cpp Demo_00.cpp


.PHONY: remove_subdirectories
remove_subdirectories:
	rm -fr objects
	rm -fr output
	rm -fr bin
	rm -fr include
	rm -fr src
	rm -fr props
	mkdir objects
	mkdir output
	mkdir bin

.PHONY: RepastHPC_Demo_01
RepastHPC_Demo_01: clean_compiled_files
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_01_Main.cpp -o ./objects/Demo_01_Main.o
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_01_Model.cpp -o ./objects/Demo_01_Model.o
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_01_Agent.cpp -o ./objects/Demo_01_Agent.o
	$(MPICXX) $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) -o ./bin/Demo_01.exe  ./objects/Demo_01_Main.o ./objects/Demo_01_Model.o ./objects/Demo_01_Agent.o $(REPAST_HPC_LIB) $(BOOST_LIBS)


.PHONY: RepastHPC_Demo_01_Step_00
RepastHPC_Demo_01_Step_00 : copy_RepastHPC_Demo_01_Step_00 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_00
copy_RepastHPC_Demo_01_Step_00 : clean
	cp -r ../RepastHPC/Demo_01/Step_00/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_00
compare_RepastHPC_Demo_01_Step_00:
	@echo "Comparing Demo_01/Step_00/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_00/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_00/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_00/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_00/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_00/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_00/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_00/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_00/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_00/src/Demo_01_main.cpp ./src/Demo_01_main.cpp

.PHONY: RepastHPC_Demo_01_Step_01
RepastHPC_Demo_01_Step_01 : copy_RepastHPC_Demo_01_Step_01 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_01
copy_RepastHPC_Demo_01_Step_01 : clean
	cp -r ../RepastHPC/Demo_01/Step_01/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_01
compare_RepastHPC_Demo_01_Step_01:
	@echo "Comparing Demo_01/Step_01/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_01/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_01/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_01/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_01/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_01/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_01/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_01/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_01/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_01/src/Demo_01_main.cpp ./src/Demo_01_main.cpp


.PHONY: RepastHPC_Demo_01_Step_02
RepastHPC_Demo_01_Step_02 : copy_RepastHPC_Demo_01_Step_02 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_02
copy_RepastHPC_Demo_01_Step_02 : clean
	cp -r ../RepastHPC/Demo_01/Step_02/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_02
compare_RepastHPC_Demo_01_Step_02:
	@echo "Comparing Demo_01/Step_02/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_02/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_02/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_02/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_02/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_02/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_02/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_02/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_02/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_02/src/Demo_01_main.cpp ./src/Demo_01_main.cpp


.PHONY: RepastHPC_Demo_01_Step_03
RepastHPC_Demo_01_Step_03 : copy_RepastHPC_Demo_01_Step_03 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_03
copy_RepastHPC_Demo_01_Step_03 : clean
	cp -r ../RepastHPC/Demo_01/Step_03/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_03
compare_RepastHPC_Demo_01_Step_03:
	@echo "Comparing Demo_01/Step_03/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_03/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_03/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_03/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_03/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_03/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_03/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_03/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_03/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_03/src/Demo_01_main.cpp ./src/Demo_01_main.cpp

.PHONY: RepastHPC_Demo_01_Step_04
RepastHPC_Demo_01_Step_04 : copy_RepastHPC_Demo_01_Step_04 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_04
copy_RepastHPC_Demo_01_Step_04 : clean
	cp -r ../RepastHPC/Demo_01/Step_04/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_04
compare_RepastHPC_Demo_01_Step_04:
	@echo "Comparing Demo_01/Step_04/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_04/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_04/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_04/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_04/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_04/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_04/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_04/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_04/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_04/src/Demo_01_main.cpp ./src/Demo_01_main.cpp


.PHONY: RepastHPC_Demo_01_Step_05
RepastHPC_Demo_01_Step_05 : copy_RepastHPC_Demo_01_Step_05 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_05
copy_RepastHPC_Demo_01_Step_05 : clean
	cp -r ../RepastHPC/Demo_01/Step_05/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_05
compare_RepastHPC_Demo_01_Step_05:
	@echo "Comparing Demo_01/Step_05/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_05/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_05/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_05/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_05/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_05/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_05/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_05/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_05/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_05/src/Demo_01_main.cpp ./src/Demo_01_main.cpp


.PHONY: RepastHPC_Demo_01_Step_06
RepastHPC_Demo_01_Step_06 : copy_RepastHPC_Demo_01_Step_06 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_06
copy_RepastHPC_Demo_01_Step_06 : clean
	cp -r ../RepastHPC/Demo_01/Step_06/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_06
compare_RepastHPC_Demo_01_Step_06:
	@echo "Comparing Demo_01/Step_06/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_06/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_06/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_06/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_06/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_06/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_06/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_06/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_06/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_06/src/Demo_01_main.cpp ./src/Demo_01_main.cpp


.PHONY: RepastHPC_Demo_01_Step_07
RepastHPC_Demo_01_Step_07 : copy_RepastHPC_Demo_01_Step_07 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_07
copy_RepastHPC_Demo_01_Step_07 : clean
	cp -r ../RepastHPC/Demo_01/Step_07/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_07
compare_RepastHPC_Demo_01_Step_07:
	@echo "Comparing Demo_01/Step_07/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_07/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_07/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_07/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_07/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_07/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_07/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_07/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_07/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_07/src/Demo_01_main.cpp ./src/Demo_01_main.cpp


.PHONY: RepastHPC_Demo_01_Step_08
RepastHPC_Demo_01_Step_08 : copy_RepastHPC_Demo_01_Step_08 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_08
copy_RepastHPC_Demo_01_Step_08 : clean
	cp -r ../RepastHPC/Demo_01/Step_08/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_08
compare_RepastHPC_Demo_01_Step_08:
	@echo "Comparing Demo_01/Step_08/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_08/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_08/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_08/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_08/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_08/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_08/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_08/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_08/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_08/src/Demo_01_main.cpp ./src/Demo_01_main.cpp


.PHONY: RepastHPC_Demo_01_Step_09
RepastHPC_Demo_01_Step_09 : copy_RepastHPC_Demo_01_Step_09 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_09
copy_RepastHPC_Demo_01_Step_09 : clean
	cp -r ../RepastHPC/Demo_01/Step_09/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_09
compare_RepastHPC_Demo_01_Step_09:
	@echo "Comparing Demo_01/Step_09/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_09/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_09/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_09/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_09/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_09/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_09/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_09/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_09/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_09/src/Demo_01_main.cpp ./src/Demo_01_main.cpp


.PHONY: RepastHPC_Demo_01_Step_10
RepastHPC_Demo_01_Step_10 : copy_RepastHPC_Demo_01_Step_10 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_10
copy_RepastHPC_Demo_01_Step_10 : clean
	cp -r ../RepastHPC/Demo_01/Step_10/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_10
compare_RepastHPC_Demo_01_Step_10:
	@echo "Comparing Demo_01/Step_10/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_10/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_10/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_10/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_10/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_10/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_10/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_10/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_10/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_10/src/Demo_01_main.cpp ./src/Demo_01_main.cpp

.PHONY: RepastHPC_Demo_01_Step_11
RepastHPC_Demo_01_Step_11 : copy_RepastHPC_Demo_01_Step_11 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_11
copy_RepastHPC_Demo_01_Step_11 : clean
	cp -r ../RepastHPC/Demo_01/Step_11/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_11
compare_RepastHPC_Demo_01_Step_11:
	@echo "Comparing Demo_01/Step_11/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_11/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_11/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_11/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_11/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_11/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_11/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_11/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_11/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_11/src/Demo_01_main.cpp ./src/Demo_01_main.cpp


.PHONY: RepastHPC_Demo_01_Step_12
RepastHPC_Demo_01_Step_12 : copy_RepastHPC_Demo_01_Step_12 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_12
copy_RepastHPC_Demo_01_Step_12 : clean
	cp -r ../RepastHPC/Demo_01/Step_12/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_12
compare_RepastHPC_Demo_01_Step_12:
	@echo "Comparing Demo_01/Step_12/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_12/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_12/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_12/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_12/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_12/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_12/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_12/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_12/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_12/src/Demo_01_main.cpp ./src/Demo_01_main.cpp


.PHONY: RepastHPC_Demo_01_Step_13
RepastHPC_Demo_01_Step_13 : copy_RepastHPC_Demo_01_Step_13 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_13
copy_RepastHPC_Demo_01_Step_13 : clean
	cp -r ../RepastHPC/Demo_01/Step_13/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_13
compare_RepastHPC_Demo_01_Step_13:
	@echo "Comparing Demo_01/Step_13/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_13/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_13/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_13/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_13/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_13/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_13/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_13/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_13/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_13/src/Demo_01_main.cpp ./src/Demo_01_main.cpp

.PHONY: RepastHPC_Demo_01_Step_14
RepastHPC_Demo_01_Step_14 : copy_RepastHPC_Demo_01_Step_14 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_14
copy_RepastHPC_Demo_01_Step_14 : clean
	cp -r ../RepastHPC/Demo_01/Step_14/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_14
compare_RepastHPC_Demo_01_Step_14:
	@echo "Comparing Demo_01/Step_14/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_14/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_14/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_14/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_14/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_14/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_14/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_14/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_14/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_14/src/Demo_01_main.cpp ./src/Demo_01_main.cpp


.PHONY: RepastHPC_Demo_01_Step_15
RepastHPC_Demo_01_Step_15 : copy_RepastHPC_Demo_01_Step_15 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_15
copy_RepastHPC_Demo_01_Step_15 : clean
	cp -r ../RepastHPC/Demo_01/Step_15/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_15
compare_RepastHPC_Demo_01_Step_15:
	@echo "Comparing Demo_01/Step_15/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_15/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_15/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_15/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_15/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_15/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_15/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_15/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_15/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_15/src/Demo_01_main.cpp ./src/Demo_01_main.cpp



.PHONY: RepastHPC_Demo_01_Step_16
RepastHPC_Demo_01_Step_16 : copy_RepastHPC_Demo_01_Step_16 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_16
copy_RepastHPC_Demo_01_Step_16 : clean
	cp -r ../RepastHPC/Demo_01/Step_16/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_16
compare_RepastHPC_Demo_01_Step_16:
	@echo "Comparing Demo_01/Step_16/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_16/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_16/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_16/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_16/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_16/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_16/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_16/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_16/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_16/src/Demo_01_main.cpp ./src/Demo_01_main.cpp


.PHONY: RepastHPC_Demo_01_Step_17
RepastHPC_Demo_01_Step_17 : copy_RepastHPC_Demo_01_Step_17 RepastHPC_Demo_01

.PHONY: copy_RepastHPC_Demo_01_Step_17
copy_RepastHPC_Demo_01_Step_17 : clean
	cp -r ../RepastHPC/Demo_01/Step_17/* ./

.PHONY: compare_RepastHPC_Demo_01_Step_17
compare_RepastHPC_Demo_01_Step_17:
	@echo "Comparing Demo_01/Step_17/Demo_01_Agent.h:"
	-@diff ../RepastHPC/Demo_01/Step_17/include/Demo_01_Agent.h ./include/Demo_01_Agent.h
	@echo "Comparing Demo_01/Step_17/Demo_01_Model.h:"
	-@diff ../RepastHPC/Demo_01/Step_17/include/Demo_01_Model.h ./include/Demo_01_Model.h
	@echo "Comparing Demo_01/Step_17/Demo_01_Agent.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_17/src/Demo_01_Agent.cpp ./src/Demo_01_Agent.cpp
	@echo "Comparing Demo_01/Step_17/Demo_01_Model.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_17/src/Demo_01_Model.cpp ./src/Demo_01_Model.cpp
	@echo "Comparing Demo_01/Step_17/Demo_01_main.cpp:"
	-@diff ../RepastHPC/Demo_01/Step_17/src/Demo_01_main.cpp ./src/Demo_01_main.cpp


.PHONY: RepastHPC_Demo_02
RepastHPC_Demo_02: clean_compiled_files
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_02_Main.cpp -o ./objects/Demo_02_Main.o
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_02_Model.cpp -o ./objects/Demo_02_Model.o
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_02_Agent.cpp -o ./objects/Demo_02_Agent.o
	$(MPICXX) $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) -o ./bin/Demo_02.exe  ./objects/Demo_02_Main.o ./objects/Demo_02_Model.o ./objects/Demo_02_Agent.o $(REPAST_HPC_LIB) $(BOOST_LIBS)



.PHONY: RepastHPC_Demo_02_Step_00
RepastHPC_Demo_02_Step_00 : copy_RepastHPC_Demo_02_Step_00 RepastHPC_Demo_02

.PHONY: copy_RepastHPC_Demo_02_Step_00
copy_RepastHPC_Demo_02_Step_00 : clean
	cp -r ../RepastHPC/Demo_02/Step_00/* ./


.PHONY: compare_RepastHPC_Demo_02_Step_00
compare_RepastHPC_Demo_02_Step_00:
	@echo "Comparing Demo_02/Step_00/Demo_02_Agent.h:"
	-@diff ../RepastHPC/Demo_02/Step_00/include/Demo_02_Agent.h ./include/Demo_02_Agent.h
	@echo "Comparing Demo_02/Step_00/Demo_02_Model.h:"
	-@diff ../RepastHPC/Demo_02/Step_00/include/Demo_02_Model.h ./include/Demo_02_Model.h
	@echo "Comparing Demo_02/Step_00/Demo_02_Agent.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_00/src/Demo_02_Agent.cpp ./src/Demo_02_Agent.cpp
	@echo "Comparing Demo_02/Step_00/Demo_02_Model.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_00/src/Demo_02_Model.cpp ./src/Demo_02_Model.cpp
	@echo "Comparing Demo_02/Step_00/Demo_02_main.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_00/src/Demo_02_main.cpp ./src/Demo_02_main.cpp



.PHONY: RepastHPC_Demo_02_Step_01
RepastHPC_Demo_02_Step_01 : copy_RepastHPC_Demo_02_Step_01 RepastHPC_Demo_02

.PHONY: copy_RepastHPC_Demo_02_Step_01
copy_RepastHPC_Demo_02_Step_01 : clean
	cp -r ../RepastHPC/Demo_02/Step_01/* ./


.PHONY: compare_RepastHPC_Demo_02_Step_01
compare_RepastHPC_Demo_02_Step_01:
	@echo "Comparing Demo_02/Step_01/Demo_02_Agent.h:"
	-@diff ../RepastHPC/Demo_02/Step_01/include/Demo_02_Agent.h ./include/Demo_02_Agent.h
	@echo "Comparing Demo_02/Step_01/Demo_02_Model.h:"
	-@diff ../RepastHPC/Demo_02/Step_01/include/Demo_02_Model.h ./include/Demo_02_Model.h
	@echo "Comparing Demo_02/Step_01/Demo_02_Agent.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_01/src/Demo_02_Agent.cpp ./src/Demo_02_Agent.cpp
	@echo "Comparing Demo_02/Step_01/Demo_02_Model.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_01/src/Demo_02_Model.cpp ./src/Demo_02_Model.cpp
	@echo "Comparing Demo_02/Step_01/Demo_02_main.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_01/src/Demo_02_main.cpp ./src/Demo_02_main.cpp



.PHONY: RepastHPC_Demo_02_Step_02
RepastHPC_Demo_02_Step_02 : copy_RepastHPC_Demo_02_Step_02 RepastHPC_Demo_02

.PHONY: copy_RepastHPC_Demo_02_Step_02
copy_RepastHPC_Demo_02_Step_02 : clean
	cp -r ../RepastHPC/Demo_02/Step_02/* ./


.PHONY: compare_RepastHPC_Demo_02_Step_02
compare_RepastHPC_Demo_02_Step_02:
	@echo "Comparing Demo_02/Step_02/Demo_02_Agent.h:"
	-@diff ../RepastHPC/Demo_02/Step_02/include/Demo_02_Agent.h ./include/Demo_02_Agent.h
	@echo "Comparing Demo_02/Step_02/Demo_02_Model.h:"
	-@diff ../RepastHPC/Demo_02/Step_02/include/Demo_02_Model.h ./include/Demo_02_Model.h
	@echo "Comparing Demo_02/Step_02/Demo_02_Agent.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_02/src/Demo_02_Agent.cpp ./src/Demo_02_Agent.cpp
	@echo "Comparing Demo_02/Step_02/Demo_02_Model.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_02/src/Demo_02_Model.cpp ./src/Demo_02_Model.cpp
	@echo "Comparing Demo_02/Step_02/Demo_02_main.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_02/src/Demo_02_main.cpp ./src/Demo_02_main.cpp


.PHONY: RepastHPC_Demo_02_Step_03
RepastHPC_Demo_02_Step_03 : copy_RepastHPC_Demo_02_Step_03 RepastHPC_Demo_02

.PHONY: copy_RepastHPC_Demo_02_Step_03
copy_RepastHPC_Demo_02_Step_03 : clean
	cp -r ../RepastHPC/Demo_02/Step_03/* ./


.PHONY: compare_RepastHPC_Demo_02_Step_03
compare_RepastHPC_Demo_02_Step_03:
	@echo "Comparing Demo_02/Step_03/Demo_02_Agent.h:"
	-@diff ../RepastHPC/Demo_02/Step_03/include/Demo_02_Agent.h ./include/Demo_02_Agent.h
	@echo "Comparing Demo_02/Step_03/Demo_02_Model.h:"
	-@diff ../RepastHPC/Demo_02/Step_03/include/Demo_02_Model.h ./include/Demo_02_Model.h
	@echo "Comparing Demo_02/Step_03/Demo_02_Agent.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_03/src/Demo_02_Agent.cpp ./src/Demo_02_Agent.cpp
	@echo "Comparing Demo_02/Step_03/Demo_02_Model.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_03/src/Demo_02_Model.cpp ./src/Demo_02_Model.cpp
	@echo "Comparing Demo_02/Step_03/Demo_02_main.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_03/src/Demo_02_main.cpp ./src/Demo_02_main.cpp







.PHONY: RepastHPC_Demo_02_Step_04
RepastHPC_Demo_02_Step_04 : copy_RepastHPC_Demo_02_Step_04 RepastHPC_Demo_02

.PHONY: copy_RepastHPC_Demo_02_Step_04
copy_RepastHPC_Demo_02_Step_04 : clean
	cp -r ../RepastHPC/Demo_02/Step_04/* ./


.PHONY: compare_RepastHPC_Demo_02_Step_04
compare_RepastHPC_Demo_02_Step_04:
	@echo "Comparing Demo_02/Step_04/Demo_02_Agent.h:"
	-@diff ../RepastHPC/Demo_02/Step_04/include/Demo_02_Agent.h ./include/Demo_02_Agent.h
	@echo "Comparing Demo_02/Step_04/Demo_02_Model.h:"
	-@diff ../RepastHPC/Demo_02/Step_04/include/Demo_02_Model.h ./include/Demo_02_Model.h
	@echo "Comparing Demo_02/Step_04/Demo_02_Agent.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_04/src/Demo_02_Agent.cpp ./src/Demo_02_Agent.cpp
	@echo "Comparing Demo_02/Step_04/Demo_02_Model.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_04/src/Demo_02_Model.cpp ./src/Demo_02_Model.cpp
	@echo "Comparing Demo_02/Step_04/Demo_02_main.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_04/src/Demo_02_main.cpp ./src/Demo_02_main.cpp



.PHONY: RepastHPC_Demo_02_Step_05
RepastHPC_Demo_02_Step_05 : copy_RepastHPC_Demo_02_Step_05 RepastHPC_Demo_02

.PHONY: copy_RepastHPC_Demo_02_Step_05
copy_RepastHPC_Demo_02_Step_05 : clean
	cp -r ../RepastHPC/Demo_02/Step_05/* ./


.PHONY: compare_RepastHPC_Demo_02_Step_05
compare_RepastHPC_Demo_02_Step_05:
	@echo "Comparing Demo_02/Step_05/Demo_02_Agent.h:"
	-@diff ../RepastHPC/Demo_02/Step_05/include/Demo_02_Agent.h ./include/Demo_02_Agent.h
	@echo "Comparing Demo_02/Step_05/Demo_02_Model.h:"
	-@diff ../RepastHPC/Demo_02/Step_05/include/Demo_02_Model.h ./include/Demo_02_Model.h
	@echo "Comparing Demo_02/Step_05/Demo_02_Agent.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_05/src/Demo_02_Agent.cpp ./src/Demo_02_Agent.cpp
	@echo "Comparing Demo_02/Step_05/Demo_02_Model.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_05/src/Demo_02_Model.cpp ./src/Demo_02_Model.cpp
	@echo "Comparing Demo_02/Step_05/Demo_02_main.cpp:"
	-@diff ../RepastHPC/Demo_02/Step_05/src/Demo_02_main.cpp ./src/Demo_02_main.cpp




.PHONY: RepastHPC_Demo_03
RepastHPC_Demo_03: clean_compiled_files
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_03_Main.cpp -o ./objects/Demo_03_Main.o
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_03_Model.cpp -o ./objects/Demo_03_Model.o
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_03_Agent.cpp -o ./objects/Demo_03_Agent.o
	$(MPICXX) $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) -o ./bin/Demo_03.exe  ./objects/Demo_03_Main.o ./objects/Demo_03_Model.o ./objects/Demo_03_Agent.o $(REPAST_HPC_LIB) $(BOOST_LIBS)


.PHONY: RepastHPC_Demo_03_Step_00
RepastHPC_Demo_03_Step_00 : copy_RepastHPC_Demo_03_Step_00 RepastHPC_Demo_03

.PHONY: copy_RepastHPC_Demo_03_Step_00
copy_RepastHPC_Demo_03_Step_00 : clean
	cp -r ../RepastHPC/Demo_03/Step_00/* ./


.PHONY: compare_RepastHPC_Demo_03_Step_00
compare_RepastHPC_Demo_03_Step_00:
	@echo "Comparing Demo_03/Step_00/Demo_03_Agent.h:"
	-@diff ../RepastHPC/Demo_03/Step_00/include/Demo_03_Agent.h ./include/Demo_03_Agent.h
	@echo "Comparing Demo_03/Step_00/Demo_03_Model.h:"
	-@diff ../RepastHPC/Demo_03/Step_00/include/Demo_03_Model.h ./include/Demo_03_Model.h
	@echo "Comparing Demo_03/Step_00/Demo_03_Agent.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_00/src/Demo_03_Agent.cpp ./src/Demo_03_Agent.cpp
	@echo "Comparing Demo_03/Step_00/Demo_03_Model.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_00/src/Demo_03_Model.cpp ./src/Demo_03_Model.cpp
	@echo "Comparing Demo_03/Step_00/Demo_03_main.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_00/src/Demo_03_main.cpp ./src/Demo_03_main.cpp



.PHONY: RepastHPC_Demo_03_Step_01
RepastHPC_Demo_03_Step_01 : copy_RepastHPC_Demo_03_Step_01 RepastHPC_Demo_03

.PHONY: copy_RepastHPC_Demo_03_Step_01
copy_RepastHPC_Demo_03_Step_01 : clean
	cp -r ../RepastHPC/Demo_03/Step_01/* ./


.PHONY: compare_RepastHPC_Demo_03_Step_01
compare_RepastHPC_Demo_03_Step_01:
	@echo "Comparing Demo_03/Step_01/Demo_03_Agent.h:"
	-@diff ../RepastHPC/Demo_03/Step_01/include/Demo_03_Agent.h ./include/Demo_03_Agent.h
	@echo "Comparing Demo_03/Step_01/Demo_03_Model.h:"
	-@diff ../RepastHPC/Demo_03/Step_01/include/Demo_03_Model.h ./include/Demo_03_Model.h
	@echo "Comparing Demo_03/Step_01/Demo_03_Agent.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_01/src/Demo_03_Agent.cpp ./src/Demo_03_Agent.cpp
	@echo "Comparing Demo_03/Step_01/Demo_03_Model.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_01/src/Demo_03_Model.cpp ./src/Demo_03_Model.cpp
	@echo "Comparing Demo_03/Step_01/Demo_03_main.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_01/src/Demo_03_main.cpp ./src/Demo_03_main.cpp

.PHONY: RepastHPC_Demo_03_Step_02
RepastHPC_Demo_03_Step_02 : copy_RepastHPC_Demo_03_Step_02 RepastHPC_Demo_03

.PHONY: copy_RepastHPC_Demo_03_Step_02
copy_RepastHPC_Demo_03_Step_02 : clean
	cp -r ../RepastHPC/Demo_03/Step_02/* ./


.PHONY: compare_RepastHPC_Demo_03_Step_02
compare_RepastHPC_Demo_03_Step_02:
	@echo "Comparing Demo_03/Step_02/Demo_03_Agent.h:"
	-@diff ../RepastHPC/Demo_03/Step_02/include/Demo_03_Agent.h ./include/Demo_03_Agent.h
	@echo "Comparing Demo_03/Step_02/Demo_03_Model.h:"
	-@diff ../RepastHPC/Demo_03/Step_02/include/Demo_03_Model.h ./include/Demo_03_Model.h
	@echo "Comparing Demo_03/Step_02/Demo_03_Agent.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_02/src/Demo_03_Agent.cpp ./src/Demo_03_Agent.cpp
	@echo "Comparing Demo_03/Step_02/Demo_03_Model.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_02/src/Demo_03_Model.cpp ./src/Demo_03_Model.cpp
	@echo "Comparing Demo_03/Step_02/Demo_03_main.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_02/src/Demo_03_main.cpp ./src/Demo_03_main.cpp


.PHONY: RepastHPC_Demo_03_Step_03
RepastHPC_Demo_03_Step_03 : copy_RepastHPC_Demo_03_Step_03 RepastHPC_Demo_03

.PHONY: copy_RepastHPC_Demo_03_Step_03
copy_RepastHPC_Demo_03_Step_03 : clean
	cp -r ../RepastHPC/Demo_03/Step_03/* ./


.PHONY: compare_RepastHPC_Demo_03_Step_03
compare_RepastHPC_Demo_03_Step_03:
	@echo "Comparing Demo_03/Step_03/Demo_03_Agent.h:"
	-@diff ../RepastHPC/Demo_03/Step_03/include/Demo_03_Agent.h ./include/Demo_03_Agent.h
	@echo "Comparing Demo_03/Step_03/Demo_03_Model.h:"
	-@diff ../RepastHPC/Demo_03/Step_03/include/Demo_03_Model.h ./include/Demo_03_Model.h
	@echo "Comparing Demo_03/Step_03/Demo_03_Agent.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_03/src/Demo_03_Agent.cpp ./src/Demo_03_Agent.cpp
	@echo "Comparing Demo_03/Step_03/Demo_03_Model.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_03/src/Demo_03_Model.cpp ./src/Demo_03_Model.cpp
	@echo "Comparing Demo_03/Step_03/Demo_03_main.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_03/src/Demo_03_main.cpp ./src/Demo_03_main.cpp


.PHONY: RepastHPC_Demo_03_Step_04
RepastHPC_Demo_03_Step_04 : copy_RepastHPC_Demo_03_Step_04 RepastHPC_Demo_03

.PHONY: copy_RepastHPC_Demo_03_Step_04
copy_RepastHPC_Demo_03_Step_04 : clean
	cp -r ../RepastHPC/Demo_03/Step_04/* ./


.PHONY: compare_RepastHPC_Demo_03_Step_04
compare_RepastHPC_Demo_03_Step_04:
	@echo "Comparing Demo_03/Step_04/Demo_03_Agent.h:"
	-@diff ../RepastHPC/Demo_03/Step_04/include/Demo_03_Agent.h ./include/Demo_03_Agent.h
	@echo "Comparing Demo_03/Step_04/Demo_03_Model.h:"
	-@diff ../RepastHPC/Demo_03/Step_04/include/Demo_03_Model.h ./include/Demo_03_Model.h
	@echo "Comparing Demo_03/Step_04/Demo_03_Agent.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_04/src/Demo_03_Agent.cpp ./src/Demo_03_Agent.cpp
	@echo "Comparing Demo_03/Step_04/Demo_03_Model.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_04/src/Demo_03_Model.cpp ./src/Demo_03_Model.cpp
	@echo "Comparing Demo_03/Step_04/Demo_03_main.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_04/src/Demo_03_main.cpp ./src/Demo_03_main.cpp


.PHONY: RepastHPC_Demo_03_Step_05
RepastHPC_Demo_03_Step_05 : copy_RepastHPC_Demo_03_Step_05 RepastHPC_Demo_03

.PHONY: copy_RepastHPC_Demo_03_Step_05
copy_RepastHPC_Demo_03_Step_05 : clean
	cp -r ../RepastHPC/Demo_03/Step_05/* ./


.PHONY: compare_RepastHPC_Demo_03_Step_05
compare_RepastHPC_Demo_03_Step_05:
	@echo "Comparing Demo_03/Step_05/Demo_03_Agent.h:"
	-@diff ../RepastHPC/Demo_03/Step_05/include/Demo_03_Agent.h ./include/Demo_03_Agent.h
	@echo "Comparing Demo_03/Step_05/Demo_03_Model.h:"
	-@diff ../RepastHPC/Demo_03/Step_05/include/Demo_03_Model.h ./include/Demo_03_Model.h
	@echo "Comparing Demo_03/Step_05/Demo_03_Agent.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_05/src/Demo_03_Agent.cpp ./src/Demo_03_Agent.cpp
	@echo "Comparing Demo_03/Step_05/Demo_03_Model.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_05/src/Demo_03_Model.cpp ./src/Demo_03_Model.cpp
	@echo "Comparing Demo_03/Step_05/Demo_03_main.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_05/src/Demo_03_main.cpp ./src/Demo_03_main.cpp



.PHONY: RepastHPC_Demo_03_Step_06
RepastHPC_Demo_03_Step_06 : copy_RepastHPC_Demo_03_Step_06 RepastHPC_Demo_03

.PHONY: copy_RepastHPC_Demo_03_Step_06
copy_RepastHPC_Demo_03_Step_06 : clean
	cp -r ../RepastHPC/Demo_03/Step_06/* ./


.PHONY: compare_RepastHPC_Demo_03_Step_06
compare_RepastHPC_Demo_03_Step_06:
	@echo "Comparing Demo_03/Step_06/Demo_03_Agent.h:"
	-@diff ../RepastHPC/Demo_03/Step_06/include/Demo_03_Agent.h ./include/Demo_03_Agent.h
	@echo "Comparing Demo_03/Step_06/Demo_03_Model.h:"
	-@diff ../RepastHPC/Demo_03/Step_06/include/Demo_03_Model.h ./include/Demo_03_Model.h
	@echo "Comparing Demo_03/Step_06/Demo_03_Agent.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_06/src/Demo_03_Agent.cpp ./src/Demo_03_Agent.cpp
	@echo "Comparing Demo_03/Step_06/Demo_03_Model.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_06/src/Demo_03_Model.cpp ./src/Demo_03_Model.cpp
	@echo "Comparing Demo_03/Step_06/Demo_03_main.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_06/src/Demo_03_main.cpp ./src/Demo_03_main.cpp



.PHONY: RepastHPC_Demo_03_Step_07
RepastHPC_Demo_03_Step_07 : copy_RepastHPC_Demo_03_Step_07 RepastHPC_Demo_03

.PHONY: copy_RepastHPC_Demo_03_Step_07
copy_RepastHPC_Demo_03_Step_07 : clean
	cp -r ../RepastHPC/Demo_03/Step_07/* ./


.PHONY: compare_RepastHPC_Demo_03_Step_07
compare_RepastHPC_Demo_03_Step_07:
	@echo "Comparing Demo_03/Step_07/Demo_03_Agent.h:"
	-@diff ../RepastHPC/Demo_03/Step_07/include/Demo_03_Agent.h ./include/Demo_03_Agent.h
	@echo "Comparing Demo_03/Step_07/Demo_03_Model.h:"
	-@diff ../RepastHPC/Demo_03/Step_07/include/Demo_03_Model.h ./include/Demo_03_Model.h
	@echo "Comparing Demo_03/Step_07/Demo_03_Agent.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_07/src/Demo_03_Agent.cpp ./src/Demo_03_Agent.cpp
	@echo "Comparing Demo_03/Step_07/Demo_03_Model.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_07/src/Demo_03_Model.cpp ./src/Demo_03_Model.cpp
	@echo "Comparing Demo_03/Step_07/Demo_03_main.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_07/src/Demo_03_main.cpp ./src/Demo_03_main.cpp



.PHONY: RepastHPC_Demo_03_Step_08
RepastHPC_Demo_03_Step_08 : copy_RepastHPC_Demo_03_Step_08 RepastHPC_Demo_03

.PHONY: copy_RepastHPC_Demo_03_Step_08
copy_RepastHPC_Demo_03_Step_08 : clean
	cp -r ../RepastHPC/Demo_03/Step_08/* ./


.PHONY: compare_RepastHPC_Demo_03_Step_08
compare_RepastHPC_Demo_03_Step_08:
	@echo "Comparing Demo_03/Step_08/Demo_03_Agent.h:"
	-@diff ../RepastHPC/Demo_03/Step_08/include/Demo_03_Agent.h ./include/Demo_03_Agent.h
	@echo "Comparing Demo_03/Step_08/Demo_03_Model.h:"
	-@diff ../RepastHPC/Demo_03/Step_08/include/Demo_03_Model.h ./include/Demo_03_Model.h
	@echo "Comparing Demo_03/Step_08/Demo_03_Agent.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_08/src/Demo_03_Agent.cpp ./src/Demo_03_Agent.cpp
	@echo "Comparing Demo_03/Step_08/Demo_03_Model.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_08/src/Demo_03_Model.cpp ./src/Demo_03_Model.cpp
	@echo "Comparing Demo_03/Step_08/Demo_03_main.cpp:"
	-@diff ../RepastHPC/Demo_03/Step_08/src/Demo_03_main.cpp ./src/Demo_03_main.cpp




.PHONY: RepastHPC_Demo_04
RepastHPC_Demo_04: clean_compiled_files
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_04_Main.cpp -o ./objects/Demo_04_Main.o
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_04_Model.cpp -o ./objects/Demo_04_Model.o
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Demo_04_Agent.cpp -o ./objects/Demo_04_Agent.o
	$(MPICXX) $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) -o ./bin/Demo_04.exe  ./objects/Demo_04_Main.o ./objects/Demo_04_Model.o ./objects/Demo_04_Agent.o $(REPAST_HPC_LIB) $(BOOST_LIBS)



.PHONY: RepastHPC_Demo_04_Step_00
RepastHPC_Demo_04_Step_00 : copy_RepastHPC_Demo_04_Step_00 RepastHPC_Demo_04

.PHONY: copy_RepastHPC_Demo_04_Step_00
copy_RepastHPC_Demo_04_Step_00 : clean
	cp -r ../RepastHPC/Demo_04/Step_00/* ./


.PHONY: compare_RepastHPC_Demo_04_Step_00
compare_RepastHPC_Demo_04_Step_00:
	@echo "Comparing Demo_04/Step_00/Demo_04_Agent.h:"
	-@diff ../RepastHPC/Demo_04/Step_00/include/Demo_04_Agent.h ./include/Demo_04_Agent.h
	@echo "Comparing Demo_04/Step_00/Demo_04_Model.h:"
	-@diff ../RepastHPC/Demo_04/Step_00/include/Demo_04_Model.h ./include/Demo_04_Model.h
	@echo "Comparing Demo_04/Step_00/Demo_04_Agent.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_00/src/Demo_04_Agent.cpp ./src/Demo_04_Agent.cpp
	@echo "Comparing Demo_04/Step_00/Demo_04_Model.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_00/src/Demo_04_Model.cpp ./src/Demo_04_Model.cpp
	@echo "Comparing Demo_04/Step_00/Demo_04_main.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_00/src/Demo_04_main.cpp ./src/Demo_04_main.cpp


.PHONY: RepastHPC_Demo_04_Step_01
RepastHPC_Demo_04_Step_01 : copy_RepastHPC_Demo_04_Step_01 RepastHPC_Demo_04

.PHONY: copy_RepastHPC_Demo_04_Step_01
copy_RepastHPC_Demo_04_Step_01 : clean
	cp -r ../RepastHPC/Demo_04/Step_01/* ./


.PHONY: compare_RepastHPC_Demo_04_Step_01
compare_RepastHPC_Demo_04_Step_01:
	@echo "Comparing Demo_04/Step_01/Demo_04_Agent.h:"
	-@diff ../RepastHPC/Demo_04/Step_01/include/Demo_04_Agent.h ./include/Demo_04_Agent.h
	@echo "Comparing Demo_04/Step_01/Demo_04_Model.h:"
	-@diff ../RepastHPC/Demo_04/Step_01/include/Demo_04_Model.h ./include/Demo_04_Model.h
	@echo "Comparing Demo_04/Step_01/Demo_04_Agent.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_01/src/Demo_04_Agent.cpp ./src/Demo_04_Agent.cpp
	@echo "Comparing Demo_04/Step_01/Demo_04_Model.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_01/src/Demo_04_Model.cpp ./src/Demo_04_Model.cpp
	@echo "Comparing Demo_04/Step_01/Demo_04_main.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_01/src/Demo_04_main.cpp ./src/Demo_04_main.cpp



.PHONY: RepastHPC_Demo_04_Step_02
RepastHPC_Demo_04_Step_02 : copy_RepastHPC_Demo_04_Step_02 RepastHPC_Demo_04

.PHONY: copy_RepastHPC_Demo_04_Step_02
copy_RepastHPC_Demo_04_Step_02 : clean
	cp -r ../RepastHPC/Demo_04/Step_02/* ./


.PHONY: compare_RepastHPC_Demo_04_Step_02
compare_RepastHPC_Demo_04_Step_02:
	@echo "Comparing Demo_04/Step_02/Demo_04_Agent.h:"
	-@diff ../RepastHPC/Demo_04/Step_02/include/Demo_04_Agent.h ./include/Demo_04_Agent.h
	@echo "Comparing Demo_04/Step_02/Demo_04_Model.h:"
	-@diff ../RepastHPC/Demo_04/Step_02/include/Demo_04_Model.h ./include/Demo_04_Model.h
	@echo "Comparing Demo_04/Step_02/Demo_04_Agent.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_02/src/Demo_04_Agent.cpp ./src/Demo_04_Agent.cpp
	@echo "Comparing Demo_04/Step_02/Demo_04_Model.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_02/src/Demo_04_Model.cpp ./src/Demo_04_Model.cpp
	@echo "Comparing Demo_04/Step_02/Demo_04_main.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_02/src/Demo_04_main.cpp ./src/Demo_04_main.cpp



.PHONY: RepastHPC_Demo_04_Step_03
RepastHPC_Demo_04_Step_03 : copy_RepastHPC_Demo_04_Step_03 RepastHPC_Demo_04

.PHONY: copy_RepastHPC_Demo_04_Step_03
copy_RepastHPC_Demo_04_Step_03 : clean
	cp -r ../RepastHPC/Demo_04/Step_03/* ./


.PHONY: compare_RepastHPC_Demo_04_Step_03
compare_RepastHPC_Demo_04_Step_03:
	@echo "Comparing Demo_04/Step_03/Demo_04_Agent.h:"
	-@diff ../RepastHPC/Demo_04/Step_03/include/Demo_04_Agent.h ./include/Demo_04_Agent.h
	@echo "Comparing Demo_04/Step_03/Demo_04_Model.h:"
	-@diff ../RepastHPC/Demo_04/Step_03/include/Demo_04_Model.h ./include/Demo_04_Model.h
	@echo "Comparing Demo_04/Step_03/Demo_04_Agent.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_03/src/Demo_04_Agent.cpp ./src/Demo_04_Agent.cpp
	@echo "Comparing Demo_04/Step_03/Demo_04_Model.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_03/src/Demo_04_Model.cpp ./src/Demo_04_Model.cpp
	@echo "Comparing Demo_04/Step_03/Demo_04_main.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_03/src/Demo_04_main.cpp ./src/Demo_04_main.cpp



.PHONY: RepastHPC_Demo_04_Step_04
RepastHPC_Demo_04_Step_04 : copy_RepastHPC_Demo_04_Step_04 RepastHPC_Demo_04

.PHONY: copy_RepastHPC_Demo_04_Step_04
copy_RepastHPC_Demo_04_Step_04 : clean
	cp -r ../RepastHPC/Demo_04/Step_04/* ./


.PHONY: compare_RepastHPC_Demo_04_Step_04
compare_RepastHPC_Demo_04_Step_04:
	@echo "Comparing Demo_04/Step_04/Demo_04_Agent.h:"
	-@diff ../RepastHPC/Demo_04/Step_04/include/Demo_04_Agent.h ./include/Demo_04_Agent.h
	@echo "Comparing Demo_04/Step_04/Demo_04_Model.h:"
	-@diff ../RepastHPC/Demo_04/Step_04/include/Demo_04_Model.h ./include/Demo_04_Model.h
	@echo "Comparing Demo_04/Step_04/Demo_04_Agent.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_04/src/Demo_04_Agent.cpp ./src/Demo_04_Agent.cpp
	@echo "Comparing Demo_04/Step_04/Demo_04_Model.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_04/src/Demo_04_Model.cpp ./src/Demo_04_Model.cpp
	@echo "Comparing Demo_04/Step_04/Demo_04_main.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_04/src/Demo_04_main.cpp ./src/Demo_04_main.cpp




.PHONY: RepastHPC_Demo_04_Step_05
RepastHPC_Demo_04_Step_05 : copy_RepastHPC_Demo_04_Step_05 RepastHPC_Demo_04

.PHONY: copy_RepastHPC_Demo_04_Step_05
copy_RepastHPC_Demo_04_Step_05 : clean
	cp -r ../RepastHPC/Demo_04/Step_05/* ./


.PHONY: compare_RepastHPC_Demo_04_Step_05
compare_RepastHPC_Demo_04_Step_05:
	@echo "Comparing Demo_04/Step_05/Demo_04_Agent.h:"
	-@diff ../RepastHPC/Demo_04/Step_05/include/Demo_04_Agent.h ./include/Demo_04_Agent.h
	@echo "Comparing Demo_04/Step_05/Demo_04_Model.h:"
	-@diff ../RepastHPC/Demo_04/Step_05/include/Demo_04_Model.h ./include/Demo_04_Model.h
	@echo "Comparing Demo_04/Step_05/Demo_04_Agent.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_05/src/Demo_04_Agent.cpp ./src/Demo_04_Agent.cpp
	@echo "Comparing Demo_04/Step_05/Demo_04_Model.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_05/src/Demo_04_Model.cpp ./src/Demo_04_Model.cpp
	@echo "Comparing Demo_04/Step_05/Demo_04_main.cpp:"
	-@diff ../RepastHPC/Demo_04/Step_05/src/Demo_04_main.cpp ./src/Demo_04_main.cpp


