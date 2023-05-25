/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef SERIALIZATION_SERIALIZER_H_
#define SERIALIZATION_SERIALIZER_H_

#include <string>
#include <unordered_set>

#include "../PdfDocument.h"
#include "../Types.h"

using ppp::types::SemanticRole;
using std::string;
using std::unordered_set;

/**
 * This class is the super class for all serializer classes (each of which is responsible for
 * writing the text extracted from a PDF to a specified file (or to stdout) in a specified format.
 */
class Serializer {
 public:
  /**
   * This method writes the text extracted from the given PDF document to the file given by
   * `targetPath`. If `targetPath` is specified as "-", the text is written to stdout instead.
   *
   * @param doc
   *   The PDF document to process.
   * @param roles
   *   If not empty, only the text of text blocks with the specified roles is written to the file.
   *   If empty, all text is written to the file.
   * @param targetPath
   *   The path to the file to which the text should be written. If specified as "-", the text is
   *   written to stdout.
   */
  virtual void serialize(PdfDocument* doc, const unordered_set<string>& roles,
      const string& targetPath) = 0;
};

#endif  // SERIALIZATION_SERIALIZER_H_
