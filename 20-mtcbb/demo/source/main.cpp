#include "mtlog.hpp"
#include <iostream>

int main() 
{
    std::cout << "Initializing logger..." << std::endl;
    
    if (!MTLOG::mtlog::instance().Init()) {
        std::cerr << "Failed to initialize logger!" << std::endl;
        return 1;
    }
    
    std::cout << "Logger initialized successfully!" << std::endl;
    
    // 测试各个级别
    LOG_NORMAL << "Normal message";
    LOG_NOTIFY << "Notification message";
    LOG_WARNING << "Warning message";
    LOG_ERROR << "Error message";
    LOG_CRITICAL << "Critical message";
    
    std::cout << "Check mtlog.log file for output" << std::endl;
    
    return 0;
}
