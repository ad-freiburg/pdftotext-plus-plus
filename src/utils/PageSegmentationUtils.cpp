/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <algorithm>  // std::max
#include <tuple>
#include <unordered_map>
#include <utility>  // std::pair
#include <vector>

#include "../Config.h"
#include "./MathUtils.h"
#include "./PageSegmentationUtils.h"
#include "./TextUtils.h"

using std::make_tuple;
using std::max;
using std::min;
using std::pair;
using std::tuple;
using std::unordered_map;

using ppp::config::PageSegmentationConfig;
using ppp::utils::text::createRandomString;
using ppp::utils::math::equalOrLarger;
using ppp::utils::math::round;

// =================================================================================================

namespace ppp::utils {

// _________________________________________________________________________________________________
PageSegmentationUtils::PageSegmentationUtils(const PageSegmentationConfig& config) {
  _config = config;
}

// _________________________________________________________________________________________________
PageSegmentationUtils::~PageSegmentationUtils() = default;

// _________________________________________________________________________________________________
PdfPageSegment* PageSegmentationUtils::createPageSegment(
    const vector<PdfElement*>& elements, const PdfDocument* doc) {
  PdfPageSegment* segment = new PdfPageSegment();
  segment->doc = doc;

  // Create a (unique) id.
  segment->id = createRandomString(_config.idLength, "segment-");

  // Set the page number.
  segment->pos->pageNum = !elements.empty() ? elements[0]->pos->pageNum : -1;

  // Compute and set the coordinates of the bounding box.
  for (const auto* element : elements) {
    segment->pos->leftX = min(segment->pos->leftX, element->pos->leftX);
    segment->pos->upperY = min(segment->pos->upperY, element->pos->upperY);
    segment->pos->rightX = max(segment->pos->rightX, element->pos->rightX);
    segment->pos->lowerY = max(segment->pos->lowerY, element->pos->lowerY);
  }

  // Set the elements.
  segment->elements = elements;

  return segment;
}

}  // namespace ppp::utils
