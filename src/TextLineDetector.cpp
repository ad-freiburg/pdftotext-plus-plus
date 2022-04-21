/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <cmath>
#include <iostream>
#include <stack>
#include <vector>

#include "./PdfDocument.h"
#include "./TextLineDetector.h"
#include "./utils/Utils.h"

// _________________________________________________________________________________________________
TextLineDetector::TextLineDetector(PdfDocument* doc) {
  _doc = doc;
}

// _________________________________________________________________________________________________
TextLineDetector::~TextLineDetector() = default;

// _________________________________________________________________________________________________
void TextLineDetector::detect() {
  tokenize();
}

// _________________________________________________________________________________________________
void TextLineDetector::tokenize() {
  // Abort if no document is given.
  if (!_doc) {
    return;
  }

  // Abort if the document contains no pages.
  if (_doc->pages.size() == 0) {
    return;
  }

  for (auto* page : _doc->pages) {
    for (auto* segment : page->segments) {
      // Create a vector containing only the words of the segments.
      std::vector<PdfWord*> words;
      for (auto* element : segment->elements) {
        PdfWord* word = dynamic_cast<PdfWord*>(element);
        if (word) {
          words.push_back(word);
        }
      }

      // Ignore the segment if it doesn't contain any words.
      if (words.size() == 0) {
        continue;
      }

      // Cluster the words by their lower y-values.
      std::unordered_map<int, std::unordered_map<double, std::vector<PdfWord*>>> clusters;
      for (auto* word : words) {
        // TODO: What is a stacked word? (-> summation symbol with sub- and superscripts).
        if (word->isPartOfStackedWord) {
          continue;
        }

        double rotation = word->position->rotation;
        double lowerY = round(word->position->getRotLowerY(), 1);
        clusters[rotation][lowerY].push_back(word);

        // TODO: What is a stacked word? (-> summation symbol with sub- and superscripts).
        for (auto* stackedWord : word->isBaseOfStackedWords) {
          clusters[rotation][lowerY].push_back(stackedWord);
        }
      }

      // From each cluster create a text line.
      for (const auto& rotation : clusters) {
        std::vector<PdfTextLine*> lines;
        for (const auto& cluster : rotation.second) {
          PdfTextLine* line = new PdfTextLine();
          line->words = cluster.second;
          line->segment = segment;
          computeTextLineProperties(line);
          lines.push_back(line);
        }

        // Abort if there are no text lines.
        if (lines.size() == 0) {
          continue;
        }

        // Sort the text lines by their lower y-values in ascending or descending order, depending
        // on the rotation.
        double rot = rotation.first;
        if (rot == 0 || rot == 1) {
          std::sort(lines.begin(), lines.end(), [](const PdfTextLine* l1, const PdfTextLine* l2) {
            return l1->position->getRotLowerY() < l2->position->getRotLowerY();
          });
        } else {
          std::sort(lines.begin(), lines.end(), [](const PdfTextLine* l1, const PdfTextLine* l2) {
            return l1->position->getRotLowerY() > l2->position->getRotLowerY();
          });
        }

        // int x = 0;
        while (true) {
          bool hasMerged = false;
          std::vector<PdfTextLine*> newLines;
          // Merge text lines that overlap vertically.
          for (size_t i = 0; i < lines.size(); i++) {
            PdfTextLine* prevLine = newLines.size() > 0 ? newLines.at(newLines.size() - 1) : nullptr;
            PdfTextLine* currLine = lines.at(i);
            PdfTextLine* nextLine = i < lines.size() - 1 ? lines.at(i + 1) : nullptr;

            // if (x == 1) {
            //   std::cout << "=====" << std::endl;
            //   if (prevLine) {
            //     std::cout << "prev: " << prevLine->toString() << std::endl;
            //   } else {
            //     std::cout << "prev: -" << std::endl;
            //   }
            //   std::cout << "curr: " << currLine->toString() << std::endl;
            //   if (nextLine) {
            //     std::cout << "next: " << nextLine->toString() << std::endl;
            //   } else {
            //     std::cout << "next: -" << std::endl;
            //   }
            // }

            double prevLineXGap = 0.0;
            double prevLineYOverlap = 0.0;
            if (prevLine) {
              prevLineXGap = computeHorizontalGap(prevLine, currLine);
              prevLineYOverlap = computeMaximumYOverlapRatio(prevLine, currLine);
            }

            double nextLineXGap = 0.0;
            double nextLineYOverlap = 0.0;
            if (nextLine) {
              nextLineXGap = computeHorizontalGap(currLine, nextLine);
              nextLineYOverlap = computeMaximumYOverlapRatio(currLine, nextLine);
            }

            double prevLineThreshold = prevLineXGap < 3 * _doc->avgGlyphWidth ? 0.4 : 0.8;
            double nextLineThreshold = nextLineXGap < 3 * _doc->avgGlyphWidth ? 0.4 : 0.8;

            if (prevLineYOverlap >= prevLineThreshold && prevLineYOverlap > nextLineYOverlap) {
              prevLine->words.insert(prevLine->words.end(), currLine->words.begin(), currLine->words.end());
              computeTextLineProperties(prevLine);
              hasMerged = true;
              // std::cout << "A" << std::endl;
            } else if (nextLineYOverlap >= nextLineThreshold && nextLineYOverlap > prevLineYOverlap) {
              currLine->words.insert(currLine->words.end(), nextLine->words.begin(), nextLine->words.end());
              computeTextLineProperties(currLine);
              currLine->rank = _numTextLines++;
              newLines.push_back(currLine);
              hasMerged = true;
              i++;
              // std::cout << "B" << std::endl;
            } else {
              currLine->rank = _numTextLines++;
              newLines.push_back(currLine);
              // std::cout << "C" << std::endl;
            }
          }

          if (!hasMerged) {
            for (auto* line : newLines) {
              line->rank = _numTextLines++;
              segment->lines.push_back(line);
            }
            break;
          }

          lines = newLines;
          // x++;
        }

        //   double overlapRatio = computeMaximumYOverlapRatio(refLine, currLine);
        //   if (overlapRatio < 0.4) {
        //     refLine->rank = _numTextLines++;
        //     segment->lines.push_back(refLine);
        //     refLine = currLine;
        //     continue;
        //   }

        //   refLine->words.insert(refLine->words.end(), currLine->words.begin(), currLine->words.end());
        //   computeTextLineProperties(refLine);
        // }
        // refLine->rank = _numTextLines++;
        // segment->lines.push_back(refLine);
      }
    }
  }
}

