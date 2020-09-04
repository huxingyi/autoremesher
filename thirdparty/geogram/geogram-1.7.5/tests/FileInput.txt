*** Settings ***
Test Setup        Prepare Test
Test Teardown     Cleanup Test
Force Tags        FileInput    smoke    daily    daily_valgrind
Library           OperatingSystem
Library           lib/VorpatestLibrary.py

*** Variables ***
${DATADIR}        %{VORPATEST_ROOT_DIR}${/}data${/}Small

*** Test Cases ***
xfail unknown file extension
    Run Fail Test    ${DATADIR}${/}dummy.x

xfail file not found
    Run Fail Test    ${DATADIR}${/}dummy.obj

xfail corrupted.obj invalid integer value
    Run Fail Test    ${DATADIR}${/}corrupted_invalid_int.obj

xfail corrupted.obj invalid floating point value
    Run Fail Test    ${DATADIR}${/}corrupted_invalid_double.obj

xfail corrupted.obj invalid facet vertex index
    Run Fail Test    ${DATADIR}${/}corrupted_invalid_facet_vertex.obj

xfail corrupted.off invalid integer value
    Run Fail Test    ${DATADIR}${/}corrupted_invalid_int.off

xfail corrupted.off invalid floating point value
    Run Fail Test    ${DATADIR}${/}corrupted_invalid_double.off

xfail corrupted.off invalid facet vertex count
    Run Fail Test    ${DATADIR}${/}corrupted_invalid_facet_vertex_count.off

xfail corrupted.off invalid facet vertex
    Run Fail Test    ${DATADIR}${/}corrupted_invalid_facet_vertex.off

xfail corrupted.off invalid vertex count
    Run Fail Test    ${DATADIR}${/}corrupted_invalid_vertex_count.off

xfail truncated.off
    Run Fail Test    ${DATADIR}${/}truncated.off

xfail corrupted invalid face count.meshb
    Run Fail Test    ${DATADIR}${/}corrupted_invalid_face_count.meshb

xfail corrupted invalid facet vertex.meshb
    Run Fail Test    ${DATADIR}${/}corrupted_invalid_facet_vertex.meshb

xfail truncated.meshb
    Run Fail Test    ${DATADIR}${/}truncated.meshb

xfail corrupted invalid face count.ply
    Run Fail Test    ${DATADIR}${/}corrupted_invalid_face_count.ply

xfail corrupted invalid vertex count.ply
    Run Fail Test    ${DATADIR}${/}corrupted_invalid_vertex_count.ply

xfail truncated.ply
    Run Fail Test    ${DATADIR}${/}truncated.ply

xfail corrupted invalid triangle count.stl
    Run Fail Test    ${DATADIR}${/}corrupted_invalid_triangle_count.stl

xfail truncated.stl
    Run Fail Test    ${DATADIR}${/}truncated.stl

## TODO: Add positive tests to measure load performances

*** Keywords ***
Run Test
    [Arguments]    ${input_name}=${TEST NAME}    @{options}
    [Documentation]    Runs a vorpaline test
    ...    The name of the input file is taken from the test name.
    run vorpaline    ${DATADIR}${/}${input_name}    @{options}    remesh=false

Run Fail Test
    [Arguments]    @{args}
    [Documentation]    Runs a vorpaline test that is expected to fail
    Run Keyword And Expect Error    CalledProcessError: Command*returned non-zero exit status*    run vorpaline    @{args}    remesh=false
