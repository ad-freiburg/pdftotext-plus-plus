/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef PDFELEMENTSUTILS_H_
#define PDFELEMENTSUTILS_H_

#include <utility>  // pair
#include <vector>

#include "../Constants.h"
#include "../PdfDocument.h"

using std::pair;
using std::vector;

// =================================================================================================

namespace element_utils {

/**
 * This method computes the horizontal gap between the two given elements.
 *
 * It first checks which of the two elements is the leftmost element, that is: which of the
 * elements has the minimum leftX. Let e1 be the leftmost element. The horizontal gap is then
 * computed as: e2.leftX - e1.rightX. Here is an example illustrating which value is actually
 * computed:
 *
 *     e1.rightX       e2.leftX
 *             v       v
 *
 *     | HELLO |       | WORLD |
 *
 *             └ ----- ┘
 *                 ^
 *           horizontal gap
 *
 * NOTE: The horizontal gap can be negative, when the two elements overlap horizontally.
 *
 * @param e1
 *    The first element to process.
 * @param e2
 *    The second element to process.
 *
 * @return
 *    The horizontal gap between e1 and e2.
 */
double computeHorizontalGap(const PdfElement* e1, const PdfElement* e2);

/**
 * This method computes the vertical gap between the two given elements.
 *
 * It first checks which of the two elements is the upper element, that is: which of the elements
 * has the minimum upperY. Let e1 be the upper element. The vertical gap is then computed as:
 * e2.upperY - e1.lowerY. Here is an example illustrating which value is actually computed:
 *
 *              -------------------
 *               A text line.
 * e1.lowerY -> -------------------   ┐
 *                                    |<- vertical gap
 * e2.upperY -> -------------------   ┘
 *               Another text line
 *              -------------------
 *
 * NOTE: The vertical gap can be negative, when the two elements overlap vertically.
 *
 * @param e1
 *    The first element to process.
 * @param e2
 *    The second element to process.
 *
 * @return
 *    The vertical gap between e1 and e2.
 */
double computeVerticalGap(const PdfElement* e1, const PdfElement* e2);

/**
 * This method computes the overlap ratios between the interval [s1, e1] and [s2, e2]. The returned
 * value is a pair of doubles. The first double represents the percentage of the first interval
 * that is overlapped by the second interval. The second double represents the percentage of the
 * second interval that is overlapped by the first interval. Here are three examples that help to
 * understand the meaning of the computed values:
 *
 * (1) Both intervals are   |   (2) The intervals do not   |   (3) Interval 1 is [1,7],
 *     equal:               |       overlap:               |       interval 2 is [3,18]:
 *     |------|             |       |-----|                |       |-----|
 *     |------|             |                |---|         |          |--------------|
 *
 * In case (1), the returned pair is (1, 1) since 100% of the first interval is overlapped by the
 * second interval and 100% of the second interval is overlapped by the first interval.
 *
 * In case (2), the returned pair is (0, 0) since no parts of the intervals are overlapped by the
 * respective other interval.
 *
 * In case (3), the returned pair is (0.5, 0.25) since 50% of the first interval is overlapped by
 * the second interval, and 25% of the second interval is overlapped by the first interval.
 *
 * @param s1
 *    The first endpoint of the first interval.
 * @param e1
 *    The second endpoint of the first interval.
 * @param s2
 *    The first endpoint of the second interval.
 * @param e2
 *    The second endpoint of the second interval.
 *
 * @return
 *    A pair of doubles representing the percentages of the intervals which are overlapped by
 *    the respective other interval.
 */
pair<double, double> computeOverlapRatios(double s1, double e1, double s2, double e2);

/**
 * This method computes the horizontal overlap ratios between the two given elements, that is:
 * the overlap ratios between the intervals [elem1.leftX, elem1.rightX] and
 * [elem2.leftX, elem2.rightX]. The general concept behind overlap ratios is described in the
 * comment of the computeOverlapRatios() method, so see this comment for more information and
 * some examples.
 *
 * @param elem1
 *    The first element to process.
 * @param elem2
 *    The second element to process.
 *
 * @return
 *    A pair of doubles representing the percentages of the elements which are overlapped
 *    horizontally by the respective other element.
 */
pair<double, double> computeXOverlapRatios(const PdfElement* elem1, const PdfElement* elem2);

/**
 * This method computes the vertical overlap ratios between the two given elements, that is:
 * the overlap ratios between the intervals [elem1.upperY, elem1.lowerY] and
 * [elem2.upperY, elem2.lowerY]. The general concept behind overlap ratios is described in the
 * comment of the computeOverlapRatios() method, so see this comment for more information and
 * some examples.
 *
 * @param elem1
 *    The first element to process.
 * @param elem2
 *    The second element to process.
 *
 * @return
 *    A pair of doubles representing the percentages of the elements which are overlapped
 *    vertically by the respective other element.
 */
pair<double, double> computeYOverlapRatios(const PdfElement* elem1, const PdfElement* elem2);

/**
 * This method computes the maximum horizontal overlap ratio between the two given elements, that
 * is: max(computeOverlapRatios(elem1.leftX, elem1.rightX, elem2.leftX, elem2.rightX)). The general
 * concept behind overlap ratios is described in the comment of the computeOverlapRatios() method,
 * so see this comment for more information and some examples.
 *
 * @param elem1
 *    The first element to process.
 * @param elem2
 *    The second element to process.
 *
 * @return
 *    The maximum horizontal overlap ratio between the two given elements.
 */
double computeMaxXOverlapRatio(const PdfElement* elem1, const PdfElement* elem2);

/**
 * This method computes the maximum vertical overlap ratio between the two given elements, that
 * is: max(computeOverlapRatios(elem1.upperY, elem1.lowerY, elem2.upperY, elem2.lowerY)). The
 * general concept behind overlap ratios is described in the comment of the computeOverlapRatios()
 * method, so see this comment for more information and some examples.
 *
 * @param elem1
 *    The first element to process.
 * @param elem2
 *    The second element to process.
 *
 * @return
 *    The maximum vertical overlap ratio between the two given elements.
 */
double computeMaxYOverlapRatio(const PdfElement* elem1, const PdfElement* elem2);

// =================================================================================================

/**
 * This method returns true, if the leftX values of the two given elements are (approximately)
 * equal.
 *
 * Whether or not the two values are considered equal depends on the given tolerance, which
 * represents the maximum allowed difference between the values.
 *
 * @param elem1
 *    The first element.
 * @param elem2
 *    The second element.
 * @param tolerance
 *    The (absolute) tolerance.
 *
 * @return
 *    True if the leftX values of the two elements are (approximately) equal, false otherwise.
 */
bool computeHasEqualLeftX(const PdfElement* elem1, const PdfElement* elem2,
    double tolerance = EQUAL_TOLERANCE);

/**
 * This method returns true, if the upperY values of the two given elements are (approximately)
 * equal.
 *
 * Whether or not the two values are considered equal depends on the given tolerance, which
 * represents the maximum allowed difference between the values.
 *
 * @param elem1
 *    The first element.
 * @param elem2
 *    The second element.
 * @param tolerance
 *    The (absolute) tolerance.
 *
 * @return
 *    True if the upperY values of the two elements are (approximately) equal, false otherwise.
 */
bool computeHasEqualUpperY(const PdfElement* elem1, const PdfElement* elem2,
    double tolerance = EQUAL_TOLERANCE);

/**
 * This method returns true, if the rightX values of the two given elements are (approximately)
 * equal.
 *
 * Whether or not the two values are considered equal depends on the given tolerance, which
 * represents the maximum allowed difference between the values.
 *
 * @param elem1
 *    The first element.
 * @param elem2
 *    The second element.
 * @param tolerance
 *    The (absolute) tolerance.
 *
 * @return
 *    True if the rightX values of the two elements are (approximately) equal, false otherwise.
 */
bool computeHasEqualRightX(const PdfElement* elem1, const PdfElement* elem2,
    double tolerance = EQUAL_TOLERANCE);

/**
 * This method returns true, if the lowerY values of the two given elements are (approximately)
 * equal.
 *
 * Whether or not the two values are considered equal depends on the given tolerance, which
 * represents the maximum allowed difference between the values.
 *
 * @param elem1
 *    The first element.
 * @param elem2
 *    The second element.
 * @param tolerance
 *    The (absolute) tolerance.
 *
 * @return
 *    True if the lowerY values of the two elements are (approximately) equal, false otherwise.
 */
bool computeHasEqualLowerY(const PdfElement* elem1, const PdfElement* elem2,
    double tolerance = EQUAL_TOLERANCE);

// =================================================================================================

/**
 * This method computes the offset of the leftX of the first given element compared to the leftX of
 * the second given element.
 *
 * Formally, this method computes elem1.leftX - elem2.leftX.
 *
 * @param elem1
 *   The first element to process.
 * @param elem2
 *   The second element to process.
 *
 * @return
 *    The offset of elem1.leftX compared to elem2.leftX.
 */
double computeLeftXOffset(const PdfElement* elem1, const PdfElement* elem2);

/**
 * This method computes the offset of the rightX of the first given element compared to the rightX
 * of the second given element.
 *
 * Formally, this method computes elem1.rightX - elem2.rightX.
 *
 * @param elem1
 *   The first element to process.
 * @param elem2
 *   The second element to process.
 *
 * @return
 *    The offset of elem1.rightX compared to elem2.rightX.
 */
double computeRightXOffset(const PdfElement* elem1, const PdfElement* elem2);

/**
 * This method iterates through the given figures and returns the first figure which horizontally
 * overlaps the given element by a ratio larger than minXOverlapRatio and that vertically overlaps
 * the given element by a ratio larger than minYOverlapRatio.
 *
 * This method is primarily used by the text block detector, for determining whether or not two
 * text lines are part of the same figure (because they are overlapped by the same figure) and for
 * deciding whether or not both text lines belond to different text blocks.
 *
 * @param element
 *    The element which should be checked whether or not it is overlapped by a figure.
 * @param figures
 *    The vector of figures.
 * @param minXOverlapRatio
 *    The minimum percentage of the element's width which must be overlapped by a figure in order
 *    to be returned by this method.
 * @param minYOverlapRatio
 *    The minimum percentage of the element's height which must be overlapped by a figure in order
 *    to be returned by this method.
 *
 * @return
 *    The first figure in the given vector which fulfills the given minimum overlap ratios, or
 *    nullptr if there is no such figure.
 */
PdfFigure* computeOverlapsFigure(const PdfElement* element, const vector<PdfFigure*>& figures,
    double minXOverlapRatio = 0.5, double minYOverlapRatio = 0.5);

}  // namespace element_utils