// _________________________________________________________________________________________________
void TextLineDetector::computeTextLineProperties(PdfTextLine* line) {
  // Abort if no text line is given.
  if (!line) {
    return;
  }

  // Abort if the text line does not contain any words.
  if (line->words.size() == 0) {
    return;
  }

  // Set the writing mode.
  line->position->wMode = line->words[0]->position->wMode;

  // Set the rotation value.
  line->position->rotation = line->words[0]->position->rotation;

  if (line->position->rotation == 0 || line->position->rotation == 1) {
    // Sort the words by their left x-coordinates, in ascending order.
    std::sort(line->words.begin(), line->words.end(), [](const PdfWord* w1, const PdfWord* w2) {
      return w1->position->getRotLeftX() < w2->position->getRotLeftX();
    });
  } else {
    // Sort the words by their left x-coordinates, in descending order.
    std::sort(line->words.begin(), line->words.end(), [](const PdfWord* w1, const PdfWord* w2) {
      return w1->position->getRotLeftX() > w2->position->getRotLeftX();
    });
  }

  // Compute a unique id.
  line->id = createRandomString(8, "tl-");

  // Compute they x,y-coordinates of the bounding box and the font information.
  std::unordered_map<std::string, int> fontNameFreqs;
  std::unordered_map<double, int> fontSizeFreqs;
  std::unordered_map<double, int> baseFreqs;
  for (auto* word : line->words) {
    double wordMinX = std::min(word->position->leftX, word->position->rightX);
    double wordMinY = std::min(word->position->lowerY, word->position->upperY);
    double wordMaxX = std::max(word->position->leftX, word->position->rightX);
    double wordMaxY = std::max(word->position->lowerY, word->position->upperY);

    line->position->leftX = std::min(line->position->leftX, wordMinX);
    line->position->upperY = std::min(line->position->upperY, wordMinY);
    line->position->rightX = std::max(line->position->rightX, wordMaxX);
    line->position->lowerY = std::max(line->position->lowerY, wordMaxY);

    // Compute the most frequent font name, font size and baseline among the glyphs.
    for (const auto* glyph : word->glyphs) {
      fontNameFreqs[glyph->fontName]++;
      fontSizeFreqs[glyph->fontSize]++;
      baseFreqs[glyph->base]++;
    }

    word->line = line;
  }

  // Compute the most frequent font name.
  int mostFreqFontNameCount = 0;
  for (const auto& pair : fontNameFreqs) {
    if (pair.second > mostFreqFontNameCount) {
      line->fontName = pair.first;
      mostFreqFontNameCount = pair.second;
    }
  }

  line->text = "";
  for (size_t i = 0; i < line->words.size(); i++) {
    PdfWord* word = line->words[i];
    line->text += word->text;
    if (i < line->words.size() - 1) {
      line->text += " ";
    }
  }

  // Compute the most frequent font size.
  int mostFreqFontSizeCount = 0;
  double mostFreqFontSize = 0;
  double maxFontSize = 0;
  for (const auto& pair : fontSizeFreqs) {
    if (pair.second > mostFreqFontSizeCount) {
      mostFreqFontSize = pair.first;
      mostFreqFontSizeCount = pair.second;
    }
    maxFontSize = std::max(maxFontSize, pair.first);
  }
  line->fontSize = mostFreqFontSize;

  // Compute the most frequent baseline.
  int mostFreqBaseCount = 0;
  for (const auto& pair : baseFreqs) {
    if (pair.second > mostFreqBaseCount) {
      line->base = pair.first;
      mostFreqBaseCount = pair.second;
    }
  }

  // Set the page number.
  line->position->pageNum = line->words[0]->position->pageNum;

  // Iterate through the words and join them, each separated by a whitespace.
  line->text = "";
  for (size_t i = 0; i < line->words.size(); i++) {
    PdfWord* word = line->words[i];
    line->text += word->text;
    if (i < line->words.size() - 1) {
      line->text += " ";
    }
  }
}

