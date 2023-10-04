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

#include "../../src/Config.h"
#include "../../src/Types.h"
#include "../../src/PdfToTextPlusPlus.h"
#include "../../src/utils/MathUtils.h"
#include "../../src/utils/WordsDetectionUtils.h"

using std::string;
using std::vector;

using ppp::PdfToTextPlusPlus;
using ppp::config::Config;
using ppp::config::WordsDetectionConfig;
using ppp::types::PdfCharacter;
using ppp::types::PdfDocument;
using ppp::types::PdfPage;
using ppp::types::PdfWord;
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
    config.pageSegmentation.disabled = true;
    config.textLinesDetection.disabled = true;
    config.subSuperScriptsDetection.disabled = true;
    config.textBlocksDetection.disabled = true;
    config.readingOrderDetection.disabled = true;
    config.semanticRolesPrediction.disabled = true;
    config.wordsDehyphenation.disabled = true;

    PdfToTextPlusPlus engine(&config);
    pdf = new PdfDocument();
    pdf->pdfFilePath = PDF_FILE_PATH;
    engine.process(pdf);
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
  WordsDetectionUtils utils(&config);

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
  ASSERT_NEAR(word->pos->leftX, 56.7, TOL);
  ASSERT_NEAR(word->pos->upperY, 90.8, TOL);
  ASSERT_NEAR(word->pos->rightX, 90.7, TOL);
  ASSERT_NEAR(word->pos->lowerY, 104.9, TOL);
  ASSERT_NEAR(word->pos->getRotLeftX(), 56.7, TOL);
  ASSERT_NEAR(word->pos->getRotUpperY(), 90.8, TOL);
  ASSERT_NEAR(word->pos->getRotRightX(), 90.7, TOL);
  ASSERT_NEAR(word->pos->getRotLowerY(), 104.9, TOL);
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
  ASSERT_NEAR(word->pos->leftX, 53.8, TOL);
  ASSERT_NEAR(word->pos->upperY, 114.1, TOL);
  ASSERT_NEAR(word->pos->rightX, 68, TOL);
  ASSERT_NEAR(word->pos->lowerY, 143.6, TOL);
  ASSERT_NEAR(word->pos->getRotLeftX(), 143.6, TOL);
  ASSERT_NEAR(word->pos->getRotUpperY(), 53.8, TOL);
  ASSERT_NEAR(word->pos->getRotRightX(), 114.1, TOL);
  ASSERT_NEAR(word->pos->getRotLowerY(), 68, TOL);
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
  ASSERT_NEAR(word->pos->leftX, 61.9, TOL);
  ASSERT_NEAR(word->pos->upperY, 158.3, TOL);
  ASSERT_NEAR(word->pos->rightX, 71.8, TOL);
  ASSERT_NEAR(word->pos->lowerY, 172.5, TOL);
  ASSERT_NEAR(word->pos->getRotLeftX(), 71.8, TOL);
  ASSERT_NEAR(word->pos->getRotUpperY(), 172.5, TOL);
  ASSERT_NEAR(word->pos->getRotRightX(), 61.9, TOL);
  ASSERT_NEAR(word->pos->getRotLowerY(), 158.3, TOL);
  ASSERT_EQ(word->pos->wMode, 0);
  ASSERT_EQ(word->pos->rotation, 2);
  ASSERT_EQ(word->text, "Levy");
  ASSERT_EQ(word->fontName, "AQMQUF+CMSS10");
  ASSERT_NEAR(word->fontSize, 10.9, TOL);
  ASSERT_EQ(word->characters, characters);
  ASSERT_EQ(word->doc, pdf);
}
