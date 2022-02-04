# Proposal to add <architecture name> support to SpiderMonkey

This is a template for you to fill out **before** writing a port. If you write a port without filling out this form, please do not be surprised if we reject it.

Once this form is filled, post it on the [spidermonkey discourse](https://discourse.mozilla.org/c/spidermonkey/551) for discussion

## Overall impact of the Port

**If the answer to any of the next two questions is “no”, then we will not be able to accept the port:**

### Is the stack-direction the same as x86?

< your answer here >

### Is the code for a little-endian architecture?

< your answer here >

**If either of the above questions is “no”, it will result in a highly invasive port. We cannot accept it.**

### How is this architecture different from x64 or ARM64?
This could be about data representations, flag bits, atomics, special registers, special constraints on registers, and things of that nature. It could also be if it is 32 bit.

< your answer here >

### Do you expect that changes will be needed to the JITs to support this architecture?

< your answer here >

### Any other comments regarding the technical impact of porting to this architecture.

Relevant information may be:
* A proof of concept / prototype demonstrating the complexity of the port
* A detailed analysis of the SpiderMonkey codebase and target architecture.

< your answer here >

## Maintenance of the port

SpiderMonkey is an actively developed project, the SpiderMonkey team is not responsible for independent ports. Maintaining a port requires multiple hours per week of dedicated work. At times, when larger changes are made to the engine, this may grow to several days to several weeks. To make sure the port is successful, we want to understand what resources you have available to maintain it.

### Who will be the contact person(s) of this project?

< your answer here >

### Do the contact person(s) have dedicated time to maintain the port?

< your answer here >

### Do you have a plan for testing the port (continuous integration or similar)

< your answer here >

### Any other comments about maintenance?
Relevant information might be:
* The port has funding
* You have experience maintaining similar size projects
* You are involved in developing spidermonkey in general

< your answer here >

## Establishing relevance of the port

Finally, we want to understand what communities this port serves. It helps us determine the relevance and the importance of the work

### How commonly used is the architecture you are targeting?

< your answer here >

### What use cases does SpiderMonkey running on this port open up which were closed before?

< your answer here >

### What use case(s) does this open for Firefox?

< your answer here >

### Is there a business case for this port for Mozilla or Firefox?

< your answer here >

### Any other comments regarding impacts the port may have more broadly?
Relevant information may be:
* Planned work not captured by the above questions

< your answer here >


### Do you have any further information you’d like us to know?

<your answer here>


