# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/vkim70/src/AquetiTools

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/vkim70/src/AquetiTools

# Include any dependencies generated for this target.
include CMakeFiles/i2CControl.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/i2CControl.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/i2CControl.dir/flags.make

CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.o: CMakeFiles/i2CControl.dir/flags.make
CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.o: test/testI2CControl.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/vkim70/src/AquetiTools/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.o -c /home/vkim70/src/AquetiTools/test/testI2CControl.cpp

CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/vkim70/src/AquetiTools/test/testI2CControl.cpp > CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.i

CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/vkim70/src/AquetiTools/test/testI2CControl.cpp -o CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.s

CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.o.requires:

.PHONY : CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.o.requires

CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.o.provides: CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.o.requires
	$(MAKE) -f CMakeFiles/i2CControl.dir/build.make CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.o.provides.build
.PHONY : CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.o.provides

CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.o.provides.build: CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.o


# Object files for target i2CControl
i2CControl_OBJECTS = \
"CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.o"

# External object files for target i2CControl
i2CControl_EXTERNAL_OBJECTS =

i2CControl: CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.o
i2CControl: CMakeFiles/i2CControl.dir/build.make
i2CControl: libaquetitools.a
i2CControl: CMakeFiles/i2CControl.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/vkim70/src/AquetiTools/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable i2CControl"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/i2CControl.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/i2CControl.dir/build: i2CControl

.PHONY : CMakeFiles/i2CControl.dir/build

CMakeFiles/i2CControl.dir/requires: CMakeFiles/i2CControl.dir/test/testI2CControl.cpp.o.requires

.PHONY : CMakeFiles/i2CControl.dir/requires

CMakeFiles/i2CControl.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/i2CControl.dir/cmake_clean.cmake
.PHONY : CMakeFiles/i2CControl.dir/clean

CMakeFiles/i2CControl.dir/depend:
	cd /home/vkim70/src/AquetiTools && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/vkim70/src/AquetiTools /home/vkim70/src/AquetiTools /home/vkim70/src/AquetiTools /home/vkim70/src/AquetiTools /home/vkim70/src/AquetiTools/CMakeFiles/i2CControl.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/i2CControl.dir/depend

