/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "../src/utils/StringUtils.h"

// _________________________________________________________________________________________________
TEST(StringUtils, splitStringIntoWords) {
  std::vector<std::string> words1;
  string_utils::splitIntoWords("", &words1);
  ASSERT_EQ(words1.size(), 0);

  std::vector<std::string> words2;
  string_utils::splitIntoWords("foo bar baz", &words2);
  ASSERT_EQ(words2.size(), 3);
  ASSERT_EQ(words2[0], "foo");
  ASSERT_EQ(words2[1], "bar");
  ASSERT_EQ(words2[2], "baz");

  std::vector<std::string> words3;
  string_utils::splitIntoWords("Monday Tuesday\tWednesday\n\nThursday", &words3);
  ASSERT_EQ(words3.size(), 4);
  ASSERT_EQ(words3[0], "Monday");
  ASSERT_EQ(words3[1], "Tuesday");
  ASSERT_EQ(words3[2], "Wednesday");
  ASSERT_EQ(words3[3], "Thursday");
}

// _________________________________________________________________________________________________
TEST(StringUtils, splitWStringIntoWords) {
  std::wstring string1 = L"";
  std::vector<std::wstring> words1;
  string_utils::splitIntoWords(string1, &words1);
  ASSERT_EQ(words1.size(), 0);

  std::wstring string2 = L"foo bar baz";
  std::vector<std::wstring> words2;
  string_utils::splitIntoWords(string2, &words2);
  ASSERT_EQ(words2.size(), 3);
  std::wstring word20 = L"foo";
  ASSERT_EQ(words2[0], word20);
  std::wstring word21 = L"bar";
  ASSERT_EQ(words2[1], word21);
  std::wstring word22 = L"baz";
  ASSERT_EQ(words2[2], word22);

  std::wstring string3 = L"Januar Februar\tMärz\n\nApril";
  std::vector<std::wstring> words3;
  string_utils::splitIntoWords(string3, &words3);
  ASSERT_EQ(words3.size(), 4);
  std::wstring word30 = L"Januar";
  ASSERT_EQ(words3[0], word30);
  std::wstring word31 = L"Februar";
  ASSERT_EQ(words3[1], word31);
  std::wstring word32 = L"März";
  ASSERT_EQ(words3[2], word32);
  std::wstring word33 = L"April";
  ASSERT_EQ(words3[3], word33);
}

// _________________________________________________________________________________________________
TEST(StringUtils, createRandomString) {
  std::string s1 = string_utils::createRandomString(0);
  ASSERT_EQ(s1, "");

  std::string s2 = string_utils::createRandomString(5);
  ASSERT_EQ(s2.size(), 5);

  std::string s3 = string_utils::createRandomString(6, "foo-");
  ASSERT_EQ(s3.size(), 10);
  ASSERT_EQ(s3.find("foo-"), 0);
}

// _________________________________________________________________________________________________
TEST(StringUtils, escapeJson) {
  std::string s1 = string_utils::escapeJson("");
  ASSERT_EQ(s1, "");

  std::string s2 = string_utils::escapeJson("James Bond");
  ASSERT_EQ(s2, "James Bond");

  std::string s3 = string_utils::escapeJson("James\tBond");
  ASSERT_EQ(s3, "James\\tBond");

  std::string s4 = string_utils::escapeJson("James \"Bond\"");
  ASSERT_EQ(s4, "James \\\"Bond\\\"");

  std::string s5 = string_utils::escapeJson("James\t\"Bond\"");
  ASSERT_EQ(s5, "James\\t\\\"Bond\\\"");
}

// _________________________________________________________________________________________________
TEST(StringUtils, shorten) {
  std::string s1 = string_utils::shorten("This is a long text", 0);
  ASSERT_EQ(s1, "...");

  std::string s2 = string_utils::shorten("This is a long text", 4);
  ASSERT_EQ(s2, "This...");

  std::string s4 = string_utils::shorten("This is a long text", 18);
  ASSERT_EQ(s4, "This is a long tex...");

  std::string s5 = string_utils::shorten("This is a long text", 19);
  ASSERT_EQ(s5, "This is a long text");

  std::string s6 = string_utils::shorten("This is a long text", 50);
  ASSERT_EQ(s6, "This is a long text");
}
