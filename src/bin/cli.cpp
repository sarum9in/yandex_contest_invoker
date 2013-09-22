/*!
 * \file
 *
 * \brief Command line interface to Invoker.
 */

#include <yandex/contest/invoker/All.hpp>
#include <yandex/contest/invoker/detail/VectorToString.hpp>

#include <yandex/contest/detail/LogHelper.hpp>
#include <yandex/contest/TypeInfo.hpp>

#include <bunsan/config/output_archive.hpp>
#include <bunsan/filesystem/fstream.hpp>

#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <iostream>

namespace yandex{namespace contest{namespace invoker{namespace cli
{
    ContainerConfig parseConfig(const boost::filesystem::path &config)
    {
        STREAM_INFO << "Trying to load configuration file at " << config << ".";
        ContainerConfig cfg;
        bunsan::filesystem::ifstream cfgin(config);
        BUNSAN_FILESYSTEM_FSTREAM_WRAP_BEGIN(cfgin)
        {
            cfgin >> cfg;
        }
        BUNSAN_FILESYSTEM_FSTREAM_WRAP_END(cfgin)
        cfgin.close();
        return cfg;
    }

    template <typename T>
    void printSerializable(std::ostream &out, const T &obj)
    {
        boost::property_tree::ptree ptree;
        bunsan::config::output_archive<boost::property_tree::ptree> oa(ptree);
        oa << obj;
        boost::property_tree::write_json(out, ptree);
    }

    void execute(const ContainerConfig &config,
                 const boost::filesystem::path &executable,
                 const ProcessArguments &arguments,
                 const ProcessGroup::ResourceLimits &processGroupResourceLimits,
                 const Process::ResourceLimits &processResourceLimits,
                 const boost::filesystem::path &inFile,
                 const boost::filesystem::path &outFile,
                 const boost::filesystem::path &errFile)
    {
        STREAM_INFO <<
            "Trying to execute " << executable << " with " <<
            detail::vectorToString(arguments) << " arguments where " <<
            STREAM_OBJECT(processResourceLimits);

        ContainerPointer container = Container::create(config);
        ProcessGroupPointer processGroup = container->createProcessGroup();
        processGroup->setResourceLimits(processGroupResourceLimits);
        ProcessPointer process = processGroup->createProcess(executable);
        process->setArguments(arguments);
        process->setResourceLimits(processResourceLimits);
        if (inFile != "/dev/null")
        {
            container->filesystem().push(inFile, "/stdin", system::unistd::access::Id(0, 0), 0400);
            process->setStream(0, File("/stdin", AccessMode::READ_ONLY));
        }
        if (outFile != "/dev/null")
            process->setStream(1, File("/stdout", AccessMode::WRITE_ONLY));
        if (errFile != "/dev/null")
            process->setStream(2, File("/stderr", AccessMode::WRITE_ONLY));
        const ProcessGroup::Result processGroupResult = processGroup->synchronizedCall();
        const Process::Result processResult = process->result();
        STREAM_INFO << "Process group has terminated";
        // output results
        std::cout << "Process group result:" << std::endl;
        printSerializable(std::cout, processGroupResult);
        std::cout << "Process result:" << std::endl;
        printSerializable(std::cout, processResult);
        if (outFile != "/dev/null")
            container->filesystem().pull("/stdout", outFile);
        if (errFile != "/dev/null")
            container->filesystem().pull("/stderr", errFile);
    }
}}}}

int main(int argc, char *argv[])
{
    namespace ya = yandex::contest::invoker;
    namespace po = boost::program_options;
    po::options_description desc("Usage");
    try
    {
        std::string config, executable, inFile, outFile, errFile;
        std::uint64_t timeLimitNanos;
        std::uint64_t memoryLimitBytes;
        std::uint64_t outputLimitBytes;
        std::uint64_t realTimeLimitMillis;
        ya::ProcessArguments arguments;
        desc.add_options()
            ("config,c", po::value<std::string>(&config), "configuration file")
            ("executable,e", po::value<std::string>(&executable)->required(), "executable")
            ("time-limit,t", po::value<std::uint64_t>(&timeLimitNanos),
                "time limit in nanoseconds")
            ("memory-limit,m", po::value<std::uint64_t>(&memoryLimitBytes),
                "memory limit in bytes")
            ("output-limit,o", po::value<std::uint64_t>(&outputLimitBytes),
                "output limit in bytes")
            ("real-time-limit,l", po::value<std::uint64_t>(&realTimeLimitMillis),
                "real time limit in milliseconds")
            ("stdin", po::value<std::string>(&inFile)->default_value("/dev/null"), "file for stdin")
            ("stdout", po::value<std::string>(&outFile)->default_value("/dev/null"), "file for stdout")
            ("stderr", po::value<std::string>(&errFile)->default_value("/dev/null"), "file for stderr")
            ("argument,a", po::value<ya::ProcessArguments>(&arguments)->composing(), "arguments");
        po::positional_options_description pdesc;
        pdesc.add("argument", -1);
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(pdesc).run(), vm);
        po::notify(vm);

        const ya::ContainerConfig cfg = vm.count("config") ?
            ya::cli::parseConfig(config) :
            ya::ContainerConfig::fromEnvironment();

        ya::Process::ResourceLimits processResourceLimits =
            cfg.processGroupDefaultSettings.processDefaultSettings.resourceLimits;

        if (vm.count("time-limit"))
            processResourceLimits.timeLimit =
                std::chrono::nanoseconds(timeLimitNanos);

        if (vm.count("memory-limit"))
            processResourceLimits.memoryLimitBytes = memoryLimitBytes;

        if (vm.count("output-limit"))
            processResourceLimits.outputLimitBytes = outputLimitBytes;

        ya::ProcessGroup::ResourceLimits processGroupResourceLimits =
            cfg.processGroupDefaultSettings.resourceLimits;

        if (vm.count("real-time-limit"))
            processGroupResourceLimits.realTimeLimit =
                std::chrono::milliseconds(realTimeLimitMillis);

        ya::cli::execute(cfg, executable, arguments,
                         processGroupResourceLimits,
                         processResourceLimits,
                         inFile, outFile, errFile);
    }
    catch (po::error &e)
    {
        std::cerr << e.what() << std::endl << std::endl << desc << std::endl;
        return 200;
    }
    catch (std::exception &e)
    {
        std::cerr << "Program terminated due to exception of type \"" <<
                     yandex::contest::typeinfo::name(e) << "\"." << std::endl;
        std::cerr << "what() returns the following message:" << std::endl <<
                     e.what() << std::endl;
        return 1;
    }
}
