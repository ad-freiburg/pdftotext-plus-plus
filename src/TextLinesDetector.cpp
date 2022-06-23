/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // min, max
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "./utils/Comparators.h"
#include "./utils/Counter.h"
#include "./utils/Log.h"
#include "./utils/MathUtils.h"
#include "./utils/PageSegmentsUtils.h"
#include "./utils/PdfElementsUtils.h"
#include "./utils/StringUtils.h"
#include "./utils/TextLinesUtils.h"

#include "./PdfDocument.h"
#include "./TextLinesDetector.h"

using global_config::COORDS_PREC;

using std::endl;
using std::get;
using std::max;
using std::min;
using std::string;
using std::tuple;
using std::unordered_map;
using std::vector;

// _________________________________________________________________________________________________
TextLinesDetector::TextLinesDetector(const PdfDocument* doc, bool debug, int debugPageFilter) {
  _log = new Logger(debug ? DEBUG : INFO, debugPageFilter);
  _config = new TextLinesDetectorConfig();
  _doc = doc;
}

// _________________________________________________________________________________________________
TextLinesDetector::~TextLinesDetector() {
  delete _config;
  delete _log;
}

// _________________________________________________________________________________________________
void TextLinesDetector::process() {
  assert(_doc);

  _log->debug() << BOLD << "Text Lines Detection - DEBUG MODE" << OFF << endl;

  // Do nothing if the document does not contain any pages.
  if (_doc->pages.empty()) {
    return;
  }

  // Process the PDF document page-wise and segment-wise.
  int numLines = 0;
  for (const auto* page : _doc->pages) {
    int p = page->pageNum;
    _log->debug(p) << "=======================================" << endl;
    _log->debug(p) << BOLD << "PROCESSING PAGE " << p << OFF << endl;
    _log->debug(p) << " └─ # segments: " << page->segments.size() << endl;

    for (auto* segment : page->segments) {
      _log->debug(p) << "---------------------------------------" << endl;
      _log->debug(p) << "PROCESSING SEGMENT " << segment->id << endl;

      // Create a vector containing only the words (but not figures or shapes) of the segment.
      vector<PdfWord*> words;
      for (auto* element : segment->elements) {
        PdfWord* word = dynamic_cast<PdfWord*>(element);
        if (word) {
          words.push_back(word);
        }
      }

      // Skip the segment if it doesn't contain any words.
      if (words.empty()) {
        continue;
      }

      _log->debug(p) << "----------- CLUSTERING WORDS -----------" << endl;

      // Cluster the words first by their rotations, then by their lowerY values.
      unordered_map<int, unordered_map<double, vector<PdfWord*>>> clusters;
      for (auto* word : words) {
        int p = word->position->pageNum;

        _log->debug(p) << BOLD << "word: \"" << word->text << "\"" << OFF << endl;
        _log->debug(p) << " └─ word.page: " << word->position->pageNum << endl;
        _log->debug(p) << " └─ word.leftX: " << word->position->leftX << endl;
        _log->debug(p) << " └─ word.upperY: " << word->position->upperY << endl;
        _log->debug(p) << " └─ word.rightX: " << word->position->rightX << endl;
        _log->debug(p) << " └─ word.lowerY: " << word->position->lowerY << endl;
        if (word->position->rotation != 0) {
          _log->debug(p) << " └─ word.rot: " << word->position->rotation << endl;
          _log->debug(p) << " └─ word.rotLeftX: " << word->position->getRotLeftX() << endl;
          _log->debug(p) << " └─ word.rotUpperY: " << word->position->getRotUpperY() << endl;
          _log->debug(p) << " └─ word.rotRightX: " << word->position->getRotRightX() << endl;
          _log->debug(p) << " └─ word.rotLowerY: " << word->position->getRotLowerY() << endl;
        }

        // Skip the word if it is part of a stacked math symbol.
        if (word->isPartOfStackedMathSymbol) {
          _log->debug(p) << BOLD << "skipping word (part of stacked math symbol)." << OFF << endl;
          continue;
        }

        double rotation = word->position->rotation;
        double lowerY = math_utils::round(word->position->getRotLowerY(), COORDS_PREC);
        clusters[rotation][lowerY].push_back(word);
        _log->debug(p) << " └─ cluster: (" << rotation << ", " << lowerY << ")" << endl;

        // If the word is the base word of a stacked math symbol, add each word that is part
        // of the same stacked math symbol to the same cluster.
        for (auto* w : word->isBaseOfStackedMathSymbol) {
          _log->debug(p) << "Is base word of stacked math symbol; adding " << w->text << endl;
          clusters[rotation][lowerY].push_back(w);
        }
      }

      _log->debug(p) << "--------- CREATING TEXT LINES ---------" << endl;

      // Iterate through the clusters and create a text line for each.
      for (const auto& pair : clusters) {  // pair: (rotation, words per leftX value)
        int rot = pair.first;

        vector<PdfTextLine*> lines;
        for (const auto& e : pair.second) {  // e: (leftX, words)
          double lowerY = e.first;

          PdfTextLine* line = createTextLine(e.second, segment, &lines);

          _log->debug(p) << "Created line from cluster (" << rot << ", " << lowerY << ")" << endl;
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
        // This should sort the lines from "top to bottom".
        _log->debug(p) << "-------" << endl;
        _log->debug(p) << BOLD << "Sorting text lines..." << OFF << endl;
        if (rot == 0 || rot == 1) {
          sort(lines.begin(), lines.end(), comparators::RotLowerYAscComparator());
        } else {
          sort(lines.begin(), lines.end(), comparators::RotLowerYDescComparator());
        }

        // Merge consecutive text lines that vertically overlap in rounds. Repeat this until there
        // are no text lines anymore which vertically overlap. This should merge words that were
        // assigned to different clusters but actually belong to the same text line, because they
        // are sub- or superscripted, or they are parts of fractions in formulas.
        int round = 0;
        while (true) {
          _log->debug(p) << "=======" << endl;
          _log->debug(p) << BOLD << "Merging overlapping lines, round " << ++round << OFF << endl;

          bool merged = false;
          vector<PdfTextLine*> mergedLines;
          for (size_t i = 0; i < lines.size(); i++) {
            PdfTextLine* prevLine = !mergedLines.empty() ? mergedLines.back() : nullptr;
            PdfTextLine* currLine = lines[i];

            _log->debug(p) << "-------" << endl;

            if (prevLine) {
              _log->debug(p) << BOLD << "prevLine: \"" << prevLine->text << "\"" << OFF << endl;
              _log->debug(p) << " └─ prevLine.pageNum: " << prevLine->position->pageNum << endl;
              _log->debug(p) << " └─ prevLine.leftX: " << prevLine->position->leftX << endl;
              _log->debug(p) << " └─ prevLine.upperY: " << prevLine->position->upperY << endl;
              _log->debug(p) << " └─ prevLine.rightX: " << prevLine->position->rightX << endl;
              _log->debug(p) << " └─ prevLine.lowerY: " << prevLine->position->lowerY << endl;
            }

            _log->debug(p) << BOLD << "currLine: \"" << currLine->text << "\"" << OFF << endl;
            _log->debug(p) << " └─ currLine.pageNum: " << currLine->position->pageNum << endl;
            _log->debug(p) << " └─ currLine.leftX: " << currLine->position->leftX << endl;
            _log->debug(p) << " └─ currLine.upperY: " << currLine->position->upperY << endl;
            _log->debug(p) << " └─ currLine.rightX: " << currLine->position->rightX << endl;
            _log->debug(p) << " └─ currLine.lowerY: " << currLine->position->lowerY << endl;

            // Compute the horizontal gap and the vertical overlap ratio to the previous line.
            double xGap = 0.0;
            double yOverlapRatio = 0.0;
            if (prevLine) {
              xGap = element_utils::computeHorizontalGap(prevLine, currLine);
              yOverlapRatio = element_utils::computeMaxYOverlapRatio(prevLine, currLine);
            }
            _log->debug(p) << " └─ xGap (prevLine/currLine): " << xGap << endl;
            _log->debug(p) << " └─ yOverlapRatio (prevLine/currLine): " << yOverlapRatio << endl;

            // Define a threshold for the vertical overlap ratio between the current line and the
            // previous line. The current line must exceed this threshold in order to be merged
            // with the previous line
            // The threshold is defined dependent on the horizontal gap between the lines. The
            // rationale behind is as follows: If the horizontal gap between two lines is small,
            // the threshold should be less restrictive. If the horizontal gap is large, the
            // threshold should be more restrictive.
            double yOverlapRatioThreshold = _config->getYOverlapRatioThreshold(_doc, xGap);
            _log->debug(p) << " └─ yOverlapThreshold: " << yOverlapRatioThreshold << endl;

            // Merge the current line with the previous line when the vertical overlap between the
            // lines is larger or equal to the threshold.
            if (math_utils::equalOrLarger(yOverlapRatio, yOverlapRatioThreshold)) {
              mergeTextLines(currLine, prevLine);

              _log->debug(p) << BOLD << "Merged curr line with prev line." << OFF << endl;
              _log->debug(p) << " └─ prevLine.pageNum: " << prevLine->position->pageNum << endl;
              _log->debug(p) << " └─ prevLine.leftX: " << prevLine->position->leftX << endl;
              _log->debug(p) << " └─ prevLine.upperY: " << prevLine->position->upperY << endl;
              _log->debug(p) << " └─ prevLine.rightX: " << prevLine->position->rightX << endl;
              _log->debug(p) << " └─ prevLine.lowerY: " << prevLine->position->lowerY << endl;
              _log->debug(p) << " └─ prevLine.text: \"" << prevLine->text << "\"" << endl;

              merged = true;
              continue;
            }

            // Do not merge the lines. Instead, append the current line to the vector.
            mergedLines.push_back(currLine);
          }
          lines = mergedLines;

          // Abort if no text lines were merged in this round.
          if (!merged) {
            break;
          }
        }

        // For each line, set the references to the respective previous and next line.
        // Append the lines to segment->lines.
        for (size_t i = 0; i < lines.size(); i++) {
          PdfTextLine* prevLine = i > 0 ? lines[i - 1] : nullptr;
          PdfTextLine* currLine = lines[i];
          PdfTextLine* nextLine = i < lines.size() - 1 ? lines[i + 1] : nullptr;

          currLine->rank = numLines++;
          currLine->prevLine = prevLine;
          currLine->nextLine = nextLine;

          segment->lines.push_back(currLine);
        }
      }

      // Compute the trim box of the segment.
      tuple<double, double, double, double> trimBox = page_segment_utils::computeTrimBox(segment);
      segment->trimLeftX = get<0>(trimBox);
      segment->trimUpperY = get<1>(trimBox);
      segment->trimRightX = get<2>(trimBox);
      segment->trimLowerY = get<3>(trimBox);
    }

    // Compute the text lines hierarchies.
    text_lines_utils::computeTextLineHierarchy(page);
  }
}

// _________________________________________________________________________________________________
PdfTextLine* TextLinesDetector::createTextLine(const vector<PdfWord*>& words,
    const PdfPageSegment* segment, vector<PdfTextLine*>* lines) const {
  assert(!words.empty());
  assert(segment);
  assert(lines);

  PdfTextLine* line = new PdfTextLine();
  line->doc = _doc;

  // Create a (unique) id.
  line->id = string_utils::createRandomString(global_config::ID_LENGTH, "line-");

  // Set the words.
  line->words = words;

  // Set the reference to the parent segment.
  line->segment = segment;

  // Compute all other layout properties.
  computeTextLineProperties(line);

  lines->push_back(line);

  return line;
}

// _________________________________________________________________________________________________
void TextLinesDetector::mergeTextLines(const PdfTextLine* line1, PdfTextLine* line2) const {
  assert(line1);
  assert(line2);

  line2->words.insert(line2->words.end(), line1->words.begin(), line1->words.end());
  computeTextLineProperties(line2);
}

// _________________________________________________________________________________________________
void TextLinesDetector::computeTextLineProperties(PdfTextLine* line) const {
  assert(line);

  // Do nothing if the line contains no words.
  if (line->words.empty()) {
    return;
  }

  // Set the rotation value.
  double rotation = line->position->rotation = line->words[0]->position->rotation;

  // Set the writing mode.
  line->position->wMode = line->words[0]->position->wMode;

  // Set the page number.
  line->position->pageNum = line->words[0]->position->pageNum;

  // Sort the words by their leftX-coordinates, in ascending or descending order, depending
  // on the rotation.
  if (rotation == 0 || rotation == 1) {
    sort(line->words.begin(), line->words.end(), comparators::RotLeftXAscComparator());
  } else {
    sort(line->words.begin(), line->words.end(), comparators::RotLeftXDescComparator());
  }

  // Iterate through the words from left to right and compute the text, the x,y-coordinates of the
  // bounding box, and the font info.
  string text;
  StringCounter fontNameCounter;
  DoubleCounter fontSizeCounter;
  DoubleCounter baseCounter;
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

    // Compute the most frequent font name, font size and baseline among the characters.
    for (const auto* ch : word->characters) {
      fontNameCounter[ch->fontName]++;
      fontSizeCounter[ch->fontSize]++;
      baseCounter[ch->base]++;
    }

    // Append the text of the word, separated by a whitespace.
    text += word->text;
    if (i < line->words.size() - 1) {
      text += " ";
    }

    // For each word, set the reference to the text line.
    word->line = line;
  }

  // Set the text.
  line->text = text;

  // Compute and set the font info.
  line->fontName = fontNameCounter.mostFreq();
  line->fontSize = fontSizeCounter.mostFreq();
  line->maxFontSize = fontSizeCounter.max();
  line->base = baseCounter.mostFreq();
}
