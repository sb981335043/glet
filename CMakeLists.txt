cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(gpu-let)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_TYPE Debug)
set(GPULET_ROOT $ENV{PWD})

# cuda toolkit的版本限定
set(CUDA_PATH /usr/local/cuda)
set(CUDA_TOOLKIT_ROOT_DIR /usr/local/cuda)
set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)

# boost库
set(Boost_INCLUDE_DIRS /home/scc/glet/boost/include)
set(Boost_LIBRARY_DIRS /home/scc/glet/boost/lib)

# find_package(Boost REQUIRED)

# add_compile_options(-I${Boost_INCLUDE_DIRS})

find_package(Torch REQUIRED)
message(STATUS "Find Torch VERSION: ${Torch_VERSION}")
add_definitions(-DTORCH_VERSION_MAJOR=${Torch_VERSION_MAJOR})
add_definitions(-DTORCH_VERSION_MINOR=${Torch_VERSION_MINOR})
add_definitions(-DTORCH_VERSION_PATCH=${Torch_VERSION_PATCH})

find_package(OpenCV REQUIRED)

message(${OpenCV_INCLUDE_DIRS})

include_directories(
	#include
        src/common
	src/proxy
	src/scheduler
	src/service
	#${TONIC_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Boost_INCLUDE_DIRS} #boost
)

link_directories(${Boost_LIBRARY_DIRS}) #boost

file(GLOB COMMON_SRCS src/common/*.cpp)
file(GLOB SERVER_SRCS src/service/*.cpp)
file(GLOB SCHEDULER_SRCS src/scheduler/*.cpp)


#------Essential Flags for experiment-----
add_compile_definitions(EPOCH_DEBUG)
add_compile_definitions(CHECK_MEM)
add_compile_definitions(CHECK_NETBW)
add_compile_definitions(PROFILE)
add_compile_definitions(SCALE)
add_compile_definitions(ADD_RATE)
add_compile_definitions(PROXY_LOG)

#-----Useful Flags for Debugging----------
add_compile_definitions(NO_NET)
#add_compile_definitions(BACKEND_DEBUG)
#add_compile_definitions(FRONTEND_DEBUG)
#add_compile_definitions(DEBUG)
#add_compile_definitions(SCHED_DEBUG)
#add_compile_definitions(SCALE_DEBUG)
#add_compile_definitions(DROP_DEBUG)
#add_compile_definitions(ST_DEBUG)
#add_compile_definitions(LB_DEBUG)
#add_compile_definitions(OVERHEAD)

#----- Flags left in case someone finds them useful -------
#add_compile_definitions(EAGER_DROP)
#add_compile_definitions(DEV_LOG)
#add_compile_definitions(MUTRACE)


#----- Clean ----- If there is any problem, do this first
# rm -rf ./bin/CMakeFiles
# rm -rf ./build ./bin


#---------standalone_inference---------

add_executable(standalone_inference src/standalone_inference.cpp ${SCHEDULER_SRCS} ${COMMON_SRCS})
target_link_libraries(standalone_inference "${TORCH_LIBRARIES}" ${OpenCV_LIBS} boost_program_options boost_exception boost_regex  boost_system nvidia-ml)
set_property(TARGET standalone_inference PROPERTY CXX_STANDARD 14)     

#---------standalone_scheduler---------
add_executable(standalone_scheduler src/standalone_scheduler.cpp
                ${SCHEDULER_SRCS}
                src/common/scheduler_utils.cpp
                src/common/jsoncpp.cpp
                src/common/config.cpp
                )
target_link_libraries(standalone_scheduler boost_program_options boost_exception boost_regex  boost_system)
set_property(TARGET standalone_scheduler PROPERTY CXX_STANDARD 14)


#---------proxy---------
add_executable(proxy src/proxy.cpp 
	src/proxy/profile.cpp
	src/service/proxy_ctrl.cpp
       	${COMMON_SRCS})
target_link_libraries(proxy "${TORCH_LIBRARIES}" ${OpenCV_LIBS} boost_program_options boost_exception boost_regex boost_system nvidia-ml)
set_property(TARGET proxy PROPERTY CXX_STANDARD 14) 

#---------frontend---------

add_executable(frontend src/frontend.cpp ${COMMON_SRCS} ${SCHEDULER_SRCS}
        src/service/app_instance.cpp
        src/service/app_spec.cpp
        src/service/app_thread.cpp
        src/service/backend_delegate.cpp
        src/service/batched_request.cpp
        src/service/drop_thread.cpp
        src/service/EWMA.cpp
        src/service/input.cpp
        src/service/monitor_thread.cpp
        src/service/proxy_thread.cpp
        src/service/request.cpp
        src/service/request_thread.cpp
        src/service/router_proxy.cpp
        src/service/router_scheduling.cpp
        src/service/router_setup.cpp
        src/service/server_thread.cpp
        src/service/backend_setup.cpp
        src/service/backend_proxy_ctrl.cpp
        src/service/device_spec.cpp
	src/service/load_balancer.cpp
        )
target_link_libraries(frontend "${TORCH_LIBRARIES}" ${OpenCV_LIBS} boost_program_options boost_exception boost_regex boost_system nvidia-ml)

set_property(TARGET frontend PROPERTY CXX_STANDARD 14) 
#
#---------backend---------
add_executable(backend src/backend.cpp  
        ${COMMON_SRCS} 
        src/service/app_thread.cpp
        src/service/backend_thread.cpp
        src/service/app_instance.cpp
        src/service/app_spec.cpp
        src/service/backend_control.cpp
        src/service/backend_data.cpp
	src/service/input.cpp
	src/service/proxy_ctrl.cpp
        )
target_link_libraries(backend "${TORCH_LIBRARIES}" ${OpenCV_LIBS} boost_program_options boost_exception boost_regex boost_system nvidia-ml)
target_compile_definitions(backend PRIVATE BACKEND=1)

set_property(TARGET backend PROPERTY CXX_STANDARD 14) 


#---------client for testing---------
add_executable(client src/client.cpp 
	src/common/socket.cpp
	src/common/img_utils.cpp
	src/common/torch_utils.cpp
	src/common/common_utils.cpp
	src/common/exp_model.cpp
	)
target_link_libraries(client "${TORCH_LIBRARIES}" ${OpenCV_LIBS} boost_program_options boost_exception boost_regex boost_system nvidia-ml)
set_property(TARGET client PROPERTY CXX_STANDARD 14)     
