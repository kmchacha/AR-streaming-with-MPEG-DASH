# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/mcnl/.local/lib/python3.8/site-packages/cmake/data/bin/cmake

# The command to remove a file.
RM = /home/mcnl/.local/lib/python3.8/site-packages/cmake/data/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build

# Include any dependencies generated for this target.
include source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/compiler_depend.make

# Include the progress variables for this target.
include source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/progress.make

# Include the compile flags for this target's objects.
include source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/flags.make

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/AnnexBread.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/flags.make
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/AnnexBread.cpp.o: ../source/Lib/TLibDecoder/AnnexBread.cpp
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/AnnexBread.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/AnnexBread.cpp.o"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/AnnexBread.cpp.o -MF CMakeFiles/TLibDecoder.dir/AnnexBread.cpp.o.d -o CMakeFiles/TLibDecoder.dir/AnnexBread.cpp.o -c /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/AnnexBread.cpp

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/AnnexBread.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TLibDecoder.dir/AnnexBread.cpp.i"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/AnnexBread.cpp > CMakeFiles/TLibDecoder.dir/AnnexBread.cpp.i

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/AnnexBread.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TLibDecoder.dir/AnnexBread.cpp.s"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/AnnexBread.cpp -o CMakeFiles/TLibDecoder.dir/AnnexBread.cpp.s

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/NALread.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/flags.make
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/NALread.cpp.o: ../source/Lib/TLibDecoder/NALread.cpp
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/NALread.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/NALread.cpp.o"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/NALread.cpp.o -MF CMakeFiles/TLibDecoder.dir/NALread.cpp.o.d -o CMakeFiles/TLibDecoder.dir/NALread.cpp.o -c /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/NALread.cpp

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/NALread.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TLibDecoder.dir/NALread.cpp.i"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/NALread.cpp > CMakeFiles/TLibDecoder.dir/NALread.cpp.i

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/NALread.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TLibDecoder.dir/NALread.cpp.s"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/NALread.cpp -o CMakeFiles/TLibDecoder.dir/NALread.cpp.s

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/SEIread.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/flags.make
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/SEIread.cpp.o: ../source/Lib/TLibDecoder/SEIread.cpp
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/SEIread.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/SEIread.cpp.o"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/SEIread.cpp.o -MF CMakeFiles/TLibDecoder.dir/SEIread.cpp.o.d -o CMakeFiles/TLibDecoder.dir/SEIread.cpp.o -c /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/SEIread.cpp

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/SEIread.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TLibDecoder.dir/SEIread.cpp.i"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/SEIread.cpp > CMakeFiles/TLibDecoder.dir/SEIread.cpp.i

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/SEIread.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TLibDecoder.dir/SEIread.cpp.s"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/SEIread.cpp -o CMakeFiles/TLibDecoder.dir/SEIread.cpp.s

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/SyntaxElementParser.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/flags.make
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/SyntaxElementParser.cpp.o: ../source/Lib/TLibDecoder/SyntaxElementParser.cpp
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/SyntaxElementParser.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/SyntaxElementParser.cpp.o"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/SyntaxElementParser.cpp.o -MF CMakeFiles/TLibDecoder.dir/SyntaxElementParser.cpp.o.d -o CMakeFiles/TLibDecoder.dir/SyntaxElementParser.cpp.o -c /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/SyntaxElementParser.cpp

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/SyntaxElementParser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TLibDecoder.dir/SyntaxElementParser.cpp.i"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/SyntaxElementParser.cpp > CMakeFiles/TLibDecoder.dir/SyntaxElementParser.cpp.i

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/SyntaxElementParser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TLibDecoder.dir/SyntaxElementParser.cpp.s"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/SyntaxElementParser.cpp -o CMakeFiles/TLibDecoder.dir/SyntaxElementParser.cpp.s

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecBinCoderCABAC.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/flags.make
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecBinCoderCABAC.cpp.o: ../source/Lib/TLibDecoder/TDecBinCoderCABAC.cpp
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecBinCoderCABAC.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecBinCoderCABAC.cpp.o"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecBinCoderCABAC.cpp.o -MF CMakeFiles/TLibDecoder.dir/TDecBinCoderCABAC.cpp.o.d -o CMakeFiles/TLibDecoder.dir/TDecBinCoderCABAC.cpp.o -c /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecBinCoderCABAC.cpp

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecBinCoderCABAC.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TLibDecoder.dir/TDecBinCoderCABAC.cpp.i"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecBinCoderCABAC.cpp > CMakeFiles/TLibDecoder.dir/TDecBinCoderCABAC.cpp.i

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecBinCoderCABAC.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TLibDecoder.dir/TDecBinCoderCABAC.cpp.s"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecBinCoderCABAC.cpp -o CMakeFiles/TLibDecoder.dir/TDecBinCoderCABAC.cpp.s

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecCAVLC.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/flags.make
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecCAVLC.cpp.o: ../source/Lib/TLibDecoder/TDecCAVLC.cpp
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecCAVLC.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecCAVLC.cpp.o"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecCAVLC.cpp.o -MF CMakeFiles/TLibDecoder.dir/TDecCAVLC.cpp.o.d -o CMakeFiles/TLibDecoder.dir/TDecCAVLC.cpp.o -c /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecCAVLC.cpp

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecCAVLC.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TLibDecoder.dir/TDecCAVLC.cpp.i"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecCAVLC.cpp > CMakeFiles/TLibDecoder.dir/TDecCAVLC.cpp.i

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecCAVLC.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TLibDecoder.dir/TDecCAVLC.cpp.s"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecCAVLC.cpp -o CMakeFiles/TLibDecoder.dir/TDecCAVLC.cpp.s

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecConformance.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/flags.make
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecConformance.cpp.o: ../source/Lib/TLibDecoder/TDecConformance.cpp
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecConformance.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecConformance.cpp.o"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecConformance.cpp.o -MF CMakeFiles/TLibDecoder.dir/TDecConformance.cpp.o.d -o CMakeFiles/TLibDecoder.dir/TDecConformance.cpp.o -c /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecConformance.cpp

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecConformance.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TLibDecoder.dir/TDecConformance.cpp.i"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecConformance.cpp > CMakeFiles/TLibDecoder.dir/TDecConformance.cpp.i

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecConformance.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TLibDecoder.dir/TDecConformance.cpp.s"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecConformance.cpp -o CMakeFiles/TLibDecoder.dir/TDecConformance.cpp.s

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecCu.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/flags.make
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecCu.cpp.o: ../source/Lib/TLibDecoder/TDecCu.cpp
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecCu.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecCu.cpp.o"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecCu.cpp.o -MF CMakeFiles/TLibDecoder.dir/TDecCu.cpp.o.d -o CMakeFiles/TLibDecoder.dir/TDecCu.cpp.o -c /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecCu.cpp

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecCu.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TLibDecoder.dir/TDecCu.cpp.i"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecCu.cpp > CMakeFiles/TLibDecoder.dir/TDecCu.cpp.i

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecCu.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TLibDecoder.dir/TDecCu.cpp.s"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecCu.cpp -o CMakeFiles/TLibDecoder.dir/TDecCu.cpp.s

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecEntropy.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/flags.make
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecEntropy.cpp.o: ../source/Lib/TLibDecoder/TDecEntropy.cpp
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecEntropy.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecEntropy.cpp.o"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecEntropy.cpp.o -MF CMakeFiles/TLibDecoder.dir/TDecEntropy.cpp.o.d -o CMakeFiles/TLibDecoder.dir/TDecEntropy.cpp.o -c /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecEntropy.cpp

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecEntropy.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TLibDecoder.dir/TDecEntropy.cpp.i"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecEntropy.cpp > CMakeFiles/TLibDecoder.dir/TDecEntropy.cpp.i

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecEntropy.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TLibDecoder.dir/TDecEntropy.cpp.s"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecEntropy.cpp -o CMakeFiles/TLibDecoder.dir/TDecEntropy.cpp.s

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecGop.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/flags.make
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecGop.cpp.o: ../source/Lib/TLibDecoder/TDecGop.cpp
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecGop.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecGop.cpp.o"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecGop.cpp.o -MF CMakeFiles/TLibDecoder.dir/TDecGop.cpp.o.d -o CMakeFiles/TLibDecoder.dir/TDecGop.cpp.o -c /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecGop.cpp

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecGop.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TLibDecoder.dir/TDecGop.cpp.i"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecGop.cpp > CMakeFiles/TLibDecoder.dir/TDecGop.cpp.i

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecGop.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TLibDecoder.dir/TDecGop.cpp.s"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecGop.cpp -o CMakeFiles/TLibDecoder.dir/TDecGop.cpp.s

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecSbac.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/flags.make
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecSbac.cpp.o: ../source/Lib/TLibDecoder/TDecSbac.cpp
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecSbac.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecSbac.cpp.o"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecSbac.cpp.o -MF CMakeFiles/TLibDecoder.dir/TDecSbac.cpp.o.d -o CMakeFiles/TLibDecoder.dir/TDecSbac.cpp.o -c /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecSbac.cpp

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecSbac.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TLibDecoder.dir/TDecSbac.cpp.i"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecSbac.cpp > CMakeFiles/TLibDecoder.dir/TDecSbac.cpp.i

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecSbac.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TLibDecoder.dir/TDecSbac.cpp.s"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecSbac.cpp -o CMakeFiles/TLibDecoder.dir/TDecSbac.cpp.s

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecSlice.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/flags.make
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecSlice.cpp.o: ../source/Lib/TLibDecoder/TDecSlice.cpp
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecSlice.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building CXX object source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecSlice.cpp.o"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecSlice.cpp.o -MF CMakeFiles/TLibDecoder.dir/TDecSlice.cpp.o.d -o CMakeFiles/TLibDecoder.dir/TDecSlice.cpp.o -c /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecSlice.cpp

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecSlice.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TLibDecoder.dir/TDecSlice.cpp.i"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecSlice.cpp > CMakeFiles/TLibDecoder.dir/TDecSlice.cpp.i

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecSlice.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TLibDecoder.dir/TDecSlice.cpp.s"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecSlice.cpp -o CMakeFiles/TLibDecoder.dir/TDecSlice.cpp.s

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecTop.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/flags.make
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecTop.cpp.o: ../source/Lib/TLibDecoder/TDecTop.cpp
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecTop.cpp.o: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Building CXX object source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecTop.cpp.o"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecTop.cpp.o -MF CMakeFiles/TLibDecoder.dir/TDecTop.cpp.o.d -o CMakeFiles/TLibDecoder.dir/TDecTop.cpp.o -c /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecTop.cpp

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecTop.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/TLibDecoder.dir/TDecTop.cpp.i"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecTop.cpp > CMakeFiles/TLibDecoder.dir/TDecTop.cpp.i

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecTop.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/TLibDecoder.dir/TDecTop.cpp.s"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder/TDecTop.cpp -o CMakeFiles/TLibDecoder.dir/TDecTop.cpp.s

