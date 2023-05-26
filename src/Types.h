/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace ppp::types {

// =================================================================================================
// Document units.

// The available document units.
enum struct DocumentUnit {
  CHARACTERS,
  WORDS,
  TEXT_BLOCKS,
  FIGURES,
  SHAPES,
  PAGES
};

// The names of the document units.
const vector<string> DOCUMENT_UNIT_NAMES {
  "characters",
  "words",
  "blocks",
  "figures",
  "shapes",
  "pages"
};

/**
 * This function returns the name of the given document unit.
 *
 * @param unit
 *    The document unit.
 *
 * @return
 *    The name of the document unit.
 */
string getName(DocumentUnit unit);

/**
 * This function returns a vector containing all document units.
 *
 * @return
 *    A vector containing all document units.
 */
std::vector<DocumentUnit> getDocumentUnits();

/**
 * This function returns a string containing the names of all document units. The
 * document units are separated from each other by the given separator.
 *
 * @param separator
 *    The separator.
 *
 * @return
 *    A string containing the names of all document units.
 */
string getDocumentUnitsStr(const string& separator = ", ");

// =================================================================================================
// Serialization formats.

// The available serialization formats.
enum struct SerializationFormat {
  TXT,
  TXT_EXTENDED,
  JSONL
};

// The names of the serialization formats.
const vector<string> SERIALIZATION_FORMAT_NAMES {
  "txt",
  "txt-extended",
  "jsonl"
};

/**
 * This function returns the name of the given serialization format.
 *
 * @param format
 *    The serialization format.
 *
 * @return
 *    The name of the serialization format.
 */
string getName(SerializationFormat format);

/**
 * This function returns a vector containing all serialization formats.
 *
 * @return
 *    A vector containing all serialization formats.
 */
std::vector<SerializationFormat> getSerializationFormats();

/**
 * This function returns a string containing the names of all serialization formats. The
 * serialization formats are separated from each other by the given separator.
 *
 * @param separator
 *    The separator.
 *
 * @return
 *    A string containing the names of all serialization formats.
 */
string getSerializationFormatsStr(const string& separator = ", ");

// =================================================================================================
// Semantic roles.

// The available semantic roles.
enum struct SemanticRole {
  PARAGRAPH,
  REFERENCE,
  MARGINAL,
  FOOTNOTE,
  HEADING,
  FORMULA,
  TITLE,
  AUTHOR_INFO,
  ABSTRACT,
  DATE,
  CAPTION,
  TABLE,
  OTHER,
  TOC
};

// The names of the semantic roles.
const vector<string> SEMANTIC_ROLE_NAMES {
  "paragraph",
  "reference",
  "marginal",
  "footnote",
  "heading",
  "formula",
  "title",
  "author-info",
  "abstract",
  "date",
  "caption",
  "table",
  "other",
  "toc",
};

/**
 * This function returns the name of the given semantic role.
 *
 * @param role
 *    The semantic role.
 *
 * @return
 *    The name of the semantic role.
 */
string getName(SemanticRole role);

/**
 * This function returns a vector containing all semantic roles.
 *
 * @return
 *    A vector containing all semantic roles.
 */
std::vector<SemanticRole> getSemanticRoles();

/**
 * This function returns a string containing the names of all semantic roles. The semantic roles
 * are separated from each other by the given separator.
 *
 * @param separator
 *    The separator.
 *
 * @return
 *    A string containing the names of all semantic roles.
 */
string getSemanticRolesStr(const string& separator = ", ");

// =================================================================================================
// Timing.

/**
 * A struct for storing a running time needed by a particular action or method to complete.
 */
struct Timing {
  /**
   * This constructor creates and initializes a new instance of this struct.
   *
   * @param nameA
   *    A (short) name describing the action/method.
   * @param timeA
   *    The running time of the action/method.
   */
  Timing(const string& nameA, int64_t timeA) {
    name = nameA;
    time = timeA;
  }

  // The name of the action/method.
  string name;
  // The running time of the action/method.
  int64_t time = 0;
};

}  // namespace ppp::types

#endif  // TYPES_H_
