# SpiderMonkey Embedding Resources #

This repository contains documentation and examples for people who want
to embed the SpiderMonkey JavaScript engine.

The information on this `esr78` branch applies to SpiderMonkey 78.x, an
Extended Support Release (ESR).
For other versions of SpiderMonkey, check the other branches: `next` for
information that will apply in the next as-yet-unreleased ESR, or
earlier `esr` branches for previous versions.

# Docs #

Check the [`docs/`](docs/) directory for howtos and documentation, such as:

-  [Building SpiderMonkey for embedding](docs/Building%20SpiderMonkey.md )
-  [A JSAPI Introduction](docs/JSAPI%20Introduction.md)
-  [The Migration Guide from previous versions](docs/Migration%20Guide.md)


# Examples #

The [`examples/`](examples/) directory contains code examples.
See the [README in that directory](examples/README.md) for build
instructions.

## Custom architectures
SpiderMonkey supports a number of architectures as part of broadly distributing Firefox. However,
there may be an architecture that is not supported, on which you may wish to run SpiderMonkey.

In some cases, it will be easier to maintain a fork of SpiderMonkey. However if you have a broad use
case that furthers the mission of SpiderMonkey and Firefox, we have a series of questions for you to read and answer before embarking on the the implementation. The template for proposing a new architecture can be found [here](custom-architecture-questionnaire.md).

A successful proposal will usually have the following features:
* It will be minimally invasive
* It has a reasonable plan to be actively maintained
* It contributes to SpiderMonkey's success — by supporting its development, increasing its ecosystem influence, or both

### Review Process for new Architectures

Once we have your filled out proposal, the SpiderMonkey team will review it for any high level concerns, and consult internally about accepting this architecture. One of two things will happen: we will either give you a “go ahead” to start the work, or we will respond with a conclusion and reason why it would not be accepted. This will save you work and open an early communication channel.

If you get the “go ahead”, you will likely start on your patch. You can ask questions in the #spidermonkey channel if you run into trouble. Once your patch is finished, designated reviewers will *lightly* check your work. This will not be a full review, so make sure you have a partner who can fully review your work as well as a designated SpiderMonkey engineer.

Once the review is finished with at least 1 SpiderMonkey engineer, and 1 external reviewer familiar with your work accepting it, if there are no obvious issues raised by the SpiderMonkey engineer or the reviewer, the port will be merged behind a flag. We reserve the right to reject the port after it has been written, if we find it to be too invasive.

### Removal Process of Architectures

In some cases, we need to drop support for an architecture. If you're custom embedding relies on a custom community maintained architecture, it is important to keep an eye on the support of that architecture. If we find that an integrated custom architecture is:

* unmaintained
* causing issues with tier-1 support,
* or the situation since the acceptance of the port has changed

Then we will begin a removal process. The process looks like this:

1. A bug will be opened on bugzilla, cc'ing contact persons, if known.
2. The proposed removal will be announced in the [SpiderMonkey Newsletter](https://spidermonkey.dev/), and on the [SpiderMonkey Discourse](https://discourse.mozilla.org/c/spidermonkey/551).
3. If the bug cannot be resolved, or is not resolved within 3 months (time limit negotiable), the port will be flagged as a candidate for removal.
eg an "#error" in the source code that has to be removed to make the port compile, or a MOZ_CRASH on startup, there are probably other solutions
spidermonkey.dev/comments list the port as "unmaintained”
4. A patch will be created to remove the port, reviewed, and merged

To prevent having a port removed, make sure to follow bugzilla and work with SpiderMonkey engineers on any issues, and have a running CI to catch errors early. Sometimes, we will still have to make a decision, but this will hopefully be rare.

Any questions about this process can be asked in the #spidermonkey:mozilla.org matrix channel.