# Object files for target TLibDecoder
TLibDecoder_OBJECTS = \
"CMakeFiles/TLibDecoder.dir/AnnexBread.cpp.o" \
"CMakeFiles/TLibDecoder.dir/NALread.cpp.o" \
"CMakeFiles/TLibDecoder.dir/SEIread.cpp.o" \
"CMakeFiles/TLibDecoder.dir/SyntaxElementParser.cpp.o" \
"CMakeFiles/TLibDecoder.dir/TDecBinCoderCABAC.cpp.o" \
"CMakeFiles/TLibDecoder.dir/TDecCAVLC.cpp.o" \
"CMakeFiles/TLibDecoder.dir/TDecConformance.cpp.o" \
"CMakeFiles/TLibDecoder.dir/TDecCu.cpp.o" \
"CMakeFiles/TLibDecoder.dir/TDecEntropy.cpp.o" \
"CMakeFiles/TLibDecoder.dir/TDecGop.cpp.o" \
"CMakeFiles/TLibDecoder.dir/TDecSbac.cpp.o" \
"CMakeFiles/TLibDecoder.dir/TDecSlice.cpp.o" \
"CMakeFiles/TLibDecoder.dir/TDecTop.cpp.o"

# External object files for target TLibDecoder
TLibDecoder_EXTERNAL_OBJECTS =

