cmake_minimum_required(VERSION 3.1)

project(MdlToObj)

set(CMAKE_CXX_STANDARD 17)

add_executable(MdlToObj main.cpp MdlToObj.hpp)
		
set(FINAL_BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_INSTALL_PREFIX "${FINAL_BIN_DIR}")

#############################################################
### INSTALL #################################################
#############################################################

install(TARGETS MdlToObj RUNTIME DESTINATION / COMPONENT APP_BINARY)

#############################################################
### PACKAGE #################################################
## https://gitlab.kitware.com/cmake/community/-/wikis/doc/cpack/Component-Install-With-CPack
#############################################################

set(ORGANIZATION_NAME "FunParadigm")
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)

set(CPACK_GENERATOR "ZIP")
set(CPACK_MONOLITHIC_INSTALL 1)
set(CPACK_THREADS 8)
if (UNIX)
	set(CPACK_SET_DESTDIR ON)
endif()
set(CPACK_ZIP_COMPONENT_INSTALL ON)
set(CPACK_PACKAGE_VENDOR ${ORGANIZATION_NAME})
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 1)
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_NAME "MdlToObj")
set(CPACK_PACKAGE_FILE_NAME "MdlToObj")
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "${PROJECT}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "MdlToObj")
SET(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_INSTALL_PREFIX}/")
set(CPACK_PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}")

include(CPack)
