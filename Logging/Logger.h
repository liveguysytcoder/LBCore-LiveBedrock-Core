#pragma once
#include <string>

// Installs a "tee" on std::cout so every line printed from here on lands in
// both the console AND the given log file. The file is opened in truncate
// mode, which discards whatever the previous run wrote — so exactly one
// run's worth of output is ever kept on disk, and it's gone the moment the
// next run starts logging.
//
// Call this once, as the very first thing in main(), before anything else
// prints.
void initLogger(const std::string& logFilePath = "lbcore.log.txt");
