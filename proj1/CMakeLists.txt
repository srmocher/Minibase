cmake_minimum_required(VERSION 3.6)
project(proj1)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES

        HFPage/include/buf.h
        HFPage/include/da_types.h
        HFPage/include/db.h
        HFPage/include/hfp_driver.h
        HFPage/include/hfpage.h
        HFPage/include/minirel.h
        HFPage/include/new_error.h
        HFPage/include/page.h
        HFPage/include/scan.h
        HFPage/include/system_defs.h
        HFPage/include/test_driver.h
        HFPage/src/hfp_driver.C
        HFPage/src/hfpage.C
        HFPage/src/main.C
        HFPage/src/new_error.C
        HFPage/src/page.C
        HFPage/src/system_defs.C
        HFPage/src/test_driver.C)
include_directories(${PROJECT_SOURCE_DIR}/HFPage/include ${PROJECT_SOURCE_DIR}/HFPage/lib )
link_directories(${PROJECT_SOURCE_DIR}/HFPage/lib)
add_executable(proj1 ${SOURCE_FILES})

target_link_libraries(proj1 ${PROJECT_SOURCE_DIR}/HFPage/lib/libbm.a ${PROJECT_SOURCE_DIR}/HFPage/lib/libdb.a)
