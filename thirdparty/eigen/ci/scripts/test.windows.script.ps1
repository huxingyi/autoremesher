# Change to build directory.
$rootdir = Get-Location
cd $EIGEN_CI_BUILDDIR

# Determine number of processors for parallel tests.
$NPROC=${Env:NUMBER_OF_PROCESSORS}

# Set target based on regex or label.
$target = ""
if (${EIGEN_CI_CTEST_REGEX}) {
  $target = "-R","${EIGEN_CI_CTEST_REGEX}"
} elseif (${EIGEN_CI_CTEST_LABEL}) {
  $target = "-L","${EIGEN_CI_CTEST_LABEL}"
}

$ctest_cmd = { ctest ${EIGEN_CI_CTEST_ARGS} --parallel ${NPROC} --output-on-failure --no-compress-output --build-noclean ${target} }

Write-Host "Running initial tests..."

& $ctest_cmd "-T test"
$exit_code = $LASTEXITCODE

if ($exit_code -eq 0) {
  Write-Host "Tests passed on the first attempt."
}
else {
  Write-Host "Initial tests failed with exit code $exit_code. Retrying up to $EIGEN_CI_CTEST_REPEAT times..."
  # TODO: figure out how to use --repeat until-pass
  for ($i = 1; $i -le $EIGEN_CI_CTEST_REPEAT; $i++) {
    & $ctest_cmd "--rerun-failed"
    $exit_code = $LASTEXITCODE

    if ($exit_code -eq 0) {
      Write-Host "Tests passed on retry."
      $exit_code = 42
      break
    }
  }
}

# Return to root directory.
cd ${rootdir}

Exit $exit_code
