/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <string>
#include <unordered_set>
#include <vector>

#include "../../src/Config.h"
#include "../../src/Types.h"
#include "../../src/PdfToTextPlusPlus.h"
#include "../../src/utils/MathUtils.h"
#include "../../src/utils/TextBlocksDetectionUtils.h"

using std::string;
using std::unordered_set;
using std::vector;

using ppp::PdfToTextPlusPlus;
using ppp::config::Config;
using ppp::config::TextBlocksDetectionConfig;
using ppp::types::PdfCharacter;
using ppp::types::PdfDocument;
using ppp::types::PdfFigure;
using ppp::types::PdfPage;
using ppp::types::PdfTextBlock;
using ppp::types::PdfTextLine;
using ppp::utils::TextBlocksDetectionUtils;
using ppp::utils::math::round;

// =================================================================================================

// The path to the PDF file to process in the test cases below.
static const char* PDF_FILE_PATH = "./test/pdfs/TextBlocksDetectionUtilsTest.pdf";

// The tolerance to use on comparing two float values.
static const double TOL = ppp::config::DEFAULT_DOUBLE_EQUAL_TOLERANCE;


class TextBlocksDetectionUtilsTest : public ::testing::Test {
 protected:
  // This method is called before the first test case of this test suite is called.
  static void SetUpTestSuite() {
    Config config;
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

PdfDocument* TextBlocksDetectionUtilsTest::pdf = nullptr;

// _________________________________________________________________________________________________
TEST_F(TextBlocksDetectionUtilsTest, computeIsCentered) {
  PdfPage* page = pdf->pages[0];
  PdfTextLine* line1 = page->textLines[1];
  PdfTextLine* line2 = page->textLines[2];
  PdfTextLine* line3 = page->textLines[3];
  PdfTextLine* line4 = page->textLines[4];
  PdfTextLine* line5 = page->textLines[5];

  // Make sure we selected the correct text lines.
  ASSERT_EQ(line1->text, "When nothing is going right, go left.");
  ASSERT_EQ(line2->text, "If Cinderellas shoe fit perfectly, then why did it fall off?");
  ASSERT_EQ(line3->text, "My wallet is like an onion, opening it makes me cry.");
  ASSERT_EQ(line4->text, "Every day and night.");
  ASSERT_EQ(line5->text, "Lottery: a tax on people who are bad at math.");

  TextBlocksDetectionConfig config;
  TextBlocksDetectionUtils utils(&config);

  // Input: one (or more) nullptrs.
  ASSERT_DEATH(utils.computeIsCentered(nullptr, nullptr), "");
  ASSERT_DEATH(utils.computeIsCentered(line1, nullptr), "");
  ASSERT_DEATH(utils.computeIsCentered(nullptr, line1), "");

  // Input: two text lines that do not overlap horizontally.
  ASSERT_FALSE(utils.computeIsCentered(line1, line2));

  // Input: two text lines that partially overlap horizontally.
  ASSERT_FALSE(utils.computeIsCentered(line1, line3));

  // Input: two text lines, with one text line being completely overlapped by the other text line,
  // but the leftX offset and the rightX offset being *not* equal.
  ASSERT_FALSE(utils.computeIsCentered(line1, line4));

  // Input: two text lines, with one text line being completely overlapped by the other text line,
  // and the leftX offset and the rightX offset being equal.
  ASSERT_TRUE(utils.computeIsCentered(line1, line1));
  ASSERT_TRUE(utils.computeIsCentered(line3, line5));
  ASSERT_TRUE(utils.computeIsCentered(line5, line3));
}

// _________________________________________________________________________________________________
TEST_F(TextBlocksDetectionUtilsTest, computeIsTextLinesCentered) {
  PdfPage* page = pdf->pages[1];
  PdfTextBlock* block1 = page->blocks[1];
  PdfTextBlock* block2 = page->blocks[2];
  PdfTextBlock* block3 = page->blocks[3];
  PdfTextBlock* block4 = page->blocks[4];
  PdfTextBlock* block5 = page->blocks[5];
  // FIXME(korzen): Text block detection for ragged-right text lines is currently broken (block6
  // is divided into multiple text blocks). Fix it.
  // PdfTextBlock* block6 = page->blocks[6];

  // Make sure we selected the correct text blocks.
  ASSERT_TRUE(block1->text.starts_with("Say goodbye to mundane cleaning tasks"));
  ASSERT_TRUE(block1->text.ends_with("been waiting for."));
  ASSERT_TRUE(block2->text.starts_with("We understand that every home is unique"));
  ASSERT_TRUE(block2->text.ends_with("ready for the next cleaning session."));
  ASSERT_TRUE(block3->text.starts_with("Our robot vacuum cleaner"));
  ASSERT_TRUE(block3->text.ends_with("Buy it and have fun!"));
  ASSERT_TRUE(block4->text.starts_with("Join the revolution"));
  ASSERT_TRUE(block4->text.ends_with("leave you amazed!"));
  ASSERT_TRUE(block5->text.starts_with("Welcome to the next generation"));
  ASSERT_TRUE(block5->text.ends_with("cleaner than ever before."));
  // ASSERT_TRUE(block6->text.starts_with("Experience a new level of smart cleaning"));
  // ASSERT_TRUE(block6->text.ends_with("ensuring no spot is left untouched."));

  TextBlocksDetectionConfig config;
  TextBlocksDetectionUtils utils(&config);

  // Input: nullptr.
  ASSERT_DEATH(utils.computeIsTextLinesCentered(nullptr), "");

  // Input: Text block with left-aligned text lines.
  ASSERT_FALSE(utils.computeIsTextLinesCentered(block1));

  // Input: Text block with right-aligned text lines.
  ASSERT_FALSE(utils.computeIsTextLinesCentered(block2));

  // Input: Text block with justified text lines + the last line does *not* consume the full width.
  ASSERT_FALSE(utils.computeIsTextLinesCentered(block3));

  // Input: Text block with justified text lines + the last line *does* consume the full width.
  ASSERT_TRUE(utils.computeIsTextLinesCentered(block4));

  // Input: Text block with centered text lines.
  ASSERT_TRUE(utils.computeIsTextLinesCentered(block5));

  // Input: Text block with centered text lines and the whole text block right-aligned.
  // TODO(korzen): Text block detection for ragged-right text lines is currently broken. Fix it.
  // ASSERT_TRUE(utils.computeIsTextLinesCentered(block6)) << "Affected block: " << block6->text;
}

// _________________________________________________________________________________________________
TEST_F(TextBlocksDetectionUtilsTest, computeIsEmphasized) {
  PdfPage* page = pdf->pages[2];
  PdfTextLine* line1 = page->textLines[1];
  PdfTextLine* line2 = page->textLines[2];
  PdfTextLine* line3 = page->textLines[3];
  PdfTextLine* line4 = page->textLines[4];
  PdfTextLine* line5 = page->textLines[5];
  PdfTextLine* line6 = page->textLines[6];
  PdfTextLine* line7 = page->textLines[7];
  PdfTextLine* line8 = page->textLines[8];
  PdfTextLine* line9 = page->textLines[9];
  PdfTextLine* line10 = page->textLines[10];
  PdfTextLine* line11 = page->textLines[11];
  PdfTextLine* line12 = page->textLines[12];

  // Make sure we selected the correct text lines.
  ASSERT_TRUE(line1->text.starts_with("Groundbreaking Discovery"));
  ASSERT_TRUE(line2->text.starts_with("Global Summit"));
  ASSERT_TRUE(line3->text.starts_with("Economic Boom"));
  ASSERT_TRUE(line4->text.starts_with("OLYMPIC GAMES"));
  ASSERT_TRUE(line5->text.starts_with("New Breakthrough"));
  ASSERT_TRUE(line6->text.starts_with("Celebrity Couple"));
  ASSERT_TRUE(line7->text.starts_with("Record-Breaking"));
  ASSERT_TRUE(line8->text.starts_with("HISTORIC PEACE"));
  ASSERT_TRUE(line9->text.starts_with("Major Cybersecurity"));
  ASSERT_TRUE(line10->text.starts_with("Scientists Discover"));
  ASSERT_TRUE(line11->text.starts_with("Innovative Technology"));
  ASSERT_TRUE(line12->text.starts_with("LOCAL HERO"));

  TextBlocksDetectionConfig config;
  TextBlocksDetectionUtils utils(&config);

  // Input: nullptr.
  ASSERT_DEATH(utils.computeIsEmphasized(nullptr), "");

  // Input: Text lines with fontsize < most frequent font size and different font weights.
  ASSERT_FALSE(utils.computeIsEmphasized(line1));
  ASSERT_FALSE(utils.computeIsEmphasized(line2));
  ASSERT_FALSE(utils.computeIsEmphasized(line3));
  // TODO(korzen): Why is the expected output true here? If this is on purpose, shouldn't the
  // method return true for line2 and line3 as well?.
  ASSERT_TRUE(utils.computeIsEmphasized(line4));

  // Input: Text lines with fontsize == most frequent font size and different font weights.
  ASSERT_FALSE(utils.computeIsEmphasized(line5));
  ASSERT_TRUE(utils.computeIsEmphasized(line6));
  ASSERT_TRUE(utils.computeIsEmphasized(line7));
  ASSERT_TRUE(utils.computeIsEmphasized(line8));

  // Input: Text lines with fontsize > most frequent font size and different font weights.
  ASSERT_TRUE(utils.computeIsEmphasized(line9));
  ASSERT_TRUE(utils.computeIsEmphasized(line10));
  ASSERT_TRUE(utils.computeIsEmphasized(line11));
  ASSERT_TRUE(utils.computeIsEmphasized(line12));
}

// ______________________________________________________________________________________________
TEST_F(TextBlocksDetectionUtilsTest, computeHasPrevLineCapacity) {
  PdfPage* page = pdf->pages[3];
  PdfTextLine* line1 = page->textLines[1];
  PdfTextLine* line2 = page->textLines[2];
  PdfTextLine* line3 = page->textLines[3];
  PdfTextLine* line4 = page->textLines[4];
  PdfTextLine* line5 = page->textLines[5];
  PdfTextLine* line6 = page->textLines[6];
  PdfTextLine* line7 = page->textLines[7];
  PdfTextLine* line8 = page->textLines[8];
  PdfTextLine* line9 = page->textLines[9];
  PdfTextLine* line10 = page->textLines[10];
  PdfTextLine* line11 = page->textLines[11];

  // Make sure we selected the correct text lines.
  ASSERT_TRUE(line1->text.starts_with("Welcome to the realm"));
  ASSERT_TRUE(line2->text.starts_with("pected takes center"));
  ASSERT_TRUE(line3->text.starts_with("We proudly present"));
  ASSERT_TRUE(line4->text.starts_with("truly a sight"));
  ASSERT_TRUE(line5->text.starts_with("humor, prepare"));
  ASSERT_TRUE(line6->text.starts_with("While other entertainers"));
  ASSERT_TRUE(line7->text.starts_with("moments of hilarity"));
  ASSERT_TRUE(line8->text.starts_with("thing you thought"));
  ASSERT_TRUE(line9->text.starts_with("fails, our entertainer"));
  ASSERT_TRUE(line10->text.starts_with("seat. You never know"));
  ASSERT_TRUE(line11->text.starts_with("Embrace the Unbelievable"));

  TextBlocksDetectionConfig config;
  TextBlocksDetectionUtils utils(&config);

  // Input: nullptr.
  ASSERT_DEATH(utils.computeHasPrevLineCapacity(nullptr, nullptr), "");
  ASSERT_DEATH(utils.computeHasPrevLineCapacity(line1, nullptr), "");
  ASSERT_FALSE(utils.computeHasPrevLineCapacity(nullptr, line1));

  // Input: text line whose previous line does not have enough capacity to hold the first word of
  // the current text line.
  ASSERT_FALSE(utils.computeHasPrevLineCapacity(line1, line2));
  ASSERT_FALSE(utils.computeHasPrevLineCapacity(line2, line3));
  ASSERT_FALSE(utils.computeHasPrevLineCapacity(line3, line4));
  ASSERT_FALSE(utils.computeHasPrevLineCapacity(line4, line5));
  ASSERT_FALSE(utils.computeHasPrevLineCapacity(line6, line7));
  ASSERT_FALSE(utils.computeHasPrevLineCapacity(line7, line8));
  ASSERT_FALSE(utils.computeHasPrevLineCapacity(line8, line9));
  ASSERT_FALSE(utils.computeHasPrevLineCapacity(line9, line10));

  // Input: Text line whose previous line does have enough capacity to hold the first word of the
  // current text line.
  ASSERT_TRUE(utils.computeHasPrevLineCapacity(line5, line6));
  ASSERT_TRUE(utils.computeHasPrevLineCapacity(line10, line11));
}

// _________________________________________________________________________________________________
// TODO(korzen): Add more tests, to achieve more code coverage in this method.
TEST_F(TextBlocksDetectionUtilsTest, computeHangingIndent) {
  PdfPage* page = pdf->pages[4];
  PdfTextBlock* block1 = page->blocks[1];
  PdfTextBlock* block2 = page->blocks[2];
  PdfTextBlock* block3 = page->blocks[3];
  PdfTextBlock* block4 = page->blocks[4];
  PdfTextBlock* block5 = page->blocks[5];
  PdfTextBlock* block6 = page->blocks[6];
  PdfTextBlock* block7 = page->blocks[7];
  PdfTextBlock* block8 = page->blocks[8];
  PdfTextBlock* block9 = page->blocks[9];
  PdfTextBlock* block10 = page->blocks[10];
  PdfTextBlock* block11 = page->blocks[11];

  // Make sure we selected the correct text lines.
  ASSERT_TRUE(block1->text.starts_with("Dagobert Duck"));
  ASSERT_TRUE(block1->text.ends_with("wealth and influence."));
  ASSERT_TRUE(block2->text.starts_with("Born on a sunny"));
  ASSERT_TRUE(block2->text.ends_with("formative years."));
  ASSERT_TRUE(block3->text.starts_with("Friends and family"));
  ASSERT_TRUE(block3->text.ends_with("shrewd businessman."));
  ASSERT_TRUE(block4->text.starts_with("A broad interpretation"));
  ASSERT_TRUE(block4->text.ends_with("computational systems."));
  ASSERT_TRUE(block5->text.starts_with("This has led to"));
  ASSERT_TRUE(block5->text.ends_with("Informatics in 2002."));
  ASSERT_TRUE(block6->text.starts_with("The old definition"));
  ASSERT_TRUE(block6->text.ends_with("now obsolete."));
  ASSERT_TRUE(block7->text.starts_with("More than a dozen"));
  ASSERT_TRUE(block7->text.ends_with("Computer Science Al- liance."));
  ASSERT_TRUE(block8->text.starts_with("References"));
  ASSERT_TRUE(block9->text.starts_with("[Knuth, 1984]"));
  ASSERT_TRUE(block9->text.ends_with("111."));
  ASSERT_TRUE(block10->text.starts_with("[Lamport, 1994]"));
  ASSERT_TRUE(block10->text.ends_with("2 edition."));
  ASSERT_TRUE(block11->text.starts_with("[Lesk and Kernighan, 1977]"));
  ASSERT_TRUE(block11->text.ends_with("typesetting of"));

  TextBlocksDetectionConfig config;
  TextBlocksDetectionUtils utils(&config);

  // Input: nullptr.
  ASSERT_DEATH(utils.computeHangingIndent(nullptr), "");

  // Input: A text block with all lines unindented.
  ASSERT_NEAR(utils.computeHangingIndent(block1), 0.0, TOL);

  // Input: A text block with indented lines (but all lines have different indentation levels).
  ASSERT_NEAR(utils.computeHangingIndent(block2), 0.0, TOL);

  // Input: A text block in hanging indent format.
  ASSERT_NEAR(utils.computeHangingIndent(block3), 10.9, TOL);

  // Input: Text blocks with indented first lines (but not in hanging indent format).
  ASSERT_NEAR(utils.computeHangingIndent(block4), 0.0, TOL);
  ASSERT_NEAR(utils.computeHangingIndent(block5), 0.0, TOL);
  ASSERT_NEAR(utils.computeHangingIndent(block6), 0.0, TOL);
  ASSERT_NEAR(utils.computeHangingIndent(block7), 0.0, TOL);

  // Input: Three references, each in hanging indent format.
  ASSERT_NEAR(utils.computeHangingIndent(block9), 11.5, TOL);
  ASSERT_NEAR(utils.computeHangingIndent(block10), 11.5, TOL);
  ASSERT_NEAR(utils.computeHangingIndent(block11), 11.5, TOL);
}

// _________________________________________________________________________________________________
// TODO(korzen): Add more tests, to achieve more code coverage in this method.
TEST_F(TextBlocksDetectionUtilsTest, computeTextLineMargins) {
  PdfPage* page = pdf->pages[5];
  PdfTextBlock* block1 = page->blocks[1];
  PdfTextBlock* block2 = page->blocks[2];
  PdfTextBlock* block3 = page->blocks[3];

  // Make sure we selected the correct text blocks.
  ASSERT_TRUE(block1->text.starts_with("Dagoberts youthful ambition"));
  ASSERT_TRUE(block1->text.ends_with("importance of hard work."));
  ASSERT_TRUE(block2->text.starts_with("One notable incident"));
  ASSERT_TRUE(block2->text.ends_with("those around him."));
  ASSERT_TRUE(block3->text.starts_with("Today, as Duckburgs"));
  ASSERT_TRUE(block3->text.ends_with("who dared to dream big."));

  TextBlocksDetectionConfig config;
  TextBlocksDetectionUtils utils(&config);

  // Input: nullptr.
  ASSERT_DEATH(utils.computeTextLineMargins(nullptr), "");

  utils.computeTextLineMargins(block1);
  ASSERT_NEAR(block1->lines[0]->leftMargin, 10.9, TOL);
  ASSERT_NEAR(block1->lines[0]->rightMargin, 0.0, TOL);
  ASSERT_NEAR(block1->lines[1]->leftMargin, 0.0, TOL);
  ASSERT_NEAR(block1->lines[1]->rightMargin, 0.0, TOL);
  ASSERT_NEAR(block1->lines[2]->leftMargin, 0.0, TOL);
  ASSERT_NEAR(block1->lines[2]->rightMargin, 84.3, TOL);

  utils.computeTextLineMargins(block2);
  ASSERT_NEAR(block2->lines[0]->leftMargin, 3.7, TOL);
  ASSERT_NEAR(block2->lines[0]->rightMargin, 3.7, TOL);
  ASSERT_NEAR(block2->lines[1]->leftMargin, 2.8, TOL);
  ASSERT_NEAR(block2->lines[1]->rightMargin, 2.8, TOL);
  ASSERT_NEAR(block2->lines[2]->leftMargin, 41.6, TOL);
  ASSERT_NEAR(block2->lines[2]->rightMargin, 41.6, TOL);
  ASSERT_NEAR(block2->lines[3]->leftMargin, 0.0, TOL);
  ASSERT_NEAR(block2->lines[3]->rightMargin, 0.0, TOL);
  ASSERT_NEAR(block2->lines[4]->leftMargin, 228.4, TOL);
  ASSERT_NEAR(block2->lines[4]->rightMargin, 228.5, TOL);

  utils.computeTextLineMargins(block3);
  ASSERT_NEAR(block3->lines[0]->leftMargin, 10.9, TOL);
  ASSERT_NEAR(block3->lines[0]->rightMargin, 0.4, TOL);
  ASSERT_NEAR(block3->lines[1]->leftMargin, 0.0, TOL);
  ASSERT_NEAR(block3->lines[1]->rightMargin, -22.2, TOL);
  ASSERT_NEAR(block3->lines[2]->leftMargin, 0.0, TOL);
  ASSERT_NEAR(block3->lines[2]->rightMargin, 0.4, TOL);
  ASSERT_NEAR(block3->lines[3]->leftMargin, 0.0, TOL);
  ASSERT_NEAR(block3->lines[3]->rightMargin, 0.4, TOL);
  ASSERT_NEAR(block3->lines[4]->leftMargin, 0.0, TOL);
  ASSERT_NEAR(block3->lines[4]->rightMargin, 0.4, TOL);
  ASSERT_NEAR(block3->lines[5]->leftMargin, 0.0, TOL);
  ASSERT_NEAR(block3->lines[5]->rightMargin, 364.7, TOL);
}

// _________________________________________________________________________________________________
// TODO(korzen): Add more tests to achieve more code coverage in this method. Test in particular
// lines that start with "1." or "A)" but does not belong to an itemize.
TEST_F(TextBlocksDetectionUtilsTest, computeIsFirstLineOfItem) {
  PdfPage* page = pdf->pages[6];
  PdfTextLine* line1 = page->textLines[1];
  PdfTextLine* line2 = page->textLines[2];
  PdfTextLine* line3 = page->textLines[3];
  PdfTextLine* line4 = page->textLines[4];
  PdfTextLine* line5 = page->textLines[5];
  PdfTextLine* line6 = page->textLines[6];
  PdfTextLine* line7 = page->textLines[7];
  PdfTextLine* line8 = page->textLines[8];
  PdfTextLine* line9 = page->textLines[9];
  PdfTextLine* line10 = page->textLines[10];
  PdfTextLine* line11 = page->textLines[11];
  PdfTextLine* line12 = page->textLines[12];
  PdfTextLine* line13 = page->textLines[13];
  PdfTextLine* line14 = page->textLines[14];
  PdfTextLine* line15 = page->textLines[15];
  PdfTextLine* line16 = page->textLines[16];
  PdfTextLine* line17 = page->textLines[17];
  PdfTextLine* line18 = page->textLines[18];
  PdfTextLine* line19 = page->textLines[19];
  PdfTextLine* line20 = page->textLines[20];
  PdfTextLine* line21 = page->textLines[21];

  // Make sure we selected the correct text lines.
  ASSERT_TRUE(line1->text.starts_with("Welcome to MyMassage."));
  ASSERT_TRUE(line2->text.starts_with("Step into a world"));
  ASSERT_TRUE(line3->text.starts_with("to rejuvenate your"));
  ASSERT_TRUE(line4->text.starts_with("1. Expert Therapists:"));
  ASSERT_TRUE(line5->text.starts_with("to providing you with"));
  ASSERT_TRUE(line6->text.starts_with("various massage techniques"));
  ASSERT_TRUE(line7->text.starts_with("restore balance to your life"));
  ASSERT_TRUE(line8->text.starts_with("2. Serene Ambiance"));
  ASSERT_TRUE(line9->text.starts_with("atmosphere of tranquility"));
  ASSERT_TRUE(line10->text.starts_with("create a sanctuary of calmness"));
  ASSERT_TRUE(line11->text.starts_with("3. Tailored Treatments"));
  ASSERT_TRUE(line12->text.starts_with("time to understand"));
  ASSERT_TRUE(line13->text.starts_with("experience tailored"));
  ASSERT_TRUE(line14->text.starts_with("reduction, or pure relaxation"));
  ASSERT_TRUE(line15->text.starts_with("Here are some of the"));
  ASSERT_TRUE(line16->text.starts_with("- Feldberg: 1,493 meters"));
  ASSERT_TRUE(line17->text.starts_with("Wuerttemberg.)"));
  ASSERT_TRUE(line18->text.starts_with("- Herzogenhorn: 1,415 meters"));
  ASSERT_TRUE(line19->text.starts_with("- Belchen: 1,414 meters"));
  ASSERT_TRUE(line20->text.starts_with("- Schauinsland: 1,284 meters"));
  ASSERT_TRUE(line21->text.starts_with("- Kandel: 1,241 meters"));

  TextBlocksDetectionConfig config;
  TextBlocksDetectionUtils utils(&config);

  // Input: nullptr.
  ASSERT_DEATH(utils.computeIsFirstLineOfItem(nullptr), "");

  ASSERT_FALSE(utils.computeIsFirstLineOfItem(line1));
  ASSERT_FALSE(utils.computeIsFirstLineOfItem(line2));
  ASSERT_FALSE(utils.computeIsFirstLineOfItem(line3));
  ASSERT_TRUE(utils.computeIsFirstLineOfItem(line4));
  ASSERT_FALSE(utils.computeIsFirstLineOfItem(line5));
  ASSERT_FALSE(utils.computeIsFirstLineOfItem(line6));
  ASSERT_FALSE(utils.computeIsFirstLineOfItem(line7));
  ASSERT_TRUE(utils.computeIsFirstLineOfItem(line8));
  ASSERT_FALSE(utils.computeIsFirstLineOfItem(line9));
  ASSERT_FALSE(utils.computeIsFirstLineOfItem(line10));
  ASSERT_TRUE(utils.computeIsFirstLineOfItem(line11));
  ASSERT_FALSE(utils.computeIsFirstLineOfItem(line12));
  ASSERT_FALSE(utils.computeIsFirstLineOfItem(line13));
  ASSERT_FALSE(utils.computeIsFirstLineOfItem(line14));
  ASSERT_FALSE(utils.computeIsFirstLineOfItem(line15));
  ASSERT_TRUE(utils.computeIsFirstLineOfItem(line16));
  ASSERT_FALSE(utils.computeIsFirstLineOfItem(line17));
  ASSERT_TRUE(utils.computeIsFirstLineOfItem(line18));
  ASSERT_TRUE(utils.computeIsFirstLineOfItem(line19));
  ASSERT_TRUE(utils.computeIsFirstLineOfItem(line20));
  ASSERT_TRUE(utils.computeIsFirstLineOfItem(line21));
}

// _________________________________________________________________________________________________
// TODO(korzen): Add more tests to achieve more code coverage in this method.
TEST_F(TextBlocksDetectionUtilsTest, computeIsContinuationOfItem) {
  PdfPage* page = pdf->pages[6];
  PdfTextLine* line1 = page->textLines[1];
  PdfTextLine* line2 = page->textLines[2];
  PdfTextLine* line3 = page->textLines[3];
  PdfTextLine* line4 = page->textLines[4];
  PdfTextLine* line5 = page->textLines[5];
  PdfTextLine* line6 = page->textLines[6];
  PdfTextLine* line7 = page->textLines[7];
  PdfTextLine* line8 = page->textLines[8];
  PdfTextLine* line9 = page->textLines[9];
  PdfTextLine* line10 = page->textLines[10];
  PdfTextLine* line11 = page->textLines[11];
  PdfTextLine* line12 = page->textLines[12];
  PdfTextLine* line13 = page->textLines[13];
  PdfTextLine* line14 = page->textLines[14];
  PdfTextLine* line15 = page->textLines[15];
  PdfTextLine* line16 = page->textLines[16];
  PdfTextLine* line17 = page->textLines[17];
  PdfTextLine* line18 = page->textLines[18];
  PdfTextLine* line19 = page->textLines[19];
  PdfTextLine* line20 = page->textLines[20];
  PdfTextLine* line21 = page->textLines[21];

  // Make sure we selected the correct text lines.
  ASSERT_TRUE(line1->text.starts_with("Welcome to MyMassage."));
  ASSERT_TRUE(line2->text.starts_with("Step into a world"));
  ASSERT_TRUE(line3->text.starts_with("to rejuvenate your"));
  ASSERT_TRUE(line4->text.starts_with("1. Expert Therapists:"));
  ASSERT_TRUE(line5->text.starts_with("to providing you with"));
  ASSERT_TRUE(line6->text.starts_with("various massage techniques"));
  ASSERT_TRUE(line7->text.starts_with("restore balance to your life"));
  ASSERT_TRUE(line8->text.starts_with("2. Serene Ambiance"));
  ASSERT_TRUE(line9->text.starts_with("atmosphere of tranquility"));
  ASSERT_TRUE(line10->text.starts_with("create a sanctuary of calmness"));
  ASSERT_TRUE(line11->text.starts_with("3. Tailored Treatments"));
  ASSERT_TRUE(line12->text.starts_with("time to understand"));
  ASSERT_TRUE(line13->text.starts_with("experience tailored"));
  ASSERT_TRUE(line14->text.starts_with("reduction, or pure relaxation"));
  ASSERT_TRUE(line15->text.starts_with("Here are some of the"));
  ASSERT_TRUE(line16->text.starts_with("- Feldberg: 1,493 meters"));
  ASSERT_TRUE(line17->text.starts_with("Wuerttemberg.)"));
  ASSERT_TRUE(line18->text.starts_with("- Herzogenhorn: 1,415 meters"));
  ASSERT_TRUE(line19->text.starts_with("- Belchen: 1,414 meters"));
  ASSERT_TRUE(line20->text.starts_with("- Schauinsland: 1,284 meters"));
  ASSERT_TRUE(line21->text.starts_with("- Kandel: 1,241 meters"));

  TextBlocksDetectionConfig config;
  TextBlocksDetectionUtils utils(&config);

  // Input: nullptr.
  ASSERT_DEATH(utils.computeIsContinuationOfItem(nullptr), "");

  // Input: text line without any words.
  PdfTextLine line;
  ASSERT_FALSE(utils.computeIsContinuationOfItem(&line));

  ASSERT_FALSE(utils.computeIsContinuationOfItem(line1));
  ASSERT_FALSE(utils.computeIsContinuationOfItem(line2));
  ASSERT_FALSE(utils.computeIsContinuationOfItem(line3));
  ASSERT_FALSE(utils.computeIsContinuationOfItem(line4));
  ASSERT_TRUE(utils.computeIsContinuationOfItem(line5));
  ASSERT_TRUE(utils.computeIsContinuationOfItem(line6));
  ASSERT_TRUE(utils.computeIsContinuationOfItem(line7));
  ASSERT_FALSE(utils.computeIsContinuationOfItem(line8));
  ASSERT_TRUE(utils.computeIsContinuationOfItem(line9));
  ASSERT_TRUE(utils.computeIsContinuationOfItem(line10));
  ASSERT_FALSE(utils.computeIsContinuationOfItem(line11));
  ASSERT_TRUE(utils.computeIsContinuationOfItem(line12));
  ASSERT_TRUE(utils.computeIsContinuationOfItem(line13));
  ASSERT_TRUE(utils.computeIsContinuationOfItem(line14));
  ASSERT_FALSE(utils.computeIsContinuationOfItem(line15));
  ASSERT_FALSE(utils.computeIsContinuationOfItem(line16));
  ASSERT_TRUE(utils.computeIsContinuationOfItem(line17));
  ASSERT_FALSE(utils.computeIsContinuationOfItem(line18));
  ASSERT_FALSE(utils.computeIsContinuationOfItem(line19));
  ASSERT_FALSE(utils.computeIsContinuationOfItem(line20));
  ASSERT_FALSE(utils.computeIsContinuationOfItem(line21));
}

// _________________________________________________________________________________________________
TEST_F(TextBlocksDetectionUtilsTest, computePotentialFootnoteLabels) {
  PdfPage* page = pdf->pages[7];
  PdfTextLine* line1 = page->textLines[1];
  PdfTextLine* line2 = page->textLines[2];
  PdfTextLine* line3 = page->textLines[3];
  PdfTextLine* line4 = page->textLines[4];

  // Make sure we selected the correct text lines.
  ASSERT_TRUE(line1->text.starts_with("Rod Stewart is a legendary"));
  ASSERT_TRUE(line2->text.starts_with("stage presence"));
  ASSERT_TRUE(line3->text.starts_with("rose to prominence"));
  ASSERT_TRUE(line4->text.starts_with("industry. From"));

  TextBlocksDetectionConfig config;
  TextBlocksDetectionUtils utils(&config);

  // Input: one (or more) nullptrs.
  unordered_set<string> result0;
  ASSERT_DEATH(utils.computePotentialFootnoteLabels(nullptr, nullptr), "");
  ASSERT_DEATH(utils.computePotentialFootnoteLabels(line1, nullptr), "");
  ASSERT_DEATH(utils.computePotentialFootnoteLabels(nullptr, &result0), "");

  // Input: Text line with no footnote labels.
  unordered_set<string> result1;
  utils.computePotentialFootnoteLabels(line1, &result1);
  ASSERT_EQ(result1.size(), static_cast<unsigned int>(0));

  // Input: Text line with three footnote labels.
  unordered_set<string> result2;
  utils.computePotentialFootnoteLabels(line2, &result2);
  ASSERT_EQ(result2.size(), static_cast<unsigned int>(3));
  ASSERT_TRUE(result2.contains("*"));
  // FIXME(korzen): The other footnote labels are not detected correctly. Fix it.
  // ASSERT_TRUE(result2.contains("†"));
  // ASSERT_TRUE(result2.contains("‡"));

  // Input: Text line with one footnote label.
  unordered_set<string> result3;
  utils.computePotentialFootnoteLabels(line3, &result3);
  ASSERT_EQ(result3.size(), static_cast<unsigned int>(1));
  ASSERT_TRUE(result3.contains("§"));

  // Input: Text line with two footnote labels.
  unordered_set<string> result4;
  utils.computePotentialFootnoteLabels(line4, &result4);
  ASSERT_EQ(result4.size(), static_cast<unsigned int>(2));
  ASSERT_TRUE(result4.contains("5"));
  ASSERT_TRUE(result4.contains("6"));
}

// _________________________________________________________________________________________________
TEST_F(TextBlocksDetectionUtilsTest, computeIsPrefixedByItemLabel) {
  PdfPage* page = pdf->pages[8];
  PdfTextLine* line1 = page->textLines[1];
  PdfTextLine* line2 = page->textLines[2];
  PdfTextLine* line3 = page->textLines[3];
  PdfTextLine* line4 = page->textLines[4];
  PdfTextLine* line5 = page->textLines[5];
  PdfTextLine* line6 = page->textLines[6];
  PdfTextLine* line7 = page->textLines[7];
  PdfTextLine* line8 = page->textLines[8];
  PdfTextLine* line9 = page->textLines[9];
  PdfTextLine* line10 = page->textLines[10];
  PdfTextLine* line11 = page->textLines[11];
  PdfTextLine* line12 = page->textLines[12];
  PdfTextLine* line13 = page->textLines[13];
  PdfTextLine* line14 = page->textLines[14];
  PdfTextLine* line15 = page->textLines[15];
  PdfTextLine* line16 = page->textLines[16];
  PdfTextLine* line17 = page->textLines[17];
  PdfTextLine* line18 = page->textLines[18];
  PdfTextLine* line19 = page->textLines[19];
  PdfTextLine* line20 = page->textLines[20];
  PdfTextLine* line21 = page->textLines[21];
  PdfTextLine* line22 = page->textLines[22];
  PdfTextLine* line23 = page->textLines[23];
  PdfTextLine* line24 = page->textLines[24];
  PdfTextLine* line25 = page->textLines[25];
  PdfTextLine* line26 = page->textLines[26];
  PdfTextLine* line27 = page->textLines[27];
  PdfTextLine* line28 = page->textLines[28];

  // Make sure we selected the correct text lines.
  ASSERT_TRUE(line1->text.starts_with("Meryl Streep"));
  ASSERT_TRUE(line2->text.starts_with("1Asa Akira"));
  ASSERT_TRUE(line3->text.starts_with("2Katharine Hepburn"));
  ASSERT_TRUE(line4->text.starts_with("aNessa Devil"));
  ASSERT_TRUE(line5->text.starts_with("bCate Blanchett"));
  ASSERT_TRUE(line6->text.starts_with("∗Audrey Hepburn"));
  ASSERT_TRUE(line7->text.starts_with("- Judi Dench"));
  ASSERT_TRUE(line8->text.starts_with("+ Ingrid Bergman"));
  ASSERT_TRUE(line9->text.starts_with("2 Frances McDormand"));
  ASSERT_TRUE(line10->text.starts_with("I. Jodie Foster"));
  ASSERT_TRUE(line11->text.starts_with("II. Nicole Kidman"));
  ASSERT_TRUE(line12->text.starts_with("IV. Kate Winslet"));
  ASSERT_TRUE(line13->text.starts_with("(I) Julia Roberts"));
  ASSERT_TRUE(line14->text.starts_with("(II) Charlize Theron"));
  ASSERT_TRUE(line15->text.starts_with("(IV) Viola Davis"));
  ASSERT_TRUE(line16->text.starts_with("a. Emma Thompson"));
  ASSERT_TRUE(line17->text.starts_with("b. Natalie Portman"));
  ASSERT_TRUE(line18->text.starts_with("c. Angelina Jolie"));
  ASSERT_TRUE(line19->text.starts_with("1. Tilda Swinton"));
  ASSERT_TRUE(line20->text.starts_with("2. Diane Keaton"));
  ASSERT_TRUE(line21->text.starts_with("3. Helen Mirren"));
  ASSERT_TRUE(line22->text.starts_with("(A) Sandra Bullock"));
  ASSERT_TRUE(line23->text.starts_with("(C1) Marion Cotillard"));
  ASSERT_TRUE(line24->text.starts_with("[1] Jennifer Lawrence"));
  ASSERT_TRUE(line25->text.starts_with("[JeLa20] Jessica Lange"));
  ASSERT_TRUE(line26->text.starts_with("A) Michelle Pfeiffer"));
  ASSERT_TRUE(line27->text.starts_with("c) Saoirse Ronan"));
  ASSERT_TRUE(line28->text.starts_with("a1) Misses X"));

  TextBlocksDetectionConfig config;
  TextBlocksDetectionUtils utils(&config);

  // Input: nullptr.
  ASSERT_DEATH(utils.computeIsPrefixedByItemLabel(nullptr), "");

  // Input: Text lines with different labels.
  ASSERT_FALSE(utils.computeIsPrefixedByItemLabel(line1));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line2));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line3));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line4));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line5));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line6));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line7));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line8));
  ASSERT_FALSE(utils.computeIsPrefixedByItemLabel(line9));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line10));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line11));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line12));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line13));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line14));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line15));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line16));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line17));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line18));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line19));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line20));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line21));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line22));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line23));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line24));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line25));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line26));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line27));
  ASSERT_TRUE(utils.computeIsPrefixedByItemLabel(line28));
}

