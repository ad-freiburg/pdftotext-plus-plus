/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>

#include "./Serializer.h"
#include "../Types.h"

using std::cerr;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::unordered_set;

using ppp::types::PdfDocument;
using ppp::types::PdfElementType;
using ppp::types::SemanticRole;

// =================================================================================================

namespace ppp::serialization {

// _________________________________________________________________________________________________
void Serializer::serialize(
    const PdfDocument* doc,
    const unordered_set<SemanticRole>& roles,
    const unordered_set<PdfElementType>& types,
    const string& targetFilePath) const {
  // If the target file path is specified as "-", output the text to stdout.
  if (targetFilePath.size() == 1 && targetFilePath[0] == '-') {
    serializeToStream(doc, roles, types, cout);
    return;
  }

  // Compute the path to the parent directory of the target file.
  string parentDirPath = ".";
  size_t posLastSlash = targetFilePath.find_last_of("/");
  if (posLastSlash != string::npos) {
    parentDirPath = targetFilePath.substr(0, posLastSlash);
  }

  // Try to create all intermediate directories if the parent directory does not exist.
  if (system(("mkdir -p " + parentDirPath).c_str())) {
    cerr << "Could not create directory '" << parentDirPath << "'." << endl;
    return;
  }

  ofstream outFile(targetFilePath);
  if (!outFile.is_open()) {
    cerr << "Could not open file '" << targetFilePath << "'." << endl;
    return;
  }

  serializeToStream(doc, roles, types, outFile);
  outFile.close();
}

}  // namespace ppp::serialization
