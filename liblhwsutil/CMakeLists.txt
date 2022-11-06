cmake_minimum_required( VERSION 3.10 )

project( liblhwsutil 
         DESCRIPTION "lhwsutil library"
         HOMEPAGE_URL "https://github.com/fbuonaro/lhwsutil/liblhwsutil"
         LANGUAGES CXX
         VERSION 1.0.0 )

##############################################################
# general
##############################################################

include( GNUInstallDirs )

set( CMAKE_CXX_STANDARD 11 )

add_compile_options( -Wall -Wextra -pedantic -Werror -DBOOST_LOG_DYN_LINK) 

##############################################################
# library
##############################################################

# install directories
set( INSTALL_BINDIR "${CMAKE_INSTALL_BINDIR}" )
set( INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}/liblhwsutil_${PROJECT_VERSION}" )
set( INSTALL_INCDIR "${CMAKE_INSTALL_INCLUDEDIR}/lhwsutil_${PROJECT_VERSION}" )

# pull in liblhmiscutil
find_package( liblhmiscutil REQUIRED )
# pull in libsslutil
find_package( liblhsslutil REQUIRED )
# just does not work with multiple versions
find_package( OpenSSL 1.0 REQUIRED )
# pull in curl
find_package( CURL REQUIRED )
# pull in boost log
find_package( Boost 1.69 COMPONENTS log REQUIRED )
# rapidjson
find_package( RapidJSON REQUIRED )

# source files
set( LIB_SRC_FILES 
     "src/ijwtissuercache.cxx"
     "src/ijwtvalidator.cxx"
     "src/isimplehttpclient.cxx"
     "src/jwtissuercache.cxx"
     "src/jwtutils.cxx"
     "src/jwtvalidator.cxx"
     "src/logging.cxx"
     "src/rsa.cxx"
     "src/simplehttpclientcurl.cxx" )

# library dependencies
set( LIB_PUBLIC_LINKLIBS 
     "${Boost_LIBRARIES}"
     "${OPENSSL_CRYPTO_LIBRARY}"
     "${CURL_LIBRARIES}"
     liblhmiscutil::lhmiscutil
     liblhsslutil::lhsslutil
     jwt )
set( LIB_PRIVATE_LINKLIBS )
# header dependencies
set( LIB_PUBLIC_INCLUDES 
     "${Boost_INCLUDE_DIR}"
     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/lhwsutil>   # build
     $<INSTALL_INTERFACE:${INSTALL_INCDIR}> )                          # export
set( LIB_PRIVATE_INCLUDES 
     "${OPENSSL_INCLUDE_DIR}"
     "${CURL_INCLUDE_DIRS}"
     "${CMAKE_CURRENT_BINARY_DIR}" 
     "${CMAKE_CURRENT_SOURCE_DIR}/include/lhwsutil_impl" )

# boilerplate ..
# libs
add_library( lhwsutil SHARED ${LIB_SRC_FILES} )
add_library( lhwsutil_static STATIC ${LIB_SRC_FILES} )
# lib deps
target_link_libraries( lhwsutil
                       PUBLIC "${LIB_PUBLIC_LINKLIBS}"
                       PRIVATE "${LIB_PRIVATE_LINKLIBS}" )
target_link_libraries( lhwsutil_static
                       PUBLIC "${LIB_PUBLIC_LINKLIBS}"
                       PRIVATE "${LIB_PRIVATE_LINKLIBS}" )
# header deps
target_include_directories( lhwsutil
                            PUBLIC "${LIB_PUBLIC_INCLUDES}"
                            PRIVATE "${LIB_PRIVATE_INCLUDES}" )
target_include_directories( lhwsutil_static
                            PUBLIC "${LIB_PUBLIC_INCLUDES}"
                            PRIVATE "${LIB_PRIVATE_INCLUDES}" )
# properties
set_target_properties( lhwsutil
                       PROPERTIES 
                           LIBRARY_OUTPUT_DIRECTORY "${INSTALL_LIBDIR}"
                           SOVERSION ${PROJECT_VERSION}
                           VERSION ${PROJECT_VERSION}
                           CLEAN_DIRECT_OUTPUT 1)
set_target_properties( lhwsutil_static 
                       PROPERTIES
                           ARCHIVE_OUTPUT_DIRECTORY "${INSTALL_LIBDIR}"
                           OUTPUT_NAME lhwsutil )
# .. boilerplate

##############################################################
# unit tests
##############################################################

# pull in gtest
find_package( GTest REQUIRED )

add_executable( testlhwsutil "test/testlhwsutil.cxx" )

target_link_libraries( testlhwsutil
                       PRIVATE
                           "${GTEST_BOTH_LIBRARIES}"
                           gmock
                           pthread
                           lhwsutil )

target_include_directories( testlhwsutil
                            PRIVATE
                                "${LIB_PRIVATE_INCLUDES}"
                                "${CMAKE_CURRENT_BINARY_DIR}"
                                "${GTEST_INCLUDE_DIRS}" )

include( GoogleTest )
gtest_add_tests( TARGET testlhwsutil )

##############################################################
# installation
##############################################################

