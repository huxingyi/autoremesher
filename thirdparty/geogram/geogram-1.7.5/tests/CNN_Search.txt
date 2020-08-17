*** Settings ***
Test Setup        Prepare Test
Test Teardown     Cleanup Test
Force Tags        NNSearch    daily
Library           OperatingSystem
Library           String
Library           lib/VorpatestLibrary.py

*** Variables ***
${DATADIR}        %{VORPATEST_ROOT_DIR}${/}data${/}RVD

*** Test Cases ***
center_square.obj
    [Tags]    smoke    daily_valgrind
    Run Test    center_square.obj

913_fusee.obj
    [Tags]    weekly_valgrind
    Run Test

cranck_cse.obj
    [Tags]    weekly_valgrind
    Run Test

cylinder_brep.obj
    [Tags]    weekly_valgrind
    Run Test

cyl.meshb
    [Tags]    smoke    daily_valgrind
    Run Test

disk1.obj
    [Tags]    smoke    daily_valgrind
    Run Test

disk2.obj
    [Tags]    smoke    daily_valgrind
    Run Test

fandisk.obj
    [Tags]    smoke    daily_valgrind
    Run Test

fingertip.obj
    [Tags]    weekly_valgrind
    Run Test

grid1.obj
    [Tags]    smoke    daily_valgrind
    Run Test    grid1.obj

grid2.obj
    [Tags]    smoke    daily_valgrind
    Run Test

grid3.obj
    [Tags]    smoke    daily_valgrind
    Run Test

grid4.obj
    [Tags]    smoke    daily_valgrind
    Run Test

grid5.obj
    [Tags]    smoke    daily_valgrind
    Run Test

grid6.obj
    [Tags]    smoke    daily_valgrind
    Run Test

grid.obj
    [Tags]    smoke    daily_valgrind
    Run Test

moto.meshb
    [Tags]    weekly_valgrind
    Run Test

SaraPezzini.obj
    [Tags]    smoke    daily_valgrind
    Run Test

tordu.meshb
    [Tags]    smoke    daily_valgrind
    Run Test

two_grids.obj
    [Tags]    smoke    daily_valgrind
    Run Test

v8_engine.meshb
    [Tags]    weekly_valgrind
    Run Test

Zylkopf.meshb
    [Tags]    smoke    daily_valgrind
    Run Test

xfail_missing_input_file
    [Tags]    smoke    daily_valgrind
    Run Keyword And Expect Error    CalledProcessError: Command*returned non-zero exit status*    run command    test_nn_search

xfail_two_input_files
    [Tags]    smoke    daily_valgrind
    Run Keyword And Expect Error    CalledProcessError: Command*returned non-zero exit status*    run command    test_nn_search    ${DATADIR}${/}grid1.obj    ${DATADIR}${/}grid2.obj

*** Keywords ***
Run Test
    [Arguments]    ${input_name}=${TEST NAME}    @{options}
    [Documentation]    Runs test_nn_search on a single input file.
    ...    The name of the input file is taken from the test name.
    run command    test_nn_search    algo:predicates=exact    algo:nn_search=CNN    @{options}    ${DATADIR}${/}${input_name}

