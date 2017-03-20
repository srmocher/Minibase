cmake_minimum_required(VERSION 3.6)
project(proj2)

set(CMAKE_CXX_STANDARD 11)
AUX_SOURCE_DIRECTORY(BufMgr/include INCLUDE)
AUX_SOURCE_DIRECTORY(BufMgr/src SOURCE)

set(SOURCE_FILES
        BufMgr/include/BMTester.h
        BufMgr/include/buf.h
        BufMgr/include/da_types.h
        BufMgr/include/da_types.h
        BufMgr/include/db.h
        BufMgr/include/minirel.h
        BufMgr/include/system_defs.h
        BufMgr/include/test_driver.h
        BufMgr/include/page.h
        BufMgr/include/new_error.h
       BufMgr/src/BMTester.C
        BufMgr/src/system_defs.C
        BufMgr/src/buf.C
        BufMgr/src/main.C
        BufMgr/src/new_error.C
        BufMgr/src/page.C
        BufMgr/src/test_driver.C
        HeapFile/include/buf.h
        HeapFile/include/db.h
#
#        HeapFile/include/heap_driver.h
#        HeapFile/include/heapfile.h
#        HeapFile/include/hfp_driver.h
#        HeapFile/include/da_types.h
#        HeapFile/include/page.h
#
#        HeapFile/include/minirel.h
#        HeapFile/include/new_error.h
#        HeapFile/include/hfpage.h
#        HeapFile/include/scan.h
#        HeapFile/include/system_defs.h
#        HeapFile/include/test_driver.h
#
#        HeapFile/src/heapfile.C
#        HeapFile/src/buf.C
#        HeapFile/src/hfpage.C
#        HeapFile/src/main.C
#        HeapFile/src/new_error.C
#        HeapFile/src/page.C
#        HeapFile/src/scan.C
#        HeapFile/src/system_defs.C
#        HeapFile/src/heap_driver.C
#        HeapFile/src/test_driver.C
        )

include_directories(${PROJECT_SOURCE_DIR}/BufMgr/include)
link_directories(${PROJECT_SOURCE_DIR}/BufMgr/lib)
add_executable(proj2 ${SOURCE_FILES} ${PROJECT_SOURCE_DIR}/BufMgr/lib)

target_link_libraries(proj2 ${PROJECT_SOURCE_DIR}/BufMgr/lib/libdb.a)