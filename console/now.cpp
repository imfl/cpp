// 18/04/23 = Mon

// Get time now.

#include <ctime>
#include <iostream>
#include <stdexcept>

#define MAX 100

int main()
{
    std::time_t t = time(NULL);
    std::tm now = *std::localtime(&t);
    char text[100];
    if (std::strftime(text, sizeof text, "%Y-%m-%d %a %H:%M:%S", &now))
        std::cout << text << std::endl;
    else
        throw std::runtime_error("Error in calling std::strftime!");
    return 0;
}