build_example(
  NAME dcqcn-evaluation-burst
  SOURCE_FILES 
    dcqcn-evaluation-burst.cc
    cdf.c
  HEADER_FILES
    cdf.h
  LIBRARIES_TO_LINK
    ${libpoint-to-point}
    ${libtraffic-control}
    ${libinternet}
    ${libapplications}
)
build_example(
  NAME dcqcn-evaluation-fairness
  SOURCE_FILES 
    dcqcn-evaluation-fairness.cc
    cdf.c
  HEADER_FILES
    cdf.h
  LIBRARIES_TO_LINK
    ${libpoint-to-point}
    ${libtraffic-control}
    ${libinternet}
    ${libapplications}
)
build_example(
  NAME dcqcn-evaluation-workload
  SOURCE_FILES 
    dcqcn-evaluation-workload.cc
    cdf.c
  HEADER_FILES
    cdf.h
  LIBRARIES_TO_LINK
    ${libpoint-to-point}
    ${libtraffic-control}
    ${libinternet}
    ${libapplications}
)
set_source_files_properties(cdf.c PROPERTIES SKIP_PRECOMPILE_HEADERS ON)