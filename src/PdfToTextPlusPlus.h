/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PDFTOTEXTPLUSPLUS_H_
#define PDFTOTEXTPLUSPLUS_H_

#include <string>
#include <vector>

#include "./PdfDocument.h"

/**
 * This class is the main class of pdftotext++. It is responsible for processing a given PDF
 * file and invoking the different modules of the extraction pipeline to produce the extraction
 * result.
 */
class PdfToTextPlusPlus {
 public:
  /**
   * This constructor creates and initializes a new `PdfToTextPlusPlus` object.
   *
   * @param parseEmbeddedFontFiles
   *   A boolean flag indicating whether or not to parse the embedded font files of a PDF file in
   *   order to get more accurate font information, for example: the weight of a font or the exact
   *   bounding boxes of the glyphs. Setting this flag to true results in a faster extraction
   *   process but less accurate extraction results.
   */
  explicit PdfToTextPlusPlus(bool parseEmbeddedFontFiles, bool disableWordsDehyphenation,
    bool parseMode);

  /** The deconstructor */
  ~PdfToTextPlusPlus();

  /**
   * This method processes the given PDF file.
   *
   * @param pdfFilePath
   *   The path to the PDF file to process.
   * @param doc
   *   The `PdfDocument` to which the extracted information should be stored.
   *
   * @return 0 if the PDF was processed successfully, and an integer > 0 otherwise.
   */
  int process(const std::string& pdfFilePath, PdfDocument* doc,
      std::vector<Timing>* timings = nullptr);

 private:
  /** Wether or not to parse the embedded font files of a PDF file. */
  bool _parseEmbeddedFontFiles;

  bool _disableWordsDehyphenation;

  bool _parseMode;
};

#endif  // PDFTOTEXTPLUSPLUS_H_
