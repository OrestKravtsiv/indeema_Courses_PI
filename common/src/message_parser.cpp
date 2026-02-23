#include "message_parser.hpp"

MessageParser::MessageParser() : buffer("") {}

void MessageParser::addData(const std::vector<uint8_t>& data) {
    // Convert raw bytes to string and append to buffer
    for (uint8_t byte : data) {
        buffer += static_cast<char>(byte);
    }
}

bool MessageParser::parseNextMessage(Json::Value& message) {
    // Find a complete JSON message in the buffer
    size_t messageEnd = findCompleteMessage();
    
    if (messageEnd == std::string::npos) {
        return false; // No complete message found
    }
    
    // Extract the JSON string
    std::string jsonStr = buffer.substr(0, messageEnd);
    
    // Try to parse it
    if (tryParseJSON(jsonStr, message)) {
        // Remove the parsed message from buffer
        buffer.erase(0, messageEnd);
        return true;
    }
    
    // If parsing failed, remove the bad data
    buffer.erase(0, messageEnd);
    return false;
}

std::string MessageParser::getBufferContent() const {
    return buffer;
}

void MessageParser::clearBuffer() {
    buffer.clear();
}

size_t MessageParser::findCompleteMessage() {
    int braceCount = 0;
    bool inString = false;
    bool escapeNext = false;
    
    for (size_t i = 0; i < buffer.size(); ++i) {
        char c = buffer[i];
        
        // Handle escape sequences in strings
        if (escapeNext) {
            escapeNext = false;
            continue;
        }
        
        if (c == '\\' && inString) {
            escapeNext = true;
            continue;
        }
        
        // Track if we're inside a string
        if (c == '"') {
            inString = !inString;
            continue;
        }
        
        // Only count braces outside of strings
        if (!inString) {
            if (c == '{') {
                braceCount++;
            } else if (c == '}') {
                braceCount--;
                
                // Found a complete JSON object
                if (braceCount == 0) {
                    return i + 1;
                }
            }
        }
    }
    
    return std::string::npos; // No complete message
}

bool MessageParser::tryParseJSON(const std::string& jsonStr, Json::Value& root) {
    Json::CharReaderBuilder builder;
    std::string errs;
    
    std::istringstream stream(jsonStr);
    return Json::parseFromStream(builder, stream, &root, &errs);
}
