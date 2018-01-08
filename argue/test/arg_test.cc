// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <gtest/gtest.h>

#include "argue/argue.h"

void ResetParser(argue::Parser* parser, const argue::Metadata& meta) {
  using argue::Parser;
  (parser)->~Parser();
  new (parser) Parser(meta);
}

void ResetParser(argue::Parser* parser) {
  using argue::Parser;
  (parser)->~Parser();
  new (parser) Parser();
}

TEST(StoreTest, StoreScalar) {
  int foo = 0;
  argue::Parser parser;

  // Expect failure if we have too few args, manditory positional argument
  // remains
  ResetParser(&parser);
  parser.AddArgument("foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({}));
  EXPECT_EQ(0, foo);

  // Expect failure if we have too many args, which ensures that the parser
  // does not consume the remaining args.
  ResetParser(&parser);
  parser.AddArgument("foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({"1", "2"}));
  EXPECT_EQ(1, foo);

  // Expect failure if we have too many args, which ensures that the parser
  // does not consume the remaining args.
  ResetParser(&parser);
  parser.AddArgument("foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"1"}));
  EXPECT_EQ(1, foo);

  // Flags default optional so an empty args list should be OK
  ResetParser(&parser);
  parser.AddArgument("-f", "--foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({}));
  EXPECT_EQ(0, foo);

  // Expect failure if we have too many args, which ensures that the parser
  // does not consume the remaining args.
  ResetParser(&parser);
  parser.AddArgument("-f", "--foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({"1", "2"}));
  EXPECT_EQ(0, foo);

  ResetParser(&parser);
  parser.AddArgument("-f", "--foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"-f", "1"}));
  EXPECT_EQ(1, foo);

  ResetParser(&parser);
  parser.AddArgument("-f", "--foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"--foo", "1"}));
  EXPECT_EQ(1, foo);

  // Ensure that flag deduction works correctly
  ResetParser(&parser);
  parser.AddArgument("-f", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"-f", "1"}));
  EXPECT_EQ(1, foo);

  ResetParser(&parser);
  parser.AddArgument("--foo", &foo, {});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"--foo", "1"}));
  EXPECT_EQ(1, foo);

  // If argument is optional then parse should not fail on empty string
  ResetParser(&parser);
  parser.AddArgument("foo", &foo,
                     {.action_ = "store", .nargs_ = argue::ZERO_OR_ONE});
  foo = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({}));
  EXPECT_EQ(0, foo);
}

TEST(StoreTest, StoreTypes) {
  argue::Parser parser;

  ResetParser(&parser);
  int32_t i32_foo = 0;
  parser.AddArgument("foo", &i32_foo, {});
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"123"}));
  EXPECT_EQ(123, i32_foo);

  ResetParser(&parser);
  uint32_t u32_foo = 0;
  parser.AddArgument("foo", &u32_foo, {});
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"123"}));
  EXPECT_EQ(123, u32_foo);

  ResetParser(&parser);
  float f32_foo = 0;
  parser.AddArgument("foo", &f32_foo, {});
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"123"}));
  EXPECT_EQ(123, f32_foo);

  ResetParser(&parser);
  double f64_foo = 0;
  parser.AddArgument("foo", &f64_foo, {});
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"123"}));
  EXPECT_EQ(123, f64_foo);

  ResetParser(&parser);
  std::string str_foo = "hello";
  parser.AddArgument("foo", &str_foo, {});
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"123"}));
  EXPECT_EQ("123", str_foo);
}

TEST(StoreTest, StoreOneOrMore) {
  std::list<int> container;
  std::list<int> expected;
  const std::list<int> empty_list;
  argue::Parser parser;

  // An empty argument list should fail if the requirement is one or more
  ResetParser(&parser);
  parser.AddArgument("foo", &container,
                     {.action_ = "store", .nargs_ = argue::ONE_OR_MORE});
  container = {};
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({}));
  EXPECT_EQ(empty_list, container);

  ResetParser(&parser);
  parser.AddArgument("foo", &container,
                     {.action_ = "store", .nargs_ = argue::ONE_OR_MORE});
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"1"}));
  expected = {1};
  EXPECT_EQ(expected, container);

  ResetParser(&parser);
  parser.AddArgument("foo", &container,
                     {.action_ = "store", .nargs_ = argue::ONE_OR_MORE});
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"1", "2", "3"}));
  expected = {1, 2, 3};
  EXPECT_EQ(expected, container);
}

