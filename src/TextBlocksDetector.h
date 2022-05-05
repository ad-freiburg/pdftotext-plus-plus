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

#include "./utils/LogUtils.h"
#include "./PdfDocument.h"


class TextBlocksDetector {
 public:
  TextBlocksDetector(PdfDocument* doc, bool debug=false, int debugPageFilter=-1);

  ~TextBlocksDetector();

  void detect();

 private:
  bool startsPreliminaryTextBlock(const PdfTextLine* prevLine, const PdfTextLine* currLine,
      const PdfTextLine* nextLine) const;

  bool startsTextBlock(const PdfTextLine* prevLine, const PdfTextLine* currLine,
      const PdfTextLine* nextLine, const std::unordered_set<std::string>* potentialFootnoteMarkers,
      double hangingIndent);


  void createTextBlock(const std::vector<PdfTextLine*>& lines, std::vector<PdfTextBlock*>* blocks);

  bool isTextBlockEmphasized(const std::vector<PdfTextLine*>& lines);
  bool isTextLineEmphasized(const PdfTextLine* line);

  void computeMostFreqTextLineDistance();

  void computeTextLineIndentHierarchies();

  // void computeTextLineAlignments(const std::vector<PdfTextBlock*>& blocks);
  void computeTextLineMargins();

  bool isFirstLineOfItem(const PdfTextLine* line) const;
  bool isContinuationLineOfItem(const PdfTextLine* line) const;
  bool isFirstLineOfFootnote(const PdfTextLine* line, const std::unordered_set<std::string>*
      potentialFootnoteMarkers) const;
  bool isContinuationLineOfFootnote(const PdfTextLine* line,
      const std::unordered_set<std::string>* potentialFootnoteMarkers) const;
  double computeHangingIndent(const PdfTextBlock* block) const;
  void computePotentialFootnoteMarkers(const PdfPage* page,
      std::unordered_set<std::string>* footnoteMarkers) const;

  PdfFigure* isPartOfFigure(const PdfTextLine* line) const;

  PdfDocument* _doc;

  // The most frequent line distance in the document.
  double _mostFreqLineDistance;
  double _mostFreqLineLeftMargin;
  double _mostFreqLineRightMargin;
  double _percZeroRightMarginTextLines;

  // A mapping of a page number to the most freq. line distance in the respective page.
  std::unordered_map<int, double> _mostFreqLineDistancePerPage;

  // A mapping of a font size to the most freq. line distance among the text lines with the
  // respective font size.
  std::unordered_map<double, double> _mostFreqLineDistancePerFontSize;

  Logger* _log;
};

#endif  // TEXTBLOCKDETECTOR_H_
