#include <iostream>
#include "mtlog.hpp"

int main(int argc, char *argv[])
{
    MTLOG::mtlog logger;
    logger.Init();
    std::cout
        << "helo world" << std::endl;
}