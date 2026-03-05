/**
 * Example ESP32 Code for LED Control
 * This is a reference implementation showing the expected JSON schema
 * 
 * NOTE: This file is for reference only - it should be implemented on the ESP32
 */

// Expected JSON Command Format (received from Raspberry Pi):
// {
//   "command": "SET_COLOR",
//   "rgb": [255, 128, 0]
// }

// Expected JSON Status Format (sent to Raspberry Pi):
// {
//   "device_id": "ESP32_LED_001",
//   "state": "ON",
//   "uptime": 12345,
//   "rgb": [255, 128, 0],
//   "timestamp": 1234567890
// }

/*
 * MQTT Topics:
 * - Subscribe to: studio/led/cmd (receive commands)
 * - Publish to: studio/led/status (send status updates)
 * 
 * UART Settings:
 * - Baud Rate: 115200
 * - Data Bits: 8
 * - Parity: None
 * - Stop Bits: 1
 * - Flow Control: None
 * 
 * JSON Message Format:
 * - Messages are terminated with newline (\n) for UART
 * - RGB values are in range [0, 255]
 * - All JSON must be valid and properly formatted
 * 
 * Example Command Processing:
 * 
 * void handleSetColorCommand(JsonObject& cmd) {
 *     if (cmd.containsKey("rgb") && cmd["rgb"].is<JsonArray>()) {
 *         JsonArray rgb = cmd["rgb"];
 *         if (rgb.size() == 3) {
 *             int r = rgb[0];
 *             int g = rgb[1];
 *             int b = rgb[2];
 *             
 *             // Validate range
 *             if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
 *                 // Set LED color
 *                 setLEDColor(r, g, b);
 *                 
 *                 // Send status update
 *                 sendStatusUpdate(r, g, b);
 *             }
 *         }
 *     }
 * }
 * 
 * void sendStatusUpdate(int r, int g, int b) {
 *     JsonDocument doc;
 *     doc["device_id"] = "ESP32_LED_001";
 *     doc["state"] = "ON";
 *     doc["uptime"] = millis() / 1000;
 *     
 *     JsonArray rgb = doc.createNestedArray("rgb");
 *     rgb.add(r);
 *     rgb.add(g);
 *     rgb.add(b);
 *     
 *     doc["timestamp"] = getTimestamp();
 *     
 *     // For MQTT
 *     mqttClient.publish("studio/led/status", doc.as<String>());
 *     
 *     // For UART
 *     serializeJson(doc, Serial);
 *     Serial.println(); // Add newline delimiter
 * }
 */
