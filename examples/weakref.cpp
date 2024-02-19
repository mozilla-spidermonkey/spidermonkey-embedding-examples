#include <stdio.h>

#include <jsapi.h>
#include <jsfriendapi.h>
#include <js/CompilationAndEvaluation.h>
#include <js/Initialization.h>
#include <js/Promise.h>
#include <js/Realm.h>
#include <js/SourceText.h>
#include <mozilla/Unused.h>

#include "boilerplate.h"

// This example illustrates what you have to do in your embedding to make
// WeakRef and FinalizationRegistry work. Without notifying SpiderMonkey when to
// clear out WeakRefs and run FinalizationRegistry callbacks, they will appear
// not to work correctly.
//
// See 'boilerplate.cpp' for the parts of this example that are reused in many
// simple embedding examples.

// This function silently ignores errors in a way that production code probably
// wouldn't.
static void LogPendingException(JSContext* cx) {
  // Nothing we can do about uncatchable exceptions.
  if (!JS_IsExceptionPending(cx)) return;

  JS::ExceptionStack exnStack{cx};
  if (!JS::StealPendingExceptionStack(cx, &exnStack)) return;

  JS::ErrorReportBuilder builder{cx};
  if (!builder.init(cx, exnStack, JS::ErrorReportBuilder::NoSideEffects)) {
    return;
  }
  JS::PrintError(stderr, builder, /* reportWarnings = */ false);
}

// This example integrates the FinalizationRegistry job queue together with the
// Promise job handling, since that's a logical place that you might put it in
// your embedding.
//
// However, it's not necessary to use JS::JobQueue and it's not necessary to
// handle Promise jobs, in order to have FinalizationRegistry work. You do need
// to have some kind of job queue, but it can be very minimal. It doesn't have
// to be based on JS::JobQueue. The only requirement is that the enqueued
// cleanup functions must be run "some time in the future".
//
// To approximate a minimal job queue, you might remove m_queue from this class
// and remove the inheritance from JS::JobQueue and its overridden methods.
class CustomJobQueue : public JS::JobQueue {
 public:
  explicit CustomJobQueue(JSContext* cx)
      : m_queue(cx, js::SystemAllocPolicy{}),
        m_finalizationRegistryCallbacks(cx),
        m_draining(false) {}
  ~CustomJobQueue() = default;

  // JS::JobQueue override
  JSObject* getIncumbentGlobal(JSContext* cx) override {
    return JS::CurrentGlobalOrNull(cx);
  }

  // JS::JobQueue override
  bool enqueuePromiseJob(JSContext* cx, JS::HandleObject promise,
                         JS::HandleObject job, JS::HandleObject allocationSite,
                         JS::HandleObject incumbentGlobal) override {
    if (!m_queue.append(job)) {
      JS_ReportOutOfMemory(cx);
      return false;
    }

    JS::JobQueueMayNotBeEmpty(cx);
    return true;
  }

  // JS::JobQueue override
  void runJobs(JSContext* cx) override {
    // Ignore nested calls of runJobs.
    if (m_draining) {
      return;
    }

    m_draining = true;

    JS::Rooted<JSObject*> job{cx};
    JS::Rooted<JS::Value> unused_rval{cx};

    while (true) {
      // Execute jobs in a loop until we've reached the end of the queue.
      while (!m_queue.empty()) {
        job = m_queue[0];
        m_queue.erase(m_queue.begin());  // In production code, use a FIFO queue

        // If the next job is the last job in the job queue, allow skipping the
        // standard job queuing behavior.
        if (m_queue.empty()) {
          JS::JobQueueIsEmpty(cx);
        }

        JSAutoRealm ar{cx, job};
        if (!JS::Call(cx, JS::UndefinedHandleValue, job,
                      JS::HandleValueArray::empty(), &unused_rval)) {
          // We can't throw the exception here, because there is nowhere to
          // catch it. So, log it.
          LogPendingException(cx);
        }
      }

      // FinalizationRegistry callbacks may queue more jobs, so only stop
      // running jobs if there were no FinalizationRegistry callbacks to run.
      if (!maybeRunFinalizationRegistryCallbacks(cx)) break;
    }

    m_draining = false;
    m_queue.clear();
  }

  // JS::JobQueue override
  bool empty() const override { return m_queue.empty(); }

  void queueFinalizationRegistryCallback(JSFunction* callback) {
    mozilla::Unused << m_finalizationRegistryCallbacks.append(callback);
  }

 private:
  using JobQueueStorage = JS::GCVector<JSObject*, 0, js::SystemAllocPolicy>;
  JS::PersistentRooted<JobQueueStorage> m_queue;

  using FunctionVector = JS::GCVector<JSFunction*, 0, js::SystemAllocPolicy>;
  JS::PersistentRooted<FunctionVector> m_finalizationRegistryCallbacks;

  // True if we are in the midst of draining jobs from this queue. We use this
  // to avoid re-entry (nested calls simply return immediately).
  bool m_draining : 1;

  class SavedQueue : public JobQueue::SavedJobQueue {
   public:
    SavedQueue(JSContext* cx, CustomJobQueue* jobQueue)
        : m_jobQueue(jobQueue),
          m_saved(cx, std::move(jobQueue->m_queue.get())),
          m_draining(jobQueue->m_draining) {}

