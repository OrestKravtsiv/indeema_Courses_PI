find_package(PkgConfig REQUIRED)
pkg_check_modules(SERIAL REQUIRED libserial)

add_library(common STATIC
src/logger.cpp
src/ConfigManager.cpp
src/message_parser.cpp
src/serial_port.cpp
)

target_include_directories(common
PUBLIC
include
${SERIAL_INCLUDE_DIRS}
/usr/include  # fallback: ensures <libserial/SerialPort.hpp> is found
)

target_link_libraries(common PUBLIC JsonCpp::JsonCpp ${SERIAL_LIBRARIES})
target_link_directories(common PUBLIC ${SERIAL_LIBRARY_DIRS})