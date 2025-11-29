#include <pthread.h>

#include <string>

#include "cli.hpp"

void setThreadName(std::string name);
bool create_directory(std::string path);
bool is_directory(std::string path);
bool exists(std::string path);
bool remove(std::string path);

bool setWorkDir(std::string& workdir, Source source);
