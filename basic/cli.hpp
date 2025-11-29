#pragma once

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

enum class Source { CLI, DEFALUT };

class OptionBase {
 public:
  std::pair<std::string, std::string> name;
  Source source;
  std::string help;

  virtual void setValue(const std::string& v) = 0;
  virtual std::string getValue() const = 0;
  virtual ~OptionBase() = default;
};

using OptionTable = std::unordered_map<std::string, OptionBase*>;
static OptionTable Options;

template <typename T>
class Option : public OptionBase {
 private:
  T value;

 public:
  Option(const std::pair<std::string, std::string>& name,
         const std::string& help, const T& value) {
    this->name = name;
    this->help = help;
    this->value = value;
    this->source = Source::DEFALUT;

    Options[name.first] = this;
    Options[name.second] = this;
  }

  void setValue(const std::string& v) override {
    source = Source::CLI;
    if constexpr (std::is_same_v<T, std::vector<std::string>>) {
      value.push_back(v);
    } else if constexpr (std::is_same_v<T, std::vector<int>>) {
      value.push_back(std::stoi(v));
    } else {
      std::istringstream iss(v);
      iss >> value;
    }
  }

  std::string getValue() const override {
    if constexpr (std::is_same_v<T, std::vector<std::string>>) {
      std::string out;
      for (auto& s : value) out += s + " ";
      return out;
    } else if constexpr (std::is_same_v<T, std::vector<int>>) {
      std::string out;
      for (auto& n : value) out += std::to_string(n) + " ";
      return out;
    } else {
      std::ostringstream oss;
      oss << value;
      return oss.str();
    }
  }

  Source getSource() { return source; }

  operator bool() const { return value; }
};

template <>
inline void Option<bool>::setValue(const std::string& value) {
  if (value == "true" || value == "1")
    this->value = true;
  else if (value == "false" || value == "0")
    this->value = false;
  else
    this->value = true;
}

inline OptionTable& parseOptions(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    std::string key;
    std::string val = "true";

    if (arg.rfind("--", 0) == 0) {
      key = arg.substr(2);
    } else if (arg.rfind("-", 0) == 0) {
      key = arg.substr(1);
    } else {
      continue;
    }

    // If the next argument is present and does not start with '-'
    // it is considered a value
    if (i + 1 < argc && argv[i + 1][0] != '-') {
      val = argv[++i];
    }

    if (Options.count(key)) {
      Options[key]->setValue(val);
    } else {
      std::cerr << "Unknown option: " << key << "\n";
    }
  }
  return Options;
}

inline void printHelp() {
  std::cerr << "Options:\n";

  // calculate the width for aligment
  size_t shortW = 6, longW = 0;
  for (auto& [kind, value] : Options) {
    if (kind.size() > 1) {
      longW = std::max(longW, kind.size() + 2);
    }
  }

  std::unordered_set<OptionBase*> printed;
  for (auto& [kind, value] : Options) {
    if (printed.count(value)) continue;

    std::string shortOpt, longOpt;
    if (!value->name.second.empty()) {
      shortOpt = "-" + value->name.second;
    }
    if (!value->name.first.empty()) {
      longOpt = "--" + value->name.first;
    }

    std::cerr << std::setw(4) << " " << std::left << std::setw(shortW)
              << shortOpt << std::setw(longW + 4) << longOpt << value->help
              << "\n";

    printed.insert(value);
  }
}
