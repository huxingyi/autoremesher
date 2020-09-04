*** Settings ***
Test Setup        Prepare Test
Test Teardown     Cleanup Test
Force Tags        RVD    daily
Library           OperatingSystem
Library           String
Library           lib/VorpatestLibrary.py

*** Variables ***
${DATADIR}        %{VORPATEST_ROOT_DIR}${/}data${/}RVD

*** Test Cases ***
center_square.obj
    [Tags]    smoke    daily_valgrind
    Run Test

center_square.obj with two_grids.obj
    [Tags]    smoke    daily_valgrind
    Run Test With 2 Files    center_square.obj    two_grids.obj

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
    Run Test

grid1_dual.obj
    [Tags]    smoke    daily_valgrind
    Run Test With Dual

grid2.obj
    [Tags]    smoke    daily_valgrind
    Run Test

grid2_dual.obj
    [Tags]    smoke    daily_valgrind
    Run Test With Dual

grid3.obj
    [Tags]    smoke    daily_valgrind
    Run Test

grid3_dual.obj
    [Tags]    smoke    daily_valgrind
    Run Test With Dual

grid4.obj
    [Tags]    smoke    daily_valgrind
    Run Test

grid5.obj
    [Tags]    smoke    daily_valgrind
    Run Test

grid5_dual.obj
    [Tags]    smoke    daily_valgrind
    Run Test With Dual

grid6.obj
    [Tags]    smoke    daily_valgrind
    Run Test

grid6_dual.obj
    [Tags]    smoke    daily_valgrind
    Run Test With Dual

grid.obj
    [Tags]    smoke    daily_valgrind
    Run Test

grid_dual.obj
    [Tags]    smoke    daily_valgrind
    Run Test With Dual

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
    Run Keyword And Expect Error    CalledProcessError: Command*returned non-zero exit status*    run command    compute_RVD

xfail_bad_results
    [Tags]    smoke    daily_valgrind
    Run Keyword And Expect Error    CalledProcessError: Command*returned non-zero exit status*    run command    compute_RVD    algo:predicates=fast    algo:nn_search=BNN    ${DATADIR}${/}grid4.obj

*** Keywords ***
Run Test
    [Documentation]    Runs compute_RVD on a single input file.
    ...    The name of the input file is taken from the test name.
    run command    compute_RVD    algo:predicates=exact    algo:nn_search=BNN    ${DATADIR}${/}${TEST_NAME}

Run Test With Dual
    [Documentation]    Runs compute_RVD with dual input files
    ...    The name of the dual input file is taken from the test name.
    ...    The name of the original input file is computed from the test name: input_dual.ext -> input.ext
    ${input_name} =    Replace String    ${TEST_NAME}    _dual.    .
    run command    compute_RVD    algo:predicates=exact    algo:nn_search=BNN    ${DATADIR}${/}${input_name}    ${DATADIR}${/}${TEST_NAME}

Run Test With 2 Files
    [Arguments]    ${input_name1}    ${input_name2}
    [Documentation]    Runs compute_RVD on 2 input files.
    run command    compute_RVD    algo:predicates=exact    algo:nn_search=BNN    ${DATADIR}${/}${input_name1}    ${DATADIR}${/}${input_name2}
