SOURCES = main.cpp \
          Zombie.cpp \
          Human.cpp \
          ZombieObserver.cpp \
          InfectionSum.cpp
         
local_dir := zombies
local_src := $(addprefix $(local_dir)/, $(SOURCES))

zombie_src += $(local_src)



