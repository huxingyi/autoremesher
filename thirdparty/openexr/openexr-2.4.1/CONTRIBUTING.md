# Contributing to OpenEXR

Thank you for your interest in contributing to OpenEXR. This document
explains our contribution process and procedures:

* [Getting Information](#Getting-Information)
* [Legal Requirements](#Legal-Requirements)
* [Development Workflow](#Development-Workflow)
* [Coding Style](#Coding-Style)
* [Versioning Policy](#Versioning-Policy)
* [Creating a Release](#Creating-a-Release)

For a description of the roles and responsibilities of the various
members of the OpenEXR community, see [GOVERNANCE](GOVERNANCE.md), and
for further details, see the project's [Technical
Charter](ASWF/charter/OpenEXR-Technical-Charter.md). Briefly,
Contributors are anyone who submits content to the project, Committers
review and approve such submissions, and the Technical Steering
Committee provides general project oversight.

## Getting Information

There are two primary ways to connect with the OpenEXR project:

* The [openexr-dev](https://lists.aswf.io/g/openexr-dev) mail list:
  This is a development focused mail list with a deep history of
  technical conversations and decisions that have shaped the project.

* [GitHub Issues](https://github.com/openexr/openexr/issues): GitHub
  Issues are used both to track bugs and to discuss feature requests.

### How to Ask for Help

If you have trouble installing, building, or using the library, but
there's not yet reason to suspect you've encountered a genuine bug,
start by posting a question to the
[openexr-dev](http://lists.aswf.io/openexr-dev) mailing list. This is
the place for question such has "How do I...".

### How to Report a Bug

OpenEXR use GitHub's issue tracking system for bugs and enhancements:
https://github.com/openexr/openexr/issues

If you are submitting a bug report, please be sure to note which
version of OpenEXR you are using, on what platform (OS/version, which
compiler you used, and any special build flags or other unusual
environmental issues). Please give a specific account of

* what you tried
* what happened
* what you expected to happen instead

with enough detail that others can reproduce the problem.

### How to Request a Change

Open a GitHub issue: https://github.com/openexr/openexr/issues.

Describe the situation and the objective in as much detail as
possible. Feature requests will almost certainly spawn a discussion
among the project community.

### How to Report a Security Vulnerability

If you think you've found a potential vulnerability in OpenEXR, please
report it by emailing security@openexr.com. Only TSC members and ASWF
project management have access to these messages. Include detailed
steps to reproduce the issue, and any other information that could aid
an investigation.

### How to Contribute a Bug Fix or Change

To contribute code to the project, first read over the [GOVERNANCE](GOVERNANCE.md) page to understand the roles involved. You'll need:

* A good knowledge of git.

* A fork of the GitHub repo.

* An understanding of the project's development workflow.

* Legal authorization, that is, you need to have signed a Contributor
  License Agreement. See below for details.

## Legal Requirements

OpenEXR is a project of the Academy Software Foundation and follows the
open source software best practice policies of the Linux Foundation.

### License

OpenEXR is licensed under the [BSD-3-Clause](LICENSE.md)
license. Contributions to the library should abide by that standard
license.

### Contributor License Agreements

Developers who wish to contribute code to be considered for inclusion
in the OpenEXR distribution must first complete a **Contributor
License Agreement**.

OpenEXR uses EasyCLA for managing CLAs, which automatically
checks to ensure CLAs are signed by a contributor before a commit
can be merged. 

* If you are an individual writing the code on your own time and
  you're SURE you are the sole owner of any intellectual property you
  contribute, you can [signing the CLA as an individual contributor](https://github.com/communitybridge/easycla/blob/master/docs/Sign-a-CLA-as-an-Individual-Contributor-to-GitHub.md).

* If you are writing the code as part of your job, or if there is any
  possibility that your employers might think they own any
  intellectual property you create, then you should use the [Corporate
  Contributor Licence
  Agreement](https://github.com/communitybridge/easycla/blob/master/docs/Contribute-to-a-GitHub-Company-Project.md).

The OpenEXR CLA's are the standard forms used by Linux Foundation
projects and [recommended by the ASWF TAC](https://github.com/AcademySoftwareFoundation/tac/blob/master/process/contributing.md#contributor-license-agreement-cla).

### Commit Sign-Off

Every commit must be signed off.  That is, every commit log message
must include a “`Signed-off-by`” line (generated, for example, with
“`git commit --signoff`”), indicating that the committer wrote the
code and has the right to release it under the
[Modified-BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)
license. See https://github.com/AcademySoftwareFoundation/tac/blob/master/process/contributing.md#contribution-sign-off for more information on this requirement.

## Development Workflow

### Git Basics

Working with OpenEXR requires understanding a significant amount of
Git and GitHub based terminology. If you’re unfamiliar with these
tools or their lingo, please look at the [GitHub
Glossary](https://help.github.com/articles/github-glossary/) or browse
[GitHub Help](https://help.github.com/).

To contribute, you need a GitHub account. This is needed in order to
push changes to the upstream repository, via a pull request.

You will also need Git installed on your local development machine. If
you need setup assistance, please see the official [Git
Documentation](https://git-scm.com/doc).

### Repository Structure and Commit Policy

The OpenEXR repository uses a simple branching and merging strategy.

All development work is done directly on the master branch. The master
branch represents the bleeding-edge of the project and most
contributions should be done on top of it.

After sufficient work is done on the master branch and the OpenEXR
leadership determines that a release is due, we will bump the relevant
internal versioning and tag a commit with the corresponding version
number, e.g. v2.0.1. Each Minor version also has its own “Release
Branch”, e.g. RB-1.1. This marks a branch of code dedicated to that
Major.Minor version, which allows upstream bug fixes to be
cherry-picked to a given version while still allowing the master
branch to continue forward onto higher versions. This basic repository
structure keeps maintenance low, while remaining simple to understand.

To reiterate, the master branch represents the latest development
version, so beware that it may include untested features and is not
generally stable enough for release.  To retrieve a stable version of
the source code, use one of the release branches.

### The Git Workflow

This development workflow is sometimes referred to as
[OneFlow](https://www.endoflineblog.com/oneflow-a-git-branching-model-and-workflow). It
leads to a simple, clean, linear edit history in the repo.

The OpenEXR GitHub repo allows rebase merging and disallows merge
commits and squash merging. This ensures that the repo edit history
remains linear, avoiding the "bubbles" characteristic of the
[GitFlow](https://www.endoflineblog.com/gitflow-considered-harmful)
workflow.

### Use the Fork, Luke.

In a typical workflow, you should **fork** the OpenEXR repository to
your account. This creates a copy of the repository under your user
namespace and serves as the “home base” for your development branches,
from which you will submit **pull requests** to the upstream
repository to be merged.

Once your Git environment is operational, the next step is to locally
**clone** your forked OpenEXR repository, and add a **remote**
pointing to the upstream OpenEXR repository. These topics are
covered in the GitHub documentation [Cloning a
repository](https://help.github.com/articles/cloning-a-repository/)
and [Configuring a remote for a
fork](https://help.github.com/articles/configuring-a-remote-for-a-fork/),
but again, if you need assistance feel free to reach out on the
openexr-dev@lists.aswf.io mail list.

### Pull Requests

Contributions should be submitted as Github pull requests. See
[Creating a pull request](https://help.github.com/articles/creating-a-pull-request/)
if you're unfamiliar with this concept. 

The development cycle for a code change should follow this protocol:

1. Create a topic branch in your local repository, following the naming format
"feature/<your-feature>" or "bugfix/<your-fix>".

2. Make changes, compile, and test thoroughly. Code style should match existing
style and conventions, and changes should be focused on the topic the pull
request will be addressing. Make unrelated changes in a separate topic branch
with a separate pull request.

3. Push commits to your fork.

4. Create a Github pull request from your topic branch.

5. Pull requests will be reviewed by project Committers and Contributors,
who may discuss, offer constructive feedback, request changes, or approve
the work.

6. Upon receiving the required number of Committer approvals (as
outlined in [Required Approvals](#required-approvals)), a Committer
other than the PR contributor may merge changes into the master
branch.

### Code Review and Required Approvals

Modifications of the contents of the OpenEXR repository are made on a
collaborative basis. Anyone with a GitHub account may propose a
modification via pull request and it will be considered by the project
Committers.

Pull requests must meet a minimum number of Committer approvals prior
to being merged. Rather than having a hard rule for all PRs, the
requirement is based on the complexity and risk of the proposed
changes, factoring in the length of time the PR has been open to
discussion. The following guidelines outline the project's established
approval rules for merging:

* Core design decisions, large new features, or anything that might be
perceived as changing the overall direction of the project should be
discussed at length in the mail list before any PR is submitted, in
order to: solicit feedback, try to get as much consensus as possible,
and alert all the stakeholders to be on the lookout for the eventual
PR when it appears.

* Small changes (bug fixes, docs, tests, cleanups) can be approved and
merged by a single Committer.

* Big changes that can alter behavior, add major features, or present
a high degree of risk should be signed off by TWO Committers, ideally
one of whom should be the "owner" for that section of the codebase (if
a specific owner has been designated). If the person submitting the PR
is him/herself the "owner" of that section of the codebase, then only
one additional Committer approval is sufficient. But in either case, a
48 hour minimum is helpful to give everybody a chance to see it,
unless it's a critical emergency fix (which would probably put it in
the previous "small fix" category, rather than a "big feature").

* Escape valve: big changes can nonetheless be merged by a single
Committer if the PR has been open for over two weeks without any
unaddressed objections from other Committers. At some point, we have
to assume that the people who know and care are monitoring the PRs and
that an extended period without objections is really assent.

Approval must be from Committers who are not authors of the change. If
one or more Committers oppose a proposed change, then the change
cannot be accepted unless:

* Discussions and/or additional changes result in no Committers
objecting to the change. Previously-objecting Committers do not
necessarily have to sign-off on the change, but they should not be
opposed to it.

* The change is escalated to the TSC and the TSC votes to approve the
change.  This should only happen if disagreements between Committers
cannot be resolved through discussion.

Committers may opt to elevate significant or controversial
modifications to the TSC by assigning the `tsc-review` label to a pull
request or issue. The TSC should serve as the final arbiter where
required.

### Test Policy

All functionality in the library must be covered by an automated
test. Each library has a companion ``Test`` project - ``ImathTest``,
``HalfTest``, ``IlmImfTest`, etc.  This test suite is collectively
expected to validate the behavior of very part of the library.

* Any new functionality should be accompanied by a test that validates
  its behavior.

* Any change to existing functionality should have tests added if they
  don't already exist.

The test should should be run, via ``make check``, before submitting a
pull request.

In addition, the ``IlmImfFuzzTest`` project validates the library by
feeding it corrupted input data. This test is time-consuming (possible
over 24 hours), so it will only be run occasionally, but it must
succeed before a release is made.

### Project Issue Handling Process

Incoming new issues are labeled promptly by the TSC using GitHub labels. 

The labels include:

* **Autotools** - A problem with the autoconf configuration setup.

* **Bug** - A bug in the source code. Something appears to be
    functioning improperly: a compile error, a crash, unexpected behavior, etc. 

* **Build/Install Issue** - A problem with building or installing the
    library: configuration file, external dependency, a compile error
    with a release version that prevents installation.

* **C++** - A C++ compilation issue: a compiler warning, syntax issue,
    or language usage or suggested upgrade.

* **CMake** - A build issue with the CMake configuration files.

* **CVE** - A security vulnerability bug.

* **Documentation** - The project documentation: developer or user
    guide, web site, project policies, etc.

* **Feature Request** - A suggested change or addition of
    functionality to the library.

* **Mac OS** - A build issue specific to Mac OS.

* **MinGW** - An issue specific to MinGW

* **Modification** - A modification to the code, refactoring or
    optimization without significant additional behavior

* **Needs Info** - Issue is waiting for more information from the
    submitter.

* **Question/Problem/Help** - A request for help or further
    investigation, possibly just user error or misunderstanding.

* **Test Failure** - One of the automated tests is failing, or an
    analysis tool is reporting problematic behavior.

* **TSC** - To be discussed in the technical steering committee.

* **Windows** - A build issue specific to Windows

* **Won't Fix** - No further action will taken.

## Coding Style

#### Formatting

When modifying existing code, follow the surrounding formatting
conventions so that new or modified code blends in with the current
code.

* Indent with spaces, never tabs. Each indent level should be 4 spaces.

* Function return types go on a separate line:

        const float &	
        Header::pixelAspectRatio () const
        {
            ...
        }

* Use a space between function names and the following parentheses
  (although you can eliminate the space for functions with no
  arguments):

        void
        Header::insert (const string& name, const Attribute& attribute)
        {
            insert (name.c_str(), attribute);
        }

* Place curly braces on their own lines:

        void
        RgbaOutputFile::ToYca::padTmpBuf ()
        {
            for (int i = 0; i < N2; ++i)
            {
                _tmpBuf[i] = _tmpBuf[N2];
                _tmpBuf[_width + N2 + i] = _tmpBuf[_width + N2 - 2];
            }
        }

#### Naming Conventions

* In general, classes and template type names should start with upper
  case and capitalize new words: `class CustomerList;`

* In general, local variables should use camelCase. Macros and
  constants should use `ALL_CAPS`.

* Member fields in a class should start with an underscore. No other
  variables should begin with underscore.

#### File conventions

C++ implementation should be named `*.cpp`. Headers should be named `.h`.

All headers should contain:

    #pragma once

#### Type Conventions

Because OpenEXR must deal properly with large images, whose width
and/or height approach the maximum allowable in 32-bit signed
integers, take special care that integer arithmatic doesn't overlow,
and make it as clear as possible exactly what the code is doing,
especially in the edge cases.

To clarify the intention, prefer to cast between types using
``static_cast<>()`` rather than the basic C-style ``()`` notation:

    // good:
    size_t x = static_cast <size_t> (y);

    // bad:
    x = (size_t) y;
    x = size_t (y);

Prefer to use ``std::numeric_limits<>`` instead of preprocesser
define's such as ``INT_MAX``:

    // good:
    if (x > std::numeric_limits<int>::max())
        std::cout << "That's too freakin' high.\n";

    // bad:
    if (x > INT_MAX)

#### Copyright Notices

All new source files should begin with a copyright and license stating:

    // Copyright (c) Contributors to the OpenEXR Project. All rights reserved.
    // SPDX-License-Identifier: BSD-3-Clause

#### Third-party libraries

Prefer C++11 `std` over boost where possible.  Use boost classes you
already see in the code base, but check with the project leadership
before adding new boost usage.

#### Comments and Doxygen

Comment philosophy: try to be clear, try to help teach the reader
what's going on in your code.

Prefer C++ comments (starting line with `//`) rather than C comments
(`/* ... */`).

For public APIs, use Doxygen-style comments (start with `///`), such as:

    /// Explanation of a class.  Note THREE slashes!
    /// Also, you need at least two lines like this.  If you don't have enough
    /// for two lines, make one line blank like this:
    ///
    class myclass {
        ....
        float foo;  ///< Doxygen comments on same line look like this
    }

## Versioning Policy

OpenEXR uses [semantic versioning](https://semver.org), which labels
each version with three numbers: Major.Minor.Patch, where:

* **MAJOR** indicates incompatible API changes
* **MINOR** indicates functionality added in a backwards-compatible manner
* **PATCH** indicates backwards-compatible bug fixes 

## Creating a Release

To create a new release from the master branch:

1. Update the release notes in ``CHANGES.md``.

   Write a high-level summary of the features and
   improvements. Include the summary in ``CHANGES.md`` and also in the
   Release comments.

   Include the log of all changes since the last release, via:

        git log v2.2.1...v2.3.0 --date=short --pretty=format:"[%s](https://github.com/openexr/openexr/commit/%H) ([%an](@%ae) %ad)"

   Include diff status via:

        git diff --stat v2.2.1
       
2. Create a new release on the GitHub Releases page.

3. Tag the release with name beginning with '``v``', e.g. '``v2.3.0``'.

4. Download and sign the release tarball, as described
[here](https://wiki.debian.org/Creating%20signed%20GitHub%20releases),

5. Attach the detached ``.asc`` signature file to the GitHub release as a
binary file.
    
