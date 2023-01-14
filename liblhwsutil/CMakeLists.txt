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
set( LH_INSTALL_BINDIR "${CMAKE_INSTALL_BINDIR}" )
set( LH_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}/liblhwsutil_${PROJECT_VERSION}" )
set( LH_INSTALL_INCDIR "${CMAKE_INSTALL_INCLUDEDIR}/lhwsutil_${PROJECT_VERSION}" )

set( LH_LIB_NAME lhwsutil )

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

# source files
set( LH_LIB_SRC_FILES 
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
set( LH_LIB_PUBLIC_LINKLIBS 
     "${Boost_LIBRARIES}"
     "${OPENSSL_CRYPTO_LIBRARY}"
     "${CURL_LIBRARIES}"
     liblhmiscutil::lhmiscutil
     liblhsslutil::lhsslutil
     jwt )
set( LH_LIB_PRIVATE_LINKLIBS )
# header dependencies
set( LH_LIB_PUBLIC_INCLUDES 
     "${Boost_INCLUDE_DIR}"
     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/lhwsutil>   # build
     $<INSTALL_INTERFACE:${LH_INSTALL_INCDIR}> )                          # export
set( LH_LIB_PRIVATE_INCLUDES 
     "${OPENSSL_INCLUDE_DIR}"
     "${CURL_INCLUDE_DIRS}"
     "${CMAKE_CURRENT_BINARY_DIR}" 
     "${CMAKE_CURRENT_SOURCE_DIR}/include/lhwsutil_impl" )

lh_add_library()

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
                                "${LH_LIB_PRIVATE_INCLUDES}"
                                "${CMAKE_CURRENT_BINARY_DIR}"
                                "${GTEST_INCLUDE_DIRS}" )

include( GoogleTest )
gtest_add_tests( TARGET testlhwsutil )

##############################################################
# installation
##############################################################

# name of component to install
set(LH_COMPONENT_NAME liblhwsutil )
# directories to install
set(LH_LIB_PUBLIC_INCLUDE_DIRS "include/lhwsutil/lhwsutil")

lh_add_install_library()

##############################################################
# pkg-config
##############################################################
set( PC_INCDIR "${LH_INSTALL_INCDIR}")
set( PC_LIBDIR "${LH_INSTALL_LIBDIR}")
set( PC_PUBREQS )
set( PC_PRIVREQS )
set( PC_CFLAGS "-DBOOST_LOG_DYN_LINK")
set( PC_PUBLIBS "-llhwsutil -llhmiscutil -llhsslutil" )
set( PC_PRIVLIBS )
set( PC_INSTALL_DIR "${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig" )
set( PC_INSTALL_FILENAME "${PROJECT_NAME}-${CMAKE_PROJECT_VERSION_MAJOR}.pc" )
set( PC_INSTALL_FILE "${PC_INSTALL_DIR}/${PC_INSTALL_FILENAME}" )
set( PC_RELOCATE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rpm_relocate_pc.sh" )

lh_add_pkgconfig()

##############################################################
# cmake packaging
##############################################################

lh_add_install_cmake_config()

##############################################################
# rpm packaging
##############################################################
 
# TODO - version
set( CPACK_RPM_LIBLHWSUTIL_PACKAGE_REQUIRES "boost169-devel" PARENT_SCOPE)

# to relocate the pc file prefix to /somewhere if 'rpm --install --prefix /somewhere'
set( CPACK_RPM_LIBLHWSUTIL_PACKAGE_REQUIRES_POST "sed" PARENT_SCOPE) 
set( CPACK_RPM_LIBLHWSUTIL_POST_INSTALL_SCRIPT_FILE "${PC_RELOCATE_SCRIPT}" PARENT_SCOPE)

set(LH_PACKAGE_DESC "liblhwsutil libraries package")

lh_add_cpack_component()
