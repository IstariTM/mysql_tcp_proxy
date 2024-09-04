cmake_minimum_required(VERSION 3.10)

# Название проекта
project(TCPProxy)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Добавляем все исходные файлы проекта
set(SOURCE_FILES
    main.cpp
    tcp_proxy.cpp
)

# Ищем библиотеку Boost через vcpkg
find_package(Boost REQUIRED COMPONENTS system thread)

# Проверка, что Boost был найден
if(Boost_FOUND)
    add_executable(tcp_proxy_sql ${SOURCE_FILES})
    target_link_libraries(tcp_proxy_sql Boost::system Boost::thread)  # Использование системных и thread библиотек Boost
else()
    message(FATAL_ERROR "Boost library not found.")
endif()
