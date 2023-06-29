/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <vector>

#include "../../src/utils/Comparators.h"
#include "../../src/PdfDocument.h"

using std::vector;

using ppp::utils::comparators::LeftXAscComparator;
using ppp::utils::comparators::RightXDescComparator;
using ppp::utils::comparators::RotLeftXAscComparator;
using ppp::utils::comparators::RotLeftXDescComparator;
using ppp::utils::comparators::RotLowerYAscComparator;
using ppp::utils::comparators::RotLowerYDescComparator;
using ppp::utils::comparators::UpperYAscComparator;

static PdfCharacter char0(1, 2.0, 4.1, 2.5, 7.8, 0, 0);
static PdfCharacter char1(1, 1.7, 7.5, 8.1, 7.7, 0, 0);
static PdfCharacter char2(1, 5.0, 1.2, 7.2, 9.2, 0, 0);
static PdfCharacter char3(1, 3.2, 4.5, 5.1, 5.5, 0, 0);
static PdfCharacter char4(1, 3.0, 2.0, 4.7, 3.1, 0, 0);
static vector<PdfCharacter*> characters = { &char0, &char1, &char2, &char3, &char4 };

static PdfWord word0(1, 4.8, 2.0, 9.9, 2.7, 1, 0);
static PdfWord word1(1, 5.2, 3.1, 5.5, 8.1, 1, 0);
static PdfWord word2(1, 1.0, 5.5, 7.2, 5.9, 1, 0);
static PdfWord word3(1, 3.3, 6.2, 6.5, 9.0, 1, 0);
static vector<PdfWord*> words = { &word0, &word1, &word2, &word3 };

// =================================================================================================

// _________________________________________________________________________________________________
TEST(Comparators, LeftXAscComparator) {
  // Sort the characters by their leftX values in ascending order.
  std::sort(characters.begin(), characters.end(), LeftXAscComparator());
  ASSERT_EQ(characters[0], &char1);
  ASSERT_EQ(characters[1], &char0);
  ASSERT_EQ(characters[2], &char4);
  ASSERT_EQ(characters[3], &char3);
  ASSERT_EQ(characters[4], &char2);

  // Sort the words by their leftX values in ascending order.
  std::sort(words.begin(), words.end(), LeftXAscComparator());
  ASSERT_EQ(words[0], &word2);
  ASSERT_EQ(words[1], &word3);
  ASSERT_EQ(words[2], &word0);
  ASSERT_EQ(words[3], &word1);
}

// _________________________________________________________________________________________________
TEST(Comparators, RightXDescComparator) {
  // Sort the characters by their rightX values in descending order.
  std::sort(characters.begin(), characters.end(), RightXDescComparator());
  ASSERT_EQ(characters[0], &char1);
  ASSERT_EQ(characters[1], &char2);
  ASSERT_EQ(characters[2], &char3);
  ASSERT_EQ(characters[3], &char4);
  ASSERT_EQ(characters[4], &char0);

  // Sort the words by their rightX values in descending order.
  std::sort(words.begin(), words.end(), RightXDescComparator());
  ASSERT_EQ(words[0], &word0);
  ASSERT_EQ(words[1], &word2);
  ASSERT_EQ(words[2], &word3);
  ASSERT_EQ(words[3], &word1);
}

// _________________________________________________________________________________________________
TEST(Comparators, UpperYAscComparator) {
  // Sort the characters by their upperY values in ascending order.
  std::sort(characters.begin(), characters.end(), UpperYAscComparator());
  ASSERT_EQ(characters[0], &char2);
  ASSERT_EQ(characters[1], &char4);
  ASSERT_EQ(characters[2], &char0);
  ASSERT_EQ(characters[3], &char3);
  ASSERT_EQ(characters[4], &char1);

  // Sort the words by their upperY values in ascending order.
  std::sort(words.begin(), words.end(), UpperYAscComparator());
  ASSERT_EQ(words[0], &word0);
  ASSERT_EQ(words[1], &word1);
  ASSERT_EQ(words[2], &word2);
  ASSERT_EQ(words[3], &word3);
}

// _________________________________________________________________________________________________
TEST(Comparators, RotLeftXAscComparator) {
  // Sort the characters by their rotLeftX values in ascending order.
  std::sort(characters.begin(), characters.end(), RotLeftXAscComparator());
  ASSERT_EQ(characters[0], &char1);
  ASSERT_EQ(characters[1], &char0);
  ASSERT_EQ(characters[2], &char4);
  ASSERT_EQ(characters[3], &char3);
  ASSERT_EQ(characters[4], &char2);

  // Sort the words by their rotLeftX values in ascending order.
  std::sort(words.begin(), words.end(), RotLeftXAscComparator());
  ASSERT_EQ(words[0], &word0);
  ASSERT_EQ(words[1], &word1);
  ASSERT_EQ(words[2], &word2);
  ASSERT_EQ(words[3], &word3);
}

// _________________________________________________________________________________________________
TEST(Comparators, RotLeftXDescComparator) {
  // Sort the characters by their rotLeftX values in descending order.
  std::sort(characters.begin(), characters.end(), RotLeftXDescComparator());
  ASSERT_EQ(characters[0], &char2);
  ASSERT_EQ(characters[1], &char3);
  ASSERT_EQ(characters[2], &char4);
  ASSERT_EQ(characters[3], &char0);
  ASSERT_EQ(characters[4], &char1);

  // Sort the words by their rotLeftX values in descending order.
  std::sort(words.begin(), words.end(), RotLeftXDescComparator());
  ASSERT_EQ(words[0], &word3);
  ASSERT_EQ(words[1], &word2);
  ASSERT_EQ(words[2], &word1);
  ASSERT_EQ(words[3], &word0);
}

// _________________________________________________________________________________________________
TEST(Comparators, RotLowerYAscComparator) {
  // Sort the characters by their rotLowerY values in ascending order.
  std::sort(characters.begin(), characters.end(), RotLowerYAscComparator());
  ASSERT_EQ(characters[0], &char4);
  ASSERT_EQ(characters[1], &char3);
  ASSERT_EQ(characters[2], &char1);
  ASSERT_EQ(characters[3], &char0);
  ASSERT_EQ(characters[4], &char2);

  // Sort the words by their rotLowerY values in ascending order.
  std::sort(words.begin(), words.end(), RotLowerYAscComparator());
  ASSERT_EQ(words[0], &word2);
  ASSERT_EQ(words[1], &word3);
  ASSERT_EQ(words[2], &word0);
  ASSERT_EQ(words[3], &word1);
}

// _________________________________________________________________________________________________
TEST(Comparators, RotLowerYDescComparator) {
  // Sort the characters by their rotLowerY values in descending order.
  std::sort(characters.begin(), characters.end(), RotLowerYDescComparator());
  ASSERT_EQ(characters[0], &char2);
  ASSERT_EQ(characters[1], &char0);
  ASSERT_EQ(characters[2], &char1);
  ASSERT_EQ(characters[3], &char3);
  ASSERT_EQ(characters[4], &char4);

  // Sort the words by their rotLowerY values in descending order.
  std::sort(words.begin(), words.end(), RotLowerYDescComparator());
  ASSERT_EQ(words[0], &word1);
  ASSERT_EQ(words[1], &word0);
  ASSERT_EQ(words[2], &word3);
  ASSERT_EQ(words[3], &word2);
}
