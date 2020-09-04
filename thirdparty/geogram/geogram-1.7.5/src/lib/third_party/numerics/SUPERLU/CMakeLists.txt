
set(headers 
    supermatrix.h
    slu_Cnames.h
    slu_dcomplex.h
    slu_scomplex.h
    slu_util.h
    superlu_enum_consts.h)

set(sources
  superlu_timer.c
  util.c
  memory.c
  get_perm_c.c
  mmd.c
  sp_coletree.c
  sp_preorder.c
  sp_ienv.c
  relax_snode.c
  heap_relax_snode.c
  colamd.c
  ilu_relax_snode.c
  ilu_heap_relax_snode.c
  mark_relax.c
  mc64ad.c
  qselect.c
  input_error.c
)
set_source_files_properties(superlu_timer.c PROPERTIES COMPILE_FLAGS -O0)

if(enable_single)
  list(APPEND headers
    slu_sdefs.h
  )

  list(APPEND sources
    slacon2.c
    smach.c
    sgssv.c
    sgssvx.c
    ssp_blas2.c
    ssp_blas3.c
    sgscon.c
    slangs.c
    sgsequ.c
    slaqgs.c
    spivotgrowth.c
    sgsrfs.c
    sgstrf.c
    sgstrs.c
    scopy_to_ucol.c
    ssnode_dfs.c
    ssnode_bmod.c
    spanel_dfs.c
    spanel_bmod.c
    sreadhb.c
    sreadrb.c
    sreadtriple.c
    scolumn_dfs.c
    scolumn_bmod.c
    spivotL.c
    spruneL.c
    smemory.c
    sutil.c
    smyblas2.c
    sgsisx.c
    sgsitrf.c
    sldperm.c
    ilu_sdrop_row.c
    ilu_ssnode_dfs.c
    ilu_scolumn_dfs.c
    ilu_spanel_dfs.c
    ilu_scopy_to_ucol.c
    ilu_spivotL.c
    sdiagonal.c
  )
  set_source_files_properties(smach.c PROPERTIES COMPILE_FLAGS -O0)
endif()

if(enable_double)
  list(APPEND headers
    slu_ddefs.h
  )

  list(APPEND sources
    dlacon2.c
    dmach.c
    dgssv.c
    dgssvx.c
    dsp_blas2.c
    dsp_blas3.c
    dgscon.c
    dlangs.c
    dgsequ.c
    dlaqgs.c
    dpivotgrowth.c
    dgsrfs.c
    dgstrf.c
    dgstrs.c
    dcopy_to_ucol.c
    dsnode_dfs.c
    dsnode_bmod.c
    dpanel_dfs.c
    dpanel_bmod.c
    dreadhb.c
    dreadrb.c
    dreadtriple.c
    dcolumn_dfs.c
    dcolumn_bmod.c
    dpivotL.c
    dpruneL.c
    dmemory.c
    dutil.c
    dmyblas2.c
    dgsisx.c
    dgsitrf.c
    dldperm.c
    ilu_ddrop_row.c
    ilu_dsnode_dfs.c
    ilu_dcolumn_dfs.c
    ilu_dpanel_dfs.c
    ilu_dcopy_to_ucol.c
    ilu_dpivotL.c
    ddiagonal.c
  )
  set_source_files_properties(dmach.c PROPERTIES COMPILE_FLAGS -O0)
endif()

if(enable_complex)
  list(APPEND headers
    slu_cdefs.h
  )

  list(APPEND sources
    clacon2.c
    scsum1.c
    icmax1.c
    scomplex.c
    cgssv.c
    cgssvx.c
    csp_blas2.c
    csp_blas3.c
    cgscon.c
    clangs.c
    cgsequ.c
    claqgs.c
    cpivotgrowth.c
    cgsrfs.c
    cgstrf.c
    cgstrs.c
    ccopy_to_ucol.c
    csnode_dfs.c
    csnode_bmod.c
    cpanel_dfs.c
    cpanel_bmod.c
    creadhb.c
    creadrb.c
    creadtriple.c
    ccolumn_dfs.c
    ccolumn_bmod.c
    cpivotL.c
    cpruneL.c
    cmemory.c
    cutil.c
    cmyblas2.c
    cgsisx.c
    cgsitrf.c
    cldperm.c
    ilu_cdrop_row.c
    ilu_csnode_dfs.c
    ilu_ccolumn_dfs.c
    ilu_cpanel_dfs.c
    ilu_ccopy_to_ucol.c
    ilu_cpivotL.c
    cdiagonal.c
  )
endif()

if(enable_complex16)
  list(APPEND headers
    slu_zdefs.h
  )

  list(APPEND sources
    zlacon2.c
    dzsum1.c
    izmax1.c
    dcomplex.c
    zgssv.c
    zgssvx.c
    zsp_blas2.c
    zsp_blas3.c
    zgscon.c
    zlangs.c
    zgsequ.c
    zlaqgs.c
    zpivotgrowth.c
    zgsrfs.c
    zgstrf.c
    zgstrs.c
    zcopy_to_ucol.c
    zsnode_dfs.c
    zsnode_bmod.c
    zpanel_dfs.c
    zpanel_bmod.c
    zreadhb.c
    zreadrb.c
    zreadtriple.c
    zcolumn_dfs.c
    zcolumn_bmod.c
    zpivotL.c
    zpruneL.c
    zmemory.c
    zutil.c
    zmyblas2.c
    zgsisx.c
    zgsitrf.c
    zldperm.c
    ilu_zdrop_row.c
    ilu_zsnode_dfs.c
    ilu_zcolumn_dfs.c
    ilu_zpanel_dfs.c
    ilu_zcopy_to_ucol.c
    ilu_zpivotL.c
    zdiagonal.c
  )
endif()

add_library(superlu ${sources} ${HEADERS})

target_link_libraries(superlu ${BLAS_LIB} m)
set_target_properties(superlu PROPERTIES
  VERSION ${PROJECT_VERSION} SOVERSION ${VERSION_MAJOR}
  )

include(GNUInstallDirs)

install(TARGETS superlu
#  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${headers}
#  DESTINATION ${CMAKE_INSTALL_PREFIX}/include
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
