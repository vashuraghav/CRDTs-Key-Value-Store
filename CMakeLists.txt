cmake_minimum_required (VERSION 3.5)
if (UNIX)
  add_definitions(-Wno-sign-compare -Wno-enum-compare)
endif(UNIX)
project(restserver)
set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
option(DBMS "Dbms" OFF)
include_directories("${CMAKE_SOURCE_DIR}" "/usr/local/include" "/opt/homebrew/opt/cpprestsdk/include" "/opt/homebrew/opt/boost/include" "/opt/homebrew/opt/openssl/include")
# Add library paths
link_directories(
    "/opt/homebrew/opt/cpprestsdk/lib"
    "/opt/homebrew/opt/boost/lib"
    "/opt/homebrew/opt/openssl/lib"
)
set (CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/Build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)


add_executable(restserver
  main.cpp
  src/handler.cpp
  core/timestamp.cc
  )

set(Casablanca_LIBRARIES "-lcpprest -lboost_system -lcrypto -lssl")
set(LINKING_LIBRARIES ${Casablanca_LIBRARIES})

if(DBMS)
	set(NEW_DB_SRC_DIR ${CMAKE_SOURCE_DIR}/dbms/src)
	add_library(dbms SHARED
		${NEW_DB_SRC_DIR}/Dbms.cpp
		)
	set(Dbms_dep_lib "-lpqxx -lpq")
	set(LINKING_LIBRARIES ${LINKING_LIBRARIES} dbms ${Dbms_dep_lib})
endif(DBMS)

if (UNIX)
	# Linking POSIX threads
	find_package(Threads REQUIRED)
	set(LINKING_LIBRARIES ${LINKING_LIBRARIES} Threads::Threads)
endif (UNIX)

target_link_libraries(restserver PRIVATE ${LINKING_LIBRARIES})


