*** Settings ***
Test Setup        Prepare Test
Test Teardown     Cleanup Test
Force Tags        Remesh    smoke    daily
Library           OperatingSystem
Library           lib/VorpatestLibrary.py

*** Variables ***
${DATADIR}        %{VORPATEST_ROOT_DIR}${/}data${/}Small

*** Test Cases ***
icosa.obj no remesh
    [Tags]    daily_valgrind
    Run Test    icosa.obj    remesh\=false

icosa.obj
    [Tags]    daily_valgrind
    Run Test

mask3kf.obj
    [Tags]    daily_valgrind
    Run Test

S2.obj
    [Tags]    daily_valgrind
    Run Test

three_holes.obj
    [Tags]    daily_valgrind
    Run Test

cube.obj (profile=cad)
    [Tags]    daily_valgrind
    Run Test    cube.obj    profile=cad

joint.off (profile=cad)
    [Tags]    daily_valgrind
    Run Test    joint.off    profile=cad

mask.off (gradation=1)
    [Tags]    weekly_valgrind
    Run Test    mask.off    gradation=1   sys:multithread=false

test_isect.obj (profile=convert, isect=true)
    [Tags]    daily_valgrind
    Run Test    test_isect.obj    profile=convert    post=true    post:isect=true

three_holes.obj (profile=tet)
    [Tags]    daily_valgrind
    Run Test    three_holes.obj     profile=tet


*** Keywords ***
Run Test
    [Arguments]    ${input_name}=${TEST NAME}    @{options}
    [Documentation]    Runs a vorpaline test
    ...    The name of the input file is taken from the test name.
    run vorpaline    ${DATADIR}${/}${input_name}    pts=5000    @{options}
    run vorpastat

Run Fail Test
    [Arguments]    @{args}
    [Documentation]    Runs a vorpaline test that is expected to fail
    Run Keyword And Expect Error    CalledProcessError: Command*returned non-zero exit status*    run vorpaline    @{args}
