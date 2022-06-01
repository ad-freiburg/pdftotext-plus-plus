/**
 * Copyright 2022, University of Freiburg,
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
#include "./TextLinesDetector.h"
#include "./utils/LogUtils.h"
#include "./utils/Utils.h"
#include "./utils/PageSegmentUtils.h"
#include "./utils/TextLinesUtils.h"

using namespace std;

// _________________________________________________________________________________________________
TextLinesDetector::TextLinesDetector(PdfDocument* doc, bool debug, int debugPageFilter) {
  _doc = doc;
  _log = new Logger(debug ? DEBUG : INFO, debugPageFilter);

  _log->debug() << "=======================================" << endl;
  _log->debug() << "\033[1mDEBUG MODE | Detecting Text Lines\033[0m" << endl;
  _log->debug() << " ─ debug page filter: " << debugPageFilter << endl;
}

// _________________________________________________________________________________________________
TextLinesDetector::~TextLinesDetector() {
  delete _log;
};

// _________________________________________________________________________________________________
void TextLinesDetector::detect() {
  // Do nothing if no document is given.
  if (!_doc) {
    return;
  }

  // Do nothing if the document does not contain any pages.
  if (_doc->pages.empty()) {
    return;
  }

  for (auto* page : _doc->pages) {
    int p = page->pageNum;
    _log->debug(p) << "=======================================" << endl;
    _log->debug(p) << "\033[1mPROCESSING PAGE " << p << "\033[0m" << endl;
    _log->debug(p) << " └─ # segments: " << page->segments.size() << endl;

    for (auto* segment : page->segments) {
      _log->debug(p) << "=======================================" << endl;
      _log->debug(p) << "PROCESSING SEGMENT " << segment->id << endl;

      // Create a vector containing only the *words* (not figures or shapes) of the segment.
      vector<PdfWord*> words;
      for (auto* element : segment->elements) {
        PdfWord* word = dynamic_cast<PdfWord*>(element);
        if (word) {
          words.push_back(word);
        }
      }

      // Skip the segment if it doesn't contain any words.
      if (words.size() == 0) {
        continue;
      }

      _log->debug(p) << "=======================================" << endl;
      _log->debug(p) << "CLUSTERING" << endl;

      // Cluster the words by their rotations and their lower y-values.
      unordered_map<int, unordered_map<double, vector<PdfWord*>>> clusters;
      for (size_t i = 0; i < words.size(); i++) {
        PdfWord* word = words[i];
        int p = word->position->pageNum;

        if (i > 0) { _log->debug(p) << "-------" << endl; }
        _log->debug(p) << "word: text: \"" << word->text << "\""
            << "; page: " << word->position->pageNum
            << "; leftX: " << word->position->leftX
            << "; upperY: " << word->position->upperY
            << "; rightX: " << word->position->rightX
            << "; lowerY: " << word->position->lowerY << endl;
        if (word->position->rotation != 0) {
          _log->debug(p) << "rot: " << word->position->rotation
              << "; rotLeftX: " << word->position->getRotLeftX()
              << "; rotUpperY: " << word->position->getRotUpperY()
              << "; rotRightX: " << word->position->getRotRightX()
              << "; rotLowerY: " << word->position->getRotLowerY() << endl;
        }

        // Skip the word if it is part of a stacked math symbol which was merged with the base word.
        if (word->isPartOfStackedMathSymbol) {
          _log->debug(p) << "\033[1mskipping (is part of stacked math symbol).\033[0m" << endl;
          continue;
        }

        double rotation = word->position->rotation;
        double lowerY = round(word->position->getRotLowerY(), 1);
        clusters[rotation][lowerY].push_back(word);
        _log->debug(p) << " └─ cluster: (" << rotation << ", " << lowerY << ")" << endl;

        // If the word is the base word of a stacked math symbol, add each word that is part
        // of the stacked math symbol to the same cluster as the base word.
        for (auto* w : word->isBaseOfStackedMathSymbol) {
          _log->debug(p) << "Adding part of stacked math symbol: " << w->text << endl;
          clusters[rotation][lowerY].push_back(w);
        }
      }

      _log->debug(p) << "=======================================" << endl;
      _log->debug(p) << "CREATING PRELIMINARY TEXT LINES" << endl;

      for (const auto& pair : clusters) {
        int rot = pair.first;

        // Create a text line from each cluster.
        vector<PdfTextLine*> lines;
        for (const auto& e : pair.second) {
          createTextLine(e.second, segment, &lines);

          PdfTextLine* line = lines[lines.size() - 1];
          double y = e.first;
          _log->debug(p) << "Created line from cluster (" << rot << ", " << y << ")" << endl;
          _log->debug(p) << " └─ line.pageNum: " << line->position->pageNum << endl;
          _log->debug(p) << " └─ line.leftX: " << line->position->leftX << endl;
          _log->debug(p) << " └─ line.upperY: " << line->position->upperY << endl;
          _log->debug(p) << " └─ line.rightX: " << line->position->rightX << endl;
          _log->debug(p) << " └─ line.lowerY: " << line->position->lowerY << endl;
          _log->debug(p) << " └─ line.text: \"" << line->text << "\"" << endl;
        }

        // Skip the cluster if it contains no text lines.
        if (lines.empty()) {
          continue;
        }

        // Sort the lines by their lower y-values in asc or desc order, depending on the rotation.
        // This should sort the lines according to the natural reading order, from top to bottom.
        _log->debug(p) << "-------" << endl;
        _log->debug(p) << "\033[1mSorting text lines...\033[0m" << endl;
        if (rot == 0 || rot == 1) {
          sort(lines.begin(), lines.end(), [](const PdfTextLine* l1, const PdfTextLine* l2) {
            return l1->position->getRotLowerY() < l2->position->getRotLowerY();
          });
        } else {
          sort(lines.begin(), lines.end(), [](const PdfTextLine* l1, const PdfTextLine* l2) {
            return l1->position->getRotLowerY() > l2->position->getRotLowerY();
          });
        }

        // Merge text lines that overlap vertically in rounds. Repeat until there are no text lines
        // anymore that vertically overlap. This should merge words that were assigned to different
        // clusters but actually belong to the same text line, which particulary often happens in
        // case of sub- and superscripts, or fractions in formulas.
        int round = 0;
        while (true) {
          _log->debug(p) << "=======" << endl;
          _log->debug(p) << "\033[1mMerging lines, round " << ++round << "\033[0m" << endl;

          bool merged = false;
          vector<PdfTextLine*> nLines;
          for (size_t i = 0; i < lines.size(); i++) {
            PdfTextLine* prevLine = nLines.size() > 0 ? nLines.at(nLines.size() - 1) : nullptr;
            PdfTextLine* currLine = lines.at(i);
            PdfTextLine* nextLine = i < lines.size() - 1 ? lines.at(i + 1) : nullptr;

            _log->debug(p) << "-------" << endl;
            if (prevLine) {
              _log->debug(p) << "\033[1mPrev Line: "
                  << "\033[1mpage:\033[0m " << prevLine->position->pageNum
                  << "; \033[1mleftX:\033[0m " << prevLine->position->leftX
                  << "; \033[1mupperY:\033[0m " << prevLine->position->upperY
                  << "; \033[1mrightX:\033[0m " << prevLine->position->rightX
                  << "; \033[1mlowerY:\033[0m " << prevLine->position->lowerY
                  << "; \033[1mtext:\033[0m \"" << prevLine->text << "\"" << endl;
            }
            _log->debug(p) << "\033[1mCurr Line: "
                << "\033[1mpage:\033[0m " << currLine->position->pageNum
                << "; \033[1mleftX:\033[0m " << currLine->position->leftX
                << "; \033[1mupperY:\033[0m " << currLine->position->upperY
                << "; \033[1mrightX:\033[0m " << currLine->position->rightX
                << "; \033[1mlowerY:\033[0m " << currLine->position->lowerY
                << "; \033[1mtext:\033[0m \"" << currLine->text << "\"" << endl;
            if (nextLine) {
              _log->debug(p) << "\033[1mNext Line: "
                  << "\033[1mpage:\033[0m " << nextLine->position->pageNum
                  << "; \033[1mleftX:\033[0m " << nextLine->position->leftX
                  << "; \033[1mupperY:\033[0m " << nextLine->position->upperY
                  << "; \033[1mrightX:\033[0m " << nextLine->position->rightX
                  << "; \033[1mlowerY:\033[0m " << nextLine->position->lowerY
                  << "; \033[1mtext:\033[0m \"" << nextLine->text << "\"" << endl;
            }

            // Compute the horizontal distance and the vertical overlap to the previous line.
            double prevLineXGap = 0.0;
            double prevLineYOverlap = 0.0;
            if (prevLine) {
              prevLineXGap = computeHorizontalGap(prevLine, currLine);
              std::pair<double, double> yOverlapRatios = computeYOverlapRatios(prevLine, currLine);
              prevLineYOverlap = max(yOverlapRatios.first, yOverlapRatios.second);
            }
            _log->debug(p) << " └─ prevLine.xGap: " << prevLineXGap << endl;
            _log->debug(p) << " └─ prevLine.yOverlap: " << prevLineYOverlap << endl;

            // Compute the horizontal distance and the vertical overlap to the next line.
            double nextLineXGap = 0.0;
            double nextLineYOverlap = 0.0;
            if (nextLine) {
              nextLineXGap = computeHorizontalGap(currLine, nextLine);
              std::pair<double, double> yOverlapRatios = computeYOverlapRatios(currLine, nextLine);
              nextLineYOverlap = max(yOverlapRatios.first, yOverlapRatios.second);
            }
            _log->debug(p) << " └─ nextLine.xGap: " << nextLineXGap << endl;
            _log->debug(p) << " └─ nextLine.yOverlap: " << nextLineYOverlap << endl;

            // Define a threshold for the vertical overlap between the current line and the
            // previous line (next line). The previous line (next line) must exceed this threshold
            // in order to be considered to be merged with the current line.
            // The threshold is defined dependent on the horizontal distance between the lines.
            // The premise is as follows: If the horizontal distance between two lines is small,
            // the threshold should be less restrictive. If the horizontal distance is large, the
            // threshold should be more restrictive.
            // TODO: Parameterize the values.
            double prevLineThreshold = prevLineXGap < 3 * _doc->avgGlyphWidth ? 0.4 : 0.8;
            double nextLineThreshold = nextLineXGap < 3 * _doc->avgGlyphWidth ? 0.4 : 0.8;
            _log->debug(p) << " └─ prevLineThreshold: " << prevLineThreshold << endl;
            _log->debug(p) << " └─ nextLineThreshold: " << nextLineThreshold << endl;

            // Check if to merge the previous line with the current line.
            if (larger(prevLineYOverlap, nextLineYOverlap, 0.001)) {
              if (equalOrLarger(prevLineYOverlap, prevLineThreshold, 0.001)) {
                mergeTextLines(prevLine, currLine);
                merged = true;

                _log->debug(p) << "\033[1mMerged prev line with curr line.\033[0m" << endl;
                _log->debug(p) << " └─ line.pageNum: " << prevLine->position->pageNum << endl;
                _log->debug(p) << " └─ line.leftX: " << prevLine->position->leftX << endl;
                _log->debug(p) << " └─ line.upperY: " << prevLine->position->upperY << endl;
                _log->debug(p) << " └─ line.rightX: " << prevLine->position->rightX << endl;
                _log->debug(p) << " └─ line.lowerY: " << prevLine->position->lowerY << endl;
                _log->debug(p) << " └─ line.text: \"" << prevLine->text << "\"" << endl;

                continue;
              }
            }

            // Do not merge.
            nLines.push_back(currLine);
          }

          if (!merged) {
            _log->debug(p) << "-------" << endl;
            _log->debug(p) << "Final text lines:" << endl;

            for (size_t i = 0; i < nLines.size(); i++) {
              PdfTextLine* line = nLines[i];
              line->rank = i;
              segment->lines.push_back(line);

              _log->debug(p) << "\033[1mLine: \033[1mpage:\033[0m " << line->position->pageNum
                << "; \033[1mleftX:\033[0m " << line->position->leftX
                << "; \033[1mupperY:\033[0m " << line->position->upperY
                << "; \033[1mrightX:\033[0m " << line->position->rightX
                << "; \033[1mlowerY:\033[0m " << line->position->lowerY
                << "; \033[1mtext:\033[0m \"" << line->text << "\"" << endl;
            }
            break;
          }

          lines = nLines;
        }
      }

      for (size_t i = 0; i < segment->lines.size(); i++) {
        PdfTextLine* prevLine = i > 0 ? segment->lines[i-1] : nullptr;
        PdfTextLine* currLine = segment->lines[i];
        PdfTextLine* nextLine = i < segment->lines.size() - 1 ? segment->lines[i+1] : nullptr;

        currLine->prevLine = prevLine;
        currLine->nextLine = nextLine;
      }

      tuple<double, double, double, double> trimBox = page_segment_utils::computeTrimBox(segment);
      segment->trimLeftX = get<0>(trimBox);
      segment->trimUpperY = get<1>(trimBox);
      segment->trimRightX = get<2>(trimBox);
      segment->trimLowerY = get<3>(trimBox);
    }

    text_lines_utils::computeTextLineHierarchies(page);
  }
}

// _________________________________________________________________________________________________
void TextLinesDetector::createTextLine(const vector<PdfWord*>& words,
    const PdfPageSegment* segment, vector<PdfTextLine*>* lines) const {
  if (words.empty()) {
    return;
  }

  PdfTextLine* line = new PdfTextLine();
  line->id = createRandomString(8, "line-");
  line->doc = _doc;

  // Set the segment.
  line->segment = segment;

  // Set the words.
  line->words = words;

  computeTextLineProperties(line);

  lines->push_back(line);
}

// _________________________________________________________________________________________________
void TextLinesDetector::mergeTextLines(PdfTextLine* line1, PdfTextLine* line2) const {
  line1->words.insert(line1->words.end(), line2->words.begin(), line2->words.end());
  computeTextLineProperties(line1);
}

// _________________________________________________________________________________________________
void TextLinesDetector::computeTextLineProperties(PdfTextLine* line) const {
  // Abort if there are no words.
  if (line->words.empty()) {
    return;
  }

  // Set the writing mode.
  line->position->wMode = line->words[0]->position->wMode;

  // Set the rotation value.
  line->position->rotation = line->words[0]->position->rotation;

  // Set the page number.
  line->position->pageNum = line->words[0]->position->pageNum;

  // Sort the words by their leftX-coordinates, in ascending or descending order, depending
  // on the rotation.
  if (line->position->rotation == 0 || line->position->rotation == 1) {
    sort(line->words.begin(), line->words.end(), [](const PdfWord* w1, const PdfWord* w2) {
      return w1->position->getRotLeftX() < w2->position->getRotLeftX();
    });
  } else {
    sort(line->words.begin(), line->words.end(), [](const PdfWord* w1, const PdfWord* w2) {
      return w1->position->getRotLeftX() > w2->position->getRotLeftX();
    });
  }

  // Iteratively compute the text, the x,y-coordinates of the bounding box, and the font info.
  string text;
  unordered_map<string, int> fontNameFreqs;
  unordered_map<double, int> fontSizeFreqs;
  unordered_map<double, int> baseFreqs;
  for (size_t i = 0; i < line->words.size(); i++) {
    PdfWord* word = line->words[i];

    double wordMinX = min(word->position->leftX, word->position->rightX);
    double wordMinY = min(word->position->lowerY, word->position->upperY);
    double wordMaxX = max(word->position->leftX, word->position->rightX);
    double wordMaxY = max(word->position->lowerY, word->position->upperY);

    line->position->leftX = min(line->position->leftX, wordMinX);
    line->position->upperY = min(line->position->upperY, wordMinY);
    line->position->rightX = max(line->position->rightX, wordMaxX);
    line->position->lowerY = max(line->position->lowerY, wordMaxY);

    // Compute the most frequent font name, font size and baseline among the glyphs.
    for (const auto* glyph : word->glyphs) {
      fontNameFreqs[glyph->fontName]++;
      fontSizeFreqs[glyph->fontSize]++;
      baseFreqs[glyph->base]++;
    }

    // Append the text of the word, separated by whitespace
    text += word->text;
    if (i < line->words.size() - 1) {
      text += " ";
    }

    word->line = line;
  }

  // Set the text.
  line->text = text;

  // Compute and set the most frequent font name.
  int mostFreqFontNameCount = 0;
  for (const auto& pair : fontNameFreqs) {
    if (pair.second > mostFreqFontNameCount) {
      line->fontName = pair.first;
      mostFreqFontNameCount = pair.second;
    }
  }

  // Compute and set the most frequent font size and the maximum font size.
  int mostFreqFontSizeCount = 0;
  double mostFreqFontSize = 0;
  double maxFontSize = 0;
  for (const auto& pair : fontSizeFreqs) {
    if (pair.second > mostFreqFontSizeCount) {
      mostFreqFontSize = pair.first;
      mostFreqFontSizeCount = pair.second;
    }
    maxFontSize = max(maxFontSize, pair.first);
  }
  line->fontSize = mostFreqFontSize;
  line->maxFontSize = maxFontSize;

  // Compute and set the most frequent baseline.
  int mostFreqBaseCount = 0;
  for (const auto& pair : baseFreqs) {
    if (pair.second > mostFreqBaseCount) {
      line->base = pair.first;
      mostFreqBaseCount = pair.second;
    }
  }
}
