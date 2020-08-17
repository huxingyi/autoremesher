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
OpenNL test
    [Tags]    smoke    daily_valgrind
    Run Test


*** Keywords ***
Run Test
    [Documentation]    Runs test_NL
    run command    opennl_basic_example 