    ~SavedQueue() {
      m_jobQueue->m_queue = std::move(m_saved.get());
      m_jobQueue->m_draining = m_draining;
    }

   private:
    CustomJobQueue* m_jobQueue;
    JS::PersistentRooted<JobQueueStorage> m_saved;
    bool m_draining : 1;
  };

  // JS::JobQueue override
  js::UniquePtr<JS::JobQueue::SavedJobQueue> saveJobQueue(
      JSContext* cx) override {
    auto saved = js::MakeUnique<SavedQueue>(cx, this);
    if (!saved) {
      // When MakeUnique's allocation fails, the SavedQueue constructor is never
      // called, so this->queue is still initialized. (The move doesn't occur
      // until the constructor gets called.)
      JS_ReportOutOfMemory(cx);
      return nullptr;
    }

    m_queue.clear();
    m_draining = false;
    return saved;
  }

  bool maybeRunFinalizationRegistryCallbacks(JSContext* cx) {
    bool ranCallbacks = false;

    JS::Rooted<FunctionVector> callbacks{cx};
    std::swap(callbacks.get(), m_finalizationRegistryCallbacks.get());
    for (JSFunction* f : callbacks) {
      JS::ExposeObjectToActiveJS(JS_GetFunctionObject(f));

      JSAutoRealm ar{cx, JS_GetFunctionObject(f)};
      JS::Rooted<JSFunction*> func{cx, f};
      JS::Rooted<JS::Value> unused_rval{cx};
      if (!JS_CallFunction(cx, nullptr, func, JS::HandleValueArray::empty(),
                           &unused_rval)) {
        LogPendingException(cx);
      }

      ranCallbacks = true;
    }

    return ranCallbacks;
  }
};

static void CleanupFinalizationRegistry(JSFunction* callback,
                                        JSObject* incumbent_global
                                        [[maybe_unused]],
                                        void* user_data) {
  // Queue a cleanup task to run after each job has been run.
  // We only have one global so ignore the incumbent global parameter.
  auto* jobQueue = static_cast<CustomJobQueue*>(user_data);
  jobQueue->queueFinalizationRegistryCallback(callback);
}

static bool GC(JSContext* cx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

  JS_GC(cx, JS::GCReason::API);

  args.rval().setUndefined();
  return true;
}

static bool RunJobs(JSContext* cx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

  // This calls JS::ClearKeptObjects() after draining the job queue. If you're
  // not using js::RunJobs(), you'll have to call it yourself -- otherwise, the
  // WeakRefs will never be emptied.
  js::RunJobs(cx);

  args.rval().setUndefined();
  return true;
}

static bool ExecuteCode(JSContext* cx, const char* code) {
  JS::CompileOptions options{cx};
  options.setFileAndLine("noname", 1);

  JS::SourceText<mozilla::Utf8Unit> source;
  if (!source.init(cx, code, strlen(code), JS::SourceOwnership::Borrowed)) {
    return false;
  }

  JS::Rooted<JS::Value> rval{cx};
  return JS::Evaluate(cx, options, source, &rval);
}

static bool WeakRefExample(JSContext* cx) {
  // Using WeakRefs and FinalizationRegistry requires a job queue. The built-in
  // job queue used in repl.cpp is not sufficient, because it does not provide
  // any way to queue FinalizationRegistry cleanup callbacks.
  CustomJobQueue jobQueue{cx};
  JS::SetJobQueue(cx, &jobQueue);

  // Without this, FinalizationRegistry callbacks will never be called. The
  // embedding has to decide when to schedule them.
  JS::SetHostCleanupFinalizationRegistryCallback(
      cx, CleanupFinalizationRegistry, &jobQueue);

  JS::RealmOptions options;
  options.creationOptions().setWeakRefsEnabled(
      JS::WeakRefSpecifier::EnabledWithoutCleanupSome);

  static JSClass GlobalClass = {"WeakRefsGlobal", JSCLASS_GLOBAL_FLAGS,
                                &JS::DefaultGlobalClassOps};

  JS::Rooted<JSObject*> global{
      cx, JS_NewGlobalObject(cx, &GlobalClass, nullptr, JS::FireOnNewGlobalHook,
                             options)};
  if (!global) return false;

  JSAutoRealm ar{cx, global};

  if (!JS_DefineFunction(cx, global, "gc", &GC, 0, 0) ||
      !JS_DefineFunction(cx, global, "runJobs", &RunJobs, 0, 0)) {
    boilerplate::ReportAndClearException(cx);
    return false;
  }

  if (!ExecuteCode(cx, R"js(
    let valueFinalized;
    const registry = new FinalizationRegistry(
      heldValue => (valueFinalized = heldValue));
    let obj = {};
    const weakRef = new WeakRef(obj);
    registry.register(obj, "marker");

    obj = null;

    runJobs();  // Makes weakRef eligible for clearing
    gc();  // Clears weakRef, collects obj which is no longer live, and
           // enqueues finalization registry cleanup

    if (weakRef.deref() !== undefined) throw new Error("WeakRef");

    runJobs();  // Runs finalization registry cleanup

    if (valueFinalized !== "marker") throw new Error("FinalizationRegistry");
  )js")) {
    boilerplate::ReportAndClearException(cx);
    return false;
  }

  return true;
}

int main(int argc, const char* argv[]) {
  if (!boilerplate::RunExample(WeakRefExample)) {
    return 1;
  }
  return 0;
}
