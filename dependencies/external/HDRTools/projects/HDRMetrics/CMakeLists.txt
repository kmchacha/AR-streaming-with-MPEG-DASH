# executable
set( EXE_NAME HDRMetrics )

#get source files
file( GLOB SRC_FILES "src/*.cpp" )

#get include files
file( GLOB INC_FILES "inc/*.H" )

# add executable
add_executable( ${EXE_NAME} ${SRC_FILES} ${INC_FILES} ) 
include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_include_directories( ${EXE_NAME} PUBLIC . ./src ./inc)
target_link_libraries( ${EXE_NAME} HDRLib )

# place header files in different folder
source_group( "Include Files" FILES ${INC_FILES} )

# set the folder where to place the projects
set_target_properties( ${EXE_NAME}  PROPERTIES FOLDER bin LINKER_LANGUAGE CXX )
