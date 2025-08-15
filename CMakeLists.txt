cmake_minimum_required(VERSION 3.12)
project(ChatServer VERSION 1.0 LANGUAGES CXX)

# Настройки компилятора
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Включение всех предупреждений
add_compile_options(-Wall -Wextra -Wpedantic)

# Исходные файлы
set(SOURCES
    main.cpp
    src/handler/Messages/broadcast_handler.cpp
    src/handler/Messages/chained_handler.cpp
    src/net/connection/chat_server.cpp
    src/net/connection/connectionManager.cpp
    src/net/connection/socket.cpp
)

# Заголовочные файлы
set(HEADERS
    include/handler/Messages/chain/chained_handler.h
    include/handler/Messages/implementations/broadcast_handler.h
    include/handler/Messages/interface/imessage_handler.h
    include/net/connection/chat_server.h
    include/net/connection/connectionManager.h
    include/net/connection/IConnectionManager.h
    include/net/socket.h
    include/net/socketConfig.h
)

# Основной исполняемый файл
add_executable(chat_server ${SOURCES} ${HEADERS})

# Пути include
target_include_directories(chat_server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Библиотеки
find_package(Threads REQUIRED)
target_link_libraries(chat_server
    PRIVATE
        Threads::Threads
)

# Установка (опционально)
install(TARGETS chat_server
    RUNTIME DESTINATION bin
)

# Документация Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs)
    set(DOXYGEN_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
    
    doxygen_add_docs(docs
        ${DOXYGEN_INPUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating documentation"
    )
endif()