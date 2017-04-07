cmake_minimum_required(VERSION 3.6)
project(proj3)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        btree/include/bt.h
        btree/include/btfile.h
        btree/include/btindex_page.h
        btree/include/btleaf_page.h
        btree/include/btree_driver.h
        btree/include/btreefilescan.h
        btree/include/buf.h
        btree/include/da_types.h
        btree/include/db.h
        btree/include/hfpage.h
        btree/include/index.h
        btree/include/minirel.h
        btree/include/new_error.h
        btree/include/page.h
        btree/include/sorted_page.h
        btree/include/system_defs.h
        btree/include/test_driver.h
        btree/src/btfile.C
        btree/src/btindex_page.C
        btree/src/btleaf_page.C
        btree/src/btree_driver.C
        btree/src/btreefilescan.C
        btree/src/buf.C
        btree/src/hfpage.C
        btree/src/key.C
        btree/src/main.C
        btree/src/new_error.C
        btree/src/page.C
        btree/src/sorted_page.C
        btree/src/system_defs.C
        btree/src/test_driver.C)

include_directories(btree/include)
link_directories(btree/lib)
add_executable(proj3 ${SOURCE_FILES} btree/lib)

target_link_libraries(proj3 ${PROJECT_SOURCE_DIR}/btree/lib/libdb.a)