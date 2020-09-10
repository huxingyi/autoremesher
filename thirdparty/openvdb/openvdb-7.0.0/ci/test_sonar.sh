#!/usr/bin/env bash

set -ex

SONAR_VERSION=3.3.0.1492

wget -q https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-${SONAR_VERSION}-linux.zip
unzip sonar-scanner-cli-${SONAR_VERSION}-linux.zip

mkdir coverage
cd coverage

for g in $(find ../build -name "*.gcno" -type f); do
    gcov -p -l -o $(dirname "$g") $(echo "$g" | sed -e 's/\/build\//\//' -e 's/\.gcno/\.cc/' -e 's/\/CMakeFiles.*\.dir\//\//')
done

cd ..

sonar-scanner-${SONAR_VERSION}-linux/bin/sonar-scanner -X \
    -Dsonar.projectKey=openvdb \
    -Dsonar.links.homepage=https://www.openvdb.org/ \
    -Dsonar.links.scm=https://github.com/AcademySoftwareFoundation/openvdb \
    -Dsonar.links.issue=https://jira.aswf.io/projects/OVDB \
    -Dsonar.sources=openvdb \
    -Dsonar.exclusions=openvdb/cmd/**,openvdb/unittest/**,openvdb/viewer/**,openvdb/python/** \
    -Dsonar.binaries=build/openvdb/unittest/vdb_test \
    -Dsonar.tests=openvdb/unittest \
    -Dsonar.sourceEncoding=UTF-8 \
    -Dsonar.organization=danrbailey-github \
    -Dsonar.cfamily.build-wrapper-output=build/bw_output \
    -Dsonar.cfamily.gcov.reportsPath=coverage \
    -Dsonar.host.url=https://sonarcloud.io \
    -Dsonar.login=$SONAR_TOKEN
