// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <cxxabi.h>
#include <execinfo.h>

#include "argue/argue.h"

namespace argue {

std::ostream& operator<<(std::ostream& out, NoneType none) {
  out << "<None>";
  return out;
}

const NoneType kNone{};

bool operator<(const NoneType& a, const NoneType& b) {
  return false;
}

static void ParseTraceLine(char* symbol, TraceLine* traceline) {
  char* begin_name = 0;
  char* begin_offset = 0;
  char* end_offset = 0;
  char* begin_addr = 0;
  char* end_addr = 0;

  // Find parentheses and +address offset surrounding the mangled name:
  // ./module(function+0x15c) [0x8048a6d]
  for (char* ptr = symbol; *ptr; ++ptr) {
    if (*ptr == '(') {
      begin_name = ptr;
    } else if (*ptr == '+') {
      begin_offset = ptr;
    } else if (*ptr == ')' && begin_offset) {
      end_offset = ptr;
    } else if (*ptr == '[' && end_offset) {
      begin_addr = ptr;
    } else if (*ptr == ']' && begin_addr) {
      end_addr = ptr;
    }
  }

  if (begin_name) {
    *begin_name = '\0';
  }
  traceline->file = symbol;

  if (begin_offset) {
    *begin_offset = '\0';
  }
  if (end_offset) {
    *end_offset = '\0';
  }

  if (begin_offset && begin_name < begin_offset) {
    traceline->name = (begin_name + 1);
    traceline->offset = (begin_offset + 1);
  }

  if (end_addr) {
    *end_addr = '\0';
    traceline->saddr = (begin_addr + 1);
  }
}

// TODO(josh): use libunwind
StackTrace GetStacktrace(size_t skip_frames, size_t max_frames) {
  StackTrace result;
  std::vector<void*> addrlist(max_frames + 1);

  // http://man7.org/linux/man-pages/man3/backtrace.3.html
  int addrlen = backtrace(&addrlist[0], addrlist.size());
  if (addrlen == 0) {
    return {{0, "<empty, possibly corrupt>"}};
  }

  // symbol -> "filename(function+offset)" (this array needs to free())
  char** symbols = backtrace_symbols(&addrlist[0], addrlen);

  // We have to malloc this, can't use std::string(), because demangle function
  // may realloc() it.
  size_t funcnamesize = 256;
  char* funcnamebuf = static_cast<char*>(malloc(funcnamesize));

  // Iterate over the returned symbol lines, skipping frames as requested,
  // and one extra for this function.
  result.reserve(addrlen - skip_frames);
  for (size_t idx = skip_frames + 1; idx < addrlen; idx++) {
    TraceLine traceline{.addr = addrlist[idx]};
    ParseTraceLine(symbols[idx], &traceline);
    int status = 0;
    if (traceline.name.size()) {
      char* ret = abi::__cxa_demangle(traceline.name.c_str(), funcnamebuf,
                                      &funcnamesize, &status);
      if (status == 0) {
        funcnamebuf = ret;
        traceline.name = funcnamebuf;
      }
    }

    result.push_back(traceline);
  }

  free(funcnamebuf);
  free(symbols);
  return result;
}

// message will be appended
Assertion::Assertion(ExceptionClass ex_class, bool expr)
    : ex_class(ex_class), expr_(expr) {
  printf_buf_.resize(128);
}

// construct with message
Assertion::Assertion(ExceptionClass ex_class, bool expr,
                     const std::string& message)
    : ex_class(ex_class), expr_(expr) {
  printf_buf_.resize(128);
  if (!expr) {
    sstream << message;
  }
}

// printf wrapper
Assertion::Assertion(ExceptionClass ex_class, bool expr, const char* format,
                     ...)
    : ex_class(ex_class), expr_(expr) {
  if (!expr) {
    va_list args;
    va_start(args, format);
    int desired_size = vsnprintf(nullptr, 0, format, args);
    va_end(args);

    printf_buf_.resize(desired_size + 1);
    va_start(args, format);
    vsnprintf(&printf_buf_[0], printf_buf_.size(), format, args);
    va_end(args);

    printf_buf_.resize(desired_size);
    sstream << printf_buf_;
  }
}

void operator&&(const argue::AssertionSentinel& sentinel,
                const argue::Assertion& assertion) {
  if (!assertion.expr_) {
    switch (assertion.ex_class) {
      case argue::BUG: {
        // TODO(josh): add stack trace
        argue::BugException ex(assertion.sstream.str());
        ex.file = sentinel.file;
        ex.lineno = sentinel.lineno;
        throw ex;
      }
      case argue::CONFIG_ERROR: {
        argue::ConfigException ex(assertion.sstream.str());
        ex.file = sentinel.file;
        ex.lineno = sentinel.lineno;
        throw ex;
      }
      case argue::INPUT_ERROR: {
        argue::InputException ex(assertion.sstream.str());
        ex.file = sentinel.file;
        ex.lineno = sentinel.lineno;
        throw ex;
      }
    }
  }
}

std::ostream& operator<<(std::ostream& out, const StackTrace& trace) {
  std::string prev_file = "";
  for (const TraceLine& line : trace) {
    if (line.file != prev_file) {
      out << line.file << "\n";
      prev_file = line.file;
    }
    if (line.name.size()) {
      out << "    " << line.name << "\n";
    } else {
      out << "    ?? [" << std::hex << line.addr << "]\n";
    }
  }
  return out;
}

ArgType GetArgType(const std::string arg) {
  if (arg.size() > 1 && arg[0] == '-') {
    if (arg.size() > 2 && arg[1] == '-') {
      return LONG_FLAG;
    } else if (arg[1] != '-') {
      return SHORT_FLAG;
    } else {
      return POSITIONAL;
    }
  } else {
    return POSITIONAL;
  }
}

int Parse(const std::string& str, uint8_t* value) {
  return ParseUnsigned(str, value);
}
int Parse(const std::string& str, uint16_t* value) {
  return ParseUnsigned(str, value);
}
int Parse(const std::string& str, uint32_t* value) {
  return ParseUnsigned(str, value);
}
int Parse(const std::string& str, uint64_t* value) {
  return ParseUnsigned(str, value);
}
int Parse(const std::string& str, int8_t* value) {
  return ParseSigned(str, value);
}
int Parse(const std::string& str, int16_t* value) {
  return ParseSigned(str, value);
}
int Parse(const std::string& str, int32_t* value) {
  return ParseSigned(str, value);
}
int Parse(const std::string& str, int64_t* value) {
  return ParseSigned(str, value);
}
int Parse(const std::string& str, float* value) {
  return ParseFloat(str, value);
}
int Parse(const std::string& str, double* value) {
  return ParseFloat(str, value);
}
int Parse(const std::string& str, std::string* value) {
  *value = str;
  return 0;
}

int Parse(const std::string& str, NoneType* dummy) {
  return -1;
}

int StringToNargs(const std::string& str) {
  if (str == "+") {
    return ONE_OR_MORE;
  } else if (str == "*") {
    return ZERO_OR_MORE;
  } else if (str == "?") {
    return ZERO_OR_ONE;
  }
  return INVALID_NARGS;
}

const ColumnSpec kDefaultColumns = {4, 20, 50};

std::string Repeat(const std::string bit, int n) {
  std::stringstream out;
  for (int i = 0; i < n; i++) {
    out << bit;
  }
  return out.str();
}

// http://rosettacode.org/wiki/Word_wrap#C.2B.2B
std::string Wrap(const std::string text, size_t line_length) {
  std::istringstream words(text);
  std::ostringstream wrapped;
  std::string word;

  if (words >> word) {
    wrapped << word;
    size_t space_left = line_length - word.length();
    while (words >> word) {
      if (space_left < word.length() + 1) {
        wrapped << '\n' << word;
        space_left = line_length - word.length();
      } else {
        wrapped << ' ' << word;
        space_left -= word.length() + 1;
      }
    }
  }
  return wrapped.str();
}

Parser::Parser(const Metadata& meta) : meta_(meta) {
  if(!meta.add_help.is_set || meta.add_help.value) {
    this->AddArgument("-h", "--help", nullptr, {.action_ = "help"});
  }
  if(!meta.add_version.is_set || meta.add_version.value) {
    this->AddArgument("-v", "--version", nullptr, {.action_ = "version"});
  }
}

int Parser::ParseArgs(int argc, char** argv,
                      std::ostream* out) {
  if (argc > 0) {
    meta_.name = argv[0];
  }

  std::list<std::string> args;
  for (size_t i = 1; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }

  int retcode = ParseArgs(&args, out);
  if (retcode == PARSE_EXCEPTION) {
    PrintUsage(out);
  }
  return retcode;
}

int Parser::ParseArgs(const std::initializer_list<std::string>& init_list,
                      std::ostream* out) {
  std::list<std::string> args = init_list;
  return ParseArgs(&args, out);
}

int Parser::ParseArgs(std::list<std::string>* args,
                      std::ostream* out) {
  try {
    return ParseArgsImpl(args, out);
  } catch(const BugException& ex) {
    (*out) << ex.message << "\n";
    (*out) << ex.stack_trace;
    return PARSE_EXCEPTION;
  } catch(const ConfigException& ex) {
    (*out) << ex.message << "\n";
    (*out) << ex.stack_trace;
    return PARSE_EXCEPTION;
  } catch(const InputException& ex) {
    (*out) << ex.message << "\n";
    return PARSE_EXCEPTION;
  }
}

int Parser::ParseArgsImpl(std::list<std::string>* args, std::ostream* out) {
  ParseContext ctx{.parser = this, .out=out};

  std::list<std::shared_ptr<ActionBase>> positionals = positionals_;
  while (args->size() > 0) {
    ArgType arg_type = GetArgType(args->front());
    ActionResult out{
        .keep_active = false, .code = PARSE_FINISHED,
    };

    switch (arg_type) {
      case SHORT_FLAG: {
        ctx.arg = args->front();
        args->pop_front();
        for (size_t idx = 1; idx < ctx.arg.size(); ++idx) {
          std::string query_flag = std::string("-") + ctx.arg[idx];
          auto flag_iter = short_flags_.find(query_flag);
          ARGUE_ASSERT(INPUT_ERROR, (flag_iter != short_flags_.end())) <<
            "Unrecognized short flag: " << query_flag;
          FlagStore store = flag_iter->second;
          ARGUE_ASSERT(BUG, bool(store.action)) << "Flag " << query_flag << " was found in index with empty action pointer";
          (*store.action)(ctx, args, &out);

          if (!out.keep_active) {
            short_flags_.erase(store.short_flag);
            long_flags_.erase(store.long_flag);
          }
        }
        break;
      }

      case LONG_FLAG: {
        ctx.arg = args->front();
        args->pop_front();
        auto flag_iter = long_flags_.find(ctx.arg);
        ARGUE_ASSERT(INPUT_ERROR, (flag_iter != long_flags_.end())) <<
            "Unrecognized long flag: " << ctx.arg;
        FlagStore store = flag_iter->second;
        ARGUE_ASSERT(BUG, bool(store.action)) << "Flag " << ctx.arg << " was found in index with empty action pointer";
        (*store.action)(ctx, args, &out);
        if (!out.keep_active) {
          short_flags_.erase(store.short_flag);
          long_flags_.erase(store.long_flag);
        }
        break;
      }

      case POSITIONAL: {
        ctx.arg = "";
        if (positionals.size() < 1) {
          return PARSE_EXCEPTION;
        }
        std::shared_ptr<ActionBase> action = positionals.front();
        positionals.pop_front();
        ARGUE_ASSERT(BUG, bool(action)) << "positional with empty action pointer";
        (*action)(ctx, args, &out);
        break;
      }
    }

    if (out.code != PARSE_FINISHED) {
      return out.code;
    }
  }

  for (const std::shared_ptr<ActionBase>& action : positionals) {
    if (action->IsRequired()) {
      return PARSE_EXCEPTION;
    }
  }

  for (const auto& pair : short_flags_) {
    const FlagStore& store = pair.second;
    if (store.action->IsRequired()) {
      return PARSE_EXCEPTION;
    }
  }

  for (const auto& pair : long_flags_) {
    const FlagStore& store = pair.second;
    if (store.action->IsRequired()) {
      return PARSE_EXCEPTION;
    }
  }

  return PARSE_FINISHED;
}

void Parser::PrintUsage(std::ostream* out, size_t width) {
  std::stringstream line;

  std::list<std::string> parts = {meta_.name};
  for (const FlagHelp& flag : flag_help_) {
    parts.push_back(flag.usage_text);
  }

  for (const PositionalHelp& positional : positional_help_) {
    parts.push_back(positional.usage_text);
  }

  (*out) << Join(parts, " ") << "\n";
}

void Parser::PrintHelp(std::ostream* out, const ColumnSpec& columns) {
  size_t width = 80;
  size_t padding = (width - ContainerSum(columns)) / (columns.size() - 1);

  // TODO(josh): detect multiline and break it up
  (*out) << meta_.name << "\n" << Repeat("-", 20) << "\n";

  if (meta_.version.size() > 0) {
    (*out) << "  version: " << Join(meta_.version, ".") << "\n";
  }
  if (meta_.author.size() > 0) {
    (*out) << "  author : " << meta_.author << "\n";
  }
  if (meta_.copyright.size() > 0) {
    (*out) << "  copyright: " << meta_.copyright << "\n";
  }

  (*out) << "\n";
  PrintUsage(out, width);

  if (meta_.prolog.size() > 0) {
    (*out) << meta_.prolog;
  }

  if (flag_help_.size() > 0) {
    (*out) << "\n";
    (*out) << "Flags:\n" << Repeat("-", 20) << "\n";
    for (const FlagHelp& help : flag_help_) {
      (*out) << help.short_flag;
      (*out) << Repeat(" ", padding + columns[0] - help.short_flag.size());
      (*out) << help.long_flag;
      (*out) << Repeat(" ", padding + columns[1] - help.long_flag.size());

      if (help.long_flag.size() > columns[1]) {
        (*out) << "\n";
        (*out) << Repeat(" ", columns[0] + columns[1] + 2 * padding);
      }

      std::stringstream ss(Wrap(help.help_text, columns[2]));
      std::string line;

      if (std::getline(ss, line, '\n')) {
        (*out) << line << "\n";
      } else {
        (*out) << "\n";
      }

      while (std::getline(ss, line, '\n')) {
        (*out) << Repeat(" ", columns[0] + columns[1] + 2 * padding) << line
               << "\n";
      }
    }
  }

  if (positional_help_.size() > 0) {
    (*out) << "\n";
    (*out) << "Positionals:\n" << Repeat("-", 20) << "\n";
    for (const PositionalHelp& help : positional_help_) {
      (*out) << help.name;
      (*out) << Repeat(
          " ", 2 * padding + columns[0] + columns[1] - help.name.size());
      if (help.name.size() > padding + columns[0] + columns[1]) {
        (*out) << "\n";
        (*out) << Repeat(" ", columns[0] + columns[1] + 2 * padding);
      }

      std::stringstream ss(Wrap(help.help_text, columns[2]));
      std::string line;

      if (std::getline(ss, line, '\n')) {
        (*out) << line << "\n";
      } else {
        (*out) << "\n";
      }

      while (std::getline(ss, line, '\n')) {
        (*out) << Repeat(" ", columns[0] + columns[1] + 2 * padding) << line
               << "\n";
      }
    }
  }

  if (meta_.epilog.size() > 0) {
    (*out) << meta_.epilog;
  }
}

void Parser::PrintVersion(std::ostream* out, const ColumnSpec& columns) {
  // TODO(josh): detect multiline and break it up
  (*out) << meta_.name << " ";
  if (meta_.version.size() > 0) {
    (*out) << "  version " << Join(meta_.version, ".") << "\n";
  }
}

// TODO(josh): this shouldn't be needed, and isn't for clang
template class Optional<bool>;


}  // namespace argue
