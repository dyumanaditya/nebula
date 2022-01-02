CXX = g++									# Define the Cpp compiler to use
CXXFLAGS = -std=c++17 -w -g		# Define any compile-time flags
INCLUDE	= -Iinclude							# Define include directory

CPPFILES := main.cpp fileManager.cpp panels.cpp menus.cpp directoryReader.cpp file.cpp directory.cpp linkedList.cpp
HFILES := fileManager.h panels.h menus.h directoryReader.h file.h directory.h linkedList.h keybindings.h colors.h
OBJFILES := $(CPPFILES:.cpp=.o)

# Set correct path prefixes
CPPFILES := $(addprefix src/, $(CPPFILES))
HFILES := $(addprefix include/, $(HFILES))
OBJFILES := $(addprefix /opt/nebula/obj/, $(OBJFILES))

# Make options
all: directories main
	@echo Installation finished successfully!

directories:
	sudo mkdir -p /opt/nebula/obj

main: $(OBJFILES)
	sudo $(CXX) $(CXXFLAGS) $(INCLUDE) $(OBJFILES) -lform -lncurses -o /usr/bin/nebula


# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
/opt/nebula/obj/%.o: src/%.cpp
	sudo $(CXX) -c $(CXXFLAGS) $(INCLUDE) $< -o $@

.PHONY: clean

clean:
	sudo rm -rf /opt/nebula/obj/*.o /usr/bin/nebula
	@echo Nebula Cleanup complete!
