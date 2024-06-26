project(Spatify LANGUAGES CXX)
add_library(Spatify INTERFACE)
# if can find tbb, link tbb to Spatify
option(SPATIFY_ENABLE_GUI "Enable GUI support" OFF)
find_package(TBB)
if(TBB_FOUND)
    target_link_libraries(Spatify INTERFACE TBB::tbb)
    target_compile_definitions(Spatify INTERFACE USE_TBB)
endif()
target_include_directories(Spatify INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_include_directories(Spatify INTERFACE ${EXTERNAL_PATH}/glm)

# Add test
add_executable(spatial-hash tests/spatial-hash.cc)
target_link_libraries(spatial-hash Spatify)