// _________________________________________________________________________________________________
TEST_F(TextBlocksDetectionUtilsTest, computeIsPrefixedByFootnoteLabel) {
  PdfPage* page = pdf->pages[9];
  PdfTextLine* line1 = page->textLines[1];
  PdfTextLine* line2 = page->textLines[2];
  PdfTextLine* line3 = page->textLines[3];
  PdfTextLine* line4 = page->textLines[4];
  PdfTextLine* line5 = page->textLines[5];
  PdfTextLine* line6 = page->textLines[6];
  PdfTextLine* line7 = page->textLines[7];
  PdfTextLine* line8 = page->textLines[8];
  PdfTextLine* line9 = page->textLines[9];
  PdfTextLine* line10 = page->textLines[10];

  TextBlocksDetectionConfig config;
  TextBlocksDetectionUtils utils(&config);

  // Input: nullptr.
  ASSERT_DEATH(utils.computeIsPrefixedByFootnoteLabel(nullptr), "");

  // Make sure we selected the correct text lines.
  ASSERT_TRUE(line1->text.starts_with("Dustin Hoffman"));
  ASSERT_TRUE(line2->text.starts_with("1Marlon Brando"));
  ASSERT_TRUE(line3->text.starts_with("2Robert De Niro"));
  ASSERT_TRUE(line4->text.starts_with("aAl Pacino"));
  ASSERT_TRUE(line5->text.starts_with("bTom Hanks"));
  ASSERT_TRUE(line6->text.starts_with("∗Daniel Day-Lewis"));
  ASSERT_TRUE(line7->text.starts_with("abcJack Nicholson"));
  ASSERT_TRUE(line8->text.starts_with("+ Anthony Hopkins"));
  ASSERT_TRUE(line9->text.starts_with("1 Leonardo DiCaprio"));
  ASSERT_TRUE(line10->text.starts_with("+ Denzel Washington"));

  ASSERT_FALSE(utils.computeIsPrefixedByFootnoteLabel(line1));
  ASSERT_TRUE(utils.computeIsPrefixedByFootnoteLabel(line2));
  ASSERT_TRUE(utils.computeIsPrefixedByFootnoteLabel(line3));
  ASSERT_TRUE(utils.computeIsPrefixedByFootnoteLabel(line4));
  ASSERT_TRUE(utils.computeIsPrefixedByFootnoteLabel(line5));
  ASSERT_TRUE(utils.computeIsPrefixedByFootnoteLabel(line6));
  ASSERT_TRUE(utils.computeIsPrefixedByFootnoteLabel(line7));
  ASSERT_TRUE(utils.computeIsPrefixedByFootnoteLabel(line8));
  ASSERT_FALSE(utils.computeIsPrefixedByFootnoteLabel(line9));
  ASSERT_FALSE(utils.computeIsPrefixedByFootnoteLabel(line10));

  unordered_set<string> footnoteLabels = { "1", "2", "a", "b" };
  ASSERT_FALSE(utils.computeIsPrefixedByFootnoteLabel(line1, &footnoteLabels));
  ASSERT_TRUE(utils.computeIsPrefixedByFootnoteLabel(line2, &footnoteLabels));
  ASSERT_TRUE(utils.computeIsPrefixedByFootnoteLabel(line3, &footnoteLabels));
  ASSERT_TRUE(utils.computeIsPrefixedByFootnoteLabel(line4, &footnoteLabels));
  ASSERT_TRUE(utils.computeIsPrefixedByFootnoteLabel(line5, &footnoteLabels));
  ASSERT_FALSE(utils.computeIsPrefixedByFootnoteLabel(line6, &footnoteLabels));
  ASSERT_FALSE(utils.computeIsPrefixedByFootnoteLabel(line7, &footnoteLabels));
  ASSERT_FALSE(utils.computeIsPrefixedByFootnoteLabel(line8, &footnoteLabels));
  ASSERT_FALSE(utils.computeIsPrefixedByFootnoteLabel(line9, &footnoteLabels));
  ASSERT_FALSE(utils.computeIsPrefixedByFootnoteLabel(line10, &footnoteLabels));
}

