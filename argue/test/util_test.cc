// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <gtest/gtest.h>

#include "argue/argue.h"

TEST(StringToNargsTest, CorrectlyParsesExampleQueries) {
  EXPECT_EQ(argue::INVALID_NARGS, argue::StringToNargs("!"));
  EXPECT_EQ(argue::ONE_OR_MORE, argue::StringToNargs("+"));
  EXPECT_EQ(argue::ZERO_OR_MORE, argue::StringToNargs("*"));
  EXPECT_EQ(argue::ZERO_OR_ONE, argue::StringToNargs("?"));
}

TEST(ArgTypeTest, CorrectlyParsesExampleQueries) {
  EXPECT_EQ(argue::SHORT_FLAG, argue::GetArgType("-f"));
  EXPECT_EQ(argue::LONG_FLAG, argue::GetArgType("--foo"));
  EXPECT_EQ(argue::POSITIONAL, argue::GetArgType("foo"));
}
