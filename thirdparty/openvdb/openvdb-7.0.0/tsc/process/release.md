**Release Process for OpenVDB**

The following assumes that the current OpenVDB library version number is 6.0.0 and the new version number is 6.1.0. Adjust for the actual version numbers as appropriate.

- [ ] Open a Jira "Release OpenVDB 6.1.0" ticket with "OpenVDB_6.1.0" as the Fix Version.
- [ ] Update `CHANGES` and `doc/changes.txt` with release notes.  [_Specifics TBD, pending a review of release note management tools._]
- [ ] Open a pull request to merge the above changes into `openvdb/master`.  Associate the pull request with the Jira ticket created earlier, and verify that the CI build runs successfully.
- [ ] Draft a new [GitHub release](https://github.com/AcademySoftwareFoundation/openvdb/releases). Title it "OpenVDB 6.1.0" and tag it as `v6.1.0`.

- [ ] Update `openvdb-website/contents/index.html` with a news item announcing the release, and delete the oldest news item.  Open that page in a browser and check that the website renders correctly and that there are no broken links.
- [ ] Build the documentation (both the `doc` and `pydoc` targets) and replace the contents of `openvdb-website/contents/documentation/doxygen/` with the output.  [_This step should be automated, and the thousands of files it generates should preferably not be committed to the repository._]
- [ ] Open a pull request to merge the above changes into `openvdb-website/master`.  Associate the pull request with the Jira ticket created earlier.
- [ ] Post a release announcement to the [OpenVDB forum](https://groups.google.com/forum/#!forum/openvdb-forum).

- [ ] In preparation for the next release, change one or more of `OPENVDB_LIBRARY_PATCH_VERSION_NUMBER`, `OPENVDB_LIBRARY_MINOR_VERSION_NUMBER` and `OPENVDB_LIBRARY_MAJOR_VERSION_NUMBER` in `openvdb/version.h`.  Unless it is known that the next release will include API- or ABI-breaking changes, increment only the patch number to begin with (in this case, from 6.1.0 to 6.1.1).  In `doc/doxygen-config` update `PROJECT_NUMBER`, `OPENVDB_VERSION_NAME`, `OPENVDB_ABI_VERSION_NUMBER` and the `@vdbnamespace` alias to match `version.h`, and add a "Version 6.1.1 - In development" section to `CHANGES` and to `doc/changes.txt`.  Open a pull request to merge these changes into `openvdb/master`.
- [ ] Add an "OpenVDB_6.1.1" version to Jira.

END

