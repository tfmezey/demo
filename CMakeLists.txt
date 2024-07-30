cmake_minimum_required(VERSION 3.27)

project(demo
	VERSION 1.0
	DESCRIPTION "github test/demo project."
	LANGUAGES CXX ASM_NASM
)

get_filename_component(REPOSITORY "~/togithub" ABSOLUTE)
get_filename_component(REPOSITORY_INCLUDE_DIR "${REPOSITORY}/include" ABSOLUTE)
get_filename_component(REPOSITORY_SOURCE_DIR "${REPOSITORY}/src" ABSOLUTE)
get_filename_component(REPOSITORY_LIB_DIR "${REPOSITORY}/lib" ABSOLUTE)
get_filename_component(INCLUDE_DIR "${REPOSITORY_INCLUDE_DIR}" ABSOLUTE)
get_filename_component(SOURCE_DIR "${REPOSITORY_SOURCE_DIR}" ABSOLUTE)
get_filename_component(BIN_INSTALL_DIR "${REPOSITORY}/bin" ABSOLUTE)
get_filename_component(LIB_INSTALL_DIR "${REPOSITORY}/lib" ABSOLUTE)
get_filename_component(INCLUDE_INSTALL_DIR "${REPOSITORY_INCLUDE_DIR}" ABSOLUTE)
get_filename_component(SOURCE_INSTALL_DIR "${REPOSITORY_SOURCE_DIR}" ABSOLUTE)
get_filename_component(SOLVER_SOURCE_DIR "${REPOSITORY_INCLUDE_DIR}/solver" ABSOLUTE)

get_filename_component(GRAPHS_INCLUDE_DIR "${REPOSITORY_INCLUDE_DIR}/graphs" ABSOLUTE)
get_filename_component(ALGORITHMS_INCLUDE_DIR "${REPOSITORY_INCLUDE_DIR}/algorithms" ABSOLUTE)
get_filename_component(CONTAINERS_INCLUDE_DIR "${REPOSITORY_INCLUDE_DIR}/containers" ABSOLUTE)
get_filename_component(UTILITIES_INCLUDE_DIR "${REPOSITORY_INCLUDE_DIR}/utilities" ABSOLUTE)
get_filename_component(SOLVER_INCLUDE_DIR "${REPOSITORY_INCLUDE_DIR}/solver" ABSOLUTE)
get_filename_component(GRAPHS_SOURCE_DIR "${REPOSITORY_SOURCE_DIR}/graphs" ABSOLUTE)
get_filename_component(ALGORITHMS_SOURCE_DIR "${REPOSITORY_SOURCE_DIR}/algorithms" ABSOLUTE)
get_filename_component(CONTAINERS_SOURCE_DIR "${REPOSITORY_SOURCE_DIR}/containers" ABSOLUTE)
get_filename_component(UTILITIES_SOURCE_DIR "${REPOSITORY_SOURCE_DIR}/utilities" ABSOLUTE)
get_filename_component(SOLVER_SOURCE_DIR "${REPOSITORY_SOURCE_DIR}/solver" ABSOLUTE)

get_filename_component(ASM_SOURCE_DIRECTORY "${SOLVER_SOURCE_DIR}/asm" ABSOLUTE)

set(SourceFiles
	${REPOSITORY_SOURCE_DIR}/main.cpp
	${REPOSITORY_SOURCE_DIR}/test_algorithms.cpp
	${REPOSITORY_SOURCE_DIR}/test_graphs.cpp
	${REPOSITORY_SOURCE_DIR}/test_solver.cpp
	${REPOSITORY_SOURCE_DIR}/initial_conditions.cpp
	${REPOSITORY_SOURCE_DIR}/debug_solver.cpp
	${SOLVER_SOURCE_DIR}/gravity.cpp
	${SOLVER_SOURCE_DIR}/asm/rk4.asm
	${SOLVER_SOURCE_DIR}/asm/rkf.asm
)

# Explicitly add the header files so that they appear in the IDE.
set(IncludeFiles
	${REPOSITORY_INCLUDE_DIR}/algorithms.h
	${REPOSITORY_INCLUDE_DIR}/containers.h
	${REPOSITORY_INCLUDE_DIR}/graphs.h
	${REPOSITORY_INCLUDE_DIR}/ode_solver.h
	${REPOSITORY_INCLUDE_DIR}/utilities.h
	${REPOSITORY_INCLUDE_DIR}/initial_conditions.h
)

