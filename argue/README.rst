==================================
argue: An argument parsing library
==================================

The API tries to closely mimic that of python's argparse. For instance the
simple demo from the argparse documentation is replicated in c++ with.

.. code:: c++

    #include <list>
    #include <iostream>
    #include <memory>

    #include "argue/argue.h"

    class Accumulator {
     public:
      std::string GetName(){ return name_; }
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
          .name = "argue-demo",
          .version = {0, 0, 1},
          .author = "Josh Bialkowski <josh.bialkowski@gmail.com>",
          .copyright = "(C) 2018",
      });

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

      parser.ParseArgs(argc, argv);
      if (parser.ExitedEarly()) {
        return 0;
      }

      std::cout << accumulate->GetName() << "(" << argue::Join(int_args) << ") = "
                << (*accumulate)(int_args) << "\n";
      return 0;
    }

---------------
Example usage
---------------
::

    $ ./argue-demo -h
    ./argue_demo
    --------------------
      version: 0.0.1
      author :Josh Bialkowski <josh.bialkowski@gmail.com>
      copyright: (C) 2018

    usage: ./argue_demo [-s/--sum] [-h/--help] [-v/--version]

    Flags:
    --------------------
    -s     --sum                  sum the integers (default: find the max)
    -h     --help                 print this help message
    -v     --version              print version information and exit

    Positionals:
    --------------------
    integer                       an integer for the accumulator

    $ argue-demo 1 2 3 4
    max(1, 2, 3, 4) = 4

    $ argue-demo --sum 1 2 3 4
    sum(1, 2, 3, 4) = 10


-------------------------------
Note on designated-initializers
-------------------------------

Designated initializers are a ``C99`` feature that ``clang`` interprets correctly
when compiling ``C++``, but is not in fact a language feature. The ``GNU``
toolchain does not implement this feature. Therefore, while the following is valid when compiling with ``clang``::

    parser.AddArgument("integer", &int_args, {
      .nargs_ = "+",
      .help_ = "an integer for the accumulator",
      .metavar_ = "N"
    });

We must use the following in ``gcc``::

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
