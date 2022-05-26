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

  bool startsBlock(const PdfTextLine* line,
      const unordered_set<string>* potentialFootnoteMarkers,
      double hangingIndent, double percNoRightMarginLines, bool isCentered);


  void createTextBlock(const vector<PdfTextLine*>& lines, vector<PdfTextBlock*>* blocks);

  void computeMostFreqTextLineDistance();

  void computeTextLineIndentHierarchies();

  // void computeTextLineAlignments(const vector<PdfTextBlock*>& blocks);
  void computeTextLineMargins();

  void computeHangingIndents() const;
  void computePotentialFootnoteMarkers(const PdfPage* page,
      unordered_set<string>* footnoteMarkers) const;
  double computePercentageNoRightMarginLines(const PdfTextBlock* block) const;
  void computeTextBlockTrimBoxes() const;

  // This method returns true if the given previous and current lines are part of the same centered
  // block, false otherwise. If 'verbose' is set to true, this method will print debug information
  // for reproducing purposes.
  bool isCenteredBlock(const PdfTextLine* prevLine, const PdfTextLine* currLine,
      bool verbose=false) const;

  bool computeIsCentered(const PdfTextBlock* block) const;

  PdfDocument* _doc;

  // The most frequent line distance in the document.
  double _mostFreqLineDistance;
  double _mostFreqLineLeftMargin;
  double _mostFreqLineRightMargin;
  double _percZeroRightMarginTextLines;

  // A mapping of a page number to the most freq. line distance in the respective page.
  unordered_map<int, double> _mostFreqLineDistancePerPage;

  // A mapping of a font size to the most freq. line distance among the text lines with the
  // respective font size.
  unordered_map<double, double> _mostFreqLineDistancePerFontSize;

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
  Trool startsBlock_centered(const PdfTextLine* line, bool isCentered, bool verbose=true) const;
  Trool startsBlock_item(const PdfTextLine* line, bool isCentered,
      const unordered_set<string>* fnLabels, bool verbose=true) const;
  Trool startsBlock_emphasized(const PdfTextLine* line, bool verbose=true) const;
  Trool startsBlock_hangingIndent(const PdfTextLine* line, double hangingIndent,
      bool verbose=true) const;
  Trool startsBlock_indent(const PdfTextLine* line, double percNoRightMarginLines,
      bool verbose=true) const;
};

#endif  // TEXTBLOCKDETECTOR_H_
