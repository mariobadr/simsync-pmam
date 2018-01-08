cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(
  simsync
  VERSION 0.0.1
  LANGUAGES CXX
)

add_library(
  ${PROJECT_NAME}
  include/simsync/application.hpp
  include/simsync/architecture.hpp
  include/simsync/core.hpp
  include/simsync/core_type.hpp
  include/simsync/simulate.hpp
  include/simsync/system.hpp
  include/simsync/thread.hpp
  include/simsync/reports/report.hpp
  include/simsync/reports/criticality_stack.hpp
  include/simsync/reports/event_trace.hpp
  include/simsync/reports/scheduler_trace.hpp
  include/simsync/reports/time_stack.hpp
  include/simsync/synchronization/barrier_wait.hpp
  include/simsync/synchronization/condition_broadcast.hpp
  include/simsync/synchronization/condition_signal.hpp
  include/simsync/synchronization/condition_wait.hpp
  include/simsync/synchronization/event.hpp
  include/simsync/synchronization/lock_acquire.hpp
  include/simsync/synchronization/lock_release.hpp
  include/simsync/synchronization/thread_create.hpp
  include/simsync/synchronization/thread_finish.hpp
  include/simsync/synchronization/thread_join.hpp
  include/simsync/synchronization/thread_model.hpp
  include/simsync/synchronization/thread_start.hpp
  include/simsync/synchronization/transition.hpp
  src/application.cpp
  src/architecture.cpp
  src/core.cpp
  src/core_type.cpp
  src/simulate.cpp
  src/system.cpp
  src/thread.cpp
  src/reports/criticality_stack.cpp
  src/reports/time_stack.cpp
  src/synchronization/barrier_wait.cpp
  src/synchronization/condition_broadcast.cpp
  src/synchronization/condition_signal.cpp
  src/synchronization/condition_wait.cpp
  src/synchronization/event.cpp
  src/synchronization/lock_acquire.cpp
  src/synchronization/lock_release.cpp
  src/synchronization/thread_create.cpp
  src/synchronization/thread_finish.cpp
  src/synchronization/thread_join.cpp
  src/synchronization/thread_model.cpp
  src/synchronization/thread_start.cpp
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC include
)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE nlohmann-json
)

set_target_properties(
  ${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)
