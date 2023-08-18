/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_BYTEPAIRENCODER_H_
#define UTILS_BYTEPAIRENCODER_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>  // pair
#include <vector>

using std::pair;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;
using std::wstring;

// =================================================================================================

namespace ppp::utils {

/**
 * This class encodes given texts by using byte pair encoding.
 */
class BytePairEncoder {
 public:
  /**
   * Creates and initalizes a new BytePairEncoder from the given vocabulary. The given vocabulary
   * is a mapping of the most frequent tokens (that is: sequences of one or more characters)
   * to unique ids. This vocabulary needs to be precomputed from the same dataset on which the
   * model used on prediction was trained. An example vocabulary is {"the": 1, "eff": 2, "ici": 3}.
   *
   * @param vocabulary
   *   A (previously computed) mapping of the most frequent tokens to unique ids.
   */
  explicit BytePairEncoder(unordered_map<wstring, int>* vocabulary);

  /**
   * Splits the given text into words, encodes the words using byte pair encoding and cuts or pads
   * the resulting list of tokens to the given target length. For example, when the text is
   * "Some text" and the encoding of the word "Some" is [12, 1, 7] and the encoding of "text" is
   * [8, 3], the actual list of tokens is [12, 1, 7, 8, 3]. If targetLength is set to 3, this
   * method computes the list [12, 1, 7]. If the targetLength is 7, this method computes the list
   * [12, 1, 7, 8, 3, 99, 99], where 99 denotes a special padding symbol.
   *
   * @param text
   *   The text to encode.
   * @param targetLength
   *   The target length of the result list of tokens.
   * @param wordDelimAlphabet
   *   The characters to consider to be word delimiters.
   * @param result
   *   The vector to which the integer ids of the encoding should be added.
   */
  void encode(const wstring& text, size_t targetLength, const string& wordDelimAlphabet,
      vector<int>* result);

 private:
  /**
   * Encodes the given word using byte pair encoding. For example, when the vocabulary is
   * { "eff": 1; "the": 2; "ent": 3; "ic": 4; "i": 5; } and the word to encode is "efficient",
   * this method computes the encoding [1, 4, 5, 3].
   *
   * @param word
   *   The word to encode.
   * @param result
   *   The vector to which the integer ids of the encoding should be added.
   */
  void encodeWord(const wstring& word, vector<int>* result);

  /**
   * Computes all pairs of two consecutive tokens in the given token list, together with the
   * respective positions. For example, when the token list is ["f", "o", "x", "i", "f", "o", "x"],
   * this method computes [("fo": {0, 4}), ("ox": {1, 5}), ("xi": {2}), ("if": {3})].
   *
   * @param tokens
   *   The vector of input tokens.
   * @param result
   *   The vector to which the result pairs should be added.
   */
  static void computeTokenPairPositions(const vector<wstring>& tokens,
    vector<pair<wstring, unordered_set<size_t>* >* >* result);

  // The vocabulary, mapping tokens to unique ids.
  unordered_map<wstring, int> _vocabulary;
  // The cache with encodings already computed (mapping a word to its actual encoding).
  unordered_map<wstring, vector<int> > _encodingsCache;
  // The symbol to use as padding.
  wstring PADDING_SYMBOL = L"⊛";
  // The symbol to use instead of a character unknown to the vocabulary.
  wstring UNKNOWN_CHAR_SYMBOL = L"⌾";
  // The symbol to use as word delimiter.
  wstring WORD_DELIM_SYMBOL = L"✂";

  // same as FRIEND_TEST(BytePairEncoderTest, testConstructor);
  friend class BytePairEncoderTest_testConstructor_Test;

  // same as FRIEND_TEST(BytePairEncoderTest, testEncodeWord);
  friend class BytePairEncoderTest_testEncodeWord_Test;

  // same as FRIEND_TEST(BytePairEncoderTest, computeTokenPairPositions);
  friend class BytePairEncoderTest_computeTokenPairPositions_Test;
};

}  // namespace ppp::utils

#endif  // UTILS_BYTEPAIRENCODER_H_
