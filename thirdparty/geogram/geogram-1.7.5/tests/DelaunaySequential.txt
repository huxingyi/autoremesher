*** Settings ***
Test Setup        Prepare Test
Test Teardown     Cleanup Test
Force Tags        Delaunay    smoke    daily
Library           OperatingSystem
Library           String
Library           lib/VorpatestLibrary.py

*** Variables ***
${DATADIR}        %{VORPATEST_ROOT_DIR}${/}data${/}Small

*** Test Cases ***
cube.obj
    [Tags]    daily_valgrind
    Run Test

joint.off
    [Tags]    daily_valgrind
    Run Test

*** Keywords ***
Run Test
    [Arguments]    ${input_name}=${TEST NAME}    @{options}
    [Documentation]    Runs compute_delaunay on a single input file.
    ...    The name of the input file is taken from the test name.
    run command    compute_delaunay  ${DATADIR}${/}${input_name}   algo:delaunay=BDEL   @{options}
