/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // std::min, std::max
#include <sstream>  // std::stringstream
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "./utils/Comparators.h"
#include "./utils/Counter.h"
#include "./utils/Log.h"
#include "./utils/Math.h"
#include "./utils/PdfElementsUtils.h"
#include "./utils/Text.h"
#include "./utils/TextLinesDetectionUtils.h"
#include "./Config.h"
#include "./PdfDocument.h"
#include "./TextLinesDetection.h"

using std::endl;
using std::get;
using std::max;
using std::min;
using std::string;
using std::tuple;
using std::unordered_map;
using std::vector;

using ppp::config::TextLinesDetectionConfig;
using ppp::utils::counter::DoubleCounter;
using ppp::utils::counter::StringCounter;
using ppp::utils::text::createRandomString;
using ppp::utils::TextLinesDetectionUtils;
using ppp::utils::comparators::LeftXAscComparator;
using ppp::utils::comparators::RotLeftXAscComparator;
using ppp::utils::comparators::RotLeftXDescComparator;
using ppp::utils::comparators::RotLowerYAscComparator;
using ppp::utils::comparators::RotLowerYDescComparator;
using ppp::utils::elements::computeHorizontalGap;
using ppp::utils::elements::computeMaxYOverlapRatio;
using ppp::utils::log::Logger;
using ppp::utils::log::BOLD;
using ppp::utils::log::GRAY;
using ppp::utils::log::OFF;
using ppp::utils::math::equalOrLarger;
using ppp::utils::math::round;

// =================================================================================================

