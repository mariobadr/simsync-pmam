#include <simsync/application.hpp>
#include <simsync/architecture.hpp>
#include <simsync/estimate.hpp>
#include <simsync/system.hpp>

#include <simsync/reports/event_trace.hpp>
#include <simsync/reports/scheduler_trace.hpp>
#include <simsync/reports/criticality_stack.hpp>
#include <simsync/reports/time_stack.hpp>

#include <iostream>
#include <sstream>

#include "cxxopts.hpp"

cxxopts::Options parse_arguments(int argc, char **argv)
{
  cxxopts::Options options("simsync-cl", "A simple synchronization model.");

  options.add_options("help")("h,help", "Print this help message", cxxopts::value<bool>(), "");
  options.add_options("input")("a,arch", "Architecture config", cxxopts::value<std::string>(), "<file>");
  options.add_options("input")("t,trace", "Trace file", cxxopts::value<std::string>(), "<file>");
  options.add_options("output")("r,report", "Report type", cxxopts::value<std::string>(), "<string>");
  options.add_options("output")("o,out", "Output file", cxxopts::value<std::string>(), "<file>");

  options.parse(argc, argv);

  return options;
}

template <typename Stream>
Stream load_file(std::string const &path)
{
  Stream file(path);

  if(!file.good()) {
    throw std::runtime_error("Error: " + path + " does not exist.");
  }

  return file;
}

void validate(cxxopts::Options const &options)
{
  if(options.count("t") == 0) {
    throw std::runtime_error("Missing Argument: Please provide an application trace.");
  }
  load_file<std::ifstream>(options["t"].as<std::string>());

  if(options.count("a") == 0) {
    throw std::runtime_error("Missing Argument: Please provide an architecture configuration.");
  }
  load_file<std::ifstream>(options["a"].as<std::string>());

  if(options.count("r") == 0) {
    throw std::runtime_error("Missing Argument: Please provide a report type.");
  }

  if(options.count("o") == 0) {
    throw std::runtime_error("Missing Argument: Please provide an output file name.");
  }
}

std::deque<std::unique_ptr<simsync::report>> create_reports(
    std::deque<std::string> const &report_types,
    std::deque<std::string> const &output_files,
    simsync::system &sys)
{
  if(report_types.size() != output_files.size()) {
    throw std::runtime_error("Error: simsync needs an output file per report type.");
  }

  std::deque<std::unique_ptr<simsync::report>> reports;

  for(size_t index = 0; index < report_types.size(); ++index) {
    auto const &report_type = report_types.at(index);
    auto const &output_file = output_files.at(index);

    if(report_type == "event-trace") {
      reports.emplace_back(std::make_unique<simsync::event_trace>(output_file));
    } else if(report_type == "scheduler-trace") {
      reports.emplace_back(std::make_unique<simsync::scheduler_trace>(output_file, sys));
    } else if(report_type == "time-stack") {
      reports.emplace_back(std::make_unique<simsync::time_stack>(output_file, sys));
    } else if(report_type == "criticality-stack") {
      reports.emplace_back(std::make_unique<simsync::criticality_stack>(output_file, sys));
    } else {
      throw std::runtime_error("Error: Unknown report type specified.");
    }
  }

  return reports;
}

std::deque<std::string> split(std::string const &s, char delimiter = ',')
{
  std::deque<std::string> strings;
  size_t start = 0;
  size_t end = s.find(delimiter);

  while(end != std::string::npos) {
    strings.emplace_back(s.substr(start, end - start));
    start = end + 1;
    end = s.find(delimiter, start);
  }

  strings.emplace_back(s.substr(start, end));

  return strings;
}

int main(int argc, char **argv)
{
  using namespace std::chrono;

  try {
    auto args = parse_arguments(argc, argv);
    if(args.count("h") == 1) {
      std::cout << args.help({"help", "input", "output"});

      return EXIT_SUCCESS;
    }

    validate(args);
    std::cout << "Info: " << args["o"].as<std::string>() << "\n";
    std::cout << "Info: " << args["r"].as<std::string>() << "\n";

    auto start = high_resolution_clock::now();
    simsync::architecture architecture(args["a"].as<std::string>());
    simsync::system system(args["a"].as<std::string>(), architecture);
    auto end = high_resolution_clock::now();
    std::cout << "Perf: Timing model loaded in "
              << std::chrono::duration<double, std::milli>(end - start).count() << "ms\n";

    start = high_resolution_clock::now();
    auto trace = load_file<std::ifstream>(args["t"].as<std::string>());
    simsync::application application(trace);
    end = high_resolution_clock::now();
    std::cout << "Perf: Application trace loaded in "
              << std::chrono::duration<double, std::milli>(end - start).count() << "ms\n";

    auto reports = create_reports(
        split(args["r"].as<std::string>()), split(args["o"].as<std::string>()), system);

    start = high_resolution_clock::now();
    auto const execution_time = simsync::estimate(application, system, reports);
    end = high_resolution_clock::now();
    std::cout << "Perf: Estimation completed in "
              << std::chrono::duration<double, std::milli>(end - start).count() << "ms\n";

    std::cout << "Info: SimSync execution time estimate is "
              << std::chrono::duration<double>(execution_time).count() << "s\n";

  } catch(std::exception const &e) {
    std::cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
