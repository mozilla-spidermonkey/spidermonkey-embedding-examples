#include <cassert>
#include <codecvt>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>

#include <jsapi.h>
#include <jsfriendapi.h>

#include <mozilla/Unused.h>

#include <js/CompilationAndEvaluation.h>
#include <js/Conversions.h>
#include <js/ErrorReport.h>
#include <js/Exception.h>
#include <js/Initialization.h>
#include <js/SourceText.h>
#include <js/Warnings.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "boilerplate.h"

/* This is a longer example that illustrates how to build a simple
 * REPL (Read-Eval-Print Loop). */

/* NOTE: This example assumes that it's okay to print UTF-8 encoded text to
 * stdout and stderr. On Linux and macOS this will usually be the case. On
 * Windows you may have to set your terminal's codepage to UTF-8. */

class ReplGlobal {
  bool m_shouldQuit : 1;

  ReplGlobal(void) : m_shouldQuit(false) {}

  static ReplGlobal* priv(JSObject* global) {
    auto* retval = static_cast<ReplGlobal*>(JS_GetPrivate(global));
    assert(retval);
    return retval;
  }

  static bool quit(JSContext* cx, unsigned argc, JS::Value* vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject global(cx, JS::GetNonCCWObjectGlobal(&args.callee()));
    if (!global) return false;

    // Return an "uncatchable" exception, by returning false without setting an
    // exception to be pending. We distinguish it from any other uncatchable
    // that the JS engine might throw, by setting m_shouldQuit
    priv(global)->m_shouldQuit = true;
    js::StopDrainingJobQueue(cx);
    return false;
  }

  /* The class of the global object. */
  static constexpr JSClass klass = {"ReplGlobal",
                                    JSCLASS_GLOBAL_FLAGS | JSCLASS_HAS_PRIVATE,
                                    &JS::DefaultGlobalClassOps};

  static constexpr JSFunctionSpec functions[] = {
      JS_FN("quit", &ReplGlobal::quit, 0, 0), JS_FS_END};

 public:
  static JSObject* create(JSContext* cx);
  static void loop(JSContext* cx, JS::HandleObject global);
};
constexpr JSClass ReplGlobal::klass;
constexpr JSFunctionSpec ReplGlobal::functions[];

static void die(const char* why) {
  std::cerr << "fatal error:" << why << std::endl;
  exit(1);
}

std::string FormatString(JSContext* cx, JS::HandleString string) {
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

std::string FormatResult(JSContext* cx, JS::HandleValue value) {
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

static void ReportAndClearException(JSContext* cx) {
  /* Get exception object before printing and clearing exception. */
  JS::ExceptionStack stack(cx);
  if (!JS::StealPendingExceptionStack(cx, &stack))
    die("Uncatchable exception thrown, out of memory or something");

  JS::ErrorReportBuilder report(cx);
  if (!report.init(cx, stack, JS::ErrorReportBuilder::WithSideEffects))
    die("Couldn't build error report");

  JS::PrintError(cx, stderr, report, false);
}

JSObject* ReplGlobal::create(JSContext* cx) {
  JS::RealmOptions options;
  JS::RootedObject global(cx,
                          JS_NewGlobalObject(cx, &ReplGlobal::klass, nullptr,
                                             JS::FireOnNewGlobalHook, options));

  ReplGlobal* priv = new ReplGlobal();
  JS_SetPrivate(global, priv);

  // Define any extra global functions that we want in our environment.
  JSAutoRealm ar(cx, global);
  if (!JS_DefineFunctions(cx, global, ReplGlobal::functions)) return nullptr;

  return global;
}

bool EvalAndPrint(JSContext* cx, const std::string& buffer, unsigned lineno) {
  JS::CompileOptions options(cx);
  options.setFileAndLine("typein", lineno);

  JS::SourceText<mozilla::Utf8Unit> source;
  if (!source.init(cx, buffer.c_str(), buffer.size(),
                   JS::SourceOwnership::Borrowed)) {
    return false;
  }

  JS::RootedValue result(cx);
  if (!JS::Evaluate(cx, options, source, &result)) return false;

  JS_MaybeGC(cx);

  if (result.isUndefined()) return true;

  std::string display_str = FormatResult(cx, result);
  if (!display_str.empty()) std::cout << display_str << '\n';
  return true;
}

void ReplGlobal::loop(JSContext* cx, JS::HandleObject global) {
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
      if (line[0] != '\0') add_history(line);
      buffer += line;
      lineno++;
    } while (!JS_Utf8BufferIsCompilableUnit(cx, global, buffer.c_str(),
                                            buffer.length()));

    if (!EvalAndPrint(cx, buffer, startline)) {
      if (!priv(global)->m_shouldQuit) ReportAndClearException(cx);
    }

    js::RunJobs(cx);
  } while (!eof && !priv(global)->m_shouldQuit);
}

static bool RunREPL(JSContext* cx) {
  // In order to use Promises in the REPL, we need a job queue to process
  // events after each line of input is processed.
  //
  // A more sophisticated embedding would schedule it's own tasks and use
  // JS::SetEnqueuePromiseJobCallback(), JS::SetGetIncumbentGlobalCallback(),
  // and JS::SetPromiseRejectionTrackerCallback().
  if (!js::UseInternalJobQueues(cx)) return false;

  // We must instantiate self-hosting *after* setting up job queue.
  if (!JS::InitSelfHostedCode(cx)) return false;

  JS::RootedObject global(cx, ReplGlobal::create(cx));
  if (!global) return false;

  JSAutoRealm ar(cx, global);

  JS::SetWarningReporter(cx, [](JSContext* cx, JSErrorReport* report) {
    JS::PrintError(cx, stderr, report, true);
  });

  ReplGlobal::loop(cx, global);

  std::cout << '\n';
  return true;
}

int main(int argc, const char* argv[]) {
  if (!boilerplate::RunExample(RunREPL, /* initSelfHosting = */ false))
    return 1;
  return 0;
}
