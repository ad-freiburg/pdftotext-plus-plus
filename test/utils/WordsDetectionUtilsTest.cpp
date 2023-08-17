/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <vector>

#include "../../src/Config.h"
#include "../../src/PdfToTextPlusPlus.h"
#include "../../src/utils/MathUtils.h"
#include "../../src/utils/WordsDetectionUtils.h"

using std::vector;

using ppp::PdfToTextPlusPlus;
using ppp::config::Config;
using ppp::config::WordsDetectionConfig;
using ppp::utils::WordsDetectionUtils;
using ppp::utils::math::round;

// =================================================================================================

// The path to the PDF file to process in the test cases below.
static const char* PDF_FILE_PATH = "./test/pdfs/WordsDetectionUtilsTest.pdf";

// The tolerance to use on comparing two float values.
static const double TOL = ppp::config::DEFAULT_DOUBLE_EQUAL_TOLERANCE;


class WordsDetectionUtilsTest : public ::testing::Test {
 protected:
  // This method is called before the first test case of this test suite is called.
  static void SetUpTestSuite() {
    Config config;
    config.pageSegmentation.disable = true;
    config.textLinesDetection.disable = true;
    config.subSuperScriptsDetection.disable = true;
    config.textBlocksDetection.disable = true;
    config.readingOrderDetection.disable = true;
    config.semanticRolesPrediction.disable = true;
    config.wordsDehyphenation.disable = true;

    PdfToTextPlusPlus engine(config);
    pdf = new PdfDocument();
    engine.process(PDF_FILE_PATH, pdf);
  }

  // This method is called after the last test case of this test suite is called.
  static void TearDownTestSuite() {
    delete pdf;
  }

  static PdfDocument* pdf;
};

PdfDocument* WordsDetectionUtilsTest::pdf = nullptr;

