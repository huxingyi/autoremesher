*** Settings ***
Test Setup        Prepare Test
Test Teardown     Cleanup Test
Force Tags        ConvexCell    smoke    daily    daily_valgrind
Test Timeout      15 minutes
Library           OperatingSystem
Library           String
Library           lib/VorpatestLibrary.py

*** Variables ***
${DATADIR}        %{VORPATEST_ROOT_DIR}${/}data${/}ConvexCell

*** Test Cases ***
sphere_10.xyz
    Run Test

sphere_100.xyz
    Run Test

sphere_1000.xyz
    Run Test

cone_10.xyz
    Run Test

cone_100.xyz
    Run Test

cone_1000.xyz
    Run Test

xfail missing input file
    Run Keyword And Expect Error    CalledProcessError: Command*returned non-zero exit status*    run command    test_convex_cell    shape=file

*** Keywords ***
Run Test
    [Arguments]    ${input_name}=${TEST NAME}    @{options}
    [Documentation]    Runs test_convex_cell on a single input file.
    ...    The name of the input file is taken from the test name.
    run command    test_convex_cell    shape=file    @{options}    ${DATADIR}${/}${input_name}
    run command    test_convex_cell    shape=file    algo:predicates=exact    @{options}    ${DATADIR}${/}${input_name}
    run command    test_convex_cell    shape=file    algo:predicates=exact    nb_clip_times=10    @{options}    ${DATADIR}${/}${input_name}