# boilerplate ..
# install lib/bin
install( TARGETS lhwsutil lhwsutil_static
         EXPORT liblhwsutil-targets 
         COMPONENT liblhwsutil
         RUNTIME DESTINATION "${INSTALL_BINDIR}"
         COMPONENT liblhwsutil
         LIBRARY DESTINATION "${INSTALL_LIBDIR}"
         COMPONENT liblhwsutil
         ARCHIVE DESTINATION "${INSTALL_LIBDIR}"
         COMPONENT liblhwsutil
         INCLUDES DESTINATION "${LIB_PUBLIC_INCLUDES}" )
# install header dir
install( DIRECTORY "include/lhwsutil/lhwsutil"
         DESTINATION "${INSTALL_INCDIR}"
         COMPONENT liblhwsutil
         FILES_MATCHING PATTERN "*.h" )
# install export for things outside of build
install( EXPORT liblhwsutil-targets 
         FILE liblhwsutilTargets.cmake 
         NAMESPACE liblhwsutil:: 
         DESTINATION "${INSTALL_LIBDIR}/cmake" 
         COMPONENT liblhwsutil )
# export export for things in build
export( EXPORT liblhwsutil-targets
    FILE ${CMAKE_BINARY_DIR}/cmake/liblhwsutilTargets.cmake 
    NAMESPACE liblhwsutil:: )
ADD_CUSTOM_TARGET( install-lhwsutil
                   ${CMAKE_COMMAND}
                   -D "CMAKE_INSTALL_COMPONENT=liblhwsutil"
                   -P "cmake_install.cmake" )
# .. boilerplate

##############################################################
# pkg-config
##############################################################
set( PC_INCDIR "${INSTALL_INCDIR}")
set( PC_LIBDIR "${INSTALL_LIBDIR}")
set( PC_PUBREQS )
set( PC_PRIVREQS )
set( PC_CFLAGS "-DBOOST_LOG_DYN_LINK")
set( PC_PUBLIBS "-llhwsutil -llhmiscutil -llhsslutil" )
set( PC_PRIVLIBS )
set( PC_INSTALL_DIR "${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig" )
set( PC_INSTALL_FILENAME "${PROJECT_NAME}-${CMAKE_PROJECT_VERSION_MAJOR}.pc" )
set( PC_INSTALL_FILE "${PC_INSTALL_DIR}/${PC_INSTALL_FILENAME}" )
set( PC_RELOCATE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rpm_relocate_pc.sh" )

# boilerplate ..
# configure pc file
configure_file( "${CMAKE_SOURCE_DIR}/modules/lhscriptutil/cmake/pkgconfig/component.pc.in"
                "${CMAKE_CURRENT_BINARY_DIR}/${PC_INSTALL_FILENAME}"
                @ONLY )
# install pc file
install( FILES "${CMAKE_CURRENT_BINARY_DIR}/${PC_INSTALL_FILENAME}"
         DESTINATION "${PC_INSTALL_DIR}"
         COMPONENT liblhwsutil )
# configure pc relocation script
configure_file( "${CMAKE_SOURCE_DIR}/modules/lhscriptutil/cmake/pkgconfig/rpm_relocate_pc.sh.in"
                "${CMAKE_CURRENT_BINARY_DIR}/rpm_relocate_pc.sh"
                @ONLY )
# .. boilerplate

##############################################################
# cmake packaging
##############################################################

# boilerplate ..
include(CMakePackageConfigHelpers)

configure_package_config_file( "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
                               "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                               INSTALL_DESTINATION "${INSTALL_LIBDIR}/cmake" )

write_basic_package_version_file( "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
                                  VERSION "${PROJECT_VERSION}"
                                  COMPATIBILITY SameMajorVersion )

install( FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
         DESTINATION "${INSTALL_LIBDIR}/cmake"
         COMPONENT liblhwsutil )
# .. boilerplate

##############################################################
# rpm packaging
##############################################################
 
# boilerplate ..
set( CPACK_PROJECT_NAME ${PROJECT_NAME} )
set( CPACK_PROJECT_VERSION ${PROJECT_VERSION} )
set( CPACK_COMPONENTS_GROUPING ONE_PER_GROUP )
set( CPACK_GENERATOR "RPM" )

# set( CPACK_GENERATOR "RPM" "TGZ" )
# CPACK_RPM_BUILD_REQUIRES
set( CPACK_RPM_PACKAGE_REQUIRES "rapidjson-devel >= 1.1.0" )

set( CPACK_RPM_PACKAGE_RELOCATABLE ON)
set( CPACK_RPM_COMPONENT_INSTALL ON )

# to relocate the pc file prefix to /somewhere if 'rpm --install --prefix /somewhere'
set( CPACK_RPM_PACKAGE_REQUIRES_POST "sed" ) 
set( CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${PC_RELOCATE_SCRIPT}" )

include( CPack )

# component, group can be different; group will appended to package
cpack_add_component(liblhwsutil
    DISPLAY_NAME  "lhwsutil library"
    DESCRIPTION   "lhwsutil library"
    GROUP liblhwsutil)
cpack_add_component_group(liblhwsutil)
# .. boilerplate