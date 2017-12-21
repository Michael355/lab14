#include <string>
#include <iostream>
#include <chrono>
#include <boost/program_options.hpp>
#include <boost/process.hpp>

#define TIMEOUT_ERROR_CODE 1

namespace po = boost::program_options;
namespace bp = boost::process;
namespace ch = std::chrono;

struct ProcessParams {
    std::string command;
    int exitCode;
    bool hasTimeout;
    size_t timeout;

    ProcessParams()
        : command{}
        , exitCode{ 0 }
        , hasTimeout{ false }
        , timeout{ 0U }
    {}
};

int StartProcess(std::string const& command)
{
    // Init
    bp::ipstream out;
    bp::child c(command, bp::std_out > out);

    // Print output text
    std::string line;
    while (out && std::getline(out, line) && !line.empty()) {
        std::cout << line << std::endl;
    }

    // Return code
    c.wait();
    return c.exit_code();
}

int StartProcess(std::string const& command, size_t& timeout)
{
    // Init
    bp::ipstream out;
    bp::child c(command, bp::std_out > out);

    // Do command
    ch::system_clock::time_point beginTime{ ch::system_clock::now() };
    if (!c.wait_for(ch::seconds(timeout))) {
        c.terminate();
        timeout = 0U;
    }
    else {
        ch::system_clock::time_point endTime{ ch::system_clock::now() };
        float delta{ ch::duration<float, std::ratio<1>>(endTime - beginTime).count() };
        timeout -= std::min(static_cast<size_t>(round(delta)), timeout);
    }

    // Print output text
    std::string line;
    while (out && std::getline(out, line) && !line.empty()) {
        std::cout << line << std::endl;
    }

    // End time
    if (timeout == 0U) {
        std::cout << "Timeout!" << std::endl;
        return TIMEOUT_ERROR_CODE;
    }
    else {
        return c.exit_code();
    }
}

void StartProcess(ProcessParams& pp)
{
    if (pp.hasTimeout) {
        pp.exitCode = StartProcess(pp.command, pp.timeout);
    }
    else {
        pp.exitCode = StartProcess(pp.command);
    }
}

int main(int argc, char const* const* argv)
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config", po::value<std::string>()->default_value("Debug"), "set build configuration")
        ("install", "add installation stage (in directory '_install')")
        ("pack", "add packaging stage (in tar.gz format archive)")
        ("timeout", po::value<size_t>(), "set waiting time (in seconds)")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }
    else {
        ProcessParams pp;
        pp.hasTimeout = vm.count("timeout");
        if (pp.hasTimeout) {
            pp.timeout = vm["timeout"].as<size_t>();
        }
        pp.command = "cmake -H. -B_builds -DCMAKE_INSTALL_PREFIX=_install -DCMAKE_BUILD_TYPE=";
        pp.command += vm["config"].as<std::string>();
        StartProcess(pp);
        if (pp.exitCode == 0) {
            pp.command = "cmake --build _builds";
            StartProcess(pp);
        }
        if (pp.exitCode == 0 && vm.count("install")) {
            pp.command = "cmake --build _builds --target install";
            StartProcess(pp);
        }
        if (pp.exitCode == 0 && vm.count("pack")) {
            pp.command = "cmake --build _builds --target package";
            StartProcess(pp);
        }
        return pp.exitCode;
    }
}