// =================================================================================================

namespace text_element_utils {

/**
 * This method returns true if the given elements exhibit the same font name.
 *
 * @param element1
 *    The first element to process.
 * @param element2
 *    The second element to process.
 *
 * @return
 *    True if the given elements exhibit the same font name, false otherwise.
 */
bool computeHasEqualFont(const PdfTextElement* element1, const PdfTextElement* element2);

/**
 * This method returns true if the given elements exhibit (approximately) the same font size.
 *
 * Whether or not the font sizes of the two elements are considered equal depends on the given
 * tolerance, which represents the maximum allowed difference between the font sizes.
 *
 * @param element1
 *    The first element to process.
 * @param element2
 *    The second element to process.
 * @param tolerance
 *    The (absolute) tolerance.
 *
 * @return
 *    True if the given elements exhibit (approximately) the same font size, false otherwise.
 */
bool computeHasEqualFontSize(const PdfTextElement* element1, const PdfTextElement* element2,
    double tolerance = FONTSIZE_EQUAL_TOLERANCE);

/**
 * This method returns true if the text of the given element ends with a sentence delimiter (that
 * is: a symbol contained in SENTENCE_DELIMITER_ALPHABET).
 *
 * @param element
 *    The element to process.
 *
 * @return
 *    True if the text of the given element ends with a sentence delimiter, false otherwise.
 */
bool computeEndsWithSentenceDelimiter(const PdfTextElement* element);

/**
 * This method returns true if the text of the given element starts with an uppercase.
 *
 * @param element
 *    The element to process.
 *
 * @return
 *    True if the text of the given element starts with an uppercase, false otherwise.
 */
bool computeStartsWithUpper(const PdfTextElement* element);

/**
 * This method returns true if the text of the given element is emphasized compared to the majority
 * of the rest of the text in the document.
 *
 * An element is considered to be emphasized when one of the following requirements is fulfilled:
 *  (1) The font size of the element is larger than the most frequent font size in the document;
 *  (2) The font weight of the element is larger than the most frequent font weight in the
 *      document, and the font size of the element is not smaller than the most frequent font size;
 *  (3) The text of the element is printed in italics, and the font size of the element is not
 *      smaller than the most frequent font size;
 *  (4) The text of the element contains at least one alphabetic character and all alphabetic
 *      characters are in uppercase.
 *
 * @param element
 *    The element to process.
 *
 * @return
 *    True if the text of the given element is emphasized, false otherwise.
 */
bool computeIsEmphasized(const PdfTextElement* element);

}  // namespace text_element_utils

#endif  // PDFELEMENTSUTILS_H_
