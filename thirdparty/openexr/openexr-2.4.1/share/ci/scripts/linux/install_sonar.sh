#!/usr/bin/env bash

set -ex

SONAR_VERSION="$1"

mkdir _sonar
cd _sonar

wget https://sonarcloud.io/static/cpp/build-wrapper-linux-x86.zip
unzip build-wrapper-linux-x86.zip
mv build-wrapper-linux-x86 /var/opt/.
ln -s /var/opt/build-wrapper-linux-x86/build-wrapper-linux-x86-64 /usr/bin/build-wrapper-linux-x86-64
echo $(build-wrapper-linux-x86-64 --help)

wget https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-${SONAR_VERSION}-linux.zip
unzip sonar-scanner-cli-${SONAR_VERSION}-linux.zip
mv sonar-scanner-${SONAR_VERSION}-linux /var/opt/.
ln -s /var/opt/sonar-scanner-${SONAR_VERSION}-linux/bin/sonar-scanner /usr/bin/sonar-scanner
echo $(sonar-scanner --help)

cd ..
rm -rf _sonar
