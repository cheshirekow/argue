#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <array>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <vector>

namespace std {

template <bool B, class T, class F>
using conditional_t = typename conditional<B, T, F>::type;

}  // namespace std

namespace argue {

template <typename T, typename _ = void>
struct is_container : std::false_type {};

template <typename... Ts>
struct is_container_helper {};

/// Metaprogram resolves true if T is a container-like class, or false if it
/// is not.
template <typename T>
struct is_container<
    T, std::conditional_t<
           false,
           is_container_helper<typename T::value_type, typename T::size_type,
                               typename T::allocator_type, typename T::iterator,
                               typename T::const_iterator,
                               decltype(std::declval<T>().size()),
                               decltype(std::declval<T>().begin()),
                               decltype(std::declval<T>().end()),
                               decltype(std::declval<T>().cbegin()),
                               decltype(std::declval<T>().cend())>,
           void>> : public std::true_type {};

template <>
struct is_container<std::string, void> : std::false_type {};

template <typename T, bool is_container>
struct get_choice_type_helper {
  typedef T type;
};

template <typename T>
struct get_choice_type_helper<T, true> {
  typedef typename T::value_type type;
};

// Metaprogram yields the value type of T if it is a container, or T itself
// if it is not a container.
template <typename T>
struct get_choice_type {
  typedef typename get_choice_type_helper<T, is_container<T>::value>::type type;
};

// Sentinel type used as a placeholder in templates when the type doesn't
// matter, or as a value in Optional when assignment is meant to clear the
// storage.
struct NoneType {};
extern const NoneType kNone;

std::ostream& operator<<(std::ostream& out, NoneType none);

// NoneType isn't really ordered, but we want them to be storable in a set
// or other needs-ordering usecases.
bool operator<(const NoneType& a, const NoneType& b);

// Sentinel for what type of exception to throw.
enum ExceptionClass {
  BUG = 0,       // A bug in the library code
  CONFIG_ERROR,  // Library user error
  INPUT_ERROR,   // Program user error
};

// Base class for exceptions used in this library
struct Exception : public std::exception {
  Exception(ExceptionClass ex_class, const std::string& message)
      : ex_class(ex_class), message(message) {}

  const char* what() const noexcept override {
    return this->message.c_str();
  }

  ExceptionClass ex_class;
  std::string file;
  int lineno;
  std::string message;
};

// Element type for a stack trace
struct TraceLine {
  void* addr;
  std::string file;
  std::string name;
  std::string offset;
  std::string saddr;
};

// A stack trace is just a vector of stack line information
typedef std::vector<TraceLine> StackTrace;

// Return the current stack trace. ``skip_frames`` defaults to 2 because
// the argue::Assertion adds two calls to the stack.
StackTrace GetStacktrace(size_t skip_frames = 2, size_t max_frames = 50);

// Print the stack trace line by line to the output stream.
std::ostream& operator<<(std::ostream& out, const StackTrace& trace);

// thrown to indicate a failed assumption or exceptional condition in
// the design of the library itself.
class BugException : public Exception {
 public:
  BugException(const std::string& message)  // NOLINT(runtime/explicit)
      : Exception(BUG, message) {
    this->stack_trace = GetStacktrace();
  }

  StackTrace stack_trace;
};

// thrown to indicate a runtime error in the configuration of a parser
class ConfigException : public Exception {
 public:
  ConfigException(const std::string& message)  // NOLINT(runtime/explicit)
      : Exception(CONFIG_ERROR, message) {
    this->stack_trace = GetStacktrace();
  }

  StackTrace stack_trace;
};

// thrown to indicate that the user has provided an erroneous input into the
// command line.
class InputException : public Exception {
 public:
  InputException(const std::string& message)  // NOLINT(runtime/explicit)
      : Exception(INPUT_ERROR, message) {}
};

// Stores the file name and line number of an assertion exception, but is also
// used to hint to the compiler which overload of operator&&() to use during
// assertions.
struct AssertionSentinel {
  std::string file;
  int lineno;
};

// Exception message builder
struct Assertion {
  // message will be appended
  explicit Assertion(ExceptionClass ex_class, bool expr);

  // construct with message
  Assertion(ExceptionClass ex_class, bool expr, const std::string& message);

