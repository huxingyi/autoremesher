echo "Running ${CI_JOB_NAME}"

# Print configuration variables.
Get-Variable EIGEN* | Format-Table -Wrap
Get-Variable CMAKE* | Format-Table -Wrap

# Run a custom before-script command.
if ("${EIGEN_CI_BEFORE_SCRIPT}") { Invoke-Expression -Command "${EIGEN_CI_BEFORE_SCRIPT}" }
