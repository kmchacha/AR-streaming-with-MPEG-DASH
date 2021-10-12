# library
set( LIB_NAME HDRLib )

#get source files
file( GLOB SRC_FILES "src/*.cpp" )

#get include files
file( GLOB INC_FILES "inc/*.H" )

# library
add_library( ${LIB_NAME} STATIC ${SRC_FILES} ${INC_FILES} )
target_compile_definitions( ${LIB_NAME} PUBLIC )

target_include_directories( ${LIB_NAME} PUBLIC . ./src ./inc)

# place header files in different folders
source_group( "Include Files" FILES ${INC_FILES} )

# set the folder where to place the projects
set_target_properties( ${LIB_NAME} PROPERTIES FOLDER lib )

if(LIBPNG)
  target_link_libraries( ${LIB_NAME} LINK_PUBLIC png )
  target_include_directories( ${LIB_NAME} PUBLIC ../libs/libpng )
endif()
