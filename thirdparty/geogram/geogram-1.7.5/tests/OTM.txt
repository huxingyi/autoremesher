*** Settings ***
Test Setup        Prepare Test
Test Teardown     Cleanup Test
Force Tags        OTM    smoke    daily
Library           OperatingSystem
Library           String
Library           lib/VorpatestLibrary.py

*** Variables ***
${DATADIR}        %{VORPATEST_ROOT_DIR}${/}data${/}Tets

*** Test Cases ***
PetitCubeTets-PetiteSphereTets
    [Tags]    weekly_valgrind
    Run Test    PetitCubeTets.meshb    PetiteSphereTets.meshb

*** Keywords ***
Run Test
    [Arguments]    ${mesh1}    ${mesh2}    @{options}
    [Documentation]    Runs compute_OTM on 2 input meshes
    run command    compute_OTM    @{options}    ${DATADIR}${/}${mesh1}    ${DATADIR}${/}${mesh2}
