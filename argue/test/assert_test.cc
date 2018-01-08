// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <gtest/gtest.h>

#include "argue/argue.h"

TEST(AssertTest, TypeTagsThrowCorrectExceptionType) {
  bool there_was_an_exception = false;

  try {
    ARGUE_ASSERT(BUG, true) << "Hello!";
  } catch (...) {
    there_was_an_exception = true;
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_FALSE(there_was_an_exception);

  there_was_an_exception = false;
  try {
    ARGUE_ASSERT(BUG, false, "Hello World");
  } catch (const argue::BugException& ex) {
    there_was_an_exception = true;
    EXPECT_EQ("Hello World", ex.message);
  } catch (...) {
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_TRUE(there_was_an_exception);

  there_was_an_exception = false;
  try {
    ARGUE_ASSERT(CONFIG_ERROR, false, "Hello World");
  } catch (const argue::ConfigException& ex) {
    there_was_an_exception = true;
    EXPECT_EQ("Hello World", ex.message);
  } catch (...) {
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_TRUE(there_was_an_exception);

  there_was_an_exception = false;
  try {
    ARGUE_ASSERT(INPUT_ERROR, false, "Hello World");
  } catch (const argue::InputException& ex) {
    there_was_an_exception = true;
    EXPECT_EQ("Hello World", ex.message);
  } catch (...) {
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_TRUE(there_was_an_exception);
}

TEST(AssertTest, AllMessageMechanismsWork) {
  bool there_was_an_exception = false;

  there_was_an_exception = false;
  try {
    ARGUE_ASSERT(CONFIG_ERROR, false, "Hello World:42");
  } catch (const argue::ConfigException& ex) {
    there_was_an_exception = true;
    EXPECT_EQ("Hello World:42", ex.message);
  } catch (...) {
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_TRUE(there_was_an_exception);

  there_was_an_exception = false;
  try {
    ARGUE_ASSERT(CONFIG_ERROR, false, "Hello %s:%d", "World", 42);
  } catch (const argue::ConfigException& ex) {
    there_was_an_exception = true;
    EXPECT_EQ("Hello World:42", ex.message);
  } catch (...) {
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_TRUE(there_was_an_exception);

  there_was_an_exception = false;
  try {
    ARGUE_ASSERT(CONFIG_ERROR, false, "Hello") << " World:" << 42;
  } catch (const argue::ConfigException& ex) {
    there_was_an_exception = true;
    EXPECT_EQ("Hello World:42", ex.message);
  } catch (...) {
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_TRUE(there_was_an_exception);

  there_was_an_exception = false;
  try {
    ARGUE_ASSERT(CONFIG_ERROR, false) << "Hello World:" << 42;
  } catch (const argue::ConfigException& ex) {
    there_was_an_exception = true;
    EXPECT_EQ("Hello World:42", ex.message);
  } catch (...) {
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_TRUE(there_was_an_exception);
}

void Baz() {
  ARGUE_ASSERT(BUG, false) << "Hello!";
}

void Bar() {
  Baz();
}

void Foo() {
  Bar();
}

TEST(AssertTest, BugHasStackTrace) {
  bool there_was_an_exception = false;
  try {
    Foo();
  } catch (const argue::BugException& ex) {
    there_was_an_exception = true;
    EXPECT_EQ("Hello!", ex.message);
    ASSERT_LT(3, ex.stack_trace.size());
    EXPECT_EQ("Baz()", ex.stack_trace[0].name);
    EXPECT_EQ("Bar()", ex.stack_trace[1].name);
    EXPECT_EQ("Foo()", ex.stack_trace[2].name);
  } catch (...) {
    EXPECT_TRUE(false) << "Exception should have been caught before here";
  }
  EXPECT_TRUE(there_was_an_exception);
}

bool ReturnsTrue(int a, int b, int c) {
  return true;
}

template <typename T1, typename T2, typename T3>
bool TReturnsTrue(T1 a, T2 b, T3 c) {
  return true;
}

TEST(AssertTest, MacroTest) {
  ARGUE_ASSERT(BUG, (1 < 2), "1 >= 2??");
  ARGUE_ASSERT(BUG, (1 < 2)) << "1 >= 2??";
  ARGUE_ASSERT(BUG, (1 < 2), "%d >= %d?", 1, 2);
  ARGUE_ASSERT(BUG, ReturnsTrue(1, 2, 3)) << "Unexpected!";
  ARGUE_ASSERT(BUG, TReturnsTrue<int, int, int>(1, 2, 3)) << "Unexpected!";
}
