/**
 * Copyright 2023, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef UTILS_CHARMAP_H_
#define UTILS_CHARMAP_H_

#include <string>
#include <unordered_map>
#include <utility>  // std::make_pair

using std::make_pair;
using std::pair;
using std::string;
using std::unordered_map;

// =================================================================================================

namespace ppp::utils {

/**
 * A mapping of char names to the respective Unicode codepoints and string representations.
 * This is needed because for some chars, some PDFs only provide the char names but not the
 * respective Unicode character (for example, due to a missing or broken encoding). For example,
 * for the character "Σ", only the name "summationdisplay" may be provided, but not the actual
 * Unicode representation. This map is used to get the Unicode representation of such characters.
 */
const unordered_map<string, pair<unsigned int, string>> charMap = {
  { "summationdisplay", make_pair(0x2211, "\u2211") },
  { "summationssdisplay", make_pair(0x2211, "\u2211") },
  { "angbracketleft", make_pair(0x3008, "\u3008") },
  { "angbracketright", make_pair(0x3009, "\u3009") },
  { "circlecopyrt", make_pair(0x00A9, "\u00A9") },
  { "controlNULL", make_pair(0x0000, "\u0000") },
  { "angbracketleftbig", make_pair(0x2329, "\u2329") },
  { "angbracketleftBig", make_pair(0x2329, "\u2329") },
  { "angbracketleftbigg", make_pair(0x2329, "\u2329") },
  { "angbracketleftBigg", make_pair(0x2329, "\u2329") },
  { "angbracketrightBig", make_pair(0x232A, "\u232A") },
  { "angbracketrightbig", make_pair(0x232A, "\u232A") },
  { "angbracketrightBigg", make_pair(0x232A, "\u232A") },
  { "angbracketrightbigg", make_pair(0x232A, "\u232A") },
  { "arrowhookleft", make_pair(0x21AA, "\u21AA") },
  { "arrowhookright", make_pair(0x21A9, "\u21A9") },
  { "arrowlefttophalf", make_pair(0x21BC, "\u21BC") },
  { "arrowleftbothalf", make_pair(0x21BD, "\u21BD") },
  { "arrownortheast", make_pair(0x2197, "\u2197") },
  { "arrownorthwest", make_pair(0x2196, "\u2196") },
  { "arrowrighttophalf", make_pair(0x21C0, "\u21C0") },
  { "arrowrightbothalf", make_pair(0x21C1, "\u21C1") },
  { "arrowsoutheast", make_pair(0x2198, "\u2198") },
  { "arrowsouthwest", make_pair(0x2199, "\u2199") },
  { "backslashbig", make_pair(0x2216, "\u2216") },
  { "backslashBig", make_pair(0x2216, "\u2216") },
  { "backslashBigg", make_pair(0x2216, "\u2216") },
  { "backslashbigg", make_pair(0x2216, "\u2216") },
  { "bardbl", make_pair(0x2016, "\u2016") },
  { "bracehtipdownleft", make_pair(0xFE37, "\uFE37") },
  { "bracehtipdownright", make_pair(0xFE37, "\uFE37") },
  { "bracehtipupleft", make_pair(0xFE38, "\uFE38") },
  { "bracehtipupright", make_pair(0xFE38, "\uFE38") },
  { "braceleftBig", make_pair(0x007B, "\u007B") },
  { "braceleftbig", make_pair(0x007B, "\u007B") },
  { "braceleftbigg", make_pair(0x007B, "\u007B") },
  { "braceleftBigg", make_pair(0x007B, "\u007B") },
  { "bracerightBig", make_pair(0x007D, "\u007D") },
  { "bracerightbig", make_pair(0x007D, "\u007D") },
  { "bracerightbigg", make_pair(0x007D, "\u007D") },
  { "bracerightBigg", make_pair(0x007D, "\u007D") },
  { "bracketleftbig", make_pair(0x005B, "\u005B") },
  { "bracketleftBig", make_pair(0x005B, "\u005B") },
  { "bracketleftbigg", make_pair(0x005B, "\u005B") },
  { "bracketleftBigg", make_pair(0x005B, "\u005B") },
  { "bracketleftmath", make_pair(0x005B, "\u005B") },
  { "bracketrightBig", make_pair(0x005D, "\u005D") },
  { "bracketrightbig", make_pair(0x005D, "\u005D") },
  { "bracketrightbigg", make_pair(0x005D, "\u005D") },
  { "bracketrightBigg", make_pair(0x005D, "\u005D") },
  { "bracketrightmath", make_pair(0x005D, "\u005D") },
  { "ceilingleftbig", make_pair(0x2308, "\u2308") },
  { "ceilingleftBig", make_pair(0x2308, "\u2308") },
  { "ceilingleftBigg", make_pair(0x2308, "\u2308") },
  { "ceilingleftbigg", make_pair(0x2308, "\u2308") },
  { "ceilingrightbig", make_pair(0x2309, "\u2309") },
  { "ceilingrightBig", make_pair(0x2309, "\u2309") },
  { "ceilingrightbigg", make_pair(0x2309, "\u2309") },
  { "ceilingrightBigg", make_pair(0x2309, "\u2309") },
  { "circledotdisplay", make_pair(0x2299, "\u2299") },
  { "circledottext", make_pair(0x2299, "\u2299") },
  { "circlemultiplydisplay", make_pair(0x2297, "\u2297") },
  { "circlemultiplytext", make_pair(0x2297, "\u2297") },
  { "circleplusdisplay", make_pair(0x2295, "\u2295") },
  { "circleplustext", make_pair(0x2295, "\u2295") },
  { "contintegraldisplay", make_pair(0x222E, "\u222E") },
  { "contintegraltext", make_pair(0x222E, "\u222E") },
  { "coproductdisplay", make_pair(0x2210, "\u2210") },
  { "coproducttext", make_pair(0x2210, "\u2210") },
  { "epsilon1", make_pair(0x03B5, "\u03B5") },
  { "equalmath", make_pair(0x003D, "\u003D") },
  { "floorleftBig", make_pair(0x230A, "\u230A") },
  { "floorleftbig", make_pair(0x230A, "\u230A") },
  { "floorleftbigg", make_pair(0x230A, "\u230A") },
  { "floorleftBigg", make_pair(0x230A, "\u230A") },
  { "floorrightbig", make_pair(0x230B, "\u230B") },
  { "floorrightBig", make_pair(0x230B, "\u230B") },
  { "floorrightBigg", make_pair(0x230B, "\u230B") },
  { "floorrightbigg", make_pair(0x230B, "\u230B") },
  { "hatwide", make_pair(0x005e, "\u005e") },
  { "hatwider", make_pair(0x005e, "\u005e") },
  { "hatwidest", make_pair(0x005e, "\u005e") },
  { "intercal", make_pair(0x1D40, "\u1D40") },
  { "integraldisplay", make_pair(0x222B, "\u222B") },
  { "integraltext", make_pair(0x222B, "\u222B") },
  { "intersectiondisplay", make_pair(0x22C2, "\u22C2") },
  { "intersectiontext", make_pair(0x22C2, "\u22C2") },
  { "logicalanddisplay", make_pair(0x2227, "\u2227") },
  { "logicalandtext", make_pair(0x2227, "\u2227") },
  { "logicalordisplay", make_pair(0x2228, "\u2228") },
  { "logicalortext", make_pair(0x2228, "\u2228") },
  { "parenleftBig", make_pair(0x0028, "\u0028") },
  { "parenleftbig", make_pair(0x0028, "\u0028") },
  { "parenleftBigg", make_pair(0x0028, "\u0028") },
  { "parenleftbigg", make_pair(0x0028, "\u0028") },
  { "parenleftmath", make_pair(0x0028, "\u0028") },
  { "parenrightBig", make_pair(0x0029, "\u0029") },
  { "parenrightbig", make_pair(0x0029, "\u0029") },
  { "parenrightBigg", make_pair(0x0029, "\u0029") },
  { "parenrightbigg", make_pair(0x0029, "\u0029") },
  { "parenrightmath", make_pair(0x0029, "\u0029") },
  { "plusmath", make_pair(0x002B, "\u002B") },
  { "prime", make_pair(0x2032, "\u2032") },
  { "productdisplay", make_pair(0x220F, "\u220F") },
  { "producttext", make_pair(0x220F, "\u220F") },
  { "question_sign", make_pair(0x003F, "?") },
  { "radicalbig", make_pair(0x221A, "\u221A") },
  { "radicalBig", make_pair(0x221A, "\u221A") },
  { "radicalBigg", make_pair(0x221A, "\u221A") },
  { "radicalbigg", make_pair(0x221A, "\u221A") },
  { "radicalbt", make_pair(0x221A, "\u221A") },
  { "radicaltp", make_pair(0x221A, "\u221A") },
  { "radicalvertex", make_pair(0x221A, "\u221A") },
  { "slashbig", make_pair(0x002F, "\u002F") },
  { "slashBig", make_pair(0x002F, "\u002F") },
  { "slashBigg", make_pair(0x002F, "\u002F") },
  { "slashbigg", make_pair(0x002F, "\u002F") },
  { "summationdisplay", make_pair(0x2211, "\u2211") },
  { "summationtext", make_pair(0x2211, "\u2211") },
  { "thumbs_down", make_pair(0x1F44E, "\u1F44E") },
  { "thumbs_down_alt", make_pair(0x1F44E, "\u1F44E") },
  { "thumbs_up", make_pair(0x1F44D, "\u1F44D") },
  { "thumbs_up_alt", make_pair(0x1F44D, "\u1F44D") },
  { "tildewide", make_pair(0x02DC, "\u02DC") },
  { "tildewider", make_pair(0x02DC, "\u02DC") },
  { "tildewidest", make_pair(0x02DC, "\u02DC") },
  { "uniondisplay", make_pair(0x22C3, "\u22C3") },
  { "unionmultidisplay", make_pair(0x228E, "\u228E") },
  { "unionmultitext", make_pair(0x228E, "\u228E") },
  { "unionsqdisplay", make_pair(0x2294, "\u2294") },
  { "unionsqtext", make_pair(0x2294, "\u2294") },
  { "uniontext", make_pair(0x22C3, "\u22C3") },
  { "vextenddouble", make_pair(0x2225, "\u2225") },
  { "vextendsingle", make_pair(0x2223, "\u2223") }
};

#endif  // UTILS_CHARMAP_H_

}  // namespace ppp::utils
