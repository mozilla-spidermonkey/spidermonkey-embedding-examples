#include <cassert>
#include <codecvt>
#include <iostream>
#include <sstream>
#include <string>

#include <js/Conversions.h>
#include <js/Initialization.h>
#include <jsapi.h>
#include <mozilla/Unused.h>
#include <readline/history.h>
#include <readline/readline.h>

/* This is a longer example that illustrates how to build a simple
 * REPL (Read-Eval-Print Loop). */

/* NOTE: This example assumes that it's okay to print UTF-8 encoded text to
 * stdout and stderr. On Linux and macOS this will usually be the case. On
 * Windows you may have to set your terminal's codepage to UTF-8. */

// FIXME: need to set an enqueue promise job callback, or Promise.then crashes
// FIXME: add a "quit()" function

// clang-format off
static JSClassOps globalOps = {
  nullptr, // addProperty
  nullptr, // deleteProperty
  nullptr, // enumerate
  nullptr, // newEnumerate
  nullptr, // resolve
  nullptr, // mayResolve
  nullptr, // finalize
  nullptr, // call
  nullptr, // hasInstance
  nullptr, // construct
  JS_GlobalObjectTraceHook
};

/* The class of the global object. */
static JSClass globalClass = {
  "ReplGlobal",
  JSCLASS_GLOBAL_FLAGS,
  &globalOps
};
// clang-format on

static void
die(const char* why)
{
  std::cerr << "fatal error:" << why << std::endl;
  exit(1);
}

// The PrintError functions are modified versions of private SpiderMonkey API:
// js/src/vm/JSContext.cpp, js::PrintError()

enum class PrintErrorKind
{
  Error,
  Warning,
  StrictWarning,
  Note
};

static void
PrintErrorLine(const std::string& prefix, JSErrorReport* report)
{
  const char16_t* linebuf = report->linebuf();
  if (!linebuf)
    return;

  size_t n = report->linebufLength();

  std::cerr << ":\n";
  if (!prefix.empty())
    std::cerr << prefix;

  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter{};
  std::string linebuf_utf8 = converter.to_bytes(linebuf);
  std::cerr << linebuf_utf8;

  // linebuf usually ends with a newline. If not, add one here.
  if (n == 0 || linebuf[n - 1] != '\n')
    std::cerr << '\n';

  if (!prefix.empty())
    std::cerr << prefix;

  n = report->tokenOffset();
  size_t ndots = 0;
  for (size_t i = 0; i < n; i++) {
    if (linebuf[i] == '\t') {
      ndots += (ndots + 8) & ~7;
      continue;
    }
    ndots++;
  }
  std::cerr << std::string(ndots, '.') << '^';
}

static void
PrintErrorLine(const std::string& prefix, JSErrorNotes::Note* note)
{}

template<typename T>
static bool
PrintSingleError(T* report, PrintErrorKind kind)
{
  std::ostringstream prefix;
  if (report->filename)
    prefix << report->filename << ':';

  if (report->lineno)
    prefix << report->lineno << ':' << report->column << ' ';

  if (kind != PrintErrorKind::Error) {
    const char* kindPrefix = nullptr;
    switch (kind) {
      case PrintErrorKind::Error:
        MOZ_CRASH("unreachable");
      case PrintErrorKind::Warning:
        kindPrefix = "warning";
        break;
      case PrintErrorKind::StrictWarning:
        kindPrefix = "strict warning";
        break;
      case PrintErrorKind::Note:
        kindPrefix = "note";
        break;
    }

    prefix << kindPrefix << ": ";
  }

  const char* message = report->message().c_str();

  /* embedded newlines -- argh! */
  const char* ctmp;
  while ((ctmp = strchr(message, '\n')) != 0) {
    ctmp++;
    if (prefix)
      std::cerr << prefix.str();
    std::cerr.write(message, ctmp - message);
    message = ctmp;
  }

  /* If there were no filename or lineno, the prefix might be empty */
  if (!prefix.str().empty())
    std::cerr << prefix.str();
  std::cerr << message;

  PrintErrorLine(prefix.str(), report);
  std::cerr << std::endl; // flushes
  return true;
}

static void
PrintError(JSErrorReport* report)
{
  assert(report);

  PrintErrorKind kind = PrintErrorKind::Error;
  if (JSREPORT_IS_WARNING(report->flags)) {
    if (JSREPORT_IS_STRICT(report->flags))
      kind = PrintErrorKind::StrictWarning;
    else
      kind = PrintErrorKind::Warning;
  }
  PrintSingleError(report, kind);

  if (report->notes) {
    for (auto&& note : *report->notes)
      PrintSingleError(note.get(), PrintErrorKind::Note);
  }
}

std::string
FormatString(JSContext* cx, JS::HandleString string)
{
  std::string buf = "\"";

  JS::UniqueChars chars(JS_EncodeStringToUTF8(cx, string));
  if (!chars) {
    JS_ClearPendingException(cx);
    return "[invalid string]";
  }

  buf += chars.get();
  buf += '"';
  return buf;
}