// _________________________________________________________________________________________________
// double TextLineDetector::computeMostFrequentLineIndentation() {
//   std::unordered_map<double, int> lineIndentationCounts;

//   for (auto* page : _doc->pages) {
//     for (size_t i = 0; i < page->textLines.size(); i++) {
//       PdfTextLine* prevLine = i > 0 ? page->textLines.at(i - 1) : nullptr;
//       PdfTextLine* currLine = page->textLines.at(i);
//       PdfTextLine* nextLine = i < page->textLines.size() - 1 ? page->textLines.at(i + 1) : nullptr;

//       if (!prevLine || !currLine || !nextLine) {
//         continue;
//       }

//       if (prevLine->position->pageNum != currLine->position->pageNum) {
//         continue;
//       }

//       if (prevLine->position->wMode != 0 || currLine->position->wMode != 0) {
//         continue;
//       }

//       if (prevLine->position->rotation != 0 || currLine->position->rotation != 0) {
//         continue;
//       }

//       double prevLeftMargin = round(prevLine->position->leftX - prevLine->segment->position->leftX, 1);
//       double currLeftMargin = round(currLine->position->leftX - currLine->segment->position->leftX, 1);
//       double nextLeftMargin = round(nextLine->position->leftX - nextLine->segment->position->leftX, 1);
//       double currRightMargin = round(currLine->segment->position->leftX - currLine->position->leftX, 1);
//       double nextRightMargin = round(nextLine->segment->position->leftX - nextLine->position->leftX, 1);

//       // Only consider the line if the previous left margin and the next left margin is 0.
//       if (!equal(prevLeftMargin, 0, 1) || !equal(nextLeftMargin, 0, 1)) {
//         continue;
//       }

//       // Only consider the line if its left margin is > 0.
//       if (!larger(currLeftMargin, 0, 1)) {
//         continue;
//       }

//       // Only consider the line if the current right margin and the next right margin is 0.
//       if (!equal(currRightMargin, 0, 1) || !equal(nextRightMargin, 0, 1)) {
//         continue;
//       }

//       double indent = round(currLeftMargin, 1);
//       lineIndentationCounts[indent]++;
//     }
//   }

//   int mostFreqLineIndentationCount = 0;
//   double mostFreqLineIndentation = 0;
//   for (const auto& pair : lineIndentationCounts) {
//     if (pair.second > mostFreqLineIndentationCount) {
//       mostFreqLineIndentation = pair.first;
//       mostFreqLineIndentationCount = pair.second;
//     }
//   }

//   return mostFreqLineIndentation;
// }

// TODO: Indentation
// TODO: References to prev/next line.
