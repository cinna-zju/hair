cmake_minimum_required( VERSION 2.8 )
project( hair )

set( CMAKE_BUILD_TYPE Release )
set( CMAKE_CXX_FLAGS "-std=c++11 -O3" )

include_directories( “/opt/local/include” )
include_directories( “./common.h” )

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIR})

# GLEW
find_package( GLEW REQUIRED )
include_directories( ${GLEW_INCLUDE_DIRS} )


#glfw
find_package(GLFW3 REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})

#glm
find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

add_executable( hair main.cpp common.cpp )
target_link_libraries( hair ${OPENGL_LIBRARIES} -lglew -lglfw)
