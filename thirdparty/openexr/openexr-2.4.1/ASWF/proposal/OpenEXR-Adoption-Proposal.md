# OpenEXR Adoption Proposal


## ASWF TAC Meeting - April 10th, 2019


# Overview



1. Project Contribution Proposal
2. Incubation Stage Requirements
3. Adopted Stage Requirements
4. Additional Discussion Topics
    1. Near-Term Development Roadmap
    2. Community Discussion at SIGGRAPH 2018
    3. Technical Steering Committee


# Project Contribution Proposal

The following is an overview of the Project Contribution Proposal for OpenEXR:



*   Name of the project:
    *   OpenEXR
*   Requested project maturity level:
    *   Incubation (initially proposed as Adopted)
*   Project description:
    *   OpenEXR is a high dynamic-range image file format developed by Industrial Light & Magic (ILM) for use in computer imaging applications. OpenEXR was created by ILM in 1999 and released to the public in 2003 as an open-source library.
*   Please explain how this project is aligned with the mission of ASWF?
    *   OpenEXR is one of the foundational technologies in computer imaging, and it remains a standard HDR image format in computer graphics for linear and interactive media. OpenEXR was honored with an Academy Award for Technical Achievement in 2007.
*   What is the project’s license for code contributions?
    *   OpenEXR is shared under a modified BSD license (http://www.openexr.com/license.html), and its license agreements for individual and corporate code contributions may be found at [http://www.openexr.com/documentation.html](http://www.openexr.com/documentation.html).
*   What tool or platform is utilized for source control, and what is the location?
    *   OpenEXR uses GitHub for source control, and its repository is currently located at [https://github.com/openexr/openexr](https://github.com/openexr/openexr).
*   What are the external dependencies of the project, and what are the licenses of those dependencies?
    *   The mandatory dependency of OpenEXR and its license is:
        *   Zlib ([http://zlib.net/zlib_license.html](http://zlib.net/zlib_license.html))
    *   The optional dependencies of OpenEXR and their licenses are:
        *   Boost ([https://www.boost.org/users/license.html](https://www.boost.org/users/license.html))
        *   NumPy ([http://www.numpy.org/license.html](http://www.numpy.org/license.html))
        *   Fltk ([https://www.fltk.org/COPYING.php](https://www.fltk.org/COPYING.php))
        *   Cg ([http://developer.download.nvidia.com/cg/Cg_2.2/license.pdf](http://developer.download.nvidia.com/cg/Cg_2.2/license.pdf))
*   What roles does the project have (e.g. maintainers, committers?) Who are the current core committers of the project, or where can a list of committers be found?
    *   Recent contributors to the OpenEXR project include Cary Phillips, Nick Rasmussen, Kimball Thurston and Nick Porcino, and a complete list of current maintainers can be found at [https://github.com/orgs/openexr/people](https://github.com/orgs/openexr/people).
*   What mailing lists are currently used by the project?
    *   OpenEXR has three mailing lists: openexr-announce, openexr-user, and openexr-devel.
        *   [https://lists.nongnu.org/mailman/listinfo/openexr-announce](https://lists.nongnu.org/mailman/listinfo/openexr-announce)
        *   [https://lists.nongnu.org/mailman/listinfo/openexr-user](https://lists.nongnu.org/mailman/listinfo/openexr-user)
        *   [https://lists.nongnu.org/mailman/listinfo/openexr-devel](https://lists.nongnu.org/mailman/listinfo/openexr-devel)
*   What tool or platform is leveraged by the project for issue tracking?
    *   Issue tracking for OpenEXR is handled through its GitHub repository:

        [https://github.com/openexr/openexr](https://github.com/openexr/openexr)

*   Does the project have a Core Infrastructure Initiative security best practices badge? Do you foresee any challenges obtaining one?
    *   OpenEXR does not yet have a CII badge, but it should be straightforward for it to meet the criteria of this program in the future.
*   What is the project’s website? Is there a wiki?
    *   The website for OpenEXR is [http://www.openexr.com](http://www.openexr.com), and there is no official wiki.
*   What social media accounts are used by the project?
    *   OpenEXR does not have any official social media accounts.
*   What is the project’s release methodology and cadence?
    *   New releases of OpenEXR are organized on an as-needed basis, and do not currently follow a regular cadence.
*   Are any trademarks, registered or unregistered, leveraged by the project?  Have any trademark registrations been filed by the project or any third party anywhere in the world?
    *   OpenEXR is an unregistered trademark of Lucasfilm Ltd.


# Incubation Stage Requirements

The following are the entry requirements for an Incubation stage project of the ASWF, which we believe are currently met by the OpenEXR project:



*   Submit a completed Project Contribution Proposal Template to the TAC, or the TAC’s designated recipient for contribution proposals.
*   Provide such additional information as the TAC may reasonably request.
*   Be available to present to the TAC with respect to the project’s proposal and inclusion in ASWF. Project teams should be prepared to present a detailed (20-30 minutes in length) overview on the project in addition to speaking to the information contained in the project contribution proposal.
*   Be deemed by the TAC to add value to the mission of ASWF.
*   Have a technical charter that provides for inbound and outbound licensing of code under an OSI-approved license approved by the Governing Board of ASWF for projects. The ASWF maintains a template for projects to use.
*   Agree to transfer any relevant trademarks to an LF entity to hold for the project. In the case of projects with established trademarks where a trademark transfer is commercially difficult, we generally recommend the project use a new name upon incubation.


# Adopted Stage Requirements

The following are the entry requirements for an Adopted stage project of the ASWF, along with the status of OpenEXR with respect to these goals:



*   Demonstrate having a healthy number of committers from a diverse contributor base.  A committer is defined in the technical charter but is often used to describe the core contributors who can accept contributions to the project, or a portion thereof.
    *   The OpenEXR project on GitHub currently has 39 contributors, including significant contributions from Lucasfilm, DreamWorks Animation, and Weta Digital, and we expect the number of contributing individuals and companies to increase under the ASWF.
*   Have achieved and maintained a Core Infrastructure Initiative Best Practices Badge.
    *   Not yet
*   Demonstrate a substantial ongoing flow of commits and merged contributions.
    *   The OpenEXR project on GitHub has 1,279 commits over its lifetime, and we expect the rate of new commits to increase once the project is under the ASWF.
*   Document current project owners and current and emeritus committers in a COMMITTER file or similarly visible system. A copy of the project’s charter (or other authorized governance document) will be included or linked to in visible location.
    *   Not yet
*   Have a technical lead appointed for representation of the project to the TAC.
    *   Not yet
*   Have a completed and presented to the TAC an initial license scan of the project’s codebase.
    *   Not yet


# Additional Discussion Topics


## Near-Term Development Roadmap



*   Robust builds, installation, and tests on all platforms, using the CMake build infrastructure.  Builds should be warning-free on all modern compilers.
*   Automated building and testing of new pull requests using one or more CI solutions (e.g. CircleCI, Azure Pipelines, Travis, Appveyor, Jenkins).
*   Evaluating and merging existing pull requests from the community (23 open pull requests).
*   Evaluating and addressing existing issues from the community (122 open issues).
*   Establishing guidance for reporting new security issues (e.g. a private channel).
*   Discuss moving IlmBase into its own Git submodule.
*   Discuss creating a lightweight, shared library for vector/matrix functionality.


## Community Discussion at SIGGRAPH 2018

The following additional ideas were suggested by the OpenEXR community at the Birds of a Feather event at SIGGRAPH 2018:



*   Simple and fast write path (along the lines of TinyEXR)
*   Integer image support (reducing the need for TIFF)
*   Standalone LibHalf library (and reconciliation with CUDA half type)
*   GPU decompression for performance (potentially supporting only a subset of formats)
*   Metadata standardization


## Technical Steering Committee

The initial members of the Technical Steering Committee for OpenEXR will be Larry Gritz (SPI), Rod Bogart (Epic), Peter Hillman (Weta), and Jonathan Stone (Lucasfilm).

In addition, we believe it would be valuable to nominate a small set of core committers, including recent contributors to OpenEXR, who would have push access to the new repository.


<!-- Docs to Markdown version 1.0β17 -->