  // printf wrapper
  Assertion(ExceptionClass ex_class, bool expr, const char* format, ...);

  template <typename T>
  Assertion& operator<<(const T& x) {
    if (!expr_) {
      sstream << x;
    }
    return *this;
  }

  template <typename T>
  Assertion& operator<<(const T&& x) {
    if (!expr_) {
      sstream << x;
    }
    return *this;
  }

  bool expr_;
  ExceptionClass ex_class;
  std::stringstream sstream;

 private:
  std::string printf_buf_;
};

// The && operator has lower precidence than the << operator so this allows
// us to raise the exception after the message has been constructed but before
// the assertion object has been destroyed.
//
// Usage:
// AssertionSentinel(...) && Assertion(...) << "Message";
void operator&&(const argue::AssertionSentinel& sentinel,
                const argue::Assertion& assertion);

// Evaluate an assertion (boolean expression) and if false throw an exception
// Usage is if there were overloads with the following signatures:
//
// ostream& ARGUE_ASSERT(ExceptionClass class, bool expr);
// ostream& ARGUE_ASSERT(ExceptionClass class, bool expr,
//                       const std::string& message);
// ostream& ARGUE_ASSERT(ExceptionClass class, bool expr,
//                       const char* format, ...);
#define ARGUE_ASSERT(...)                             \
  ::argue::AssertionSentinel({__FILE__, __LINE__}) && \
      ::argue::Assertion(argue::__VA_ARGS__)

// Parse a base-10 string as into a signed integer. Matches strings of the
// form `[-?]\d+`.
template <typename T>
int ParseSigned(const std::string& str, T* value) {
  *value = 0;

  size_t idx = 0;
  T multiplier = 1;

  if (str[0] == '-') {
    multiplier = -std::pow(10, str.size() - 2);
    ++idx;
  } else {
    multiplier = std::pow(10, str.size() - 1);
  }

  for (; idx < str.size(); idx++) {
    if ('0' <= str[idx] && str[idx] <= '9') {
      *value += multiplier * static_cast<T>(str[idx] - '0');
      multiplier /= 10;
    } else {
      return -1;
    }
  }

  return 0;
}

// Parse a base-10 string into an unsigned integer. Matches strings of the
// form `\d+`.
template <typename T>
int ParseUnsigned(const std::string& str, T* value) {
  *value = 0;

  T multiplier = std::pow(10, str.size() - 1);
  for (size_t idx = 0; idx < str.size(); idx++) {
    if ('0' <= str[idx] && str[idx] <= '9') {
      *value += multiplier * static_cast<T>(str[idx] - '0');
      multiplier /= 10;
    } else {
      return -1;
    }
  }

  return 0;
}

// Parse a real-number string into a floating point value. Matches strings
// of the form `[-?]\d+\.?\d*`
template <typename T>
int ParseFloat(const std::string& str, T* value) {
  *value = 0.0;

  ssize_t decimal_idx = str.find('.');
  if (decimal_idx == std::string::npos) {
    decimal_idx = str.size();
  }

  int64_t integral_part = 0;
  int64_t fractional_part = 0;
  int64_t denominator = 10;

  size_t idx = 0;
  int32_t multiplier = 1;

  if (str[0] == '-') {
    multiplier = -std::pow(10, decimal_idx - 2);
    ++idx;
  } else {
    multiplier = std::pow(10, decimal_idx - 1);
  }

  for (; idx < decimal_idx; ++idx) {
    if ('0' <= str[idx] && str[idx] <= '9') {
      integral_part += multiplier * static_cast<uint64_t>(str[idx] - '0');
      multiplier /= 10;
    } else {
      return -1;
    }
  }

  if (decimal_idx == str.size()) {
    *value = static_cast<T>(integral_part);
    return 0;
  }

  denominator = std::pow(10, str.size() - decimal_idx);
  if (str[0] == '-') {
    multiplier = -denominator / 10;
  } else {
    multiplier = denominator / 10;
  }

  ++idx;
  for (; idx < str.size(); ++idx) {
    if ('0' <= str[idx] && str[idx] <= '9') {
      fractional_part += multiplier * static_cast<uint64_t>(str[idx] - '0');
      multiplier /= 10;
    } else {
      return -1;
    }
  }

  *value = static_cast<T>(integral_part) +
           static_cast<T>(fractional_part) / static_cast<T>(denominator);
  return 0;
}

int Parse(const std::string& str, uint8_t* value);
int Parse(const std::string& str, uint16_t* value);
int Parse(const std::string& str, uint32_t* value);
int Parse(const std::string& str, uint64_t* value);
int Parse(const std::string& str, int8_t* value);
int Parse(const std::string& str, int16_t* value);
int Parse(const std::string& str, int32_t* value);
int Parse(const std::string& str, int64_t* value);
int Parse(const std::string& str, float* value);
int Parse(const std::string& str, double* value);
int Parse(const std::string& str, std::string* value);
int Parse(const std::string& str, NoneType* dummy);

template <typename T>
int Parse(const std::string& str, std::shared_ptr<T>* ptr) {
  return -1;
}

// Optionally stores a value of type T.
template <typename T>
struct Optional {
  Optional() : is_set(false) {}