std::string
FormatResult(JSContext* cx, JS::HandleValue value)
{
  JS::RootedString str(cx);

  /* Special case format for strings */
  if (value.isString()) {
    str = value.toString();
    return FormatString(cx, str);
  }

  str = JS::ToString(cx, value);

  if (!str) {
    JS_ClearPendingException(cx);
    str = JS_ValueToSource(cx, value);
  }

  if (!str) {
    JS_ClearPendingException(cx);
    if (value.isObject()) {
      const JSClass* klass = JS_GetClass(&value.toObject());
      if (klass)
        str = JS_NewStringCopyZ(cx, klass->name);
      else
        return "[unknown object]";
    } else {
      return "[unknown non-object]";
    }
  }

  if (!str) {
    JS_ClearPendingException(cx);
    return "[invalid class]";
  }

  JS::UniqueChars bytes(JS_EncodeStringToUTF8(cx, str));
  if (!bytes) {
    JS_ClearPendingException(cx);
    return "[invalid string]";
  }

  return bytes.get();
}

static JSErrorReport*
ErrorFromExceptionValue(JSContext* cx, JS::HandleValue exception)
{
  if (!exception.isObject())
    return nullptr;
  JS::RootedObject exceptionObject(cx, &exception.toObject());
  return JS_ErrorFromException(cx, exceptionObject);
}

static void
ReportAndClearException(JSContext* cx)
{
  /* Get exception object before printing and clearing exception. */
  JS::RootedValue exception(cx);
  if (!JS_GetPendingException(cx, &exception))
    die("Uncatchable exception thrown, out of memory or something");

  JS_ClearPendingException(cx);

  JSErrorReport* report = ErrorFromExceptionValue(cx, exception);
  if (!report) {
    JS_ClearPendingException(cx);
    std::cerr << "error: " << FormatResult(cx, exception) << '\n';
    return;
  }

  assert(!JSREPORT_IS_WARNING(report->flags));
  PrintError(report);
}

static JSContext*
CreateContext(void)
{
  JSContext* cx = JS_NewContext(8L * 1024 * 1024);
  if (!cx)
    return nullptr;
  if (!JS::InitSelfHostedCode(cx))
    return nullptr;
  return cx;
}

static JSObject*
CreateGlobal(JSContext* cx)
{
  JS::CompartmentOptions options;
  JS::RootedObject global(
    cx,
    JS_NewGlobalObject(
      cx, &globalClass, nullptr, JS::FireOnNewGlobalHook, options));

  // Add standard JavaScript classes to the global so we have a useful
  // environment.
  JSAutoCompartment ac(cx, global);
  if (!JS_InitStandardClasses(cx, global))
    return nullptr;

  return global;
}

bool
EvalAndPrint(JSContext* cx, const std::string& buffer, unsigned lineno)
{
  JS::CompileOptions options(cx);
  options.setUTF8(true).setFileAndLine("typein", lineno);

  JS::RootedValue result(cx);
  if (!JS::Evaluate(cx, options, buffer.c_str(), buffer.size(), &result))
    return false;

  JS_MaybeGC(cx);

  if (result.isUndefined())
    return true;

  std::string display_str = FormatResult(cx, result);
  if (!display_str.empty())
    std::cout << display_str << '\n';
  return true;
}

void
Loop(JSContext* cx, JS::HandleObject global)
{
  bool eof = false;
  unsigned lineno = 1;
  do {
    // Accumulate lines until we get a 'compilable unit' - one that either
    // generates an error (before running out of source) or that compiles
    // cleanly.  This should be whenever we get a complete statement that
    // coincides with the end of a line.
    unsigned startline = lineno;
    std::string buffer;

    do {
      const char* prompt = startline == lineno ? "js> " : "... ";
      char* line = readline(prompt);
      if (!line) {
        eof = true;
        break;
      }
      if (line[0] != '\0')
        add_history(line);
      buffer += line;
      lineno++;
    } while (
      !JS_BufferIsCompilableUnit(cx, global, buffer.c_str(), buffer.length()));

    if (!EvalAndPrint(cx, buffer, startline))
      ReportAndClearException(cx);
  } while (!eof);
}

static bool
Run(JSContext* cx)
{
  JSAutoRequest ar(cx);

  JS::RootedObject global(cx, CreateGlobal(cx));
  if (!global)
    return false;

  JSAutoCompartment ac(cx, global);

  JS::SetWarningReporter(
    cx, [](JSContext*, JSErrorReport* report) { PrintError(report); });

  Loop(cx, global);

  std::cout << '\n';
  return true;
}

int
main(int argc, const char* argv[])
{
  if (!JS_Init())
    die("Could not initialize JavaScript engine.");

  JSContext* cx = CreateContext();
  if (!cx)
    die("Could not set up interpreter context.");

  if (!Run(cx))
    return 1;

  JS_DestroyContext(cx);
  JS_ShutDown();
  return 0;
}
