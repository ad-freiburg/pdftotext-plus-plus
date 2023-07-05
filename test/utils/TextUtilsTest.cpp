/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "../../src/utils/TextUtils.h"

using std::string;
using std::vector;
using std::wstring;

using ppp::utils::text::createRandomString;
using ppp::utils::text::endsWithSentenceDelimiter;
using ppp::utils::text::escapeJson;
using ppp::utils::text::join;
using ppp::utils::text::shorten;
using ppp::utils::text::splitIntoWords;
using ppp::utils::text::strip;
using ppp::utils::text::wrap;

// =================================================================================================

// _________________________________________________________________________________________________
TEST(TextUtilsTest, splitWStringIntoWords) {
  // Input: empty string.
  wstring string1 = L"";
  vector<wstring> words1;
  splitIntoWords(string1, &words1);
  ASSERT_EQ(words1.size(), static_cast<unsigned int>(0));

  wstring string2 = L"foo bar baz";
  vector<wstring> words2;
  splitIntoWords(string2, &words2);
  ASSERT_EQ(words2.size(), static_cast<unsigned int>(3));
  ASSERT_EQ(words2[0], L"foo");
  ASSERT_EQ(words2[1], L"bar");
  ASSERT_EQ(words2[2], L"baz");

  wstring string3 = L"Januar Februar\tMärz\n\nApril";
  vector<wstring> words3;
  splitIntoWords(string3, &words3);
  ASSERT_EQ(words3.size(), static_cast<unsigned int>(4));
  ASSERT_EQ(words3[0], L"Januar");
  ASSERT_EQ(words3[1], L"Februar");
  ASSERT_EQ(words3[2], L"März");
  ASSERT_EQ(words3[3], L"April");
}

// _________________________________________________________________________________________________
TEST(TextUtilsTest, splitStringIntoWords) {
  // Input: empty string.
  vector<string> words1;
  splitIntoWords("", &words1);
  ASSERT_EQ(words1.size(), static_cast<unsigned int>(0));

  vector<string> words2;
  splitIntoWords("foo bar baz", &words2);
  ASSERT_EQ(words2.size(), static_cast<unsigned int>(3));
  ASSERT_EQ(words2[0], "foo");
  ASSERT_EQ(words2[1], "bar");
  ASSERT_EQ(words2[2], "baz");

  vector<string> words3;
  splitIntoWords("Monday Tuesday\tWednesday\n \nThursday", &words3);
  ASSERT_EQ(words3.size(), static_cast<unsigned int>(4));
  ASSERT_EQ(words3[0], "Monday");
  ASSERT_EQ(words3[1], "Tuesday");
  ASSERT_EQ(words3[2], "Wednesday");
  ASSERT_EQ(words3[3], "Thursday");

  vector<string> words4;
  splitIntoWords("Monday Tuesday\t\tWednesday\n \nThursday\tFriday", &words4, "\t");
  ASSERT_EQ(words4.size(), static_cast<unsigned int>(3));
  ASSERT_EQ(words4[0], "Monday Tuesday");
  ASSERT_EQ(words4[1], "Wednesday\n \nThursday");
  ASSERT_EQ(words4[2], "Friday");

  vector<string> words5;
  splitIntoWords("Monday Tuesday\t\tWednesday\n \nThursday\tFriday", &words5, " \t");
  ASSERT_EQ(words5.size(), static_cast<unsigned int>(5));
  ASSERT_EQ(words5[0], "Monday");
  ASSERT_EQ(words5[1], "Tuesday");
  ASSERT_EQ(words5[2], "Wednesday\n");
  ASSERT_EQ(words5[3], "\nThursday");
  ASSERT_EQ(words5[4], "Friday");
}

// _________________________________________________________________________________________________
TEST(TextUtilsTest, endsWithSentenceDelimiter) {
  ASSERT_FALSE(endsWithSentenceDelimiter(""));
  ASSERT_FALSE(endsWithSentenceDelimiter("The Fantastic Four"));
  ASSERT_FALSE(endsWithSentenceDelimiter("The Fantastic Four:"));
  ASSERT_FALSE(endsWithSentenceDelimiter("\"The Fantastic Four\""));
  ASSERT_TRUE(endsWithSentenceDelimiter("The Fantastic Four."));
  ASSERT_TRUE(endsWithSentenceDelimiter("The Fantastic Four?"));
  ASSERT_TRUE(endsWithSentenceDelimiter("The Fantastic Four!"));
  ASSERT_TRUE(endsWithSentenceDelimiter("The Fantastic Four.", ".!"));
  ASSERT_TRUE(endsWithSentenceDelimiter("The Fantastic Four!", ".!"));
  ASSERT_FALSE(endsWithSentenceDelimiter("The Fantastic Four?", ".!"));
}