TEST(StoreTest, StoreZeroOrMore) {
  std::list<int> container;
  std::list<int> expected;
  const std::list<int> empty_list;
  argue::Parser parser;

  // An empty argument list is allowed if specification is for zero or more
  ResetParser(&parser);
  parser.AddArgument("foo", &container,
                     {.action_ = "store", .nargs_ = argue::ZERO_OR_MORE});
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({}));
  EXPECT_EQ(empty_list, container);

  ResetParser(&parser);
  parser.AddArgument("foo", &container,
                     {.action_ = "store", .nargs_ = argue::ZERO_OR_MORE});
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"1"}));
  expected = {1};
  EXPECT_EQ(expected, container);

  ResetParser(&parser);
  parser.AddArgument("foo", &container,
                     {.action_ = "store", .nargs_ = argue::ZERO_OR_MORE});
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"1", "2", "3"}));
  expected = {1, 2, 3};
  EXPECT_EQ(expected, container);
}

TEST(StoreTest, StoreFixedSize) {
  int dummy;
  std::list<int> container;
  std::list<int> expected;
  const std::list<int> empty_list;
  argue::Parser parser;

  // An empty argument list is allowed if specification is for zero or more
  ResetParser(&parser);
  parser.AddArgument("foo", &container, {.action_ = "store", .nargs_ = 0});
  container = {};
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({}));
  EXPECT_EQ(empty_list, container);

  ResetParser(&parser);
  parser.AddArgument("foo", &container, {.action_ = "store", .nargs_ = 1});
  container = {};
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"1"}));
  expected = {1};
  EXPECT_EQ(expected, container);

  ResetParser(&parser);
  parser.AddArgument("foo", &container, {.action_ = "store", .nargs_ = 1});
  parser.AddArgument("bar", &dummy, {});
  container = {};
  dummy = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"1", "2"}));
  expected = {1};
  EXPECT_EQ(expected, container);
  EXPECT_EQ(2, dummy);

  ResetParser(&parser);
  parser.AddArgument("foo", &container, {.action_ = "store", .nargs_ = 3});
  parser.AddArgument("bar", &dummy, {});
  container = {};
  dummy = 0;
  EXPECT_EQ(argue::PARSE_FINISHED, parser.ParseArgs({"1", "2", "3", "4"}));
  expected = {1, 2, 3};
  EXPECT_EQ(expected, container);
  EXPECT_EQ(4, dummy);

  ResetParser(&parser);
  parser.AddArgument("foo", &container, {.action_ = "store", .nargs_ = 4});
  container = {};
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({"1", "2"}));
  expected = {1, 2};
  EXPECT_EQ(expected, container);
}

TEST(HelpTest, HelpIsDefault) {
  std::stringstream logout;
  argue::Parser parser;
  ResetParser(&parser);
  EXPECT_EQ(argue::PARSE_ABORTED, parser.ParseArgs({"--help"}, &logout));
  ResetParser(&parser);
  EXPECT_EQ(argue::PARSE_ABORTED, parser.ParseArgs({"-h"}, &logout));

  ResetParser(&parser, {.add_help = false});
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({"--help"}, &logout));
  ResetParser(&parser, {.add_help = false});
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({"-h"}, &logout));
}

TEST(VersionTest, VersionIsDefault) {
  std::stringstream logout;
  argue::Parser parser;
  ResetParser(&parser);
  EXPECT_EQ(argue::PARSE_ABORTED, parser.ParseArgs({"--version"}, &logout));
  ResetParser(&parser);
  EXPECT_EQ(argue::PARSE_ABORTED, parser.ParseArgs({"-v"}, &logout));

  ResetParser(&parser, {.add_help = true, .add_version = false});
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({"--version"}, &logout));
  ResetParser(&parser, {.add_help = true, .add_version = false});
  EXPECT_EQ(argue::PARSE_EXCEPTION, parser.ParseArgs({"-v"}, &logout));
}
