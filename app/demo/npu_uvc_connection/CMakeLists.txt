# vi: set noexpandtab syntax=cmake:

project(rk_npu_uvc)
include(GNUInstallDirs)

pkg_check_modules(LIBEASYMEDIA REQUIRED libeasymedia)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-fno-rtti)
# add_definitions(-fexceptions -finstrument-functions -funwind-tables -g -rdynamic -O0)
add_definitions(-O2)
include_directories(./)
include_directories(${RKNPU_HEADER_DIR})
include_directories(${ROCKX_HERDER_DIR})
option(NDEBUG OFF)
if(NDEBUG)
  add_definitions("-DNDEBUG")
endif()

set(RK_NPU_UVC_SRC term_help.cc npu_pp_output.cc rknn_ssd.cc rockx_draw.cc)
if(NPU_DEVICE)
  set(RK_NPU_UVC_SRC ${RK_NPU_UVC_SRC} device/npu_uvc_device.cc)
  set(RK_NPU_UVC_TARGET rk_npu_uvc_device)
  set(RK_NPU_UVC_SCRIPT device/npu_uvc_device.sh)
  set(RK_NPU_UVC_LIBRARY easymedia rkuvc rkisp)
  if(ROCKX)
    add_definitions(-DHAVE_ROCKX=1)
    set(RK_NPU_UVC_LIBRARY ${RK_NPU_UVC_LIBRARY} rockx)
  endif()
  set(RK_NPU_UVC_LIBRARY ${RK_NPU_UVC_LIBRARY} SDL2 SDL2_ttf SDL2_gfx)
elseif(NPU_HOST)
  set(RK_NPU_UVC_SRC ${RK_NPU_UVC_SRC} host/npu_uvc_host.cc)
  set(RK_NPU_UVC_TARGET rk_npu_uvc_host)
  set(RK_NPU_UVC_SCRIPT host/npu_uvc_host.sh)
  set(RK_NPU_UVC_LIBRARY easymedia SDL2 SDL2_ttf SDL2_gfx)
endif()

add_executable(${RK_NPU_UVC_TARGET} ${RK_NPU_UVC_SRC})
# add_definitions(-fsanitize=address -g)
# target_link_libraries(${RK_NPU_UVC_TARGET} asan)
target_include_directories(${RK_NPU_UVC_TARGET}
                           PUBLIC ${LIBEASYMEDIA_INCLUDE_DIRS})

target_link_libraries(${RK_NPU_UVC_TARGET} ${RK_NPU_UVC_LIBRARY})

install(TARGETS ${RK_NPU_UVC_TARGET} RUNTIME DESTINATION "bin")
install(PROGRAMS ${RK_NPU_UVC_SCRIPT} DESTINATION "bin")
install(DIRECTORY fonts DESTINATION "/usr/lib"
        FILES_MATCHING PATTERN "*.ttf")
