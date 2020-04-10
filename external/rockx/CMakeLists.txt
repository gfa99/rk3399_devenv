project (rockx)
cmake_minimum_required(VERSION 3.8)

include(sdk/rockx-${COMPILE_PLATFORM}/RockXConfig.cmake)

install(DIRECTORY ${RockX_INCLUDE_DIRS}/ DESTINATION "include/rockx")
install(FILES ${RockX_LIBS} DESTINATION "lib64")

