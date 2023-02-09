// /**
//  * Copyright 2022, University of Freiburg,
//  * Chair of Algorithms and Data Structures.
//  * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
//  *
//  * Modified under the Poppler project - http://poppler.freedesktop.org
//  */

// #ifndef SEMANTICROLESPREDICTOR_H_
// #define SEMANTICROLESPREDICTOR_H_

// #include <codecvt>  // std::codecvt_utf8_utf16
// #include <locale>  // std::wstring_convert
// #include <string>
// #include <unordered_map>

// #include "tensorflow/cc/saved_model/loader.h"

// #include "./PdfDocument.h"

// using std::string;
// using std::unordered_map;

// // =================================================================================================

// /**
//  * This class predicts the semantic roles of the text blocks of a given PDF document by using
//  * deep learning techniques.
//  */
// class SemanticRolesPredictor {
//  public:
//   /** This constructor creates and initializes a new `SemanticRoles` object. */
//   SemanticRolesPredictor();

//   /** The deconstructor. */
//   ~SemanticRolesPredictor();

//   /**
//    * This method predicts the semantic roles of the text blocks of the given document. Writes the
//    * semantic role predicted for text block `block` to `block.role`.
//    *
//    * @param doc
//    *   The document for which to predict the semantic roles of the text blocks.
//    */
//   void predict(const PdfDocument* doc);

//  private:
//   /**
//    * This method reads a model and the associated vocabularies from a given file path. The file
//    * path is expected to be a path to a directory containing the following files:
//    *  - 'saved_model.pb'; a file representing a trained model in Tensorflow's protobuf format,
//    *  - 'vocab_bpe.tsv'; a TSV file providing the byte pair encoding to be used on encoding words.
//    *    The format is as follows: one byte pair per line, each written as <byte-pair>TAB<int>.
//    *  - 'vocab_roles.tsv'; a TSV file providing the encoding of the semantic roles. The format is
//    *    as follows: one semantic role per line, each written as <role>TAB<int>.
//    */
//   void readModel();

//   /**
//    * Creates a tensor for the "layout" input from text blocks of the given document.
//    *
//    * @param doc
//    *    The document to process.
//    *
//    * @return
//    *    The created tensor.
//    */
//   tensorflow::Tensor createLayoutInputTensor(const PdfDocument* doc);

//   /**
//    * Creates a tensor for the "words" input from text blocks of the given document.
//    *
//    * @param doc
//    *    The document to process.
//    *
//    * @return
//    *    The created tensor.
//    */
//   tensorflow::Tensor createWordsInputTensor(const PdfDocument* doc);

//   // The model loaded from file.
//   tensorflow::SavedModelBundle _bundle;

//   // The mapping of byte pairs to integer ids, for example: {"para": 0; "eff": 1, "icient": 2}.
//   unordered_map<std::wstring, int> _bpeVocab;
//   // The mapping of integer ids to semantic roles, for example: {0: "paragraph", 1: "title"}.
//   unordered_map<int, string> _rolesVocab;

//   // The converter for converting string to std::wstring.
//   std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> stringConverter;

//   // The path to this file.
//   string _filePath = __FILE__;
//   // The path to the parent directory of this source file.
//   string _parentDirPath = _filePath.substr(0, _filePath.rfind("/"));
//   // The path to the dir containing the (serialized) model to use, relative to the parent dir.
//   string _modelDirPath = _parentDirPath + "/models/2021-08-30_model-3K-documents";

//   // The name of the BPE vocabulary file within the model dir.
//   string _bpeVocabFilePath = _modelDirPath + "/bpe-vocab.tsv";
//   // The name of the roles vocabulary file within the model dir.
//   string _rolesVocabFilePath = _modelDirPath + "/roles-vocab.tsv";

//   // Whether or not the model was already loaded.
//   bool _modelOk = false;
// };

// #endif  // SEMANTICROLESPREDICTOR_H_
