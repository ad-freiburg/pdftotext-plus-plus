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
  bool startsNewTextBlock(const PdfPageSegment* segment, const PdfTextLine* prevLine, 
      const PdfTextLine* line);

  void createTextBlock(const std::vector<PdfTextLine*>& lines, std::vector<PdfTextBlock*>* blocks);

  bool computeIsTextBlockEmphasized(const std::vector<PdfTextLine*>& lines);

  void computeMostFrequentLinePitch();

  void computeMostFrequentLinePitchPerPage();

  void computeMostFrequentLinePitchPerFontFace();

  PdfDocument* _doc;

  // The most frequent line pitch in the document.
  double _mostFreqLinePitch;

  // A mapping of a page number to the most freq. line pitch in the respective page.
  std::unordered_map<int, double> _mostFreqLinePitchPerPage;

  // A mapping of a font face (= font name + font size) to the most freq. line pitch among the 
  // text lines with the respective font face.
  std::unordered_map<std::string, double> _mostFreqLinePitchPerFontFace;
};

#endif  // TEXTBLOCKDETECTOR_H_
