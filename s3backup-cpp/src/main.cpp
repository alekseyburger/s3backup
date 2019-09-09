#include <iostream>
#include <string>
#include <cstdio>
#include <iostream>
#include <string>

#include "boost/log/trivial.hpp"
#include "boost/log/utility/setup.hpp"
#include <boost/regex.hpp>

#include <dirent.h>
#include <signal.h>

#include "s3_application.hpp"

#define _DEBUG true

static void init_log(const char* log_file_name=nullptr)
{
    static const std::string COMMON_FMT("[%TimeStamp%][%Severity%]:  %Message%");

    boost::log::register_simple_formatter_factory< boost::log::trivial::severity_level, char >("Severity");

    // Output message to console
    boost::log::add_console_log(
        std::cout,
        boost::log::keywords::format = COMMON_FMT,
        boost::log::keywords::auto_flush = true
    );

    // Output message to file, rotates when file reached 1mb or at midnight every day. Each log file
    // is capped at 1mb and total is 20mb
    if (log_file_name) {
        boost::log::add_file_log (
            boost::log::keywords::file_name = log_file_name,
            boost::log::keywords::rotation_size = 1 * 1024 * 1024,
            boost::log::keywords::max_size = 20 * 1024 * 1024,
            boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
            boost::log::keywords::format = COMMON_FMT,
            boost::log::keywords::auto_flush = true
        );
    }

    boost::log::add_common_attributes();

    // Only output message with INFO or higher severity in Release
#ifndef _DEBUG
    boost::log::core::get()->set_filter(
        boost::log::trivial::severity >= boost::log::trivial::info
    );
#endif
}

void sig_term_handler(int signum, siginfo_t *info, void *ptr)
{
    s3_application::shutdown();
    BOOST_LOG_TRIVIAL(fatal) << "Service is abborted with signal " << signum;
    exit(0);
}

void catch_sigterm()
{
    static struct sigaction _sigact;

    memset(&_sigact, 0, sizeof(_sigact));
    _sigact.sa_sigaction = sig_term_handler;
    _sigact.sa_flags = SA_SIGINFO;

    sigaction(SIGTERM, &_sigact, NULL);
    sigaction(SIGABRT, &_sigact, 0);
    sigaction(SIGALRM, &_sigact, 0);
    sigaction(SIGFPE, &_sigact, 0);
    sigaction(SIGHUP, &_sigact, 0);
    sigaction(SIGILL, &_sigact, 0);
    sigaction(SIGINT, &_sigact, 0);
    sigaction(SIGPIPE, &_sigact, 0);
}

bool file_find(const std::string& src_dir, std::string& out)
{
    static const char* pattern =  "input_([0-9]+)\\.log"; // input file name pathern
    static boost::regex ip_regex(pattern);

    bool ret = false;
    out.clear();
    DIR *dirp = opendir(src_dir.c_str());

    if (!dirp) {
        std::cout << src_dir << " directory not found";
        std::cout << strerror(errno);
        return false;
    }

    struct dirent *dp;
    while ((dp = readdir(dirp)) != NULL) {
                std::string input(dp->d_name);
                boost::sregex_iterator it(input.begin(), input.end(), ip_regex);
                boost::sregex_iterator end;
                for (; it != end; ++it) {
                    out = it->str();
                    ret = true;
                }
    }

    closedir(dirp);
    return ret;
}

std::string* time_to_string(struct tm& tm)
{

    static char out_char[sizeof("YYYY-MM-DD-HHMMSS")];

    snprintf(out_char, sizeof(out_char), "%4d-%02d-%02d-%02d%02d%02d",
        tm.tm_year + 1900,
        tm.tm_mon,
        tm.tm_mday,
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec);

    return new std::string(out_char);
}

int main(int, char**)
{
    const unsigned WAIT_SEC = 10;  // wait interval
    const std::string src_dir("/input/");    // directory with input file
    const std::string bucket_name = "backup";
    std::string file_name;

    init_log("/var/log/mysrv_%3N.log");
    catch_sigterm();

    s3_application app;

    while(true) {
        file_find(src_dir, file_name);
        if (!file_name.empty()) {
            // file found: start file backup
            std::string full_name =  src_dir + file_name;
            BOOST_LOG_TRIVIAL(info) << "Start file backup. File: " << full_name;

            if (!app.backets_check_create(bucket_name)) {
                return EACCES;
            }

            // Create new object name as objectYYYYMMDD.txt
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);

            std::string object_name = bucket_name + "/object";
            std::unique_ptr<std::string> stime{time_to_string(tm)};
            object_name += *stime;
            object_name += ".txt";

            app.LogSystem->Log(Aws::Utils::Logging::LogLevel::Info, 
                "main",
                ("Copy  file '" + full_name + "' to object " +  object_name).c_str()); 

            // Put the file into the S3 bucket
            app.put_s3_object("", Aws::Utils::StringUtils::to_string(object_name), full_name);

            std::remove(full_name.c_str()); // delete file
        }

        sleep(WAIT_SEC);
    }

    return EXIT_SUCCESS;
}
