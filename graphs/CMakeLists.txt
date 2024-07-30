cmake_minimum_required(VERSION 3.5)

# This project originally was part of duplicates, but we spinned it off into an independent project.

# The parent name is ${PROJECT_NAME}, which gets overwritten by our call to project().  So capture it beforehand.
set(PARENT_NAME ${PROJECT_NAME})

project(graphs
	VERSION 1.0
	DESCRIPTION "Our graphs project."
	LANGUAGES CXX
)

# Set kdevelop->Projects->Open Configuration:  ~/work/builds/${PROJECT_NAME}/build
# or form there, Show Advanced->Show Advanced Values and change ${PROJECT_NAME}_BINARY_DIR
# Alternatively, cd {$PROJECT_NAME}; ln -s ~/work/builds/${PROJECT_NAME}/build build

get_filename_component(REPOSITORY "~/togithub" ABSOLUTE)
get_filename_component(REPOSITORY_INCLUDE_DIR "${REPOSITORY}/include" ABSOLUTE)
get_filename_component(REPOSITORY_SOURCE_DIR "${REPOSITORY}/src" ABSOLUTE)
get_filename_component(REPOSITORY_LIB_DIR "${REPOSITORY}/lib" ABSOLUTE)
get_filename_component(INCLUDE_DIR "${REPOSITORY_INCLUDE_DIR}/${PROJECT_NAME}" ABSOLUTE)
get_filename_component(SOURCE_DIR "${REPOSITORY_SOURCE_DIR}/${PROJECT_NAME}" ABSOLUTE)
get_filename_component(APPLICATIONS_INSTALL_DIR "${REPOSITORY}/applications" ABSOLUTE)
get_filename_component(BIN_INSTALL_DIR "${PROJECT_SOURCE_DIR}/bin" ABSOLUTE)
get_filename_component(LIB_INSTALL_DIR "${REPOSITORY}/lib" ABSOLUTE)
get_filename_component(INCLUDE_INSTALL_DIR "${REPOSITORY_INCLUDE_DIR}" ABSOLUTE)
get_filename_component(SOURCE_INSTALL_DIR "${REPOSITORY_SOURCE_DIR}" ABSOLUTE)

# If we have a parent, then hope that the following libraries gets built by it as well.  Otherwise,
# explicitly provide the full path name of each library and hope for the best.
if("${PARENT_NAME}" STREQUAL "")
	# get_filename_component(LIB_CONTAINERS "${REPOSITORY_LIB_DIR}/libcontainers.so" ABSOLUTE)
else()
	# set(LIB_CONTAINERS containers)
endif()

set(SourceFiles
	${SOURCE_DIR}/acyclic_lp.cpp
	${SOURCE_DIR}/acyclic_sp.cpp
	${SOURCE_DIR}/base_graph.cpp
	${SOURCE_DIR}/breadth_first_paths.cpp
	${SOURCE_DIR}/depth_first_order.cpp
	${SOURCE_DIR}/depth_first_search.cpp
	${SOURCE_DIR}/digraph.cpp
	${SOURCE_DIR}/directed_cycle.cpp
	${SOURCE_DIR}/directed_dfs_multi.cpp
	${SOURCE_DIR}/edge.cpp
	${SOURCE_DIR}/edge_weighted_digraph.cpp
	${SOURCE_DIR}/edge_weighted_graph.cpp
	${SOURCE_DIR}/graph.cpp
	${SOURCE_DIR}/_graphs.cpp
	${SOURCE_DIR}/topological.cpp
)

