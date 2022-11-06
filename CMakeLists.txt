cmake_minimum_required( VERSION 3.10 )

project( lhwsutil )
project( lhwsutil
         DESCRIPTION "lhwsutil library"
         HOMEPAGE_URL "https://github.com/fbuonaro/lhwsutil"
         LANGUAGES CXX
         VERSION 1.0.0 )

enable_testing()

add_subdirectory( liblhwsutil )
