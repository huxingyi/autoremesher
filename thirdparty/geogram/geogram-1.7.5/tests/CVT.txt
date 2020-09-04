*** Settings ***
Test Setup        Prepare Test
Test Teardown     Cleanup Test
Force Tags        Remesh    smoke    daily
Library           OperatingSystem
Library           lib/VorpatestLibrary.py

*** Variables ***
${DATADIR}        %{VORPATEST_ROOT_DIR}${/}data${/}HighD

*** Test Cases ***
small_cube.tet8 (dim8)
    [Tags]    daily_valgrind
    Run Test   small_cube.tet8  profile=poly  pts=300  poly:simplify=tets  poly:embedding_dim=8

small_cube.tet8 (dim6)
    [Tags]    daily_valgrind
    Run Test   small_cube.tet8  profile=poly  pts=300  poly:simplify=tets  poly:embedding_dim=6

small_cube.tet8 (dim4)
    [Tags]    daily_valgrind
    Run Test   small_cube.tet8  profile=poly  pts=300  poly:simplify=tets  poly:embedding_dim=4

small_cube.tet8 (dim3)
    [Tags]    daily_valgrind
    Run Test   small_cube.tet8  profile=poly  pts=300  poly:simplify=tets  poly:embedding_dim=3



*** Keywords ***
Run Test
    [Arguments]    ${input_name}=${TEST NAME}    @{options}
    [Documentation]    Runs a vorpaline high dim CVT test
    ...    The name of the input file is taken from the test name.
    run vorpaline    ${DATADIR}${/}${input_name}  @{options}