// _________________________________________________________________________________________________
TEST_F(TextBlocksDetectionUtilsTest, computeOverlapsFigure) {
  PdfPage* page = pdf->pages[10];
  PdfTextLine* line1 = page->textLines[1];
  PdfTextLine* line2 = page->textLines[2];
  PdfFigure* figure = page->figures[0];

  // Make sure we selected the correct text lines.
  ASSERT_TRUE(line1->text.starts_with("Figure 1: Some useless"));
  ASSERT_TRUE(line2->text.starts_with("Figure 2: A flower"));

  TextBlocksDetectionConfig config;
  TextBlocksDetectionUtils utils(&config);

  // Input: nullptr.
  ASSERT_DEATH(utils.computeOverlapsFigure(nullptr, page->figures), "");

  // Input: the characters within the diagram.
  for (PdfCharacter* character : figure->characters) {
    ASSERT_TRUE(utils.computeOverlapsFigure(character, page->figures));
  }

  // Input: the text line below the diagram.
  ASSERT_FALSE(utils.computeOverlapsFigure(line1, page->figures));

  // TODO(korzen): The flower picture is not a figure. Why not?
  // ASSERT_TRUE(utils.computeOverlapsFigure(line2, page->figures));
}

// _________________________________________________________________________________________________
TEST_F(TextBlocksDetectionUtilsTest, createTextBlock) {
  PdfPage* page = pdf->pages[11];
  PdfTextLine* line1 = page->textLines[1];
  PdfTextLine* line2 = page->textLines[2];
  PdfTextLine* line3 = page->textLines[3];
  PdfTextLine* line4 = page->textLines[4];
  PdfTextLine* line5 = page->textLines[5];
  PdfTextLine* line6 = page->textLines[6];
  PdfTextLine* line7 = page->textLines[7];
  PdfTextLine* line8 = page->textLines[8];
  PdfTextLine* line9 = page->textLines[9];
  PdfTextLine* line10 = page->textLines[10];
  PdfTextLine* line11 = page->textLines[11];
  PdfTextLine* line12 = page->textLines[12];
  PdfTextLine* line13 = page->textLines[13];

  // Make sure we selected the correct text lines.
  ASSERT_TRUE(line1->text.starts_with("Dagoberts youthful ambition"));
  ASSERT_TRUE(line2->text.starts_with("to organizing small-scale"));
  ASSERT_TRUE(line3->text.starts_with("invaluable lesson"));
  ASSERT_TRUE(line4->text.starts_with("One notable incident"));
  ASSERT_TRUE(line5->text.starts_with("heirloom, a fabled golden"));
  ASSERT_TRUE(line6->text.starts_with("adventure that showcased"));
  ASSERT_TRUE(line7->text.starts_with("locate the golden goose"));
  ASSERT_TRUE(line8->text.starts_with("Today, as Duckburgs"));
  ASSERT_TRUE(line9->text.starts_with("acumen, his philanthropic"));
  ASSERT_TRUE(line10->text.starts_with("His story serves as"));
  ASSERT_TRUE(line11->text.starts_with("can be realized"));
  ASSERT_TRUE(line12->text.starts_with("from humble beginnings to"));
  ASSERT_TRUE(line13->text.starts_with("young duck who dared to dream big."));

  TextBlocksDetectionConfig config;
  TextBlocksDetectionUtils utils(&config);

  // Input: empty vector of text lines.
  vector<PdfTextLine*> lines;
  ASSERT_DEATH(utils.createTextBlock(lines, nullptr), "");

  // Input: non-empty vector of text lines, but no vector of blocks.
  lines = { line1, line2, line3 };
  ASSERT_DEATH(utils.createTextBlock(lines, nullptr), "");

  vector<PdfTextBlock*> blocks;
  utils.createTextBlock(lines, &blocks);
  ASSERT_EQ(blocks.size(), static_cast<unsigned int>(1));
  ASSERT_TRUE(blocks[0]->id.size() > static_cast<unsigned int>(0));
  ASSERT_EQ(blocks[0]->doc, pdf);
  ASSERT_EQ(blocks[0]->segment, line1->segment);
  ASSERT_EQ(blocks[0]->lines, lines);
  ASSERT_EQ(blocks[0]->pos->pageNum, 12);
  ASSERT_EQ(blocks[0]->pos->wMode, 0);
  ASSERT_EQ(blocks[0]->pos->rotation, 0);
  ASSERT_NEAR(blocks[0]->pos->leftX, 56.7, TOL);
  ASSERT_NEAR(blocks[0]->pos->rightX, 538.6, TOL);
  ASSERT_NEAR(blocks[0]->pos->upperY, 90.8, TOL);
  ASSERT_NEAR(blocks[0]->pos->lowerY, 132.0, TOL);
  ASSERT_NEAR(blocks[0]->trimLeftX, 56.7, TOL);
  ASSERT_NEAR(blocks[0]->trimRightX, 538.6, TOL);
  ASSERT_NEAR(blocks[0]->trimUpperY, 90.8, TOL);
  ASSERT_NEAR(blocks[0]->trimLowerY, 132.0, TOL);
  ASSERT_EQ(blocks[0]->rank, 0);
  ASSERT_EQ(blocks[0]->fontName, "KAGVWM+CMR10");
  ASSERT_NEAR(blocks[0]->fontSize, 10.9, TOL);
  ASSERT_TRUE(blocks[0]->text.starts_with("Dagoberts youthful ambition led him to explore"));
  ASSERT_TRUE(blocks[0]->text.ends_with("importance of hard work."));
  ASSERT_EQ(blocks[0]->prevBlock, nullptr);
  ASSERT_FALSE(blocks[0]->isEmphasized);
  ASSERT_FALSE(blocks[0]->isLinesCentered);
  ASSERT_NEAR(blocks[0]->hangingIndent, 0.0, TOL);

  lines = { line4, line5, line6, line7 };
  utils.createTextBlock(lines, &blocks);
  ASSERT_EQ(blocks.size(), static_cast<unsigned int>(2));
  ASSERT_TRUE(blocks[1]->id.size() > static_cast<unsigned int>(0));
  ASSERT_EQ(blocks[1]->doc, pdf);
  ASSERT_EQ(blocks[1]->segment, line4->segment);
  ASSERT_EQ(blocks[1]->lines, lines);
  ASSERT_EQ(blocks[1]->pos->pageNum, 12);
  ASSERT_EQ(blocks[1]->pos->wMode, 0);
  ASSERT_EQ(blocks[1]->pos->rotation, 0);
  ASSERT_NEAR(blocks[1]->pos->leftX, 67.5, TOL);
  ASSERT_NEAR(blocks[1]->pos->rightX, 527.8, TOL);
  ASSERT_NEAR(blocks[1]->pos->upperY, 142.7, TOL);
  ASSERT_NEAR(blocks[1]->pos->lowerY, 191.5, TOL);
  ASSERT_NEAR(blocks[1]->trimLeftX, 67.5, TOL);
  ASSERT_NEAR(blocks[1]->trimRightX, 527.8, TOL);
  ASSERT_NEAR(blocks[1]->trimUpperY, 142.7, TOL);
  ASSERT_NEAR(blocks[1]->trimLowerY, 191.5, TOL);
  ASSERT_EQ(blocks[1]->rank, 1);
  ASSERT_EQ(blocks[1]->fontName, "KAGVWM+CMR10");
  ASSERT_NEAR(blocks[1]->fontSize, 10.0, TOL);
  ASSERT_TRUE(blocks[1]->text.starts_with("One notable incident from"));
  ASSERT_TRUE(blocks[1]->text.ends_with("respect of those around him."));
  ASSERT_EQ(blocks[1]->prevBlock, blocks[0]);
  ASSERT_FALSE(blocks[1]->isEmphasized);
  ASSERT_TRUE(blocks[1]->isLinesCentered);
  ASSERT_NEAR(blocks[1]->hangingIndent, 0.0, TOL);

  lines = { line8, line9, line10, line11, line12, line13 };
  utils.createTextBlock(lines, &blocks);
  ASSERT_EQ(blocks.size(), static_cast<unsigned int>(3));
  ASSERT_TRUE(blocks[2]->id.size() > static_cast<unsigned int>(0));
  ASSERT_EQ(blocks[2]->doc, pdf);
  ASSERT_EQ(blocks[2]->segment, line8->segment);
  ASSERT_EQ(blocks[2]->lines, lines);
  ASSERT_EQ(blocks[2]->pos->pageNum, 12);
  ASSERT_EQ(blocks[2]->pos->wMode, 0);
  ASSERT_EQ(blocks[2]->pos->rotation, 0);
  ASSERT_NEAR(blocks[2]->pos->leftX, 56.7, TOL);
  ASSERT_NEAR(blocks[2]->pos->rightX, 580.2, TOL);
  ASSERT_NEAR(blocks[2]->pos->upperY, 203.1, TOL);
  ASSERT_NEAR(blocks[2]->pos->lowerY, 285.1, TOL);
  ASSERT_NEAR(blocks[2]->trimLeftX, 56.7, TOL);
  ASSERT_NEAR(blocks[2]->trimRightX, 539.0, TOL);
  ASSERT_NEAR(blocks[2]->trimUpperY, 203.1, TOL);
  ASSERT_NEAR(blocks[2]->trimLowerY, 285.1, TOL);
  ASSERT_EQ(blocks[2]->rank, 2);
  ASSERT_EQ(blocks[2]->fontName, "ADABFR+CMSSBX10");
  ASSERT_NEAR(blocks[2]->fontSize, 10.9, TOL);
  ASSERT_TRUE(blocks[2]->text.starts_with("Today, as Duckburgs most influential tycoon"));
  ASSERT_TRUE(blocks[2]->text.ends_with("who dared to dream big."));
  ASSERT_EQ(blocks[2]->prevBlock, blocks[1]);
  ASSERT_TRUE(blocks[2]->isEmphasized);
  ASSERT_FALSE(blocks[2]->isLinesCentered);
  ASSERT_NEAR(blocks[2]->hangingIndent, 0.0, TOL);
}