  Optional(const NoneType&)  // NOLINT(runtime/explicit)
      : is_set(false) {}

  Optional(const T& value)  // NOLINT(runtime/explicit)
      : is_set(true),
        value(value) {}

  Optional& operator=(const NoneType&) {
    is_set = false;
    return *this;
  }

  Optional& operator=(const T& value) {
    is_set = true;
    this->value = value;
    return *this;
  }

  bool is_set;
  T value;
};

// Specialization for NoneType is required so that the constructor is not
// ambiguous
template <>
struct Optional<NoneType> {
  Optional() : is_set(false) {}

  explicit Optional(const NoneType&) : is_set(false) {}

  Optional& operator=(const NoneType&) {
    return *this;
  }

  operator NoneType() {
    return NoneType();
  }

  bool is_set;
  NoneType value;
};

// Tokens in an argument list are one of these.
enum ArgType { SHORT_FLAG = 0, LONG_FLAG = 1, POSITIONAL = 2 };

// Return the ArgType of a string token:
//  * SHORT_FLAG if it is of the form `-[^-]`
//  * LONG_FLAG if it is of the form `--.+`
//  * POSITIONAL otherwise
ArgType GetArgType(const std::string arg);

// Sentinel integer values used to indicate special `nargs`.
enum SentinelNargs {
  INVALID_NARGS = -5,
  ONE_OR_MORE = -4,
  ZERO_OR_MORE = -3,
  ZERO_OR_ONE = -2,
  EXACTLY_ONE = -1,
};

// Parse a string sentinel into a sentinel `nargs` value.
int StringToNargs(const std::string& str);

// Simply holds an integer but provides some constructors and assignment
// operators that allow us to assign that integer through sentinel strings.
struct Nargs {
  // Default `nargs` is exactly one value
  Nargs() : value(EXACTLY_ONE) {}

  Nargs(int value)  // NOLINT(runtime/explicit)
      : value(value) {}

  Nargs(const std::string& str)  // NOLINT(runtime/explicit)
      : value(StringToNargs(str)) {}

  Nargs(const char* str)  // NOLINT(runtime/explicit)
      : value(StringToNargs(str)) {}

  Nargs& operator=(int value) {
    this->value = value;
    return *this;
  }

  Nargs& operator=(const std::string& str) {
    this->value = StringToNargs(str);
    return *this;
  }

  Nargs& operator=(const char* str) {
    this->value = StringToNargs(str);
    return *this;
  }

  operator int() {
    return value;
  }

  int value;
};

class Parser;

// Collection of objects provided to Action objects during argument parsing
struct ParseContext {
  Parser* parser;
  std::ostream* out;
  std::string arg;
};

enum ParseResult {
  PARSE_FINISHED = 0,   // Finished parsing arguments
  PARSE_ABORTED = 1,    // Parsing was terminated early but not in error,
                        // usually this means --help or --version
  PARSE_EXCEPTION = 2,  // An exception occured and parsing failed
  // TODO(josh): add separate cases for USER exception and DEVELOPER exception,
  // so that we know what to print on the console.
};

struct ActionResult {
  // Set true by the action if it wishes to remain active after processing.
  // This is only meaningful for flags and will be ignored for positionals.
  bool keep_active;

