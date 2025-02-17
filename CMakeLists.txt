cmake_minimum_required(VERSION 3.16)
project(chronowall VERSION 1.0 LANGUAGES CXX)

# Configurações básicas e simples
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Encontrar Qt6 (simplificado)
find_package(Qt6 COMPONENTS 
    Core
    Widgets 
    DBus 
    REQUIRED
)

# Fontes
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
)

# Executável
add_executable(chronowall ${SOURCES})

target_include_directories(chronowall PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(chronowall PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::DBus
)

# Instalação básica
install(TARGETS chronowall RUNTIME DESTINATION bin)
install(FILES resources/chronowall.desktop DESTINATION share/applications)
install(FILES resources/chronowall.service DESTINATION lib/systemd/user)
install(FILES resources/icons/chronowall.svg 
        DESTINATION share/icons/hicolor/scalable/apps)