// _________________________________________________________________________________________________
TEST_F(WordsDetectionUtilsTest, createWord) {
  WordsDetectionConfig config;
  WordsDetectionUtils utils(config);

  PdfPage* page = pdf->pages[0];
  PdfCharacter* char1 = page->characters[12];
  PdfCharacter* char2 = page->characters[13];
  PdfCharacter* char3 = page->characters[14];
  PdfCharacter* char4 = page->characters[15];
  PdfCharacter* char5 = page->characters[16];
  PdfCharacter* char6 = page->characters[17];
  PdfCharacter* char7 = page->characters[18];
  PdfCharacter* char8 = page->characters[19];
  PdfCharacter* char9 = page->characters[20];
  PdfCharacter* char10 = page->characters[21];
  PdfCharacter* char11 = page->characters[22];
  PdfCharacter* char12 = page->characters[23];
  PdfCharacter* char13 = page->characters[24];
  PdfCharacter* char14 = page->characters[25];
  PdfCharacter* char15 = page->characters[26];
  PdfCharacter* char16 = page->characters[27];

  // Make sure we selected the correct characters.
  ASSERT_EQ(char1->text, "A");
  ASSERT_EQ(char2->text, "s");
  ASSERT_EQ(char3->text, "h");
  ASSERT_EQ(char4->text, "t");
  ASSERT_EQ(char5->text, "o");
  ASSERT_EQ(char6->text, "n");
  ASSERT_EQ(char7->text, "T");
  ASSERT_EQ(char8->text, "r");
  ASSERT_EQ(char9->text, "a");
  ASSERT_EQ(char10->text, "v");
  ASSERT_EQ(char11->text, "i");
  ASSERT_EQ(char12->text, "s");
  ASSERT_EQ(char13->text, "L");
  ASSERT_EQ(char14->text, "e");
  ASSERT_EQ(char15->text, "v");
  ASSERT_EQ(char16->text, "y");

  vector<PdfCharacter*> characters = { char1, char2, char3, char4, char5, char6 };
  PdfWord* word = utils.createWord(characters);
  ASSERT_TRUE(word->id.size() > static_cast<unsigned int>(0));
  ASSERT_EQ(word->pos->pageNum, 1);
  // TODO(korzen): The coordinates should not be rounded here.
  ASSERT_NEAR(round(word->pos->leftX, 1), 56.7, TOL);
  ASSERT_NEAR(round(word->pos->upperY, 1), 90.8, TOL);
  ASSERT_NEAR(round(word->pos->rightX, 1), 90.7, TOL);
  ASSERT_NEAR(round(word->pos->lowerY, 1), 104.9, TOL);
  ASSERT_NEAR(round(word->pos->getRotLeftX(), 1), 56.7, TOL);
  ASSERT_NEAR(round(word->pos->getRotUpperY(), 1), 90.8, TOL);
  ASSERT_NEAR(round(word->pos->getRotRightX(), 1), 90.7, TOL);
  ASSERT_NEAR(round(word->pos->getRotLowerY(), 1), 104.9, TOL);
  ASSERT_EQ(word->pos->wMode, 0);
  ASSERT_EQ(word->pos->rotation, 0);
  ASSERT_EQ(word->text, "Ashton");
  ASSERT_EQ(word->fontName, "VUTQYG+CMR10");
  ASSERT_NEAR(word->fontSize, 10.9, TOL);
  ASSERT_EQ(word->characters, characters);
  ASSERT_EQ(word->doc, pdf);

  characters = { char7, char8, char9, char10, char11, char12 };
  word = utils.createWord(characters);
  ASSERT_TRUE(word->id.size() > static_cast<unsigned int>(0));
  ASSERT_EQ(word->pos->pageNum, 1);
  // TODO(korzen): The coordinates should not be rounded here.
  ASSERT_NEAR(round(word->pos->leftX, 1), 53.8, TOL);
  ASSERT_NEAR(round(word->pos->upperY, 1), 114.1, TOL);
  ASSERT_NEAR(round(word->pos->rightX, 1), 68, TOL);
  ASSERT_NEAR(round(word->pos->lowerY, 1), 143.6, TOL);
  ASSERT_NEAR(round(word->pos->getRotLeftX(), 1), 143.6, TOL);
  ASSERT_NEAR(round(word->pos->getRotUpperY(), 1), 53.8, TOL);
  ASSERT_NEAR(round(word->pos->getRotRightX(), 1), 114.1, TOL);
  ASSERT_NEAR(round(word->pos->getRotLowerY(), 1), 68, TOL);
  ASSERT_EQ(word->pos->wMode, 0);
  ASSERT_EQ(word->pos->rotation, 3);
  ASSERT_EQ(word->text, "Travis");
  ASSERT_EQ(word->fontName, "VUTQYG+CMR10");
  ASSERT_NEAR(word->fontSize, 10.9, TOL);
  ASSERT_EQ(word->characters, characters);
  ASSERT_EQ(word->doc, pdf);

  characters = { char13, char14, char15, char16 };
  word = utils.createWord(characters);
  ASSERT_TRUE(word->id.size() > static_cast<unsigned int>(0));
  ASSERT_EQ(word->pos->pageNum, 1);
  // TODO(korzen): The coordinates should not be rounded here.
  ASSERT_NEAR(round(word->pos->leftX, 1), 61.9, TOL);
  ASSERT_NEAR(round(word->pos->upperY, 1), 158.3, TOL);
  ASSERT_NEAR(round(word->pos->rightX, 1), 71.8, TOL);
  ASSERT_NEAR(round(word->pos->lowerY, 1), 172.5, TOL);
  ASSERT_NEAR(round(word->pos->getRotLeftX(), 1), 71.8, TOL);
  ASSERT_NEAR(round(word->pos->getRotUpperY(), 1), 172.5, TOL);
  ASSERT_NEAR(round(word->pos->getRotRightX(), 1), 61.9, TOL);
  ASSERT_NEAR(round(word->pos->getRotLowerY(), 1), 158.3, TOL);
  ASSERT_EQ(word->pos->wMode, 0);
  ASSERT_EQ(word->pos->rotation, 2);
  ASSERT_EQ(word->text, "Levy");
  ASSERT_EQ(word->fontName, "AQMQUF+CMSS10");
  ASSERT_NEAR(word->fontSize, 10.9, TOL);
  ASSERT_EQ(word->characters, characters);
  ASSERT_EQ(word->doc, pdf);
}
