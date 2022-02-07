/**
 * Copyright 2021, University of Freiburg,
 * Chair of Algorithms and Data Structures.
 * Author: Claudius Korzen <korzen@cs.uni-freiburg.de>.
 *
 * Modified under the Poppler project - http://poppler.freedesktop.org
 */

#ifndef GLYPHMAP_H_
#define GLYPHMAP_H_

#include <unordered_map>
#include <string>

const std::unordered_map<std::string, std::string> glyphMap = {
  { "summationdisplay", "\u2211" },
  { "summationssdisplay", "\u2211" },
  { "angbracketleft", "\u3008" },
  { "angbracketright", "\u3009" },
  { "circlecopyrt", "\u00A9" },
  { "controlNULL", "\u0000" },
  { "angbracketleftbig", "\u2329" },
  { "angbracketleftBig", "\u2329" },
  { "angbracketleftbigg", "\u2329" },
  { "angbracketleftBigg", "\u2329" },
  { "angbracketrightBig", "\u232A" },
  { "angbracketrightbig", "\u232A" },
  { "angbracketrightBigg", "\u232A" },
  { "angbracketrightbigg", "\u232A" },
  { "arrowhookleft", "\u21AA" },
  { "arrowhookright", "\u21A9" },
  { "arrowlefttophalf", "\u21BC" },
  { "arrowleftbothalf", "\u21BD" },
  { "arrownortheast", "\u2197" },
  { "arrownorthwest", "\u2196" },
  { "arrowrighttophalf", "\u21C0" },
  { "arrowrightbothalf", "\u21C1" },
  { "arrowsoutheast", "\u2198" },
  { "arrowsouthwest", "\u2199" },
  { "backslashbig", "\u2216" },
  { "backslashBig", "\u2216" },
  { "backslashBigg", "\u2216" },
  { "backslashbigg", "\u2216" },
  { "bardbl", "\u2016" },
  { "bracehtipdownleft", "\uFE37" },
  { "bracehtipdownright", "\uFE37" },
  { "bracehtipupleft", "\uFE38" },
  { "bracehtipupright", "\uFE38" },
  { "braceleftBig", "\u007B" },
  { "braceleftbig", "\u007B" },
  { "braceleftbigg", "\u007B" },
  { "braceleftBigg", "\u007B" },
  { "bracerightBig", "\u007D" },
  { "bracerightbig", "\u007D" },
  { "bracerightbigg", "\u007D" },
  { "bracerightBigg", "\u007D" },
  { "bracketleftbig", "\u005B" },
  { "bracketleftBig", "\u005B" },
  { "bracketleftbigg", "\u005B" },
  { "bracketleftBigg", "\u005B" },
  { "bracketleftmath", "\u005B" },
  { "bracketrightBig", "\u005D" },
  { "bracketrightbig", "\u005D" },
  { "bracketrightbigg", "\u005D" },
  { "bracketrightBigg", "\u005D" },
  { "bracketrightmath", "\u005D" },
  { "ceilingleftbig", "\u2308" },
  { "ceilingleftBig", "\u2308" },
  { "ceilingleftBigg", "\u2308" },
  { "ceilingleftbigg", "\u2308" },
  { "ceilingrightbig", "\u2309" },
  { "ceilingrightBig", "\u2309" },
  { "ceilingrightbigg", "\u2309" },
  { "ceilingrightBigg", "\u2309" },
  { "circledotdisplay", "\u2299" },
  { "circledottext", "\u2299" },
  { "circlemultiplydisplay", "\u2297" },
  { "circlemultiplytext", "\u2297" },
  { "circleplusdisplay", "\u2295" },
  { "circleplustext", "\u2295" },
  { "contintegraldisplay", "\u222E" },
  { "contintegraltext", "\u222E" },
  { "coproductdisplay", "\u2210" },
  { "coproducttext", "\u2210" },
  { "epsilon1", "\u03B5" },
  { "equalmath", "\u003D" },
  { "floorleftBig", "\u230A" },
  { "floorleftbig", "\u230A" },
  { "floorleftbigg", "\u230A" },
  { "floorleftBigg", "\u230A" },
  { "floorrightbig", "\u230B" },
  { "floorrightBig", "\u230B" },
  { "floorrightBigg", "\u230B" },
  { "floorrightbigg", "\u230B" },
  { "hatwide", "\u0302" },
  { "hatwider", "\u0302" },
  { "hatwidest", "\u0302" },
  { "intercal", "\u1D40" },
  { "integraldisplay", "\u222B" },
  { "integraltext", "\u222B" },
  { "intersectiondisplay", "\u22C2" },
  { "intersectiontext", "\u22C2" },
  { "logicalanddisplay", "\u2227" },
  { "logicalandtext", "\u2227" },
  { "logicalordisplay", "\u2228" },
  { "logicalortext", "\u2228" },
  { "parenleftBig", "\u0028" },
  { "parenleftbig", "\u0028" },
  { "parenleftBigg", "\u0028" },
  { "parenleftbigg", "\u0028" },
  { "parenleftmath", "\u0028" },
  { "parenrightBig", "\u0029" },
  { "parenrightbig", "\u0029" },
  { "parenrightBigg", "\u0029" },
  { "parenrightbigg", "\u0029" },
  { "parenrightmath", "\u0029" },
  { "plusmath", "\u002B" },
  { "prime", "\u2032" },
  { "productdisplay", "\u220F" },
  { "producttext", "\u220F" },
  { "question_sign", "?" },
  { "radicalbig", "\u221A" },
  { "radicalBig", "\u221A" },
  { "radicalBigg", "\u221A" },
  { "radicalbigg", "\u221A" },
  { "radicalbt", "\u221A" },
  { "radicaltp", "\u221A" },
  { "radicalvertex", "\u221A" },
  { "slashbig", "\u002F" },
  { "slashBig", "\u002F" },
  { "slashBigg", "\u002F" },
  { "slashbigg", "\u002F" },
  { "summationdisplay", "\u2211" },
  { "summationtext", "\u2211" },
  { "thumbs_down", "\u1F44E" },
  { "thumbs_down_alt", "\u1F44E" },
  { "thumbs_up", "\u1F44D" },
  { "thumbs_up_alt", "\u1F44D" },
  { "tildewide", "\u02DC" },
  { "tildewider", "\u02DC" },
  { "tildewidest", "\u02DC" },
  { "uniondisplay", "\u22C3" },
  { "unionmultidisplay", "\u228E" },
  { "unionmultitext", "\u228E" },
  { "unionsqdisplay", "\u2294" },
  { "unionsqtext", "\u2294" },
  { "uniontext", "\u22C3" },
  { "vextenddouble", "\u2225" },
  { "vextendsingle", "\u2223" }
};

#endif  // GLYPHMAP_H_
