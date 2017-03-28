set (CMAKE_CXX_STANDARD 14)

find_package( Boost 1.40 COMPONENTS system program_options serialization filesystem iostreams REQUIRED )

find_package(OpenMP QUIET)
if(OPENMP_FOUND)
   MESSAGE( STATUS "xc: OpenMP found" )
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_definitions(-DIL_STD)
find_package( CPLEX REQUIRED )

find_package( OpenCV REQUIRED)

FILE(GLOB MainPreprocessingSrc src/common/*.cpp src/preprocessing/*.cpp)

add_executable( preprocessingV2 ${MainPreprocessingSrc})
target_link_libraries( preprocessingV2 ${Boost_LIBRARIES} ${CPLEX_LIBRARIES} ${OpenCV_LIBRARIES})
target_compile_definitions( preprocessingV2 PRIVATE PREPROCESSING=1)
target_include_directories(preprocessingV2 PRIVATE inc inc/preprocessing ${Boost_INCLUDE_DIR} ${CPLEX_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})



FILE(GLOB MainPostprocessingSrc src/common/*.cpp src/postprocessing/*.cpp)

add_executable( postprocessingV2 ${MainPostprocessingSrc})
target_link_libraries( postprocessingV2 ${Boost_LIBRARIES} ${CPLEX_LIBRARIES} ${OpenCV_LIBRARIES})
target_compile_definitions( postprocessingV2 PRIVATE POSTPROCESSING=1)
target_include_directories( postprocessingV2 PRIVATE inc inc/postprocessing ${Boost_INCLUDE_DIR} ${CPLEX_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})

add_custom_command(TARGET preprocessingV2 POST_BUILD COMMAND cp preprocessingV2 ..)
add_custom_command(TARGET postprocessingV2 POST_BUILD COMMAND cp postprocessingV2 ..)
