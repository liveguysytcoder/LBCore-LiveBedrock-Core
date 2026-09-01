#include "Logger.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <memory>

namespace {

// Duplicates every character written to std::cout into a second buffer
// (the log file) while still passing it through to the real console. This
// way none of the existing cout<< logging scattered across the project has
// to change — it all gets captured for free once this is installed.
class TeeStreambuf : public std::streambuf {
public:
    TeeStreambuf(std::streambuf* consoleBuf, std::streambuf* fileBuf)
        : consoleBuf(consoleBuf), fileBuf(fileBuf) {}

protected:
    int overflow(int c) override {
        if (c == EOF) return !EOF;
        int consoleResult = consoleBuf->sputc(static_cast<char>(c));
        int fileResult = fileBuf->sputc(static_cast<char>(c));

        // Push completed lines to disk immediately. Without this, the
        // file's own internal buffer only flushes once it fills up or the
        // process exits cleanly — so anything printed since the last flush
        // can be silently missing from the file on disk if you copy it
        // while the server is still running (e.g. idle between clients),
        // or if the process ever exits abnormally.
        if (c == '\n') {
            fileBuf->pubsync();
        }

        return (consoleResult == EOF || fileResult == EOF) ? EOF : c;
    }

    int sync() override {
        int consoleResult = consoleBuf->pubsync();
        int fileResult = fileBuf->pubsync();
        return (consoleResult == 0 && fileResult == 0) ? 0 : -1;
    }

private:
    std::streambuf* consoleBuf;
    std::streambuf* fileBuf;
};

// These back the tee for the lifetime of the process, so they're kept at
// namespace scope rather than as locals in initLogger() that would be
// destroyed (and the streambuf left dangling under std::cout) as soon as
// the function returned.
std::ofstream logFileStream;
std::unique_ptr<TeeStreambuf> teeBuf;
std::streambuf* originalCoutBuf = nullptr;

// Puts std::cout back on its original (console) buffer. Registered with
// atexit() so it runs before logFileStream and teeBuf are destroyed —
// static-destruction order across translation units is otherwise
// unspecified, and without this, std::cout's own teardown can end up
// writing through teeBuf after logFileStream has already closed the file,
// which segfaults.
void restoreCoutBuf() {
    if (originalCoutBuf) {
        std::cout.rdbuf(originalCoutBuf);
    }
}

} // namespace

void initLogger(const std::string& logFilePath) {
    // std::ios::trunc overwrites any existing file from a previous run, so
    // this log always contains only the current run's output.
    logFileStream.open(logFilePath, std::ios::out | std::ios::trunc);
    if (!logFileStream.is_open()) {
        std::cerr << "[Logger] Failed to open log file: " << logFilePath << "\n";
        return;
    }

    originalCoutBuf = std::cout.rdbuf();
    teeBuf = std::make_unique<TeeStreambuf>(originalCoutBuf, logFileStream.rdbuf());
    std::cout.rdbuf(teeBuf.get());
    std::atexit(restoreCoutBuf);

    std::cout << "[Logger] Logging this run to " << logFilePath << "\n";
}
