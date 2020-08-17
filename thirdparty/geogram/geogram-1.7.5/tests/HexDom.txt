*** Settings ***
Test Setup        Prepare Test
Test Teardown     Cleanup Test
Force Tags        Remesh    smoke    daily
Library           OperatingSystem
Library           lib/VorpatestLibrary.py

*** Variables ***
${DATADIR}        %{VORPATEST_ROOT_DIR}${/}data${/}Tets

*** Test Cases ***
joint_with_tets.meshb
    [Tags]    daily_valgrind
    Run Test   joint_with_tets.meshb    hexdom:stage=all

*** Keywords ***
Run Test
    [Arguments]    ${input_name}=${TEST NAME}    @{options}
    [Documentation]    Runs a hexdom pipeline hex dominant test
    ...    The name of the input file is taken from the test name.
    run command    hexdom_pipeline    hexdom:tets=${DATADIR}${/}${input_name}    @{options}
