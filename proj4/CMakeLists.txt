cmake_minimum_required(VERSION 3.6)
project(proj4)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        SortMerge/src/buf.C
        SortMerge/src/heapfile.C
        SortMerge/src/hfpage.C
        SortMerge/src/main.C
        SortMerge/src/page.C
        SortMerge/src/new_error.C
        SortMerge/src/page.C
        SortMerge/src/scan.C
        SortMerge/src/SMJTester.C
        SortMerge/src/sortMerge.C
        SortMerge/src/system_defs.C
        SortMerge/src/test_driver.C
        SortMerge/include/buf.h
        SortMerge/include/da_types.h
        SortMerge/include/db.h
        SortMerge/include/heapfile.h
        SortMerge/include/hfpage.h
        SortMerge/include/minirel.h
        SortMerge/include/new_error.h
        SortMerge/include/page.h
        SortMerge/include/scan.h
        SortMerge/include/sort.h
        SortMerge/include/SMJTester.h
        SortMerge/include/sortMerge.h
        SortMerge/include/test_driver.h)

include_directories(SortMerge/include)
link_directories(SortMerge/lib)
add_executable(proj4 ${SOURCE_FILES} SortMerge/lib)

target_link_libraries(proj4 ${PROJECT_SOURCE_DIR}/SortMerge/lib/libdb.a)