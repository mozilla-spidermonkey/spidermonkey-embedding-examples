# Migration Guide #

This document describes how to port your code from using one ESR version
of SpiderMonkey to the next ESR version.

## ESR 60 to ESR 68 ##

### Remove requests ###

Previously, a "request" was a global lock on the JS interpreter that was
required in order to use many parts of the JSAPI.
This is no longer necessary, and can simply be deleted.

**Recommendation**: Remove all `JSAutoRequest`, `JS_BeginRequest()`, and
`JS_EndRequest()` from your code.