  // success/failure of the parse
  ParseResult code;
};

// Interface shared by all action objects.
struct ActionBase {
  virtual bool IsRequired() const {
    return required_;
  }

  // Return a string used for this argument in the usage statement
  virtual std::string GetUsage() {
    return "";
  }

  // Return right hand side of help text for the help table
  virtual std::string GetHelp() {
    return "";
  }

  // Parse zero or more argument values out of args. Actions should modify
  // args and leave it in a state consistent with "remaining arguments".
  virtual void operator()(const ParseContext& ctx, std::list<std::string>* args,
                          ActionResult* result) = 0;

 protected:
  bool required_ = false;
};

// Collection of options (potentially) common among all actions
template <typename T>
struct BaseOptions {
  typedef typename get_choice_type<T>::type U;

  Nargs nargs_;
  Optional<T> const_;
  Optional<T> default_;
  std::list<U> choices_;
  bool required_;
  std::string help_;
  std::string metavar_;
  T* dest_;
};

// Interface shared by all action objects.
template <typename T>
struct Action : public ActionBase {
  // Consume and validate common options
  virtual void Prep(const BaseOptions<T>& spec) = 0;
};

template <typename T>
class StoreImplBase : public Action<T> {
 public:
  void Prep(const BaseOptions<T>& spec) override {
    ARGUE_ASSERT(CONFIG_ERROR, !spec.const_.is_set)
        << ".const_= is invalid for action type `store`";
    ARGUE_ASSERT(CONFIG_ERROR, spec.dest_)
        << ".dest_= is required for action type `store`";

    // TODO(josh): should we enable this?
    // ARGUE_ASSERT(CONFIG_ERROR, spec.default_.is_set || spec.required_)
    // << `store` action must either be required or have a default value set;

    if (spec.nargs_.value == ZERO_OR_ONE || spec.nargs_.value == ZERO_OR_MORE) {
      // TODO(josh): should we require that default is specified if argument
      // is optional? Perhaps we can use a sentinel that says default is already
      // stored in the destination?
      // ARGUE_ASSERT(CONFIG_ERROR, spec.default_.is_set || spec.required_)
      // << `store` action must either be required or have a default value set;
    }

    this->dest_ = spec.dest_;
    this->nargs_ = spec.nargs_;
    this->required_ = spec.required_;

    if (spec.default_.is_set) {
      *dest_ = spec.default_.value;
    }
  }

 protected:
  Nargs nargs_;
  T* dest_;
};

// Implements the "store" action. This default template for non-container
// (i.e. Scalar) types.
template <typename T, bool is_container>
class StoreImpl : public StoreImplBase<T> {
 public:
  void Prep(const BaseOptions<T>& spec) override {
    this->StoreImplBase<T>::Prep(spec);

    if (spec.choices_.size() > 0) {
      choices_.insert(spec.choices_.begin(), spec.choices_.end());
    }
  }

  void operator()(const ParseContext& ctx, std::list<std::string>* args,
                  ActionResult* result) override;

 private:
  std::set<T> choices_;
};

// Implements the "store" action. This partial specialization is
// for container (i.e. std::vector, std::list) types.
template <typename T>
class StoreImpl<T, /*is_container=*/true> : public StoreImplBase<T> {
 public:
  typedef typename get_choice_type<T>::type U;

  void Prep(const BaseOptions<T>& spec) override {
    this->StoreImplBase<T>::Prep(spec);

    if (spec.choices_.size() > 0) {
      choices_.insert(spec.choices_.begin(), spec.choices_.end());
    }
  }

  void operator()(const ParseContext& ctx, std::list<std::string>* args,
                  ActionResult* result) override;

 private:
  std::set<U> choices_;
};

// Implements the "store" action, which simply parses the string into a value
// of the correct type and stores it in some variable.
template <typename T>
class Store : public StoreImpl<T, is_container<T>::value> {};

// Implements the "store_const" action. This deafult template is for types that
// are not containers (i.e. Scalars)
template <typename T, bool is_container>
class StoreConstImpl : public Action<T> {
 public:
  void Prep(const BaseOptions<T>& spec) override {
    ARGUE_ASSERT(CONFIG_ERROR, spec.const_.is_set,
                 "const_= is required for action_='store_const'");
    ARGUE_ASSERT(CONFIG_ERROR, spec.dest_,
                 "dest_= is required for action_='store_const'");
    ARGUE_ASSERT(CONFIG_ERROR, !spec.required_,
                 "required_ may not be true for action_='store_const'");
    // ARGUE_ASSERT(spec.default_.is_set)
    // << "default_= is required for action_='store_const'";

    this->dest_ = spec.dest_;
    this->const_ = spec.const_.value;
    this->required_ = false;

    if (spec.default_.is_set) {
      *dest_ = spec.default_.value;
    }
  }

  void operator()(const ParseContext& ctx, std::list<std::string>* args,
                  ActionResult* result) override {
    *dest_ = const_;
  }

 private:
  T* dest_;
  T const_;
};

// Implements the "store_const" action. This partial specializationis for types
// that are containers (i.e. std::vector, std::list)
template <typename T>
class StoreConstImpl<T, /*is_container=*/true> : public Action<T> {
 public:
  typedef typename get_choice_type<T>::type U;

  void Prep(const BaseOptions<T>& spec) override {
    // TODO(josh): actually it makes perfect sense if you're fully assigning the
    // contents of a container...
    ARGUE_ASSERT(BUG, false,
                 "`store_const` doesn't make sense for container types.");
  }

  void operator()(const ParseContext& ctx, std::list<std::string>* args,
                  ActionResult* result) override {}
};

// Implements the "store_const" action, which stores a specific constant value
// into the destination variable when activated.
template <typename T>
class StoreConst : public StoreConstImpl<T, is_container<T>::value> {};

// Implements the "help" action, which prints help text and terminates the
// parse loop.
template <typename T>
class Help : public Action<T> {
  std::string GetHelp() override {
    return "print this help message";
  }

  void Prep(const BaseOptions<T>& spec) override {}
  void operator()(const ParseContext& ctx, std::list<std::string>* args,
                  ActionResult* result) override;
};

// Implements the "version" action, which prints version text and terminates
// the parse loop.
template <typename T>
class Version : public Action<T> {
  std::string GetHelp() override {
    return "print version information and exit";
  }

  void Prep(const BaseOptions<T>& spec) override {}
  void operator()(const ParseContext& ctx, std::list<std::string>* args,
                  ActionResult* result) override;
};

// Basically just a std::shared_ptr<Action<T>> but has additional constructors
// and assignment operators that allow us to assign from sentinel strings.
template <typename T>
struct ActionHolder : std::shared_ptr<Action<T>> {
  void Interpret(const std::string& name) {
    if (name == "store") {
      *this = std::make_shared<Store<T>>();
    } else if (name == "store_const") {
      *this = std::make_shared<StoreConst<T>>();
    } else if (name == "help") {
      *this = std::make_shared<Help<T>>();
    } else if (name == "version") {
      *this = std::make_shared<Version<T>>();
    } else {
      ARGUE_ASSERT(CONFIG_ERROR, false, "unrecognized action_='&s'",
                   name.c_str());
    }
  }

  ActionHolder() : std::shared_ptr<Action<T>>(std::make_shared<Store<T>>()) {}

  ActionHolder(const std::string& name) {  // NOLINT(runtime/explicit)
    Interpret(name);
  }

  ActionHolder(const char* name) {  // NOLINT(runtime/explicit)
    Interpret(name);
  }

  explicit ActionHolder(const std::shared_ptr<Action<T>>& other)
      : std::shared_ptr<Action<T>>(other) {}

  ActionHolder<T>& operator=(const std::string& name) {
    Interpret(name);
    return *this;
  }

  ActionHolder<T>& operator=(const char* name) {
    Interpret(name);
    return *this;
  }

  ActionHolder<T>& operator=(const std::shared_ptr<Action<T>>& other) {
    this->std::shared_ptr<Action<T>>::operator=(other);
    return *this;
  }
};

// Collection of options (potentially) common among all actions (like
// BaseOptions) but includes a pointer to the action object itself.
template <typename T = NoneType>
struct CommonOptions {
  typedef typename get_choice_type<T>::type U;

  ActionHolder<T> action_;
  Nargs nargs_;
  Optional<T> const_;
  Optional<T> default_;
  std::list<U> choices_;
  bool required_;
  std::string help_;
  std::string metavar_;
  T* dest_;
};

// Strip the action obect from the option struct so that we don't have a
// cyclic reference when passing into the action object to Prep() it.
template <typename T>
BaseOptions<T> ConvertOptions(const CommonOptions<T>& other) {
  // clang-format off
  return BaseOptions<T>{
    .nargs_ = other.nargs_,
    .const_ = other.const_,
    .default_ = other.default_,
    .choices_ = other.choices_,
    .required_ = other.required_,
    .help_ = other.help_,
    .metavar_ = other.metavar_,
    .dest_ = other.dest_
  };
  // clang-format on
}

// Number of character columns width for each of the three columns of help
// text: 1. short flag, 2. long flag, 3. description
typedef std::array<size_t, 3> ColumnSpec;
extern const ColumnSpec kDefaultColumns;

// Create a string by joining the elements of the container using default
// stream formatting and the provided delimeter.
template <typename Container>
std::string Join(const Container& container, const std::string& delim = ", ") {
  auto iter = container.begin();
  if (iter == container.end()) {
    return "";
  }

  std::stringstream out;
  out << *(iter++);
  while (iter != container.end()) {
    out << delim << *(iter++);
  }
  return out.str();
}

// Create a string formed by repeating `bit` for `n` times.
std::string Repeat(const std::string bit, int n);

// Wrap the given text to the specified line length
std::string Wrap(const std::string text, size_t line_length = 80);

// Compute the sum of the elements of a container.
template <typename Container>
typename Container::value_type ContainerSum(const Container& container) {
  typename Container::value_type result = 0;
  for (auto x : container) {
    result += x;
  }
  return result;
}

// Collection of program metadata, used to initialize a parser.
struct Metadata {
  Optional<bool> add_help;
  Optional<bool> add_version;
  std::string name;
  std::list<int> version;
  std::string author;
  std::string copyright;
  std::string prolog;
  std::string epilog;
};

// Help entry for a flag argument
struct FlagHelp {
  std::string short_flag;
  std::string long_flag;
  std::string help_text;
  std::string usage_text;
};

// Help entry for a positional argument
struct PositionalHelp {
  std::string name;
  std::string help_text;
  std::string usage_text;
};

// TODO(josh): make this work just on non-template options
template <typename T>
std::string GetFlagUsage(const std::string& short_flag,
                         const std::string& long_flag,
                         const BaseOptions<T>& options) {
  std::stringstream token;
  if (!options.required_) {
    token << "[";
  }

  std::list<std::string> parts;

  std::list<std::string> names;
  if (short_flag.size()) {
    names.emplace_back(short_flag);
  }
  if (long_flag.size()) {
    names.emplace_back(long_flag);
  }

  std::string name = Join(names, "/");

  if (options.nargs_.value == ONE_OR_MORE) {
    parts = {name, options.metavar_ + " [..]"};
  } else if (options.nargs_.value == ZERO_OR_ONE) {
    parts = {name, "[" + options.metavar_ + "]"};
  } else if (options.nargs_.value == ZERO_OR_MORE) {
    parts = {name, "[" + options.metavar_ + " [..]]"};
  } else if (options.nargs_.value == EXACTLY_ONE) {
    parts = {name};
  } else if (options.nargs_.value > 0) {
    parts = {name, options.metavar_, options.metavar_, ".."};
  }

  token << Join(parts, " ");
  if (!options.required_) {
    token << "]";
  }

  return token.str();
}

template <typename T>
std::string GetPositionalUsage(const std::string& name,
                               const BaseOptions<T>& options) {
  if (options.nargs_.value == ONE_OR_MORE) {
    return "<" + options.metavar_ + "> [" + options.metavar_ + "..]";
  } else if (options.nargs_.value == ZERO_OR_ONE) {
    return "[" + options.metavar_ + "]";
  } else if (options.nargs_.value == ZERO_OR_MORE) {
    return "[" + options.metavar_ + " [" + options.metavar_ + "..]]";
  } else if (options.nargs_.value == EXACTLY_ONE) {
    return "<" + options.metavar_ + ">";
  } else if (options.nargs_.value > 0) {
    std::stringstream token;
    token << "<" << options.metavar_ << "> [" << options.metavar_ << "..]("
          << options.nargs_.value << ")";
    return token.str();
  }

  return "";
}

// Value type for flag maps, allows us to reverse loop up in each list.
struct FlagStore {
  std::string short_flag;
  std::string long_flag;
  std::shared_ptr<ActionBase> action;
};

// Main class for parsing command line arguments. Use AddArgument to add
// actions (flags, positionals) to the parser, then call ParseArgs.
class Parser {
 public:
  Parser(const Metadata& meta = {});  // NOLINT(runtime/explicit)

  // Add a flag argument with the given short and log flag names
  template <typename T>
  void AddArgument(const std::string& short_flag, const std::string& long_flag,
                   T* dest, const CommonOptions<T>& spec) {
    ARGUE_ASSERT(CONFIG_ERROR, short_flag.size() > 0 || long_flag.size() > 0,
                 "Cannot AddArgument with both short_flag='' and long_flag=''");
    BaseOptions<T> base_spec = ConvertOptions(spec);
    base_spec.dest_ = dest;
    spec.action_->Prep(base_spec);

    FlagStore store{.short_flag = short_flag,
                    .long_flag = long_flag,
                    .action = spec.action_};

    if (short_flag.size() > 0) {
      ARGUE_ASSERT(CONFIG_ERROR,
                   short_flags_.find(short_flag) == short_flags_.end(),
                   "Duplicate short flag %s", short_flag.c_str());
      short_flags_[short_flag] = store;
    }

    if (long_flag.size() > 0) {
      ARGUE_ASSERT(CONFIG_ERROR,
                   long_flags_.find(long_flag) == long_flags_.end(),
                   "Duplicate long flag %s", long_flag.c_str());
      long_flags_[long_flag] = store;
    }

    FlagHelp help{
        .short_flag = short_flag,
        .long_flag = long_flag,
        .help_text = spec.action_->GetHelp(),
        .usage_text = spec.action_->GetUsage(),
    };

    if (!help.help_text.size()) {
      help.help_text = spec.help_;
    }

    if (!help.usage_text.size()) {
      help.usage_text = GetFlagUsage(short_flag, long_flag, base_spec);
    }

    flag_help_.emplace_back(help);
  }

  // For things like help/version which don't need arguments
  void AddArgument(const std::string& short_flag, const std::string& long_flag,
                   std::nullptr_t, const CommonOptions<NoneType>& spec) {
    AddArgument<NoneType>(short_flag, long_flag, nullptr, spec);
  }

  // Add a positional argument or a flag argument that has either a short flag
  // or a long flag but not both.
  template <typename T>
  void AddArgument(const std::string& name_or_flag, T* dest,
                   const CommonOptions<T>& spec) {
    ARGUE_ASSERT(CONFIG_ERROR, name_or_flag.size() > 0,
                 "Cannot AddArgument with empty name_or_flag string");
    BaseOptions<T> base_spec = ConvertOptions(spec);
    base_spec.dest_ = dest;
    ArgType arg_type = GetArgType(name_or_flag);
    switch (arg_type) {
      case SHORT_FLAG: {
        AddArgument(name_or_flag, std::string(""), dest, spec);
        break;
      }

      case LONG_FLAG: {
        AddArgument(std::string(""), name_or_flag, dest, spec);
        break;
      }

      case POSITIONAL: {
        base_spec.required_ = !(base_spec.nargs_.value == ZERO_OR_MORE ||
                                base_spec.nargs_.value == ZERO_OR_ONE);
        spec.action_->Prep(base_spec);
        positionals_.emplace_back(spec.action_);

        PositionalHelp help{
            .name = name_or_flag,
            .help_text = spec.action_->GetHelp(),
            .usage_text = spec.action_->GetUsage(),
        };

        if (!help.help_text.size()) {
          help.help_text = spec.help_;
        }

        if (!help.usage_text.size()) {
          help.usage_text = GetPositionalUsage(name_or_flag, base_spec);
        }

        positional_help_.emplace_back(help);
        break;
      }
    }
  }

  int ParseArgs(int argc, char** argv, std::ostream* log = &std::cerr);
  int ParseArgs(const std::initializer_list<std::string>& init_list,
                std::ostream* log = &std::cerr);
  int ParseArgs(std::list<std::string>* args, std::ostream* log = &std::cerr);
  void PrintUsage(std::ostream* out, size_t width = 80);

  // TODO(josh): don't use ostream in the main library, make it an include
  // option
  void PrintHelp(std::ostream* out,
                 const ColumnSpec& columns = kDefaultColumns);
  void PrintVersion(std::ostream* out,
                    const ColumnSpec& columns = kDefaultColumns);

 private:
  int ParseArgsImpl(std::list<std::string>* args, std::ostream* out);

  Metadata meta_;
  std::map<std::string, FlagStore> short_flags_;
  std::map<std::string, FlagStore> long_flags_;
  std::list<std::shared_ptr<ActionBase>> positionals_;

  std::list<FlagHelp> flag_help_;
  std::list<PositionalHelp> positional_help_;
};

template <typename T, bool is_container>
void StoreImpl<T, is_container>::operator()(const ParseContext& ctx,
                                            std::list<std::string>* args,
                                            ActionResult* result) {
  ARGUE_ASSERT(CONFIG_ERROR,
               this->nargs_ == ZERO_OR_ONE || this->nargs_ == EXACTLY_ONE,
               "Invalid nargs_=%d", this->nargs_);
  ArgType arg_type = GetArgType(args->front());

  if (arg_type == POSITIONAL) {
    if (Parse(args->front(), this->dest_)) {
      result->code = PARSE_EXCEPTION;
      return;
    }
    if (choices_.size() > 0) {
      ARGUE_ASSERT(INPUT_ERROR, choices_.find(*this->dest_) != choices_.end())
          << "Invalid value '" << args->front() << "' choose from '"
          << Join(choices_, "', '") << "'";
    }
    args->pop_front();
  } else {
    ARGUE_ASSERT(INPUT_ERROR, this->nargs_ == ZERO_OR_ONE,
                 "Expected a value but instead got a flag %s", ctx.arg.c_str());
  }
}

template <typename T>
void StoreImpl<T, /*is_container=*/true>::operator()(
    const ParseContext& ctx, std::list<std::string>* args,
    ActionResult* result) {
  size_t min_args = 0;
  size_t max_args = 100000;
  if (this->nargs_ == ZERO_OR_MORE) {
  } else if (this->nargs_ == ONE_OR_MORE) {
    min_args = 1;
  } else if (this->nargs_ == ZERO_OR_ONE) {
    ARGUE_ASSERT(CONFIG_ERROR, false, "nargs_='?' invalid for container types");
  } else if (this->nargs_ == EXACTLY_ONE) {
    ARGUE_ASSERT(CONFIG_ERROR, false,
                 "nargs_=EXACTLY_ONE invalid for container types");
  } else {
    ARGUE_ASSERT(CONFIG_ERROR, this->nargs_ >= 0,
                 "I'm not sure what you want me to do with nargs_=0");
    min_args = this->nargs_;
    max_args = this->nargs_;
  }

  this->dest_->clear();
  size_t nargs = 0;
  U temp;
  for (; nargs < max_args && args->size() > 0; nargs++) {
    ArgType arg_type = GetArgType(args->front());
    if (arg_type != POSITIONAL) {
      break;
    }

    if (Parse(args->front(), &temp)) {
      result->code = PARSE_EXCEPTION;
      return;
    }
    this->dest_->emplace_back(temp);
    if (choices_.size() > 0) {
      ARGUE_ASSERT(INPUT_ERROR, choices_.find(temp) != choices_.end())
          << "Invalid value '" << args->front() << "' choose from '"
          << Join(choices_, "', '") << "'";
    }
    args->pop_front();
  }

  if (min_args <= nargs && nargs <= max_args) {
    result->code = PARSE_FINISHED;
  } else {
    result->code = PARSE_EXCEPTION;
  }
}

template <typename T>
void Help<T>::operator()(const ParseContext& ctx, std::list<std::string>* args,
                         ActionResult* result) {
  ctx.parser->PrintHelp(ctx.out);
  result->code = PARSE_ABORTED;
}

template <typename T>
void Version<T>::operator()(const ParseContext& ctx,
                            std::list<std::string>* args,
                            ActionResult* result) {
  ctx.parser->PrintVersion(ctx.out);
  result->code = PARSE_ABORTED;
}

}  // namespace argue
