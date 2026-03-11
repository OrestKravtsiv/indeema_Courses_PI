find_package(PahoMqttCpp REQUIRED)

add_library(common STATIC
src/logger.cpp
src/ConfigManager.cpp
src/message_parser.cpp
src/mqtt_client.cpp
# Note: thread_manager.cpp removed - template implementations are now in the header
)

target_include_directories(common
PUBLIC
include
)

target_link_libraries(common PUBLIC JsonCpp::JsonCpp PahoMqttCpp::paho-mqttpp3)