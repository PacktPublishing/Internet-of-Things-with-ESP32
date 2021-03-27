# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)
set(COMPONENT_ADD_INCLUDEDIRS ".")

idf_component_register(SRCS ${app_sources})

target_add_binary_data(${COMPONENT_TARGET} "../tmp/AmazonRootCA1.pem" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "../tmp/my_sensor1.cert.pem" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "../tmp/my_sensor1.private.key" TEXT)
