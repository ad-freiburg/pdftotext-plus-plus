/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef READINGORDERDETECTOR_H_
#define READINGORDERDETECTOR_H_

#include <vector>

#include "./Config.h"
#include "./PdfDocument.h"
#include "./SemanticRolesPredictor.h"

using std::vector;

// ==============================================================================================

/**
 * This class detects the natural reading order of the text blocks extracted from a PDF document,
 * under the assumption that the reading order follows a left-to-right and top-to-bottom order.
 * The basic approach is as follows: A PDF document is processed page-wise. For each page, the
 * text blocks of the page are divided into groups by using the recursive XY-cut algorithm. The
 * reading order is deduced as follows: Whenever the text blocks are divided by a vertical line
 * (x-cut), all text blocks on the left side of the cut are placed before the text blocks on the
 * right side of the cut. Whenever the text blocks are divided by a horizontal line (y-cut), all
 * text blocks above the cut are placed before the text blocks below the cut.
 *
 * There are the following two preprocessing steps:
 * (1) A deep learning model is used to predict the semantic roles of the text blocks. The semantic
 *     roles will help to identify necessary "semantic y-cuts". We will describe this in more
 *     detail in the following item.
 * (2) Each page is scanned for positions where the text blocks need to be divided by a "semantic
 *     y-cut". A semantic y-cut is an y-cut which separates text blocks that do not belong to the
 *     body text but give the impression to do so, because they are arranged within the same
 *     columns as the body text. For an example, consider the first page of this paper:
 *     https://ad-publications.cs.uni-freiburg.de/benchmark.pdf.
 *     Note that the authors are aligned within the same columns as the body text, although they
 *     do not belong to the body text. So the text blocks need to divided by a semantic y-cut
 *     between the authors and the columns, so that the reading order is detected as "title,
 *     author1, author2, column1, column2" instead of "title, author1, column1, author2, column2".
 */
class ReadingOrderDetector {
 public:
  /**
   * This constructor creates and initializes a new `ReadingOrderDetector`.
   *
   * @param doc
   *   The document to process.
   * @param config
   *   The configuration to use.
   */
  explicit ReadingOrderDetector(const PdfDocument* doc, const ppp::Config* config);

  /** The deconstructor. */
  ~ReadingOrderDetector();

  /**
   * This method starts the process of detecting the natural reading order of the text blocks,
   * which includes: (1) the detection of the semantic role of the text blocks, (2) the detection
   * of "semantic y-cuts" and (3) the detection of the reading order of the text blocks.
   */
  void detect();

 private:
  /**
   * This method detects the semantic roles of the text blocks, needed to later detect the
   * "semantic y-cuts".
   */
  void detectSemanticRoles();

  /**
   * This method detects the semantic y-cuts and the reading order the text blocks of the given
   * PDF document.
   */
  void detectReadingOrder();

  // TODO(korzen): /**
  //  * This method tells the XY-cut algorithm whether or not the position between the elements
  //  * `closestElementAbove` and `elements[cutPos]` is a valid position to divide the elements by
  //  * a "semantic y-cut".
  //  *
  //  * @param elements
  //  *   The elements to divide.
  //  * @param cutPos
  //  *   The index of the position in `elements` for which to decide whether or not it is a valid
  //  *   position to divide the elements by a semantic y-cut.
  //  * @param closestElementAbove
  //  *   The element in elements[0..cutPos] with the largest lowerY.
  //  *
  //  * @return True if the position between `closestElementAbove` and `elements[cutPos]` denotes a
  //  *   valid semantic y-cut position, false otherwise.
  //  */
  void choosePrimaryXCuts(const vector<Cut*>& cuts, const vector<PdfElement*>& elements,
      bool silent);

  // TODO(korzen): /**
  //  * This method tells the XY-cut algorithm whether or not the position between the elements
  //  * `closestElementAbove` and `elements[cutPos]` is a valid position to divide the elements by
  //  * a "semantic y-cut".
  //  *
  //  * @param elements
  //  *   The elements to divide.
  //  * @param cutPos
  //  *   The index of the position in `elements` for which to decide whether or not it is a valid
  //  *   position to divide the elements by a semantic y-cut.
  //  * @param closestElementAbove
  //  *   The element in elements[0..cutPos] with the largest lowerY.
  //  *
  //  * @return True if the position between `closestElementAbove` and `elements[cutPos]` denotes a
  //  *   valid semantic y-cut position, false otherwise.
  //  */
  void choosePrimaryYCuts(const vector<Cut*>& cuts, const vector<PdfElement*>& elements,
      bool silent);

  /**
   * TODO(korzen): This method tells the XY-cut algorithm whether or not the position between the
   * elements
   * `closestElementLeft` and `elements[cutPos]` is a valid position to divide the elements by
   * an x-cut.
   *
   * @param elements
   *    The elements to divide.
   * @param cutPos
   *    The index of the position in `elements` for which to decide whether or not it is a valid
   *    position to divide the elements by an x-cut.
   * @param closestElementLeft
   *    The element in elements[0..cutPos] with the largest rightX.
   *
   * @return
   *    True if the position between `closestElementLeft` and `elements[cutPos]` denotes a valid
   *    x-cut position, false otherwise.
   */
  void chooseXCuts(const vector<Cut*>& cuts, const vector<PdfElement*>& elements, bool silent);

  /**
   * TODO(korzen): This method tells the XY-cut algorithm whether or not the position between the
   * elements
   * `closestElementAbove` and `elements[cutPos]` is a valid position to divide the elements by
   * an y-cut.
   *
   * @param elements
   *    The elements to divide.
   * @param cutPos
   *    The index of the position in `elements` for which to decide whether or not it is a valid
   *    position to divide the elements by an y-cut.
   * @param closestElementAbove
   *    The element in elements[0..cutPos] with the largest lowerY.
   *
   * @return
   *    True if the position between `closestElementAbove` and `elements[cutPos]` denotes a
   *    valid y-cut position, false otherwise.
   */
  void chooseYCuts(const vector<Cut*>& cuts, const vector<PdfElement*>& elements, bool silent);

  // The document to process.
  const PdfDocument* _doc;

  double _minXCutGapWidth = 0;
  double _minYCutGapHeight = 0;

  // The current page.
  PdfPage* _page;

  // The coordinates of the bounding box around all page elements (blocks, figures, shapes).
  double _pageElementsMinX;
  double _pageElementsMinY;
  double _pageElementsMaxX;
  double _pageElementsMaxY;

  // The configuration to use.
  const ppp::Config* _config;

  // The device for predicting the semantic roles of the text blocks.
  SemanticRolesPredictor* _semanticRolesPredictor;
};

#endif  // READINGORDERDETECTOR_H_