// _________________________________________________________________________________________________
TEST(TextUtilsTest, createRandomString) {
  string s1 = createRandomString(0);
  ASSERT_EQ(s1, "");

  string s2 = createRandomString(0, "foo-");
  ASSERT_EQ(s2, "foo-");

  string s3 = createRandomString(5);
  ASSERT_EQ(s3.size(), static_cast<unsigned int>(5));

  string s4 = createRandomString(6, "foo-");
  ASSERT_EQ(s4.size(), static_cast<unsigned int>(10));
  ASSERT_TRUE(s4.starts_with("foo-"));
}

// _________________________________________________________________________________________________
TEST(TextUtilsTest, escapeJson) {
  string s1 = escapeJson("");
  ASSERT_EQ(s1, "");

  string s2 = escapeJson("James Bond");
  ASSERT_EQ(s2, "James Bond");

  string s3 = escapeJson("James\tBond");
  ASSERT_EQ(s3, "James\\tBond");

  string s4 = escapeJson("James \"Bond\"");
  ASSERT_EQ(s4, "James \\\"Bond\\\"");

  string s5 = escapeJson("James\t\"Bond\"");
  ASSERT_EQ(s5, "James\\t\\\"Bond\\\"");
}

// _________________________________________________________________________________________________
TEST(TextUtilsTest, shorten) {
  string s1 = shorten("", 0);
  ASSERT_EQ(s1, "");

  string s2 = shorten("", 12);
  ASSERT_EQ(s2, "");

  string s3 = shorten("This is a long text", 0);
  ASSERT_EQ(s3, "...");

  string s4 = shorten("This is a long text", 4);
  ASSERT_EQ(s4, "This...");

  string s5 = shorten("This is a long text", 18);
  ASSERT_EQ(s5, "This is a long tex...");

  string s6 = shorten("This is a long text", 19);
  ASSERT_EQ(s6, "This is a long text");

  string s7 = shorten("This is a long text", 50);
  ASSERT_EQ(s7, "This is a long text");
}

// _________________________________________________________________________________________________
TEST(TextUtilsTest, strip) {
  string s1 = strip("");
  ASSERT_EQ(s1, "");

  string s2 = strip("Washington");
  ASSERT_EQ(s2, "Washington");

  string s3 = strip("Washington  ");
  ASSERT_EQ(s3, "Washington");

  string s4 = strip("  Washington");
  ASSERT_EQ(s4, "Washington");

  string s5 = strip("\t Washington\t \n ");
  ASSERT_EQ(s5, "Washington");

  string s6 = strip("\tNew York  ");
  ASSERT_EQ(s6, "New York");
}

// _________________________________________________________________________________________________
TEST(TextUtilsTest, wrap) {
  string s1 = wrap("", 100, 0);
  ASSERT_EQ(s1, "");

  string s2 = wrap("", 100, 3);
  ASSERT_EQ(s2, "   ");

  string s3 = wrap("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam.", 20, 0);
  ASSERT_EQ(s3, "Lorem ipsum dolor\nsit amet, consetetur\nsadipscing elitr,\nsed diam.");

  string s4 = wrap("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam.", 35, 3);
  ASSERT_EQ(s4, "   Lorem ipsum dolor sit amet,\n   consetetur sadipscing elitr, sed\n   diam.");
}

// _________________________________________________________________________________________________
TEST(TextUtilsTest, join) {
  string s1 = join({}, ", ");
  ASSERT_EQ(s1, "");

  string s2 = join({ "one", "two", "three" }, "");
  ASSERT_EQ(s2, "onetwothree");

  string s3 = join({ "one", "two", "three" }, ",");
  ASSERT_EQ(s3, "one,two,three");

  string s4 = join({ "one", "two", "three" }, "+-");
  ASSERT_EQ(s4, "one+-two+-three");

  string s5 = join({ "one", "two", "three" }, " ");
  ASSERT_EQ(s5, "one two three");
}
