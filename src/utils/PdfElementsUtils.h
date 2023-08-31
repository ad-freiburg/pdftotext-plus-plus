/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_PDFELEMENTSUTILS_H_
#define UTILS_PDFELEMENTSUTILS_H_

#include <string>
#include <utility>  // std::pair
#include <vector>

#include "../PdfDocument.h"

using std::pair;
using std::string;
using std::vector;

using ppp::types::PdfElement;
using ppp::types::PdfElementType;
using ppp::types::PdfTextElement;
using ppp::types::SerializationFormat;

// =================================================================================================

/**
 * A collection of some useful and commonly used functions in context of processing PDF elements.
 */
namespace ppp::utils::elements {

/**
 * This function computes the horizontal gap between two page elements.
 *
 * It first checks which of the two elements is the leftmost element, that is: which of the
 * elements has the minimum leftX. Let e1 be the leftmost element. The horizontal gap is then
 * computed as: e2.leftX - e1.rightX. Here is an example of which value is actually computed:
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
 * NOTE: The horizontal gap can be negative, when the two elements horizontally overlap.
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
 * This function computes the vertical gap between two page elements.
 *
 * It first checks which of the two elements is the upper element, that is: which of the elements
 * has the minimum upperY. Let e1 be the upper element. The vertical gap is then computed as:
 * e2.upperY - e1.lowerY. Here is an example of which value is actually computed:
 *
 *              -------------------
 *               A text line.
 * e1.lowerY -> -------------------   ┐
 *                                    | <- vertical gap
 * e2.upperY -> -------------------   ┘
 *               Another text line
 *              -------------------
 *
 * NOTE: The vertical gap can be negative, when the two elements vertically overlap.
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
 * This function computes the overlap ratios between the intervals [s1, e1] and [s2, e2]. The
 * returned value is a pair of doubles. The first double represents the percentage of the first
 * interval that is overlapped by the second interval. The second double represents the percentage
 * of the second interval that is overlapped by the first interval. Here are three examples of
 * which values are actually computed:
 *
 * (1) Both intervals are   |   (2) The intervals do not   |   (3) Interval 1 is [1,7],
 *     equal:               |       overlap:               |       interval 2 is [4,16]:
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
 * This function computes the horizontal overlap ratios between the two given page elements, that is:
 * the overlap ratios between [elem1.leftX, elem1.rightX] and [elem2.leftX, elem2.rightX].
 * For more information about the concept behind overlap ratios, see the comment of the
 * computeOverlapRatios() function.
 *
 * @param elem1
 *    The first page element to process.
 * @param elem2
 *    The second page element to process.
 *
 * @return
 *    A pair of doubles representing the percentages of the elements which are overlapped
 *    horizontally by the respective other element.
 */
pair<double, double> computeXOverlapRatios(const PdfElement* elem1, const PdfElement* elem2);

/**
 * This function computes the vertical overlap ratios between the two given page elements, that is:
 * the overlap ratios between [elem1.upperY, elem1.lowerY] and [elem2.upperY, elem2.lowerY].
 * For more information about the concept behind overlap ratios, see the comment of the
 * computeOverlapRatios() function.
 *
 * @param elem1
 *    The first page element to process.
 * @param elem2
 *    The second page element to process.
 *
 * @return
 *    A pair of doubles representing the percentages of the elements which are overlapped
 *    vertically by the respective other element.
 */
pair<double, double> computeYOverlapRatios(const PdfElement* elem1, const PdfElement* elem2);

/**
 * This function computes the maximum of the horizontal overlap ratios between the two given page
 * elements, i.e: max(computeOverlapRatios(elem1.leftX, elem1.rightX, elem2.leftX, elem2.rightX)).
 * For more information about the concept behind overlap ratios, see the comment of the
 * computeOverlapRatios() function.
 *
 * @param elem1
 *    The first page element to process.
 * @param elem2
 *    The second page element to process.
 *
 * @return
 *    The maximum horizontal overlap ratio between the two given elements.
 */
double computeMaxXOverlapRatio(const PdfElement* elem1, const PdfElement* elem2);

/**
 * This function computes the maximum of the vertical overlap ratios between the two given page
 * elements, i.e: max(computeOverlapRatios(elem1.upperY, elem1.lowerY, elem2.upperY, elem2.lowerY)).
 * For more information about the concept behind overlap ratios, see the comment of the
 * computeOverlapRatios() function.
 *
 * @param elem1
 *    The first page element to process.
 * @param elem2
 *    The second page element to process.
 *
 * @return
 *    The maximum vertical overlap ratio between the two given elements.
 */
double computeMaxYOverlapRatio(const PdfElement* elem1, const PdfElement* elem2);

/**
 * This function checks if the leftX values of the two given page elements are (approximately) equal.
 *
 * Whether or not the two values are considered equal depends on the given tolerance, which
 * represents the maximum allowed difference between the values.
 *
 * Formally, this function returns true if: abs(elem1.leftX - elem2.leftX) <= tolerance.
 *
 * @param elem1
 *    The first page element to process.
 * @param elem2
 *    The second page element to process.
 * @param tolerance
 *    The tolerance.
 *
 * @return
 *    True if the leftX values of the two elements are (approximately) equal, false otherwise.
 */
bool computeHasEqualLeftX(const PdfElement* elem1, const PdfElement* elem2, double tolerance);

/**
 * This function checks if the upperY values of the two given page elements are (approximately) equal.
 *
 * Whether or not the two values are considered equal depends on the given tolerance, which
 * represents the maximum allowed difference between the values.
 *
 * Formally, this function returns true if: abs(elem1.upperY - elem2.upperY) <= tolerance.
 *
 * @param elem1
 *    The first page element to process.
 * @param elem2
 *    The second page element to process.
 * @param tolerance
 *    The tolerance.
 *
 * @return
 *    True if the upperY values of the two elements are (approximately) equal, false otherwise.
 */
bool computeHasEqualUpperY(const PdfElement* elem1, const PdfElement* elem2, double tolerance);

/**
 * This function checks if the rightX values of the two given page elements are (approximately) equal.
 *
 * Whether or not the two values are considered equal depends on the given tolerance, which
 * represents the maximum allowed difference between the values.
 *
 * Formally, this function returns true if: abs(elem1.rightX - elem2.rightX) <= tolerance.
 *
 * @param elem1
 *    The first page element to process.
 * @param elem2
 *    The second page element to process.
 * @param tolerance
 *    The tolerance.
 *
 * @return
 *    True if the rightX values of the two elements are (approximately) equal, false otherwise.
 */
bool computeHasEqualRightX(const PdfElement* elem1, const PdfElement* elem2, double tolerance);

/**
 * This function checks if the lowerY values of the two given page elements are (approximately) equal.
 *
 * Whether or not the two values are considered equal depends on the given tolerance, which
 * represents the maximum allowed difference between the values.
 *
 * Formally, this function returns true if: abs(elem1.lowerY - elem2.lowerY) <= tolerance.
 *
 * @param elem1
 *    The first page element to process.
 * @param elem2
 *    The second page element to process.
 * @param tolerance
 *    The tolerance.
 *
 * @return
 *    True if the lowerY values of the two elements are (approximately) equal, false otherwise.
 */
bool computeHasEqualLowerY(const PdfElement* elem1, const PdfElement* elem2, double tolerance);

// =================================================================================================

/**
 * This function computes the offset of the leftX value of the first page element compared to the
 * leftX value of the second page element.
 *
 * Formally, this function computes elem1.leftX - elem2.leftX.
 *
 * @param elem1
 *   The first page element to process.
 * @param elem2
 *   The second page element to process.
 *
 * @return
 *    The offset of elem1.leftX compared to elem2.leftX.
 */
double computeLeftXOffset(const PdfElement* elem1, const PdfElement* elem2);

/**
 * This function computes the offset of the rightX value of the first page element compared to the
 * rightX value of the second page element.
 *
 * Formally, this function computes elem1.rightX - elem2.rightX.
 *
 * @param elem1
 *   The first page element to process.
 * @param elem2
 *   The second page element to process.
 *
 * @return
 *    The offset of elem1.rightX compared to elem2.rightX.
 */
double computeRightXOffset(const PdfElement* elem1, const PdfElement* elem2);

/**
 * This function returns true if the given text elements have the same font.
 *
 * @param elem1
 *    The first text element to process.
 * @param elem2
 *    The second text element to process.
 *
 * @return
 *    True if the given text elements have the same font, false otherwise.
 */
bool computeHasEqualFont(const PdfTextElement* elem1, const PdfTextElement* elem2);

/**
 * This function returns true if the given text elements have (approximately) the same font size.
 *
 * Whether or not the font sizes of the two elements are considered equal depends on
 * the given tolerance, which represents the maximum allowed difference between the font sizes.
 *
 * Formally, this function returns true if abs(elem1.fontSize - elem2.fontSize) <= tolerance.
 *
 * @param elem1
 *    The first element to process.
 * @param elem2
 *    The second element to process.
 * @param tolerance
 *    The tolerance.
 *
 * @return
 *    True if the given elements have (approximately) the same font size, false otherwise.
 */
bool computeHasEqualFontSize(const PdfTextElement* elem1, const PdfTextElement* elem2,
    double tolerance);

// =================================================================================================
// TODO(korzen): The following functions do not quite fit into this util. Consider whether it would
// be better to move them to another util.

/**
 * This function returns a vector containing all PDF element types.
 *
 * @return
 *    A vector containing all PDF element types.
 */
vector<PdfElementType> getPdfElementTypes();

/**
 * This function returns a string containing the names of all PDF element types, separated from
 * each other by the given separator.
 *
 * @param separator
 *    The string to use to separate the names of the PDF element types.
 *
 * @return
 *    A string containing the names of all PDF element types.
 */
string getPdfElementTypesStr(const string& separator = ", ");

/**
 * This function returns the name of the given PDF element type.
 *
 * @param type
 *    The element type.
 *
 * @return
 *    The name of the PDF element type.
 */
string getPdfElementTypeName(PdfElementType type);

// -------------------------------------------------------------------------------------------------

/**
 * This function returns a vector containing all serialization formats.
 *
 * @return
 *    A vector containing all serialization formats.
 */
vector<SerializationFormat> getSerializationFormats();

/**
 * This function returns a string containing the names of all serialization formats, separated from
 * each other by the given separator.
 *
 * @param separator
 *    The string to use to separate the names of the serialization formats.
 *
 * @return
 *    A string containing the names of all serialization formats.
 */
string getSerializationFormatsStr(const string& separator = ", ");

/**
 * This function returns the name of the given serialization format.
 *
 * @param format
 *    The serialization format.
 *
 * @return
 *    The name of the serialization format.
 */
string getSerializationFormatName(SerializationFormat format);

// -------------------------------------------------------------------------------------------------

/**
 * This function returns a vector containing all semantic roles.
 *
 * @return
 *    A vector containing all semantic roles.
 */
vector<SemanticRole> getSemanticRoles();

/**
 * This function returns a string containing the names of all semantic roles, separated from each
 * other by the given separator.
 *
 * @param separator
 *    The string to use to separate the names of the semantic roles.
 *
 * @return
 *    A string containing the names of all semantic roles.
 */
string getSemanticRolesStr(const string& separator = ", ");

/**
 * This function returns the name of the given semantic role.
 *
 * @param role
 *    The semantic role.
 *
 * @return
 *    The name of the semantic role.
 */
string getSemanticRoleName(SemanticRole role);

}  // namespace ppp::utils::elements

#endif  // UTILS_PDFELEMENTSUTILS_H_