set(ExportedIncludeFiles
	${SOLVER_INCLUDE_DIR}/release/ode_solver.h
)

function(print_status INCLUDE_DIR SRC_DIR IncludeFiles SourceFiles Exports)
	
	message(STATUS "")
	message(STATUS "${PROJECT_NAME}:")
	get_target_property(project_type ${PROJECT_NAME} TYPE)
	string(TOLOWER ${CMAKE_BUILD_TYPE} buildType)
	
	if(${project_type} STREQUAL "SHARED_LIBRARY")
		message(STATUS "Shared lib, ${buildType} build")
	elseif(project_type STREQUAL "STATIC_LIBRARY")
		message(STATUS "Static lib, ${buildType} build")
	elseif(project_type STREQUAL "MODULE_LIBRARY")
		message(STATUS "Module lib, ${buildType} build")
	elseif(project_type STREQUAL "OBJECT_LIBRARY")
		message(STATUS "Object lib, ${buildType} build")
	elseif(project_type STREQUAL "INTERFACE_LIBRARY")
		message(STATUS "Interface lib, ${buildType} build")
	else()
		message(STATUS "Executable, ${buildType} build")
	endif()
	
	message(STATUS "Project directory:  ${PROJECT_SOURCE_DIR}")
	message(STATUS "Include directory:  ${INCLUDE_DIR}")
	message(STATUS "Source directory:  ${SRC_DIR}")

	list(LENGTH SourceFiles length)
	if(${length} STREQUAL "0")
	elseif(${length} STREQUAL "1")
		message(STATUS "Project has one source file:")
		message(STATUS "\t${SourceFiles}")
	else()
		message(STATUS "Project has ${length} source files:")
		foreach(f IN LISTS SourceFiles)
			message(STATUS "\t${f}")
		endforeach(f)
	endif()
	
	list(LENGTH IncludeFiles length)
	if(${length} STREQUAL "0")
	elseif(${length} STREQUAL "1")
		message(STATUS "Project has one include file:")
		message(STATUS "\t${IncludeFiles}")
	else()
		message(STATUS "length > 1")
		message(STATUS "Project has ${length} include files:")
		foreach(f IN LISTS IncludeFiles)
			message(STATUS "\t${f}")
		endforeach(f)
	endif()
	
	list(LENGTH Exports length)
	if(${length} STREQUAL "0")
	elseif(${length} STREQUAL "1")
		message(STATUS "Project will export one include file:")
		message(STATUS "\t${Exports}")
	else()
		message(STATUS "Project will export ${length} include files:")
		foreach(f IN LISTS Exports)
			message(STATUS "\t${f}")
		endforeach(f)
	endif()

	if(project_type STREQUAL "SHARED_LIBRARY")
		message(STATUS "Installing to ${LIB_INSTALL_DIR}")
	elseif(project_type STREQUAL "EXECUTABLE")
		message(STATUS "Installing to ${BIN_INSTALL_DIR}")
	endif()
	
endfunction()

