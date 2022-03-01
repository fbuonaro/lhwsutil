cmake_minimum_required( VERSION 3.10 )

project( liblhwsutil )

# general flags for building successfully
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_LOG_DYN_LINK" )

# for debugging
set( LLHC_CXX_FLAGS_DBG "${LLHC_CXX_FLAGS_DBG} -Wall -Wextra -W" )
set( LLHC_CXX_FLAGS_DBG "${LLHC_CXX_FLAGS_DBG} -DDEBUG" )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${LLHC_CXX_FLAGS_DBG}" )
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${LLHC_CXX_FLAGS_DBG}" )

##############################################################
# library
##############################################################

set( LIB_SRC_DIR "src" )
set( LIB_INC_DIR "include" )

# pull in gtest
find_package( GTest REQUIRED )
if( NOT GTEST_FOUND )
    message( FATAL_ERROR "gtest could not be found" )
endif( NOT GTEST_FOUND )

# just does not work with multiple versions
find_package( OpenSSL 1.0 REQUIRED )
if( NOT OPENSSL_FOUND )
    message( FATAL_ERROR "openssl/crypto could not be found" )
endif( NOT OPENSSL_FOUND )

find_package( CURL REQUIRED )
if( NOT CURL_FOUND )
    message( FATAL_ERROR "curl could not be found" )
endif( NOT CURL_FOUND )

# pull in boost log
find_package( Boost 1.69 COMPONENTS log REQUIRED )
if( NOT Boost_FOUND )
    message( FATAL_ERROR "boost log could not be found" )
endif( NOT Boost_FOUND )

add_library( lhwsutil SHARED
    "${LIB_SRC_DIR}/ijwtissuercache.cxx"
    "${LIB_SRC_DIR}/ijwtvalidator.cxx"
    "${LIB_SRC_DIR}/isimplehttpclient.cxx"
    "${LIB_SRC_DIR}/jwtissuercache.cxx"
    "${LIB_SRC_DIR}/jwtutils.cxx"
    "${LIB_SRC_DIR}/jwtvalidator.cxx"
    "${LIB_SRC_DIR}/logging.cxx"
    "${LIB_SRC_DIR}/rsa.cxx"
    "${LIB_SRC_DIR}/simplehttpclientcurl.cxx" )

target_link_libraries( lhwsutil
    PUBLIC
        "${Boost_LIBRARIES}"
        "${OPENSSL_CRYPTO_LIBRARY}"
        "${CURL_LIBRARIES}"
        lhmiscutil
        jwt )

target_include_directories( lhwsutil
    PUBLIC
        "${LIB_INC_DIR}/lhwsutil"
        "${Boost_INCLUDE_DIR}"
    PRIVATE
        "{OPENSSL_INCLUDE_DIR}"
        "{CURL_INCLUDE_DIRS}"
        "${CMAKE_CURRENT_BINARY_DIR}" 
        "${LIB_INC_DIR}/lhwsutil_impl" )

##############################################################
# unit tests
##############################################################

set( TST_SRC_DIR "./test" )

enable_testing()

add_executable( testlhwsutil
    "${TST_SRC_DIR}/testlhwsutil.cxx" )

target_link_libraries( testlhwsutil
    PRIVATE
        "${GTEST_BOTH_LIBRARIES}"
        gmock
        pthread
        lhwsutil )

target_include_directories( testlhwsutil
    PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}"
        "${Boost_INCLUDE_DIR}"
        "${GTEST_INCLUDE_DIRS}"
        "${LIB_INC_DIR}/lhwsutil_impl" )

include( GoogleTest )
gtest_add_tests( TARGET testlhwsutil )

##############################################################
# installation
##############################################################

# library
install( TARGETS lhwsutil
         LIBRARY
            DESTINATION lib
            COMPONENT lhwsutil )

# include directory
install( DIRECTORY "${LIB_INC_DIR}/lhwsutil/lhwsutil"
         DESTINATION include
         COMPONENT lhwsutil
         FILES_MATCHING PATTERN "*.h" )

ADD_CUSTOM_TARGET( install-lhwsutil
                   ${CMAKE_COMMAND}
                   -D "CMAKE_INSTALL_COMPONENT=lhwsutil"
                   -P "cmake_install.cmake" )
