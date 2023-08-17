/**
 * Copyright 2022, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>  // std::make_pair
#include <vector>

#include "./BytePairEncoder.h"
#include "./utils/TextUtils.h"

using std::pair;
using std::unordered_map;
using std::unordered_set;
using std::vector;
using std::wstring;

using ppp::utils::text::splitIntoWords;

// _________________________________________________________________________________________________
BytePairEncoder::BytePairEncoder(unordered_map<wstring, int>* vocabulary) {
  _vocabulary = *vocabulary;

  // Add some meta symbols (e.g., the padding symbol or the word delimiter symbol).
  _vocabulary[PADDING_SYMBOL] = _vocabulary.size();
  _vocabulary[UNKNOWN_CHAR_SYMBOL] = _vocabulary.size();
  _vocabulary[WORD_DELIM_SYMBOL] = _vocabulary.size();
}

// _________________________________________________________________________________________________
// TODO(korzen): wordDelimAlphabet is not used anymore.
void BytePairEncoder::encode(const wstring& text, size_t targetLength,
    const string& wordDelimAlphabet, vector<int>* res) {
  // Split the text into words. For example, split "This is some text" into
  // ["This", "is", "some", "text"].
  vector<wstring> words;
  splitIntoWords(text, &words);

  // Iterate through the words and encode each word using byte pair encoding.
  for (auto& word : words) {
    // Ignore all empty words.
    if (word.length() == 0) {
      continue;
    }

    // Append the word delimiter symbol to the end of the word and encode the word. Append the
    // resulting tokens to the result list. For example, when the word is "This", compute the
    // encoding of "This✂". If the tokens of this word are [2, 9, 10], append them to the result.
    word += WORD_DELIM_SYMBOL;
    vector<int> wordTokens;
    encodeWord(word, &wordTokens);
    for (auto& token : wordTokens) {
      res->push_back(token);
    }
  }

  // Bring the result list to the given target length. Append `targetLength - result.size()`-many
  // padding symbols to the end of the result list if its length is smaller than the target
  // length. Remove `result.size() - targetLength`-many elements from the end of the result list
  // if its length is larger than the target length.
  // For example, when the target length is 5 and the result list is [3, 7, 6], compute
  // [3, 7, 6, 99, 99] (where 99 is the id of the padding symbol).
  // When the target length is 3 and the result list is [3, 7, 6, 3, 2], compute [3, 7, 6].
  if (targetLength > 0) {
    // Append `targetLength - result.size()`-many padding symbols to the end of the result list.
    while (res->size() < targetLength) {
      res->push_back(_vocabulary[PADDING_SYMBOL]);
    }
    // Remove `result.size() - targetLength`-many elements from the end of the result list.
    while (res->size() > targetLength) {
      res->pop_back();
    }
  }
}

// _________________________________________________________________________________________________
void BytePairEncoder::encodeWord(const wstring& word, vector<int>* result) {
  // Do nothing if the word is empty.
  if (word.length() == 0) {
    return;
  }

  // Return the cached encoding, if available.
  if (_encodingsCache.find(word) != _encodingsCache.end()) {
    for (auto& entry : _encodingsCache[word]) {
      result->push_back(entry);
    }
    return;
  }

  // Convert the word into a list of characters and compute all possible token pairs, with their
  // respective start positions. For example, when the word is "foxifox", first convert it into
  // ["f", "o", "x", "i", "f", "o", "x"] and compute
  // [("fo": {0, 4}), ("ox": {1, 5}), ("xi": {2}), ("if": {3})}.
  vector<wstring> wordTokens;
  for (size_t k = 0; k < word.length(); k++) {
    wordTokens.push_back(word.substr(k, 1));
  }
  vector<pair<wstring, unordered_set<size_t>* >* > tokenPairPositions;
  computeTokenPairPositions(wordTokens, &tokenPairPositions);

  // If tokenPairPositions is empty, the word consists of only one character.
  if (tokenPairPositions.size() == 0) {
    result->push_back(_vocabulary[word]);
    return;
  }

  while (true) {
    // From tokenPairPositions, compute the positions of the first token pair that is also included
    // in the vocabulary. For example, when tokenPairPositions is {"fo": {0, 4}, "ox": {1, 5},
    // "xi": {2}, "if": {3}} and "ox" is the first token pair that is included in the vocabulary,
    // compute {1, 5}.
    unordered_set<size_t>* firstMatchingPairPositions = nullptr;
    for (auto& entry : tokenPairPositions) {
      // Check if the token pair is included in the vocabulary.
      if (_vocabulary.find(entry->first) != _vocabulary.end()) {
        firstMatchingPairPositions = entry->second;
        break;
      }
    }

    // Abort if none of the token pairs is included in the vocabulary.
    if (!firstMatchingPairPositions) {
      break;
    }

    // Merge all occurrences of the first matching token pair. For example, when
    // tokenPairPositions is {"fo": {0, 4}, "ox": {1, 5}, "xi": {2}, "if": {3}} and "ox" is the
    // first matching token pair, compute ["f", "ox", "i", "f", "ox"].
    size_t i = 0;
    vector<wstring> newWordTokens;
    while (i < wordTokens.size()) {
      firstMatchingPairPositions->count(i);
      if (firstMatchingPairPositions->count(i)) {
        newWordTokens.push_back(wordTokens[i] + wordTokens[i + 1]);
        i += 2;
      } else {
        newWordTokens.push_back(wordTokens[i]);
        i += 1;
      }
    }

    wordTokens = newWordTokens;
    if (wordTokens.size() == 1) {
      break;
    }

    // Compute all possible token pairs again and start a new round.
    for (auto* pair : tokenPairPositions) {
      delete pair->second;
      delete pair;
    }
    tokenPairPositions.clear();
    computeTokenPairPositions(wordTokens, &tokenPairPositions);
  }

  for (auto* pair : tokenPairPositions) {
    delete pair->second;
    delete pair;
  }

  // Translate the tokens to their ids in the vocabulary.
  for (auto& token : wordTokens) {
    if (_vocabulary.find(token) != _vocabulary.end()) {
      result->push_back(_vocabulary[token]);
      _encodingsCache[word].push_back(_vocabulary[token]);
    } else {
      result->push_back(_vocabulary[UNKNOWN_CHAR_SYMBOL]);
      _encodingsCache[word].push_back(_vocabulary[UNKNOWN_CHAR_SYMBOL]);
    }
  }
}

// _________________________________________________________________________________________________
void BytePairEncoder::computeTokenPairPositions(const vector<wstring>& tokens,
    vector<pair<wstring, unordered_set<size_t>* >* >* result) {
  if (tokens.size() == 0) {
    return;
  }

  // A mapping of byte pairs to their positions in the given vector of tokens.
  unordered_map<wstring, pair<wstring, unordered_set<size_t>* >* > cache;

  for (size_t i = 1; i < tokens.size(); i++) {
    // Merge the current token with the previous token.
    wstring merged = tokens[i - 1] + tokens[i];

    // If the cache already contains an entry for the merged token, update its position list.
    if (cache.find(merged) != cache.end()) {
      cache[merged]->second->insert(i - 1);
      continue;
    }

    // Otherwise, create a new entry of form (<merged token pair>, [<position>]).
    auto positions = new unordered_set<size_t>();
    positions->insert(i - 1);

    auto pair = new std::pair<wstring, unordered_set<size_t>* >(merged, positions);

    result->push_back(pair);
    cache[merged] = pair;
  }
}
