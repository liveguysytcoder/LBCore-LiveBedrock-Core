#ifndef LB_SERVER_MANAGER_H
#define LB_SERVER_MANAGER_H
#include <string>

// Declarations of functions that exist in other files
void start();
void stopServer();

// Declaration of the function in this file
int LBStart();
std::string getServerProperties();

#endif // SERVER_MANAGER_H
