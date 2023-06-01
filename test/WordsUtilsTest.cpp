/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <iostream>
#include <vector>

#include "../src/Constants.h"
#include "../src/PdfToTextPlusPlus.h"
#include "../src/utils/WordsUtils.h"

using words_utils::createWord;

// The allowed tolerance on comparing two float values.
const double TOL = 0.1;

// _________________________________________________________________________________________________
class WordsUtilsTest : public ::testing::Test {
 protected:
  // This method is called before the first test of this test suite.
  static void SetUpTestSuite() {
    PdfToTextPlusPlus engine;

    if (pdf1 == nullptr) {
      pdf1 = new PdfDocument();
      engine.process("./test/pdfs/1-article-two-columns.pdf", pdf1);
    }

    if (pdf2 == nullptr) {
      pdf2 = new PdfDocument();
      engine.process("./test/pdfs/2-article-one-column.pdf", pdf2);
    }
  }

  // This method is called after the last test of this test suite.
  static void TearDownTestSuite() {
    delete pdf1;
    pdf1 = nullptr;
    delete pdf2;
    pdf2 = nullptr;
  }

  static PdfDocument* pdf1;
  static PdfDocument* pdf2;
};

PdfDocument* WordsUtilsTest::pdf1 = nullptr;
PdfDocument* WordsUtilsTest::pdf2 = nullptr;

// _________________________________________________________________________________________________
TEST_F(WordsUtilsTest, createWordPdf1) {
  PdfPage* page0 = pdf1->pages[0];

  // Test a word composed from the characters of "Introduction" (in the first line).
  std::vector<PdfCharacter*> characters;
  for (size_t i = 1; i < 13; i++) { characters.push_back(page0->characters[i]); }
  PdfWord* word = createWord(characters, pdf1);
  ASSERT_EQ(word->doc, pdf1);
  ASSERT_EQ(word->id.size(), size_t(global_config::ID_LENGTH + 5));  // +5 for "word-"
  ASSERT_EQ(word->pos->pageNum, page0->pageNum);
  ASSERT_NEAR(word->pos->leftX, 96.2, TOL);
  ASSERT_NEAR(word->pos->rightX, 185.0, TOL);
  ASSERT_NEAR(word->pos->upperY, 121.1, TOL);
  ASSERT_NEAR(word->pos->lowerY, 139.8, TOL);
  ASSERT_EQ(word->pos->rotation, 0);
  ASSERT_EQ(word->pos->wMode, 0);
  ASSERT_EQ(word->text, "Introduction");
  ASSERT_EQ(word->fontName, "MPAGEP+CMBX12");
  ASSERT_NEAR(word->fontSize, 14.3, TOL);
  ASSERT_EQ(word->characters, characters);

  // Test a word composed from the characters of "ipsum" (in the second line).
  characters.clear();
  for (size_t i = 18; i < 23; i++) { characters.push_back(page0->characters[i]); }
  word = createWord(characters, pdf1);
  ASSERT_EQ(word->doc, pdf1);
  ASSERT_EQ(word->id.size(), size_t(global_config::ID_LENGTH + 5));  // +5 for "word-"
  ASSERT_EQ(word->pos->pageNum, page0->pageNum);
  ASSERT_NEAR(word->pos->leftX, 103.8, TOL);
  ASSERT_NEAR(word->pos->rightX, 129.9, TOL);
  ASSERT_NEAR(word->pos->upperY, 147.1, TOL);
  ASSERT_NEAR(word->pos->lowerY, 160.1, TOL);
  ASSERT_EQ(word->pos->rotation, 0);
  ASSERT_EQ(word->pos->wMode, 0);
  ASSERT_EQ(word->text, "ipsum");
  ASSERT_EQ(word->fontName, "SEUDFQ+CMR10");
  ASSERT_NEAR(word->fontSize, 10, TOL);
  ASSERT_EQ(word->characters, characters);
}
