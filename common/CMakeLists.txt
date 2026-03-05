find_package(PkgConfig REQUIRED)
find_package(PahoMqttCpp REQUIRED)
pkg_check_modules(SERIAL REQUIRED libserial)

add_library(common STATIC
src/logger.cpp
src/ConfigManager.cpp
src/message_parser.cpp
src/serial_port.cpp
src/mqtt_client.cpp
# Note: thread_manager.cpp removed - template implementations are now in the header
)

target_include_directories(common
PUBLIC
include
${SERIAL_INCLUDE_DIRS}
/usr/include  # fallback: ensures <libserial/SerialPort.hpp> is found
)

target_link_libraries(common PUBLIC JsonCpp::JsonCpp ${SERIAL_LIBRARIES} PahoMqttCpp::paho-mqttpp3)
target_link_directories(common PUBLIC ${SERIAL_LIBRARY_DIRS})