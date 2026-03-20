add_library(telecom_scheduler STATIC
    scheduler.cpp
    round_robin_scheduler.cpp
    proportional_fair_scheduler.cpp
    resource_block.cpp
    channel_model.cpp
    ue.cpp
    harq.cpp
)

target_include_directories(telecom_scheduler PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)

add_executable(simulator main.cpp)
target_link_libraries(simulator PRIVATE telecom_scheduler)
