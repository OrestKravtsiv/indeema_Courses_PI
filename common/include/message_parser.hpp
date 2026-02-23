#include <string>
#include <vector>
#include <json/json.h>

class MessageParser {
public:
    MessageParser();
    
    // Add raw bytes to the buffer
    void addData(const std::vector<uint8_t>& data);
    
    // Try to extract and parse the next complete JSON message
    bool parseNextMessage(Json::Value& message);
    
    // Get the internal buffer content as string
    std::string getBufferContent() const;
    
    // Clear the buffer
    void clearBuffer();

private:
    std::string buffer;
    
    // Find a complete JSON object in the buffer
    size_t findCompleteMessage();
    
    // Try to parse a JSON string
    bool tryParseJSON(const std::string& jsonStr, Json::Value& root);
};