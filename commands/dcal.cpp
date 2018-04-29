// 18/04/23 = Mon

// A date calculator to find the date before or after today.

// How to run?
// To find the date that's 100 days before today: dcal -100
// To find the date that's 200 days after today: dcal 200
// To find multiple dates before or after today: dcal

// For <ctime>, know:
// 1. types:
//  - time_t      : time since epoch
//  - tm          : time structure
// 2. functions:
//  - time()      : time_t * => time_t (save in time_t *)
//  - ctime()     : time_t * => char * ~ asctime(localtime(time_t *))
//  - asctime()   : tm * => char *
//  - localtime() : time_t * => tm *
//  - gmtime()    : time_t * => tm * (Greenwich Mean Time)
//  - mktime()    : tm * (ignore tm_wday, tm_yday, allows out of range) => time_t
//  - strftime()  : (char *, size_t, const char *, const tm *) => size_t
//  - difftime()  : (time_t, time_t) => double

#include <cmath>      // std::abs
#include <cstdlib>    // std::atoi
#include <ctime>
#include <iostream>   // std::cin, std::cout, std::endl
#include <sstream>    // std::ostringstream
#include <string>     // std::string

#define MAX 100
#define OFFSET 9


void calc(int n)
{
  std::ostringstream oss;
  oss << std::abs(n) << " day" << (std::abs(n) > 1 ? "s" : "")
      << (n < 0 ? " before" : " after") << " today is ";
  auto len = oss.str().size();

  std::time_t t = std::time(NULL);
  std::tm lt = *std::localtime(&t);

  std::cout << "Today" << std::string(len - OFFSET, ' ') << " is ";
  std::string unknown = "...... Umm ... I don't know.";
  char text[MAX];
  if (std::strftime(text, sizeof(text), "%a %d %b %Y (%Y-%m-%d)", &lt))
    std::cout << text << std::endl;
  else
    std::cout << unknown << std::endl;

  lt.tm_mday += n;
  std::mktime(&lt);

  std::cout << oss.str();
  if (std::strftime(text, sizeof(text), "%a %d %b %Y (%Y-%m-%d)", &lt))
    std::cout << text << std::endl;
  else
    std::cout << unknown << std::endl;
}

int main(int argc, char * argv[])
{
  if (argc == 1) {
    std::string enter = "Enter number of days before or after today"
              " (q to quit): ";
    std::cout << enter;
    for (int n = 0;
        std::cin >> n;
        std::cin.ignore(MAX, '\n'),
        std::cout << '\n' << enter)
      calc(n);
  }
  else
    calc(std::atoi(argv[1]));
  return 0;
}