#ifndef DEBUG_H_
#define DEBUG_H_

namespace ww {
void Print(const char *format, ...);
void Warning(const char *format, ...);
void Error(const char *format, ...);
void Abort(const char *format, ...);
} // namespace ww

#endif // DEBUG_H_
