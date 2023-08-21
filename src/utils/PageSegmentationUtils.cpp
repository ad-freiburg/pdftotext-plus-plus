/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <vector>

#include "./MathUtils.h"
#include "./PageSegmentationUtils.h"
#include "./TextUtils.h"
#include "../Config.h"
#include "../PdfDocument.h"

using std::vector;

using ppp::config::PageSegmentationConfig;
using ppp::types::PdfElement;
using ppp::types::PdfPageSegment;
using ppp::utils::math::maximum;
using ppp::utils::math::minimum;
using ppp::utils::math::round;
using ppp::utils::text::createRandomString;

// =================================================================================================

namespace ppp::utils {

// _________________________________________________________________________________________________
PageSegmentationUtils::PageSegmentationUtils(const PageSegmentationConfig& config) {
  _config = config;
}

// _________________________________________________________________________________________________
PageSegmentationUtils::~PageSegmentationUtils() = default;

// _________________________________________________________________________________________________
PdfPageSegment* PageSegmentationUtils::createPageSegment(const vector<PdfElement*>& elements) {
  PdfPageSegment* segment = new PdfPageSegment();

  // Create a (unique) id.
  segment->id = createRandomString(_config.idLength, "segment-");

  // Set the page number.
  segment->pos->pageNum = !elements.empty() ? elements[0]->pos->pageNum : -1;

  // Compute and set the coordinates of the bounding box.
  for (const auto* element : elements) {
    segment->pos->leftX = minimum(segment->pos->leftX, element->pos->leftX);
    segment->pos->upperY = minimum(segment->pos->upperY, element->pos->upperY);
    segment->pos->rightX = maximum(segment->pos->rightX, element->pos->rightX);
    segment->pos->lowerY = maximum(segment->pos->lowerY, element->pos->lowerY);
  }

  // Set the vector of page elements.
  segment->elements = elements;

  // Set the reference to the current PDF document.
  segment->doc = !elements.empty() ? elements[0]->doc : nullptr;

  return segment;
}

}  // namespace ppp::utils
