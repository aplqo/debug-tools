#ifndef COLOR_H
#define COLOR_H

namespace apdebug::Output::Escape {
/*-----ecma-48 SGR(Select Graphic Rendition)-----*/
#ifdef COLOR
#define defSGR(name, code) const static inline char name[] = "\033[" #code "m"
#else
#define defSGR(name, code) const static inline char name[] = "";
#endif
defSGR(None, 0);
defSGR(TextRed, 31);
defSGR(TextGreen, 92);
defSGR(TextYellow, 33);
defSGR(TextBlue, 94);
defSGR(TextPurple, 95);
defSGR(TextCyan, 36);
defSGR(Bold, 1);
defSGR(Underline, 4);
defSGR(CrossOut, 9);
#undef defColor

#ifdef COLOR
const static inline char ClearLine[] = "\033[2K\033[G";
#else
const static inline char ClearLine[] = "\r";
#endif
}  // namespace apdebug::Output::Escape

#endif