# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (Tutorials)
#[[
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})]]


if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()


add_subdirectory (external)

#On Visual 2005 and above, this module can set the debug working directory
cmake_policy(SET CMP0026 OLD)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-fe2273")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) # /MP

if(INCLUDE_DISTRIB)
	add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)


add_definitions(
	-DTW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

# SLIC
add_executable(SLIC
        SLIC/Slic.cpp
	Include/ImageBase.cpp
	Include/ImageBase.hpp
	Include/image_ppm.h
)

#target_link_libraries(SLIC
    #${ALL_LIBS}
    #${SDL2_LIBRARIES}
#)

# Xcode and Visual working directories
set_target_properties(SLIC PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SLIC/")
create_target_launcher(SLIC WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/SLIC/")




SOURCE_GROUP(common REGULAR_EXPRESSION ".*/Include/.*" )


if (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )
add_custom_command(
   TARGET SLIC POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/SLIC${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/SLIC/"
)
elseif (${CMAKE_GENERATOR} MATCHES "Xcode" )

endif (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )

