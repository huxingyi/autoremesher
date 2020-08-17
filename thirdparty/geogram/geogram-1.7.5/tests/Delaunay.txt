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
    Run Test    cube.obj    dbg:delaunay_verbose=true  

joint.off
    [Tags]    daily_valgrind
    Run Test    joint.off    dbg:delaunay_verbose=true  

fandisk.ply
    [Tags]    weekly_valgrind
    Run Test

cube.obj (hull)
    [Tags]    daily_valgrind
    Run Test    cube.obj    convex_hull=true    dbg:delaunay_verbose=true  

joint.off (hull)
    [Tags]    daily_valgrind
    Run Test    joint.off    convex_hull=true    dbg:delaunay_verbose=true  

fandisk.ply (hull)
    [Tags]    weekly_valgrind
    Run Test    fandisk.ply    convex_hull=true

cube.obj (2d)
    [Tags]    daily_valgrind
    Run Test    cube.obj    dimension=2

joint.off (2d)
    [Tags]    daily_valgrind
    Run Test    joint.off    dimension=2

fandisk.ply (2d)
    [Tags]    weekly_valgrind
    Run Test    fandisk.ply    dimension=2

joint.off (2d triangle)
    [Tags]    daily_valgrind
    Run Test    joint.off    dimension=2   algo:delaunay=triangle


*** Keywords ***
Run Test
    [Arguments]    ${input_name}=${TEST NAME}    @{options}
    [Documentation]    Runs compute_delaunay on a single input file.
    ...    The name of the input file is taken from the test name.
    run command    compute_delaunay  ${DATADIR}${/}${input_name}  dbg:delaunay_benchmark=true   @{options}    
