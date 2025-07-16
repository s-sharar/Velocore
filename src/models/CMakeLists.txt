set(MODELS_SOURCES
    impl/Types.cpp
    impl/Order.cpp
    impl/Trade.cpp
)

set(MODELS_HEADERS
    include/Types.h
    include/Order.h
    include/Trade.h
)

add_library(models STATIC ${MODELS_SOURCES} ${MODELS_HEADERS})

target_include_directories(models PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(Crow_FOUND)
    target_link_libraries(models PUBLIC Crow::Crow)
else()
    target_link_libraries(models PUBLIC Crow::crow)
endif()

target_compile_features(models PUBLIC cxx_std_17)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(models PRIVATE 
        -Wall -Wextra -Wpedantic
    )
endif()

add_library(Velocore::models ALIAS models) 