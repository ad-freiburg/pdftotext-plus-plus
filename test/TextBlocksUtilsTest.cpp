/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <gtest/gtest.h>

#include <vector>

#include "../src/utils/MathUtils.h"
#include "../src/utils/TextBlocksUtils.h"

#include "../src/PdfDocument.h"

using global_config::DOUBLE_EQUAL_TOLERANCE;

// _________________________________________________________________________________________________
TEST(TextBlocksUtils, computeIsTextLinesCentered) {
  PdfDocument* doc = new PdfDocument();
  doc->avgCharWidth = 4.2;

  // Block has no lines, should return false.
  PdfTextBlock* block = new PdfTextBlock();
  block->doc = doc;
  ASSERT_FALSE(text_blocks_utils::computeIsTextLinesCentered(block));

  // Block has two lines which are not centered compared to each other. Should return false.
  block = new PdfTextBlock();
  block->doc = doc;
  PdfTextLine* line1 = new PdfTextLine(1, 50.0, 320.0, 250.0, 330.0, 0, 0);
  line1->doc = doc;
  PdfTextLine* line2 = new PdfTextLine(1, 10.0, 340.0, 240.0, 350.0, 0, 0);
  line2->doc = doc;
  block->lines.push_back(line1);
  block->lines.push_back(line2);
  ASSERT_FALSE(text_blocks_utils::computeIsTextLinesCentered(block));

  // Block has two centered lines, but one of which is a formula. Should return false.
  block = new PdfTextBlock();
  block->doc = doc;
  PdfTextLine* line3 = new PdfTextLine(1, 70.0, 320.0, 250.0, 350.0, 0, 0);
  line3->doc = doc;
  line3->text = "abc";
  PdfTextLine* line4 = new PdfTextLine(1, 50.0, 340.0, 230.0, 330.0, 0, 0);
  line4->doc = doc;
  line4->text = "x+y=z";
  block->lines.push_back(line3);
  block->lines.push_back(line4);
  ASSERT_FALSE(text_blocks_utils::computeIsTextLinesCentered(block));

  // Block has two justified lines. Should return false.
  block = new PdfTextBlock();
  block->doc = doc;
  PdfTextLine* line5 = new PdfTextLine(1, 50.0, 320.0, 250.0, 330.0, 0, 0);
  line5->doc = doc;
  line5->text = "foo";
  PdfTextLine* line6 = new PdfTextLine(1, 50.0, 340.0, 240.0, 350.0, 0, 0);
  line6->doc = doc;
  line6->text = "bar";
  block->lines.push_back(line5);
  block->lines.push_back(line6);
  ASSERT_FALSE(text_blocks_utils::computeIsTextLinesCentered(block));

  // Block contains centered lines. Should return true.
  block = new PdfTextBlock();
  block->doc = doc;
  PdfTextLine* line7 = new PdfTextLine(1, 70.0, 320.0, 250.0, 330.0, 0, 0);
  line7->doc = doc;
  line7->text = "foo";
  PdfTextLine* line8 = new PdfTextLine(1, 50.0, 340.0, 270.0, 350.0, 0, 0);
  line8->doc = doc;
  line8->text = "bar";
  PdfTextLine* line9 = new PdfTextLine(1, 60.0, 360.0, 260.0, 370.0, 0, 0);
  line9->doc = doc;
  line9->text = "bar";
  block->lines.push_back(line7);
  block->lines.push_back(line8);
  block->lines.push_back(line9);
  ASSERT_TRUE(text_blocks_utils::computeIsTextLinesCentered(block));

  // Block contains centered lines, but the number of justified lines lines is larger than the
  // threshold. Should return false.
  block = new PdfTextBlock();
  block->doc = doc;
  PdfTextLine* lineA = new PdfTextLine(1, 70.0, 320.0, 250.0, 330.0, 0, 0);
  lineA->doc = doc;
  lineA->text = "foo";
  PdfTextLine* lineB = new PdfTextLine(1, 50.0, 340.0, 270.0, 350.0, 0, 0);
  lineB->doc = doc;
  lineB->text = "bar";
  PdfTextLine* lineC = new PdfTextLine(1, 60.0, 360.0, 260.0, 370.0, 0, 0);
  lineC->doc = doc;
  lineC->text = "bar";
  PdfTextLine* lineD = new PdfTextLine(1, 60.0, 360.0, 260.0, 370.0, 0, 0);
  lineD->doc = doc;
  lineD->text = "bar";
  PdfTextLine* lineE = new PdfTextLine(1, 60.0, 360.0, 260.0, 370.0, 0, 0);
  lineE->doc = doc;
  lineE->text = "bar";
  PdfTextLine* lineF = new PdfTextLine(1, 60.0, 360.0, 260.0, 370.0, 0, 0);
  lineF->doc = doc;
  lineF->text = "bar";
  PdfTextLine* lineG = new PdfTextLine(1, 60.0, 360.0, 260.0, 370.0, 0, 0);
  lineG->doc = doc;
  lineG->text = "bar";
  PdfTextLine* lineH = new PdfTextLine(1, 60.0, 360.0, 260.0, 370.0, 0, 0);
  lineH->doc = doc;
  lineH->text = "bar";
  PdfTextLine* lineI = new PdfTextLine(1, 60.0, 360.0, 260.0, 370.0, 0, 0);
  lineI->doc = doc;
  lineI->text = "bar";
  block->lines.push_back(lineA);
  block->lines.push_back(lineB);
  block->lines.push_back(lineC);
  block->lines.push_back(lineD);
  block->lines.push_back(lineE);
  block->lines.push_back(lineF);
  block->lines.push_back(lineG);
  block->lines.push_back(lineH);
  block->lines.push_back(lineI);
  ASSERT_FALSE(text_blocks_utils::computeIsTextLinesCentered(block));
}

