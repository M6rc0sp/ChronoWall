cmake_minimum_required(VERSION 3.16)
project(chronowall VERSION 1.0 LANGUAGES CXX)

# Configurações básicas
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Detectar sistema
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    # Detectar distribuição
    if(EXISTS "/etc/debian_version")
        set(LINUX_DISTRO "debian")
        set(SYSTEMD_USER_DIR "lib/systemd/user")
    elseif(EXISTS "/etc/fedora-release")
        set(LINUX_DISTRO "fedora")
        set(SYSTEMD_USER_DIR "usr/lib/systemd/user")
    elseif(EXISTS "/etc/arch-release")
        set(LINUX_DISTRO "arch")
        set(SYSTEMD_USER_DIR "usr/lib/systemd/user")
    else()
        set(LINUX_DISTRO "generic")
        set(SYSTEMD_USER_DIR "usr/lib/systemd/user")
    endif()
    message(STATUS "Detected distribution: ${LINUX_DISTRO}")
endif()

# Tentar Qt6 primeiro, depois Qt5
find_package(Qt6 COMPONENTS Core Widgets DBus QUIET)
if (Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
else()
    find_package(Qt5 COMPONENTS Core Widgets DBus REQUIRED)
    set(QT_VERSION_MAJOR 5)
endif()

message(STATUS "Using Qt ${QT_VERSION_MAJOR} on ${LINUX_DISTRO}")

# Fontes
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
)

# Executável
add_executable(chronowall ${SOURCES})

target_include_directories(chronowall PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(chronowall PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
)

# Instalação (corrigida)
install(TARGETS chronowall 
        RUNTIME DESTINATION bin)
install(FILES resources/chronowall.desktop 
        DESTINATION share/applications)
install(FILES resources/chronowall.service 
        DESTINATION lib/systemd/user)
install(FILES resources/icons/chronowall.svg 
        DESTINATION share/icons/hicolor/scalable/apps)

# Configurações
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)