../lib/umake/gcc-9.3/x86_64/release/libTLibDecoder.a: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/AnnexBread.cpp.o
../lib/umake/gcc-9.3/x86_64/release/libTLibDecoder.a: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/NALread.cpp.o
../lib/umake/gcc-9.3/x86_64/release/libTLibDecoder.a: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/SEIread.cpp.o
../lib/umake/gcc-9.3/x86_64/release/libTLibDecoder.a: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/SyntaxElementParser.cpp.o
../lib/umake/gcc-9.3/x86_64/release/libTLibDecoder.a: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecBinCoderCABAC.cpp.o
../lib/umake/gcc-9.3/x86_64/release/libTLibDecoder.a: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecCAVLC.cpp.o
../lib/umake/gcc-9.3/x86_64/release/libTLibDecoder.a: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecConformance.cpp.o
../lib/umake/gcc-9.3/x86_64/release/libTLibDecoder.a: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecCu.cpp.o
../lib/umake/gcc-9.3/x86_64/release/libTLibDecoder.a: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecEntropy.cpp.o
../lib/umake/gcc-9.3/x86_64/release/libTLibDecoder.a: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecGop.cpp.o
../lib/umake/gcc-9.3/x86_64/release/libTLibDecoder.a: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecSbac.cpp.o
../lib/umake/gcc-9.3/x86_64/release/libTLibDecoder.a: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecSlice.cpp.o
../lib/umake/gcc-9.3/x86_64/release/libTLibDecoder.a: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/TDecTop.cpp.o
../lib/umake/gcc-9.3/x86_64/release/libTLibDecoder.a: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/build.make
../lib/umake/gcc-9.3/x86_64/release/libTLibDecoder.a: source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_14) "Linking CXX static library ../../../../lib/umake/gcc-9.3/x86_64/release/libTLibDecoder.a"
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && $(CMAKE_COMMAND) -P CMakeFiles/TLibDecoder.dir/cmake_clean_target.cmake
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/TLibDecoder.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/build: ../lib/umake/gcc-9.3/x86_64/release/libTLibDecoder.a
.PHONY : source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/build

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/clean:
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder && $(CMAKE_COMMAND) -P CMakeFiles/TLibDecoder.dir/cmake_clean.cmake
.PHONY : source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/clean

source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/depend:
	cd /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/source/Lib/TLibDecoder /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder /home/mcnl/mcnl/project/mpeg-vpcc/mpeg-pcc-tmc2/dependencies/external/HM/build/source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : source/Lib/TLibDecoder/CMakeFiles/TLibDecoder.dir/depend