// _________________________________________________________________________________________________
TEST(TextBlocksUtils, computeHangingIndent) {
  PdfDocument* doc = new PdfDocument();
  doc->avgCharWidth = 4.2;

  // ===============================================================================================
  // A block is not in hanging indent format when it contains no lines.

  PdfTextBlock* block = new PdfTextBlock();
  block->doc = doc;
  ASSERT_NEAR(text_blocks_utils::computeHangingIndent(block), 0.0, DOUBLE_EQUAL_TOLERANCE);

  // ===============================================================================================
  // A block is not in hanging indent format if the percentage of lines exhibiting the most
  // frequent left margin is smaller than a threshold.

  block = new PdfTextBlock();
  block->doc = doc;

  PdfTextLine* line1 = new PdfTextLine();
  line1->leftMargin = 0.0;
  block->lines.push_back(line1);

  PdfTextLine* line2 = new PdfTextLine();
  line2->leftMargin = 0.0;
  block->lines.push_back(line2);

  PdfTextLine* line3 = new PdfTextLine();
  line3->leftMargin = 5.0;
  block->lines.push_back(line3);

  PdfTextLine* line4 = new PdfTextLine();
  line4->leftMargin = 5.0;
  block->lines.push_back(line4);

  PdfTextLine* line5 = new PdfTextLine();
  line5->leftMargin = 7.0;
  block->lines.push_back(line5);

  PdfTextLine* line6 = new PdfTextLine();
  line6->leftMargin = 7.0;
  block->lines.push_back(line6);

  PdfTextLine* line7 = new PdfTextLine();
  line7->leftMargin = 7.0;
  block->lines.push_back(line7);

  // The most frequent left margin is 7.0, which occurs 3 times. The number of lines is 7. So the
  // percentage of lines exhibiting the most frequent left margin is 3/7 = 42.8%, which is smaller
  // than the used threshold (50%).
  ASSERT_NEAR(text_blocks_utils::computeHangingIndent(block), 0.0, DOUBLE_EQUAL_TOLERANCE);

  // ===============================================================================================
  // A block is not in hanging indent format if it does not contain any indented lines.

  block = new PdfTextBlock();
  block->doc = doc;

  PdfTextLine* line8 = new PdfTextLine();
  line8->leftMargin = 0.0;
  block->lines.push_back(line8);

  PdfTextLine* line9 = new PdfTextLine();
  line9->leftMargin = 0.0;
  block->lines.push_back(line9);

  PdfTextLine* lineA = new PdfTextLine();
  lineA->leftMargin = 0.0;
  block->lines.push_back(lineA);

  // The block contains three non-indented lines (that is: lines with left margin == 0).
  ASSERT_NEAR(text_blocks_utils::computeHangingIndent(block), 0.0, DOUBLE_EQUAL_TOLERANCE);

  // ===============================================================================================
  // A block is *not* in hanging indent format if it contains at least one non-indented line
  // that starts with a lowercase character.

  block = new PdfTextBlock();
  block->doc = doc;

  // This is the non-indented line starting with a lower case.
  PdfTextLine* lineB = new PdfTextLine();
  lineB->leftMargin = 0.0;
  lineB->text = "foo bar baz";
  PdfWord* wordB = new PdfWord();
  wordB->text = "foo";
  lineB->words.push_back(wordB);
  block->lines.push_back(lineB);

  PdfTextLine* lineC = new PdfTextLine();
  lineC->leftMargin = 7.0;
  lineC->text = "Foo bar baz";
  PdfWord* wordC = new PdfWord();
  wordC->text = "Foo";
  lineC->words.push_back(wordC);
  block->lines.push_back(lineC);

  PdfTextLine* lineD = new PdfTextLine();
  lineD->leftMargin = 7.0;
  lineD->text = "Foo bar baz";
  PdfWord* wordD = new PdfWord();
  wordD->text = "Foo";
  lineD->words.push_back(wordD);
  block->lines.push_back(lineD);

  ASSERT_NEAR(text_blocks_utils::computeHangingIndent(block), 0.0, DOUBLE_EQUAL_TOLERANCE);

  // ===============================================================================================
  // A block is in hanging indent format if the first line is not indented, but all other lines.

  block = new PdfTextBlock();
  block->doc = doc;

  // The first line should be not indented.
  PdfTextLine* lineE = new PdfTextLine();
  lineE->leftMargin = 0.0;
  lineE->text = "Foo bar baz";
  lineE->rightMargin = 2.0;  // The line should have no capacity.
  lineE->prevLine = nullptr;
  lineE->doc = doc;
  PdfWord* wordE = new PdfWord();
  wordE->text = "Foo";
  lineE->words.push_back(wordE);
  block->lines.push_back(lineE);

  // The second line should be indented.
  PdfTextLine* lineF = new PdfTextLine();
  lineF->leftMargin = 7.0;
  lineF->text = "Foo bar baz";
  lineF->prevLine = lineE;
  lineF->doc = doc;
  // The previous line should have no capacity, so create a word with width > lineE->rightMargin.
  PdfWord* wordF = new PdfWord();
  wordF->text = "Foo";
  wordF->pos->leftX = 12.1;
  wordF->pos->rightX = 17.1;
  wordF->pos->upperY = 27.2;
  wordF->pos->lowerY = 35.4;
  lineF->words.push_back(wordF);
  block->lines.push_back(lineF);

  PdfTextLine* lineG = new PdfTextLine();
  lineG->leftMargin = 7.0;
  lineG->text = "Foo bar baz";
  lineG->prevLine = lineF;
  lineG->doc = doc;
  PdfWord* wordG = new PdfWord();
  wordG->text = "Foo";
  lineG->words.push_back(wordG);
  block->lines.push_back(lineG);

  // The hanging indent amount should be equal to the left margin of the last two lines.
  double hangingIndent = text_blocks_utils::computeHangingIndent(block);
  ASSERT_NEAR(hangingIndent, math_utils::round(lineG->leftMargin), DOUBLE_EQUAL_TOLERANCE);

  // ===============================================================================================
  // A block is in hanging indent format if all non-indented lines start with an uppercase
  // character and if the number of non-indented lines exceed a certain threshold.

  block = new PdfTextBlock();
  block->doc = doc;

  // Create a non-indented line starting with an uppercase. Append it x-times to the blocks, where
  // x is larger than config::HANG_INDENT_NUM_NON_INDENTED_LINES_THRESHOLD.
  PdfTextLine* lineH = new PdfTextLine();
  lineH->leftMargin = 0.0;
  lineH->text = "Foo bar baz";
  lineH->rightMargin = 0.0;
  lineH->prevLine = nullptr;
  lineH->doc = doc;
  PdfWord* wordH = new PdfWord();
  wordH->text = "Foo";
  lineH->words.push_back(wordH);

  // Create an indented line that occurs y-times, with y > x (needed so that the method
  // returns a value > 0.0).
  PdfTextLine* lineJ = new PdfTextLine();
  lineJ->leftMargin = 6.4;
  lineJ->text = "Foo bar baz";
  lineJ->rightMargin = 0.0;
  lineJ->prevLine = nullptr;
  lineJ->doc = doc;
  PdfWord* wordJ = new PdfWord();
  wordJ->text = "Foo";
  lineJ->words.push_back(wordJ);

  block->lines.push_back(lineH);
  block->lines.push_back(lineJ);
  block->lines.push_back(lineJ);
  block->lines.push_back(lineH);
  block->lines.push_back(lineJ);
  block->lines.push_back(lineH);
  block->lines.push_back(lineJ);
  block->lines.push_back(lineH);
  block->lines.push_back(lineJ);
  block->lines.push_back(lineJ);
  block->lines.push_back(lineH);
  block->lines.push_back(lineH);
  block->lines.push_back(lineH);
  block->lines.push_back(lineH);
  block->lines.push_back(lineJ);
  block->lines.push_back(lineJ);
  block->lines.push_back(lineJ);
  block->lines.push_back(lineH);
  block->lines.push_back(lineJ);
  block->lines.push_back(lineH);
  block->lines.push_back(lineH);
  block->lines.push_back(lineJ);

  // The hanging indent should be equal to the rounded identation amount of the indented lines.
  hangingIndent = text_blocks_utils::computeHangingIndent(block);
  ASSERT_NEAR(hangingIndent, math_utils::round(lineJ->leftMargin), DOUBLE_EQUAL_TOLERANCE);

  // If the number of lowercased non-indented lines is larger than
  // config::HANG_INDENT_NUM_LOWER_NON_INDENTED_LINES_THRESHOLD, the hanging indent should be 0.0.
  PdfTextLine* lineK = new PdfTextLine();
  lineK->leftMargin = 0.0;
  lineK->text = "foo bar baz";
  lineK->rightMargin = 0.0;
  lineK->prevLine = nullptr;
  lineK->doc = doc;
  PdfWord* wordK = new PdfWord();
  wordK->text = "foo";
  lineK->words.push_back(wordK);
  block->lines.push_back(lineK);

  hangingIndent = text_blocks_utils::computeHangingIndent(block);
  ASSERT_NEAR(hangingIndent, 0.0, DOUBLE_EQUAL_TOLERANCE);

  // ===============================================================================================
  // A block is in hanging indent format if there is at least one indented line that start
  // with a lowercase character.

  block = new PdfTextBlock();
  block->doc = doc;

  // Create a "long" line and append it to the block x-times, with
  // x > config::HANG_INDENT_NUM_LONG_LINES_THRESHOLD.
  PdfTextLine* lineL = new PdfTextLine();
  lineL->leftMargin = 8.2;
  lineL->text = "Foo bar baz";
  lineL->rightMargin = 0.0;
  lineL->prevLine = nullptr;
  lineL->doc = doc;
  PdfWord* wordL = new PdfWord();
  wordL->text = "Foo";
  lineL->words.push_back(wordL);

  // Create an indented line starting with a lower case character and append it y-times with
  // y > config::HANG_INDENT_NUM_LOWER_INDENTED_LINES_THRESHOLD
  PdfTextLine* lineM = new PdfTextLine();
  lineM->leftMargin = 5.2;
  lineM->text = "foo bar baz";
  lineM->rightMargin = 0.0;
  lineM->prevLine = nullptr;
  lineM->doc = doc;
  PdfWord* wordM = new PdfWord();
  wordM->text = "foo";
  lineM->words.push_back(wordM);

  block->lines.push_back(lineL);
  block->lines.push_back(lineM);
  block->lines.push_back(lineL);
  block->lines.push_back(lineL);
  block->lines.push_back(lineL);

  hangingIndent = text_blocks_utils::computeHangingIndent(block);
  ASSERT_NEAR(hangingIndent, math_utils::round(lineL->leftMargin), DOUBLE_EQUAL_TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(TextBlocksUtils, computeTextLineMargins) {
  PdfTextBlock* block = new PdfTextBlock();
  block->trimLeftX = 20.0;
  block->trimRightX = 150.0;

  PdfTextLine* line1 = new PdfTextLine();
  line1->pos->leftX = 20.0;
  line1->pos->rightX = 150.0;
  block->lines.push_back(line1);

  PdfTextLine* line2 = new PdfTextLine();
  line2->pos->leftX = 27.3;
  line2->pos->rightX = 150.0;
  block->lines.push_back(line2);

  PdfTextLine* line3 = new PdfTextLine();
  line3->pos->leftX = 20.0;
  line3->pos->rightX = 140.3;
  block->lines.push_back(line3);

  PdfTextLine* line4 = new PdfTextLine();
  line4->pos->leftX = 40.5;
  line4->pos->rightX = 100.2;
  block->lines.push_back(line4);

  text_blocks_utils::computeTextLineMargins(block);

  // Note: The margins are rounded.
  ASSERT_NEAR(line1->leftMargin, 0.0, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(line1->rightMargin, 0.0, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(line2->leftMargin, math_utils::round(7.3), DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(line2->rightMargin, 0.0, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(line3->leftMargin, 0.0, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(line3->rightMargin, math_utils::round(9.7), DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(line4->leftMargin, math_utils::round(20.5), DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(line4->rightMargin, math_utils::round(49.8), DOUBLE_EQUAL_TOLERANCE);
}

// _________________________________________________________________________________________________
TEST(TextBlocksUtils, createTextBlock) {
  std::vector<PdfTextLine*> lines;
  std::vector<PdfTextBlock*> blocks;

  PdfDocument* doc = new PdfDocument();
  doc->mostFreqFontName = "Arial";

  PdfPageSegment* segment = new PdfPageSegment();
  segment->trimLeftX = 12.1;
  segment->trimRightX = 70.2;
  segment->trimUpperY = 130.1;
  segment->trimLowerY = 331.2;

  PdfTextLine* line1 = new PdfTextLine();
  line1->doc = doc;
  line1->segment = segment;
  line1->pos->pageNum = 2;
  line1->pos->leftX = 13.7;
  line1->pos->rightX = 55.2;
  line1->pos->upperY = 130.1;
  line1->pos->lowerY = 150.3;
  line1->pos->rotation = 0;
  line1->pos->wMode = 0;
  line1->fontName = "Arial";
  line1->fontSize = 10.2;

  lines.push_back(line1);

  text_blocks_utils::createTextBlock(lines, &blocks);

  PdfTextBlock* block = blocks.back();
  ASSERT_EQ(block->id.size(), global_config::ID_LENGTH + 6);  // +6 for "block-"
  ASSERT_EQ(block->doc, doc);
  ASSERT_EQ(block->segment, segment);
  ASSERT_EQ(block->lines, lines);
  ASSERT_EQ(block->pos->pageNum, line1->pos->pageNum);
  ASSERT_NEAR(block->pos->leftX, 13.7, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->pos->rightX, 55.2, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->pos->upperY, 130.1, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->pos->lowerY, 150.3, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->trimLeftX, 13.7, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->trimRightX, 55.2, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->trimUpperY, 130.1, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->trimLowerY, 150.3, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_EQ(block->pos->wMode, line1->pos->wMode);
  ASSERT_EQ(block->pos->rotation, line1->pos->rotation);
  ASSERT_EQ(block->rank, 0);
  ASSERT_EQ(line1->prevLine, nullptr);
  ASSERT_EQ(line1->nextLine, nullptr);
  ASSERT_EQ(line1->block, block);
  ASSERT_EQ(block->fontName, line1->fontName);
  ASSERT_NEAR(block->fontSize, line1->fontSize, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_EQ(block->prevBlock, nullptr);
  ASSERT_EQ(block->nextBlock, nullptr);

  // ============

  PdfTextLine* line2 = new PdfTextLine();
  line2->doc = doc;
  line2->segment = segment;
  line2->pos->pageNum = 2;
  line2->pos->leftX = 20.2;
  line2->pos->rightX = 56.2;
  line2->pos->upperY = 155.1;
  line2->pos->lowerY = 166.3;
  line2->pos->rotation = 1;
  line2->pos->wMode = 0;
  line2->fontName = "Arial";
  line2->fontSize = 10.2;
  lines.push_back(line2);

  text_blocks_utils::createTextBlock(lines, &blocks);

  block = blocks.back();
  ASSERT_EQ(block->id.size(), global_config::ID_LENGTH + 6);  // +6 for "block-"
  ASSERT_EQ(block->doc, doc);
  ASSERT_EQ(block->segment, segment);
  ASSERT_EQ(block->lines, lines);
  ASSERT_EQ(block->pos->pageNum, line1->pos->pageNum);
  ASSERT_NEAR(block->pos->leftX, 13.7, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->pos->rightX, 56.2, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->pos->upperY, 130.1, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->pos->lowerY, 166.3, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->trimLeftX, 13.7, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->trimRightX, 56.2, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->trimUpperY, 130.1, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->trimLowerY, 166.3, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_EQ(block->pos->wMode, line1->pos->wMode);
  ASSERT_EQ(block->pos->rotation, line1->pos->rotation);
  ASSERT_EQ(block->rank, 1);
  ASSERT_EQ(line1->prevLine, nullptr);
  ASSERT_EQ(line1->nextLine, line2);
  ASSERT_EQ(line1->block, block);
  ASSERT_EQ(line2->prevLine, line1);
  ASSERT_EQ(line2->nextLine, nullptr);
  ASSERT_EQ(line2->block, block);
  ASSERT_EQ(block->fontName, line1->fontName);
  ASSERT_NEAR(block->fontSize, line1->fontSize, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_EQ(blocks[0]->prevBlock, nullptr);
  ASSERT_EQ(blocks[0]->nextBlock, blocks[1]);
  ASSERT_EQ(blocks[1]->prevBlock, blocks[0]);
  ASSERT_EQ(blocks[1]->nextBlock, nullptr);

  // ============

  PdfTextLine* line3 = new PdfTextLine();
  line3->doc = doc;
  line3->segment = segment;
  line3->pos->pageNum = 2;
  line3->pos->leftX = 12.1;
  line3->pos->rightX = 70.2;
  line3->pos->upperY = 168.0;
  line3->pos->lowerY = 180.3;
  line3->pos->rotation = 0;
  line3->pos->wMode = 0;
  line3->fontName = "Times";
  line3->fontSize = 12.2;
  lines.push_back(line3);

  text_blocks_utils::createTextBlock(lines, &blocks);

  block = blocks.back();
  ASSERT_EQ(block->id.size(), global_config::ID_LENGTH + 6);  // +6 for "block-"
  ASSERT_EQ(block->doc, doc);
  ASSERT_EQ(block->segment, segment);
  ASSERT_EQ(block->lines, lines);
  ASSERT_EQ(block->pos->pageNum, line1->pos->pageNum);
  ASSERT_NEAR(block->pos->leftX, 12.1, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->pos->rightX, 70.2, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->pos->upperY, 130.1, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->pos->lowerY, 180.3, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->trimLeftX, 12.1, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->trimRightX, 70.2, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->trimUpperY, 130.1, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_NEAR(block->trimLowerY, 180.3, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_EQ(block->pos->wMode, line1->pos->wMode);
  ASSERT_EQ(block->pos->rotation, line1->pos->rotation);
  ASSERT_EQ(block->rank, 2);
  ASSERT_EQ(line1->prevLine, nullptr);
  ASSERT_EQ(line1->nextLine, line2);
  ASSERT_EQ(line1->block, block);
  ASSERT_EQ(line2->prevLine, line1);
  ASSERT_EQ(line2->nextLine, line3);
  ASSERT_EQ(line2->block, block);
  ASSERT_EQ(line3->prevLine, line2);
  ASSERT_EQ(line3->nextLine, nullptr);
  ASSERT_EQ(line3->block, block);
  ASSERT_EQ(block->fontName, line1->fontName);
  ASSERT_NEAR(block->fontSize, line1->fontSize, DOUBLE_EQUAL_TOLERANCE);
  ASSERT_EQ(blocks[0]->prevBlock, nullptr);
  ASSERT_EQ(blocks[0]->nextBlock, blocks[1]);
  ASSERT_EQ(blocks[1]->prevBlock, blocks[0]);
  ASSERT_EQ(blocks[1]->nextBlock, blocks[2]);
  ASSERT_EQ(blocks[2]->prevBlock, blocks[1]);
  ASSERT_EQ(blocks[2]->nextBlock, nullptr);
}
