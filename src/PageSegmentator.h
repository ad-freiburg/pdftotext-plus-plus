/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PAGESEGMENTATOR_H_
#define PAGESEGMENTATOR_H_

#include <string>
#include <vector>

#include "./utils/Log.h"
#include "./PdfDocument.h"


/**
 * TODO This class detects text blocks of a given PDF document by using the recursive XY-cut algorithm.
 * A PDF document is processed page-wise. For each page, the words of the page are recursively
 * divided into groups with horizontal and/or vertical lines (or "cuts"). Groups that cannot be
 * further divided by a horizontal or vertical cuts are considered to represent a single text block.
 */
class PageSegmentator {
 public:
  /**
   * This constructor creates and initializes a new `PageSegmentator`.
   *
   * @param doc
   *   The document to process.
   */
  PageSegmentator(PdfDocument* doc, bool debug, int debugPageFilter);

  /** The deconstructor. **/
  ~PageSegmentator();

  /**
   * TODO This method detects text blocks of the given PDF document by using the recursive XY-cut
   * algorithm. The PDF document is processed page-wise. For each page the words, figures and
   * shapes of the page are recursively divided into groups with horizontal and/or vertical cuts
   * (with the requirement that a cut must not overlap any of the elements). Groups that cannot
   * be further divided by such a horizontal or vertical cut are considered to represent a single
   * text block. From the words of such a group, this method creates a new `PdfTextBlock`, computes
   * layout information about the text block and appends the text block to `page->blocks`, where
   * `page` is the `PdfPage` currently processed.
   */
  void segment();

 private:
  void segmentPage(PdfPage* page, std::vector<PdfPageSegment*>* segments);

  /**
   * This method tells the XY-cut algorithm whether or not the position between the elements
   * `closestElementLeft` and `elements[cutPos]` is a valid position for a vertical cut through the
   * given elements (which can be words, figures and shapes). It returns true, if the position is
   * a valid position; false otherwise.
   *
   * @param elements
   *   The elements to divide.
   * @param cutPos
   *   The index of the position in `elements` for which to decide whether or not it is a valid
   *   position to divide the elements by a x-cut.
   * @param closestElementLeft
   *   The element in elements[0..cutPos] with the largest rightX.
   *
   * @return True if the position between `closestElementLeft` and `elements[cutPos]` denotes a
   *   valid x-cut position, false otherwise.
   */
  void chooseXCuts(const std::vector<Cut*>& cuts, const std::vector<PdfElement*>& elements, bool silent);

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
  void choosePrimaryYCuts(const std::vector<Cut*>& cuts, const std::vector<PdfElement*>& elements,
      bool silent);

  /**
   * TODO This method tells the XY-cut algorithm whether or not the position between the elements
   * `closestElementAbove` and `elements[cutPos]` is a valid position for a horizontal cut through
   * the given elements (which can be words, figures and shapes). It returns true, if the position
   * is a valid position; false otherwise.
   *
   * @param elements
   *   The elements to divide.
   * @param cutPos
   *   The index of the position in `elements` for which to decide whether or not it is a valid
   *   position to divide the elements by a y-cut.
   * @param closestElementAbove
   *   The element in elements[0..cutPos] with the largest lowerY.
   *
   * @return True if the position between `closestElementAbove` and `elements[cutPos]` denotes a
   *   valid y-cut position, false otherwise.
   */
  void chooseYCuts(const std::vector<Cut*>& cuts, const std::vector<PdfElement*>& elements, bool silent);

  /**
   * TODO This method (1) creates a new `PdfTextBlock` from the words in the given list of
   * elements, (2) computes layout information about the text block and (3) appends the text block
   * to the given result list.
   *
   * @param elements
   *  The elements to create a text block from.
   * @param blocks
   *  The vector to which the created text block should be appended.
   */
  void createPageSegment(const std::vector<PdfElement*>& elements,
      std::vector<PdfPageSegment*>* segments) const;

  /** The document to process. */
  PdfDocument* _doc;

  double _minXCutGapWidth = 0;
  double _minYCutGapHeight = 0;

  /** The coordinates of the bounding box around all page elements (blocks, figures, shapes). */
  double _pageElementsMinX;
  double _pageElementsMinY;
  double _pageElementsMaxX;
  double _pageElementsMaxY;

  /** The logger. */
  Logger* _log;
};

#endif  // PAGESEGMENTATOR_H_
