/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TEXTBLOCKSDETECTOR_H_
#define TEXTBLOCKSDETECTOR_H_

#include <vector>

#include "./utils/Utils.h"
#include "./utils/LogUtils.h"
#include "./PdfDocument.h"

using namespace std;

// ==================================================================================================

class TextBlocksDetector {
 public:
  TextBlocksDetector(PdfDocument* doc, bool debug=false, int debugPageFilter=-1);

  ~TextBlocksDetector();

  void detect();

 private:
  bool startsPreliminaryBlock(const PdfTextLine* line) const;

  bool startsBlock(const PdfTextBlock* block, const PdfTextLine* line);

  void createTextBlock(const PdfPageSegment* segment, const vector<PdfTextLine*>& lines, vector<PdfTextBlock*>* blocks);


  PdfDocument* _doc;

  unordered_set<string> _potentialFootnoteLabels;

  Logger* _log;


  // ===============================================================================================

  Trool startsBlock_sameFigure(const PdfTextLine* line, bool verbose=true) const;
  Trool startsBlock_rotation(const PdfTextLine* line, bool verbose=true) const;
  Trool startsBlock_wMode(const PdfTextLine* line, bool verbose=true) const;
  Trool startsBlock_fontSize(const PdfTextLine* line, double maxDelta=1.0, bool verbose=true) const;
  Trool startsBlock_lineDistance(const PdfTextLine* line, double minTolerance=1.0,
      double toleranceFactor=0.1, bool verbose=true) const;
  Trool startsBlock_lineDistanceIncrease(const PdfTextLine* line, double toleranceFactor=0.5,
      bool verbose=true) const;
  Trool startsBlock_centered(const PdfTextBlock* block, const PdfTextLine* line, bool verbose=true) const;
  Trool startsBlock_item(const PdfTextBlock* block, const PdfTextLine* line, bool verbose=true) const;
  Trool startsBlock_emphasized(const PdfTextLine* line, bool verbose=true) const;
  Trool startsBlock_hangingIndent(const PdfTextBlock* block, const PdfTextLine* line,
      bool verbose=true) const;
  Trool startsBlock_indent(const PdfTextLine* line, bool verbose=true) const;
};

#endif  // TEXTBLOCKDETECTOR_H_
