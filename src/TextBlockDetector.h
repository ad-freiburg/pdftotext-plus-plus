/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TEXTBLOCKDETECTOR_H_
#define TEXTBLOCKDETECTOR_H_

#include <vector>

#include "./PdfDocument.h"


class TextBlockDetector {
 public:
  explicit TextBlockDetector(PdfDocument* doc);

  ~TextBlockDetector();

  void detect();

 private:
  bool startsNewTextBlock(const PdfTextLine* prevLine, const PdfTextLine* line,
      const PdfTextLine* nextLine);
  bool startsNewTextBlock2(const PdfTextLine* prevLine, const PdfTextLine* line);

  void createTextBlock(const std::vector<PdfTextLine*>& lines, std::vector<PdfTextBlock*>* blocks);

  bool computeIsTextBlockEmphasized(const std::vector<PdfTextLine*>& lines);

  void computeMostFrequentLinePitch();

  void computeMostFrequentLinePitchPerPage();

  void computeMostFrequentLinePitchPerFontSize();

  void computeTextLineIndentHierarchies();

  void computeTextLineAlignments(const std::vector<PdfTextBlock*>& blocks);

  bool isFirstLineOfItem(const PdfTextLine* line) const;
  bool isContinuationLineOfItem(const PdfTextLine* line) const;
  bool isFirstLineOfFootnote(const PdfTextLine* line) const;
  bool isContinuationLineOfFootnote(const PdfTextLine* line) const;

  bool isIndented(const PdfTextLine* line) const;
  bool isPartOfFigure(const PdfTextLine* line) const;

  bool isDisplayFormula(const PdfTextLine* line) const;

  PdfDocument* _doc;

  // The most frequent line pitch in the document.
  double _mostFreqLinePitch;

  // A mapping of a page number to the most freq. line pitch in the respective page.
  std::unordered_map<int, double> _mostFreqLinePitchPerPage;

  // A mapping of a font size to the most freq. line pitch among the text lines with the respective
  // font size.
  std::unordered_map<double, double> _mostFreqLinePitchPerFontSize;
};

#endif  // TEXTBLOCKDETECTOR_H_
