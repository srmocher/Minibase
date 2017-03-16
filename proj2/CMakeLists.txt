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
        BufMgr/src/test_driver.C)
include_directories(${PROJECT_SOURCE_DIR}/BufMgr/include ${PROJECT_SOURCE_DIR}/BufMgr/include ${PROJECT_SOURCE_DIR}/BufMgr/lib ${PROJECT_SOURCE_DIR}/HFPage/lib )
link_directories(${PROJECT_SOURCE_DIR}/BufMgr/lib ${PROJECT_SOURCE_DIR}/BufMgr/lib)
add_executable(proj2 ${SOURCE_FILES})

target_link_libraries(proj2 ${PROJECT_SOURCE_DIR}/BufMgr/lib/libdb.a)