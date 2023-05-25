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

// The available serialization formats.
enum struct SerializationFormat { TXT, JSONL };

// The names of the serialization formats.
const vector<string> SERIALIZATION_FORMAT_NAMES { "txt", "jsonl" };

// The available semantic roles.
enum struct SemanticRole { TITLE, HEADING, PARAGRAPH };

// The names of the semantic roles.
const vector<string> SEMANTIC_ROLE_NAMES { "title", "heading", "paragraph" };

// =================================================================================================

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
 * This function returns the name of the given semantic role.
 *
 * @param role
 *    The semantic role.
 *
 * @return
 *    The name of the semantic role.
 */
string getName(SemanticRole role);


}  // namespace ppp::types

#endif  // TYPES_H_