namespace ppp {

// _________________________________________________________________________________________________
TextLinesDetection::TextLinesDetection(PdfDocument* doc, const TextLinesDetectionConfig& config) {
  _doc = doc;
  _config = config;
  _utils = new TextLinesDetectionUtils(config);
  _log = new Logger(config.logLevel, config.logPageFilter);
}

// _________________________________________________________________________________________________
TextLinesDetection::~TextLinesDetection() {
  delete _log;
  delete _utils;
}

// _________________________________________________________________________________________________
void TextLinesDetection::process() {
  assert(_doc);

  _log->info() << "Detecting text lines..." << endl;
  _log->debug() << "=========================================================" << endl;
  _log->debug() << BOLD << "DEBUG MODE" << OFF << endl;
  _log->debug() << "=========================================================" << endl;

  // Process the PDF document page-wise and segment-wise.
  int numLines = 0;
  for (const auto* page : _doc->pages) {
    int p = page->pageNum;
    for (auto* segment : page->segments) {
      _log->debug(p) << BOLD << "PROCESSING SEGMENT \"" << segment->id << "\"." << OFF << endl;
      _log->debug(p) << "=========================================================" << endl;

      // Prefix each subsequent log message with the segment id, for convenience purposes.
      std::stringstream s;
      s << GRAY << "(" << segment->id << ") " << OFF;
      string q = s.str();

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

      _log->debug(p) << q << BOLD << "Clustering words" << OFF << endl;
      _log->debug(p) << "=========================================================" << endl;

      // Cluster the words first by their rotations, then by their lowerY values.
      unordered_map<int, unordered_map<double, vector<PdfWord*>>> clusters;
      for (size_t i = 0; i < words.size(); i++) {
        PdfWord* word = words[i];
        int p = word->pos->pageNum;

        if (i > 0) {
          _log->debug(p) << "---------------------------------------------------------" << endl;
        }
        _log->debug(p) << q << BOLD << "word: \"" << word->text << "\"" << OFF << endl;
        _log->debug(p) << q << " └─ word.page: " << word->pos->pageNum << endl;
        _log->debug(p) << q << " └─ word.leftX: " << word->pos->leftX << endl;
        _log->debug(p) << q << " └─ word.upperY: " << word->pos->upperY << endl;
        _log->debug(p) << q << " └─ word.rightX: " << word->pos->rightX << endl;
        _log->debug(p) << q << " └─ word.lowerY: " << word->pos->lowerY << endl;
        _log->debug(p) << q << " └─ word.rot: " << word->pos->rotation << endl;
        if (word->pos->rotation != 0) {
          _log->debug(p) << q << " └─ word.rotLeftX: " << word->pos->getRotLeftX() << endl;
          _log->debug(p) << q << " └─ word.rotUpperY: " << word->pos->getRotUpperY() << endl;
          _log->debug(p) << q << " └─ word.rotRightX: " << word->pos->getRotRightX() << endl;
          _log->debug(p) << q << " └─ word.rotLowerY: " << word->pos->getRotLowerY() << endl;
        }

        // Skip the word if it is part of a stacked math symbol.
        if (word->isPartOfStackedMathSymbol) {
          _log->debug(p) << q << BOLD << "skipping word (part of stacked symbol)." << OFF << endl;
          continue;
        }

        double rotation = word->pos->rotation;
        double lowerY = round(word->pos->getRotLowerY(), _config.coordinatePrecision);
        clusters[rotation][lowerY].push_back(word);
        _log->debug(p) << q << "cluster: (" << rotation << ", " << lowerY << ")" << endl;

        // If the word is the base word of a stacked math symbol, add each word that is part
        // of the same stacked math symbol to the same cluster.
        for (auto* w : word->isBaseOfStackedMathSymbol) {
          _log->debug(p) << "adding \"" << w->text << "\" (part of stacked math symbol)" << endl;
          clusters[rotation][lowerY].push_back(w);
        }
      }

      // Iterate through the clusters and create a text line for each.
      for (const auto& pair : clusters) {  // pair: (rotation, words per leftX value)
        int rot = pair.first;

        _log->debug(p) << "=========================================================" << endl;
        _log->debug(p) << q << BOLD << "PROCESSING CLUSTERS, ROTATION " << rot << OFF << endl;
        _log->debug(p) << "=========================================================" << endl;

        // Prefix each subsequent log message with the segment id and the rotation.
        std::stringstream ss;
        ss << q << GRAY << "(rot-" << rot << ") " << OFF;
        string qq = ss.str();

        _log->debug(p) << qq << BOLD << "Creating text lines" << OFF << endl;
        _log->debug(p) << "=========================================================" << endl;

        vector<PdfTextLine*> lines;
        for (const auto& e : pair.second) {  // e: (leftX, words)
          double lowerY = e.first;

          PdfTextLine* line = createTextLine(e.second, segment, &lines);
          if (lines.size() > 1) {
            _log->debug(p) << "---------------------------------------------------------" << endl;
          }
          _log->debug(p) << qq << BOLD << "cluster(" << rot << ", " << lowerY << ")" << OFF << endl;
          _log->debug(p) << qq << "  └─ line.text: \"" << line->text << "\"" << endl;
          _log->debug(p) << qq << "  └─ line.pageNum: " << line->pos->pageNum << endl;
          _log->debug(p) << qq << "  └─ line.leftX: " << line->pos->leftX << endl;
          _log->debug(p) << qq << "  └─ line.upperY: " << line->pos->upperY << endl;
          _log->debug(p) << qq << "  └─ line.rightX: " << line->pos->rightX << endl;
          _log->debug(p) << qq << "  └─ line.lowerY: " << line->pos->lowerY << endl;
        }

        // Skip the cluster if it does not contain any text lines.
        if (lines.empty()) {
          continue;
        }

        // Sort the lines by their lower y-values in asc or desc order, depending on the rotation.
        // This should sort the lines from "top to bottom".
        _log->debug(p) << "=========================================================" << endl;
        _log->debug(p) << qq << BOLD << "Sorting text lines" << OFF << endl;
        _log->debug(p) << "=========================================================" << endl;

        if (rot == 0 || rot == 1) {
          sort(lines.begin(), lines.end(), RotLowerYAscComparator());
        } else {
          sort(lines.begin(), lines.end(), RotLowerYDescComparator());
        }

        for (const auto* line : lines) {
          _log->debug(p) << qq << line->text << endl;
        }

        // Merge consecutive text lines that vertically overlap in rounds. Repeat this until there
        // are no text lines anymore which vertically overlap. This should merge words that were
        // assigned to different clusters but actually belong to the same text line, because they
        // are sub- or superscripted, or they are parts of fractions in formulas.
        int r = 0;
        while (true) {
          r++;
          _log->debug(p) << "=========================================================" << endl;
          _log->debug(p) << qq << BOLD << "Merging overlapping lines, round " << r << OFF << endl;
          _log->debug(p) << "=========================================================" << endl;

          // Prefix each subsequent log message with the segment id, the rotation, and the round.
          std::stringstream sss;
          sss << qq << GRAY << "(round-" << r << ") " << OFF;
          string qqq = sss.str();

          bool merged = false;
          vector<PdfTextLine*> mergedLines;
          for (size_t i = 0; i < lines.size(); i++) {
            PdfTextLine* prevLine = !mergedLines.empty() ? mergedLines.back() : nullptr;
            PdfTextLine* currLine = lines[i];

            if (i > 0) {
              _log->debug(p) << "-------------------------------------------------------" << endl;
            }

            if (prevLine) {
              _log->debug(p) << qqq << BOLD << "prevLine: " << OFF << prevLine->text << endl;
              _log->debug(p) << qqq << " └─ prevLine.page: " << prevLine->pos->pageNum << endl;
              _log->debug(p) << qqq << " └─ prevLine.leftX: " << prevLine->pos->leftX << endl;
              _log->debug(p) << qqq << " └─ prevLine.upperY: " << prevLine->pos->upperY << endl;
              _log->debug(p) << qqq << " └─ prevLine.rightX: " << prevLine->pos->rightX << endl;
              _log->debug(p) << qqq << " └─ prevLine.lowerY: " << prevLine->pos->lowerY << endl;
            } else {
              _log->debug(p) << qqq << BOLD << "prevLine: -" << OFF << endl;
            }

            _log->debug(p) << qqq << BOLD << "currLine: " << OFF << currLine->text << endl;
            _log->debug(p) << qqq << " └─ currLine.page: " << currLine->pos->pageNum << endl;
            _log->debug(p) << qqq << " └─ currLine.leftX: " << currLine->pos->leftX << endl;
            _log->debug(p) << qqq << " └─ currLine.upperY: " << currLine->pos->upperY << endl;
            _log->debug(p) << qqq << " └─ currLine.rightX: " << currLine->pos->rightX << endl;
            _log->debug(p) << qqq << " └─ currLine.lowerY: " << currLine->pos->lowerY << endl;
            _log->debug(p) << qqq << "------------------" << endl;

            // Compute the horizontal gap and the vertical overlap ratio to the previous line.
            double xGap = 0.0;
            double yOverlapRatio = 0.0;
            if (prevLine) {
              xGap = computeHorizontalGap(prevLine, currLine);
              yOverlapRatio = computeMaxYOverlapRatio(prevLine, currLine);
            }

            // Define a threshold for the vertical overlap ratio between the current line and the
            // previous line. The current line must exceed this threshold in order to be merged
            // with the previous line
            // The threshold is defined dependent on the horizontal gap between the lines. The
            // rationale behind is as follows: If the horizontal gap between two lines is small,
            // the threshold should be less restrictive. If the horizontal gap is large, the
            // threshold should be more restrictive.
            double threshold = _config.getYOverlapRatioThreshold(_doc, xGap);

            _log->debug(p) << qqq << "max y-overlap ratio: " << yOverlapRatio << endl;
            _log->debug(p) << qqq << "threshold: " << threshold << endl;

            // Merge the current line with the previous line when the vertical overlap between the
            // lines is larger or equal to the threshold.
            if (equalOrLarger(yOverlapRatio, threshold)) {
              mergeTextLines(currLine, prevLine);

              _log->debug(p) << qqq << BOLD << "merge currLine with prevLine" << OFF << endl;
              _log->debug(p) << qqq << " └─ prevLine.text: \"" << prevLine->text << "\"" << endl;
              _log->debug(p) << qqq << " └─ prevLine.page: " << prevLine->pos->pageNum << endl;
              _log->debug(p) << qqq << " └─ prevLine.leftX: " << prevLine->pos->leftX << endl;
              _log->debug(p) << qqq << " └─ prevLine.upperY: " << prevLine->pos->upperY << endl;
              _log->debug(p) << qqq << " └─ prevLine.rightX: " << prevLine->pos->rightX << endl;
              _log->debug(p) << qqq << " └─ prevLine.lowerY: " << prevLine->pos->lowerY << endl;

              merged = true;
              continue;
            } else {
              _log->debug(p) << qqq << BOLD << "do not merge" << OFF << endl;
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

      // TODO(korzen): Is the following still needed?
      // Compute the trim box of the segment.
      // tuple<double, double, double, double> trimBox = page_segment_utils::computeTrimBox(
      //   segment,
      //   _config.pageSegmentation.trimBoxCoordsPrec,
      //   _config.pageSegmentation.minPrecLinesSameRightX);
      // segment->trimLeftX = get<0>(trimBox);
      // segment->trimUpperY = get<1>(trimBox);
      // segment->trimRightX = get<2>(trimBox);
      // segment->trimLowerY = get<3>(trimBox);

      // _log->debug(p) << "---------------------------------------------------------" << endl;

      // _log->debug(p) << q << BOLD << "Computed trim box" << OFF << endl;
      // _log->debug(p) << q << " └─ segment.trimLeftX: " << segment->trimLeftX << endl;
      // _log->debug(p) << q << " └─ segment.trimUpperY: " << segment->trimUpperY << endl;
      // _log->debug(p) << q << " └─ segment.trimRightX: " << segment->trimRightX << endl;
      // _log->debug(p) << q << " └─ segment.trimLowerY: " << segment->trimLowerY << endl;

      // _log->debug(p) << "=========================================================" << endl;
    }

    // Compute the text lines hierarchies.
    _utils->computeTextLineHierarchy(page);
  }
}

// _________________________________________________________________________________________________
PdfTextLine* TextLinesDetection::createTextLine(const vector<PdfWord*>& words,
    const PdfPageSegment* segment, vector<PdfTextLine*>* lines) const {
  assert(!words.empty());
  assert(segment);
  assert(lines);

  PdfTextLine* line = new PdfTextLine();
  line->doc = _doc;

  // Create a (unique) id.
  line->id = createRandomString(_config.idLength, "line-");

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
void TextLinesDetection::mergeTextLines(const PdfTextLine* line1, PdfTextLine* line2) const {
  assert(line1);
  assert(line2);

  line2->words.insert(line2->words.end(), line1->words.begin(), line1->words.end());
  computeTextLineProperties(line2);
}

// _________________________________________________________________________________________________
void TextLinesDetection::computeTextLineProperties(PdfTextLine* line) const {
  assert(line);

  // Do nothing if the line contains no words.
  if (line->words.empty()) {
    return;
  }

  // Set the rotation value.
  double rotation = line->pos->rotation = line->words[0]->pos->rotation;

  // Set the writing mode.
  line->pos->wMode = line->words[0]->pos->wMode;

  // Set the page number.
  line->pos->pageNum = line->words[0]->pos->pageNum;

  // Sort the words by their leftX-coordinates, in ascending or descending order, depending
  // on the rotation.
  if (rotation == 0 || rotation == 1) {
    sort(line->words.begin(), line->words.end(), RotLeftXAscComparator());
  } else {
    sort(line->words.begin(), line->words.end(), RotLeftXDescComparator());
  }

  // Iterate through the words from left to right and compute the text, the x,y-coordinates of the
  // bounding box, and the font info.
  string text;
  StringCounter fontNameCounter;
  DoubleCounter fontSizeCounter;
  DoubleCounter baseCounter;
  for (size_t i = 0; i < line->words.size(); i++) {
    PdfWord* word = line->words[i];

    double wordMinX = min(word->pos->leftX, word->pos->rightX);
    double wordMinY = min(word->pos->lowerY, word->pos->upperY);
    double wordMaxX = max(word->pos->leftX, word->pos->rightX);
    double wordMaxY = max(word->pos->lowerY, word->pos->upperY);

    line->pos->leftX = min(line->pos->leftX, wordMinX);
    line->pos->upperY = min(line->pos->upperY, wordMinY);
    line->pos->rightX = max(line->pos->rightX, wordMaxX);
    line->pos->lowerY = max(line->pos->lowerY, wordMaxY);

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

}  // namespace ppp
