*** Settings ***
Test Setup        Prepare Test
Test Teardown     Cleanup Test
Force Tags        RVD_cells    daily
Library           OperatingSystem
Library           String
Library           lib/VorpatestLibrary.py

*** Variables ***
${DATADIR}        %{VORPATEST_ROOT_DIR}${/}data${/}Small

*** Test Cases ***

icosa.obj
    [Tags]    smoke    daily_valgrind
    Run Test

joint.off
    [Tags]    smoke    daily_valgrind
    Run Test

three_holes.obj
    [Tags]    smoke    daily_valgrind
    Run Test

*** Keywords ***

Run Test
    [Documentation]    Runs compute_RVD on a single file, with RVD_cells (new API).
    ...    The name of the input file is taken from the test name.
    run command    compute_RVD    algo:predicates=exact    algo:nn_search=BNN    RVD_cells:simplify_tets=false     RVD_cells:simplify_voronoi=false     RVD_cells:simplify_boundary=false    ${DATADIR}${/}${TEST_NAME}    ${DATADIR}${/}${TEST_NAME}
    run command    compute_RVD    algo:predicates=exact    algo:nn_search=BNN    RVD_cells:simplify_tets=true     RVD_cells:simplify_voronoi=false     RVD_cells:simplify_boundary=false    ${DATADIR}${/}${TEST_NAME}    ${DATADIR}${/}${TEST_NAME}
    run command    compute_RVD    algo:predicates=exact    algo:nn_search=BNN    RVD_cells:simplify_tets=true     RVD_cells:simplify_voronoi=true     RVD_cells:simplify_boundary=false    ${DATADIR}${/}${TEST_NAME}    ${DATADIR}${/}${TEST_NAME}
    run command    compute_RVD    algo:predicates=exact    algo:nn_search=BNN    RVD_cells:simplify_tets=true     RVD_cells:simplify_voronoi=true     RVD_cells:simplify_boundary=true    ${DATADIR}${/}${TEST_NAME}    ${DATADIR}${/}${TEST_NAME}