# Explicitly add the header files so that they appear in the IDE.
set(IncludeFiles
	${INCLUDE_DIR}/acyclic_lp.h
	${INCLUDE_DIR}/acyclic_sp.h
	${INCLUDE_DIR}/base_graph.h
	${INCLUDE_DIR}/breadth_first_paths.h
	${INCLUDE_DIR}/depth_first_order.h
	${INCLUDE_DIR}/depth_first_search.h
	${INCLUDE_DIR}/digraph.h
	${INCLUDE_DIR}/directed_cycle.h
	${INCLUDE_DIR}/directed_dfs_multi.h
	${INCLUDE_DIR}/edge.h
	${INCLUDE_DIR}/edge_weighted_digraph.h
	${INCLUDE_DIR}/edge_weighted_graph.h
	${INCLUDE_DIR}/graph.h
	${INCLUDE_DIR}/_graphs_exceptions.h
	${INCLUDE_DIR}/_graphs.h
	${INCLUDE_DIR}/topological.h
)

set(ExportedIncludeFiles
	${INCLUDE_DIR}/release/graphs.h
)

# Use this function to add a list of exported files along with their intended destination.  We also modified print_status()
# to print them out.
function(addExport exportlist location)
	# We need parent scope delim to distinguish one concatenated list of exported files from another in print_status().
	if(NOT DEFINED delim)
		set(delim "-1" PARENT_SCOPE)
	endif()
	# Set these local variables to the current values of Exports/ExportLocations.
	set(Exp ${Exports})
	set(Locations ${ExportLocations})
	# Append the export list variable first with the delimiter and then the entry.  Append locations directory.
	list(APPEND Exp ${delim})
	list(APPEND Exp ${exportlist})
	list(APPEND Locations ${location})
	# Update the parent scope variables.
	set(Exports ${Exp} PARENT_SCOPE)
	set(ExportLocations ${Locations} PARENT_SCOPE)
endfunction()

# Note that this function should be called AFTER add_${executable|library}() is called.
function(print_status INCLUDE_DIR SRC_DIR IncludeFiles SourceFiles)
	
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
		message(STATUS "Project has ${length} include files:")
		foreach(f IN LISTS IncludeFiles)
			message(STATUS "\t${f}")
		endforeach(f)
	endif()
	
	# Print out any exports created via addExport(), and their intended locations.  The parent scope
	# list variables are Exports and ExportLocations.
	list(LENGTH Exports exportCount)
	list(LENGTH ExportLocations locationsCount)
	
	if(NOT exportCount STREQUAL "0" AND NOT locationsCount STREQUAL "0")
		message(STATUS "Exports:")
		if(NOT DEFINED ${location_index})
			set(location_index 0)
		endif()
		
		foreach(f IN LISTS Exports)
			list(GET ExportLocations ${location_index} result)
			if(${f} STREQUAL "-1")
				math(EXPR location_index "${location_index}+1")
				continue()
			endif()
			
			message(STATUS "\t${f} to ${result}")
		endforeach(f)
	endif()

	# Shared libraries are installed to LIB_INSTALL_DIR, while executables are to ./bin and ~/work/applications.
	# Note that static libraries are linked into the executable and thus are not of concern here.
	if(project_type STREQUAL "SHARED_LIBRARY")
		message(STATUS "Installing to ${LIB_INSTALL_DIR}")
	elseif(project_type STREQUAL "EXECUTABLE")
		message(STATUS "Installing to ${BIN_INSTALL_DIR}")
		message(STATUS "Installing to ${APPLICATIONS_INSTALL_DIR}")
	endif()
	
endfunction()

addExport(${ExportedIncludeFiles} ${REPOSITORY_INCLUDE_DIR})

add_library(graphs STATIC ${SourceFiles} ${IncludeFiles} ${ExportedIncludeFiles})
# We do not want to have client projects inherit our include directories.  Thus, we'll use PRIVATE.
target_include_directories(graphs PRIVATE ${REPOSITORY_INCLUDE_DIR} ${INCLUDE_DIR})

# Print out project info
print_status(${INCLUDE_DIR} ${SOURCE_DIR} "${IncludeFiles}" "${SourceFiles}")

# We will primarily export include files, but just in case we end up with another repository, we'll leave this
# commented out install() command.

install(FILES ${ExportedIncludeFiles} DESTINATION ${INCLUDE_INSTALL_DIR})
# install(FILES ${ProjectSourceFiles} DESTINATION ${SOURCE_INSTALL_DIR})


