cmake_minimum_required(VERSION 3.1)

set(PROJECT MdlToObj)
set(ORGANIZATION_NAME "FunParadigm")
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(PROJECT_VER ${VERSION_MAJOR}.${VERSION_MINOR})

enable_language(C CXX)

project(
	${PROJECT} 
	VERSION ${PROJECT_VER} 
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)

add_executable(MdlToObj main.cpp MdlToObj.hpp)
	
if ("${CMAKE_EXE_LINKER_FLAGS}" STREQUAL "/machine:x64")
	set(ARCH x64)
else()
	set(ARCH x32)
endif()
	
set(FINAL_BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_INSTALL_PREFIX "${FINAL_BIN_DIR}")

set(${PROJECT}_OUTPUT_NAME "${PROJECT}_${CMAKE_SYSTEM_NAME}_$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:MinSizeRel>:MinSizeRel>$<$<CONFIG:RelWithDebInfo>:RelWithDebInfo>_${ARCH}")
set_target_properties(${PROJECT} PROPERTIES	OUTPUT_NAME "${${PROJECT}_OUTPUT_NAME}")
set_target_properties(${PROJECT} PROPERTIES	RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")

#############################################################
### INSTALL #################################################
#############################################################

install(TARGETS ${PROJECT} RUNTIME DESTINATION / COMPONENT APP_BINARY)

#############################################################
### PACKAGE #################################################
## https://gitlab.kitware.com/cmake/community/-/wikis/doc/cpack/Component-Install-With-CPack
#############################################################

set(CPACK_GENERATOR "ZIP")
set(CPACK_MONOLITHIC_INSTALL 1)
set(CPACK_THREADS 8)
if (UNIX)
	set(CPACK_SET_DESTDIR ON)
endif()
set(CPACK_ZIP_COMPONENT_INSTALL ON)
set(CPACK_PACKAGE_VENDOR ${ORGANIZATION_NAME})
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 1)
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_NAME ${PROJECT})
SET(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_INSTALL_PREFIX}/")
set(CPACK_PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}")

include(CPack)
