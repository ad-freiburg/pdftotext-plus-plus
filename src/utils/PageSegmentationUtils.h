/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_PAGESEGMENTATIONUTILS_H_
#define UTILS_PAGESEGMENTATIONUTILS_H_

#include <vector>

#include "../Config.h"
#include "../PdfDocument.h"

using std::vector;

using ppp::config::PageSegmentationConfig;

// =================================================================================================

namespace ppp::utils {

/**
 * A collection of some useful and commonly used functions in context of page segmentation.
 */
class PageSegmentationUtils {
 public:
  /**
   * The default constructor.
   *
   * @param config
   *    The configuration to use.
   */
  explicit PageSegmentationUtils(const PageSegmentationConfig& config);

  /** The deconstructor. */
  ~PageSegmentationUtils();

  /**
   * This method creates a new segment consisting of the given page elements, computes the
   * respective properties of the segment and returns the segment.
   *
   * @param elements
   *   The page elements from which to create a segment.
   */
  PdfPageSegment* createPageSegment(const vector<PdfElement*>& elements);

 private:
  // The configuration to use.
  PageSegmentationConfig _config;
};

}  // namespace ppp::utils

#endif  // UTILS_PAGESEGMENTATIONUTILS_H_
