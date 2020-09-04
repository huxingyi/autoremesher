*** Settings ***
Test Setup        Prepare Test
Test Teardown     Cleanup Test
Force Tags        Remesh    smoke    daily
Library           OperatingSystem
Library           lib/VorpatestLibrary.py

*** Variables ***
${DATADIR}        %{VORPATEST_ROOT_DIR}${/}data${/}Small

*** Test Cases ***

mask.off (profile=reconstruct)
    [Tags]    daily_valgrind
    Run Test    mask.off    profile=reconstruct

mask.off (profile=reconstruct, Psmooth=3)
    [Tags]    daily_valgrind
    Run Test    mask.off    profile=reconstruct    co3ne:Psmooth=3

mask.off (profile=reconstruct, Psmooth=3, algo=Poisson)
    [Tags]    daily_valgrind
    Run Test    mask.off    profile=reconstruct    co3ne:Psmooth=3    algo:reconstruct=Poisson

*** Keywords ***
Run Test
    [Arguments]    ${input_name}=${TEST NAME}    @{options}
    [Documentation]    Runs a vorpaline test
    ...    The name of the input file is taken from the test name.
    run vorpaline    ${DATADIR}${/}${input_name}    @{options}

Run Fail Test
    [Arguments]    @{args}
    [Documentation]    Runs a vorpaline test that is expected to fail
    Run Keyword And Expect Error    CalledProcessError: Command*returned non-zero exit status*    run vorpaline    @{args}
