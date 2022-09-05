# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.24

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
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.24.0/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.24.0/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build

# Utility rule file for PioasmBuild.

# Include any custom commands dependencies for this target.
include pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild.dir/compiler_depend.make

# Include the progress variables for this target.
include pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild.dir/progress.make

pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild: pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild-complete

pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild-complete: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-install
pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild-complete: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-mkdir
pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild-complete: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-download
pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild-complete: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-update
pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild-complete: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-patch
pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild-complete: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-configure
pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild-complete: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-build
pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild-complete: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-install
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Completed 'PioasmBuild'"
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo && /opt/homebrew/Cellar/cmake/3.24.0/bin/cmake -E make_directory /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo/CMakeFiles
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo && /opt/homebrew/Cellar/cmake/3.24.0/bin/cmake -E touch /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild-complete
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo && /opt/homebrew/Cellar/cmake/3.24.0/bin/cmake -E touch /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-done

pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-build: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-configure
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Performing build step for 'PioasmBuild'"
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pioasm && $(MAKE)

pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-configure: pico_extras/src/common/pico_scanvideo/pioasm/tmp/PioasmBuild-cfgcmd.txt
pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-configure: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-patch
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Performing configure step for 'PioasmBuild'"
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pioasm && /opt/homebrew/Cellar/cmake/3.24.0/bin/cmake "-GUnix Makefiles" /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-sdk/tools/pioasm
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pioasm && /opt/homebrew/Cellar/cmake/3.24.0/bin/cmake -E touch /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-configure

pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-download: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-source_dirinfo.txt
pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-download: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-mkdir
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "No download step for 'PioasmBuild'"
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo && /opt/homebrew/Cellar/cmake/3.24.0/bin/cmake -E echo_append
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo && /opt/homebrew/Cellar/cmake/3.24.0/bin/cmake -E touch /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-download

pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-install: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-build
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "No install step for 'PioasmBuild'"
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pioasm && /opt/homebrew/Cellar/cmake/3.24.0/bin/cmake -E echo_append
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pioasm && /opt/homebrew/Cellar/cmake/3.24.0/bin/cmake -E touch /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-install

pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-mkdir:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Creating directories for 'PioasmBuild'"
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo && /opt/homebrew/Cellar/cmake/3.24.0/bin/cmake -Dcfgdir= -P /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo/pioasm/tmp/PioasmBuild-mkdirs.cmake
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo && /opt/homebrew/Cellar/cmake/3.24.0/bin/cmake -E touch /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-mkdir

pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-patch: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-update
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "No patch step for 'PioasmBuild'"
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo && /opt/homebrew/Cellar/cmake/3.24.0/bin/cmake -E echo_append
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo && /opt/homebrew/Cellar/cmake/3.24.0/bin/cmake -E touch /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-patch

pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-update: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-download
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "No update step for 'PioasmBuild'"
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo && /opt/homebrew/Cellar/cmake/3.24.0/bin/cmake -E echo_append
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo && /opt/homebrew/Cellar/cmake/3.24.0/bin/cmake -E touch /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-update

PioasmBuild: pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild
PioasmBuild: pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild-complete
PioasmBuild: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-build
PioasmBuild: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-configure
PioasmBuild: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-download
PioasmBuild: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-install
PioasmBuild: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-mkdir
PioasmBuild: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-patch
PioasmBuild: pico_extras/src/common/pico_scanvideo/pioasm/src/PioasmBuild-stamp/PioasmBuild-update
PioasmBuild: pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild.dir/build.make
.PHONY : PioasmBuild

# Rule to build all files generated by this target.
pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild.dir/build: PioasmBuild
.PHONY : pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild.dir/build

pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild.dir/clean:
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo && $(CMAKE_COMMAND) -P CMakeFiles/PioasmBuild.dir/cmake_clean.cmake
.PHONY : pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild.dir/clean

pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild.dir/depend:
	cd /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/libraries/pico-extras/src/common/pico_scanvideo /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo /Volumes/ExtSSD/workspace/rpi-pico/RP2040-HAT-MINING-C/build/pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : pico_extras/src/common/pico_scanvideo/CMakeFiles/PioasmBuild.dir/depend
