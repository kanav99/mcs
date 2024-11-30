cmake_minimum_required(VERSION 3.16)
project(mcs VERSION 0.1.0)

find_package(Eigen3 REQUIRED)

include(FetchContent)
FetchContent_Declare(
  Corrosion
  GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
  GIT_TAG v0.4)
FetchContent_MakeAvailable(Corrosion)

corrosion_import_crate(MANIFEST_PATH Cargo.toml CRATES mcs-cpp)

add_library(mcs STATIC src/lib.cpp)
target_include_directories(
  mcs PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>)
target_link_libraries(mcs PUBLIC Eigen3::Eigen)
target_link_libraries(mcs PRIVATE mcscpp)
set_property(TARGET mcs PROPERTY CXX_STANDARD 20)
set_property(TARGET mcs PROPERTY POSITION_INDEPENDENT_CODE ON)

# install
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(
  TARGETS mcs mcscpp
  EXPORT ${PROJECT_NAME}Targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
  EXPORT ${PROJECT_NAME}Targets
  NAMESPACE mcs::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

configure_package_config_file(
  cmake/mcsConfig.cmake.in
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

install(FILES include/mcs.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# test
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
  if(BUILD_TESTING)
    add_subdirectory(tests)
  endif()
endif()