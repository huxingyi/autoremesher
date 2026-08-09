# Change to build directory.
$rootdir = Get-Location
cd ${EIGEN_CI_BUILDDIR}

# Determine the appropriate test tag and results file.
$TEST_TAG = Get-Content Testing\TAG | select -first 1

# PowerShell equivalent to xsltproc:
$XSL_FILE = Resolve-Path "..\ci\CTest2JUnit.xsl"
$INPUT_FILE = Resolve-Path Testing\$TEST_TAG\Test.xml
$OUTPUT_FILE = Join-Path -Path $pwd -ChildPath JUnitTestResults_$CI_JOB_ID.xml
$xslt = New-Object System.Xml.Xsl.XslCompiledTransform;
$xslt.Load($XSL_FILE)
$xslt.Transform($INPUT_FILE,$OUTPUT_FILE)

# Return to root directory.
cd ${rootdir}
