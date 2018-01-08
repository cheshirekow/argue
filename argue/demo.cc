// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <iostream>
#include <list>
#include <memory>

#include "argue/argue.h"

class Accumulator {
 public:
  std::string GetName() {
    return name_;
  }
  virtual int operator()(const std::list<int>& args) = 0;

 protected:
  std::string name_;
};

struct Max : public Accumulator {
  Max() {
    name_ = "max";
  }

  int operator()(const std::list<int>& args) override {
    if (args.size() == 0) {
      return 0;
    }
    int result = args.front();
    for (int x : args) {
      if (x > result) {
        result = x;
      }
    }
    return result;
  }
};

struct Sum : public Accumulator {
  Sum() {
    name_ = "sum";
  }

  int operator()(const std::list<int>& args) override {
    int result = 0;
    for (int x : args) {
      result += x;
    }
    return result;
  }
};

int main(int argc, char** argv) {
  std::list<int> int_args;
  std::shared_ptr<Accumulator> accumulate;
  std::shared_ptr<Accumulator> sum_fn = std::make_shared<Sum>();
  std::shared_ptr<Accumulator> max_fn = std::make_shared<Max>();

  argue::Parser parser({
      .add_help = true,
      .add_version = true,
      .name = "argue-demo",
      .version = {0, 0, 1},
      .author = "Josh Bialkowski <josh.bialkowski@gmail.com>",
      .copyright = "(C) 2018",
  });

// clang-format off
#ifdef __clang__
  parser.AddArgument("integer", &int_args, {
    .nargs_ = "+",
    .help_ = "an integer for the accumulator",
    .metavar_ = "N",
    .choices_ = {1, 2, 3, 4},
  });

  parser.AddArgument("-s", "--sum", &accumulate, {
    .action_ = "store_const",
    .const_ = sum_fn,
    .default_ = max_fn,
    .help_ = "sum the integers (default: find the max)",
  });

#else
  parser.AddArgument("integer", &int_args, {
    .action_ = "store",
    .nargs_ = "+",
    .const_ = argue::kNone,
    .default_ = argue::kNone,
    .choices_ = {1, 2, 3, 4},
    .required_ = false,
    .help_ = "an integer for the accumulator",
    .metavar_ = "N",
  });

  parser.AddArgument("-s", "--sum", &accumulate, {
    .action_ = "store_const",
    .nargs_ = "",
    .const_ = sum_fn,
    .default_ = max_fn,
    .choices_ = {},
    .required_ = false,
    .help_ = "sum the integers (default: find the max)",
  });

#endif
  // clang-format on

  int parse_result = parser.ParseArgs(argc, argv);
  switch (parse_result) {
    case argue::PARSE_ABORTED:
      return 0;
    case argue::PARSE_EXCEPTION:
      return 1;
    case argue::PARSE_FINISHED:
      break;
  }

  std::cout << accumulate->GetName() << "(" << argue::Join(int_args)
            << ") = " << (*accumulate)(int_args) << "\n";
  return 0;
}
