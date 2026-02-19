#include "message_parser.hpp"
#include <iostream>

MessageParser::MessageParser() : buffer("") {
}

void MessageParser::addData(const std::vector<uint8_t>& data) {
    buffer.append(data.begin(), data.end());
}

bool MessageParser::parseNextMessage(Json::Value& message) {
    size_t messageEnd = findCompleteMessage();
    
    if (messageEnd == std::string::npos) {
        return false;  // No complete message yet
    }
    
    // Extract the message
    std::string jsonStr = buffer.substr(0, messageEnd + 1);
    
    // Parse it
    if (!tryParseJSON(jsonStr, message)) {
        // If parsing fails, remove the malformed message and try next
        buffer.erase(0, messageEnd + 1);
        return false;
    }
    
    // Remove the processed message from buffer
    buffer.erase(0, messageEnd + 1);
    return true;
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
    
    for (size_t i = 0; i < buffer.length(); ++i) {
        char c = buffer[i];
        
        if (escapeNext) {
            escapeNext = false;
            continue;
        }
        
        if (c == '\\' && inString) {
            escapeNext = true;
            continue;
        }
        
        if (c == '"' && !escapeNext) {
            inString = !inString;
            continue;
        }
        
        if (!inString) {
            if (c == '{') {
                braceCount++;
            } else if (c == '}') {
                braceCount--;
                if (braceCount == 0 && i > 0) {
                    return i;  // Found a complete JSON object
                }
            }
        }
    }
    
    return std::string::npos;
}

bool MessageParser::tryParseJSON(const std::string& jsonStr, Json::Value& root) {
    try {
        Json::CharReaderBuilder reader;
        std::string errs;
        
        std::istringstream stream(jsonStr);
        if (!Json::parseFromStream(reader, stream, &root, &errs)) {
            std::cerr << "JSON parse error: " << errs << std::endl;
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception during JSON parsing: " << e.what() << std::endl;
        return false;
    }
}