function(setup_for_assembly)
	# Determine the project type and set up cmake variables pertaining to assembly language.  Note that the language
	# has to be enabled prior to calling this function!

	enable_language(ASM_NASM)
	set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>" PARENT_SCOPE)
	set(CMAKE_ASM_NASM_FLAGS "-f elf64 -i ${ASM_SOURCE_DIRECTORY}" PARENT_SCOPE)
	# set(CMAKE_ASM_NASM_FLAGS_DEBUG "-f elf64 -g -F dwarf -i${ASM_SOURCE_DIRECTORY}" PARENT_SCOPE)
	set(CMAKE_ASM_NASM_FLAGS_DEBUG "-f elf64 -g -F dwarf -i${ASM_SOURCE_DIRECTORY}" PARENT_SCOPE)

	# This variable is very poorly documented.  I suggest you copy and paste the command and find out.
	# Note that the -g option is used for debug info.  You need to assemble it with a corresponding
	# assembler debug option, -g -F dwarf.
	get_target_property(project_type ${PROJECT_NAME} TYPE)
	if(${project_type} STREQUAL "SHARED_LIBRARY")
		# Shared Library
		if(_DEBUG)
			set(CMAKE_ASM_NASM_CREATE_SHARED_LIBRARY "ld -g -lc -shared <CMAKE_ASM_NASM_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>" PARENT_SCOPE)
		else()
			set(CMAKE_ASM_NASM_CREATE_SHARED_LIBRARY "ld -lc -shared <CMAKE_ASM_NASM_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>" PARENT_SCOPE)
		endif()
	elseif(${project_type} STREQUAL "STATIC_LIBRARY")
		# Static Library
		# Note that if you create a static library, you cannot install it.
		if(_DEBUG)
			set(CMAKE_ASM_NASM_CREATE_SHARED_LIBRARY "ld -g -lc -static <CMAKE_ASM_NASM_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>" PARENT_SCOPE)
		else()
			set(CMAKE_ASM_NASM_CREATE_SHARED_LIBRARY "ld -lc -static <CMAKE_ASM_NASM_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>" PARENT_SCOPE)
		endif()
	elseif(${project_type} STREQUAL "EXECUTABLE")
		# Executable
		# In case the ASM project is an executable, either call ld with the below linker options, or call gcc with its default options, which will be
		# passed onto the linker.
		if(_DEBUG)
			add_link_options(-g -m elf_x86_64 -pie -dynamic-linker /lib64/ld-linux-x86-64.so.2 -lc)
		else()
			add_link_options(-m elf_x86_64 -pie -dynamic-linker /lib64/ld-linux-x86-64.so.2 -lc)
		endif()
		set(CMAKE_ASM_NASM_LINK_EXECUTABLE "ld <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>" PARENT_SCOPE)
		# set(CMAKE_ASM_NASM_LINK_EXECUTABLE "gcc <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>" PARENT_SCOPE)
		
		set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>" PARENT_SCOPE)
		set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64 PARENT_SCOPE)
	endif()
endfunction()

# set(_DEBUG TRUE)
# 
# if(${_DEBUG} EQUAL FALSE)
# 	set(COMPILER_DEFINITIONS)
# 	set(COMPILER_OPTIONS "-std=c++20")
# 	add_definitions(${COMPILER_DEFINITIONS})
# 	add_compile_options(${COMPILER_OPTIONS})
# else()
# 	set(COMPILER_DEFINITIONS)
# 	set(COMPILER_OPTIONS "-std=c++20")
# 	add_definitions(${COMPILER_DEFINITIONS})
# 	add_compile_options(${COMPILER_OPTIONS})
# endif()

add_executable(demo ${SourceFiles} ${IncludeFiles} ${ExportedIncludeFiles})

add_subdirectory(algorithms)
add_subdirectory(graphs)
add_subdirectory(utilities)

# set(INCLUDE_DIRECTORIES "${GRAPHS_INCLUDE_DIR} ${ALGORITHMS_INCLUDE_DIR} ${CONTAINERS_INCLUDE_DIR} ${UTILITIES_INCLUDE_DIR}")
target_include_directories(demo PRIVATE ${REPOSITORY_INCLUDE_DIR})
target_include_directories(demo PRIVATE ${SOLVER_INCLUDE_DIR})

# As we mirror using libraries of our main repository, we need to override default
# linking behavior to make use of the three shared libraries build as sub-projects.
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath ${LIB_INSTALL_DIR}")
set(CMAKE_INSTALL_RPATH ${LIB_INSTALL_PATH})
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

target_link_libraries(demo PRIVATE algorithms graphs utilities)

add_dependencies(demo algorithms graphs utilities)
add_dependencies(algorithms graphs)

setup_for_assembly()

# Print out project info
print_status(${INCLUDE_DIR} ${SOURCE_DIR} "${IncludeFiles}" "${SourceFiles}" "${ExportedIncludeFiles}")

install(TARGETS demo RUNTIME DESTINATION ${BIN_INSTALL_DIR})
install(FILES ${ExportedIncludeFiles} DESTINATION ${INCLUDE_INSTALL_DIR})

