SOURCES = main.cpp \
          Zombie.cpp \
          Human.cpp \
          ZombieObserver.cpp \
          InfectionSum.cpp
         
local_dir := src/zombies
local_src := $(addprefix $(local_dir)/, $(SOURCES))

sources += $(local_src)
bin_dir = "bin/zombie"
program = "zombie_model"



