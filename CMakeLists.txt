
cmake_minimum_required(VERSION 3.16.3)

project("generated_code")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find requirements
find_package(fastcdr REQUIRED)
find_package(fastrtps REQUIRED)

add_executable(HelloWorld A.cpp B.cpp C.cpp PubSubEniity.cpp HelloWorldPubSubTypes.cxx HelloWorldPubSubMain.cxx HelloWorld.cxx)
target_link_libraries(HelloWorld fastcdr fastrtps)



