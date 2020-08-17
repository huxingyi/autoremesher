#ifndef H_THIRD_PARTY_NUMERICS_LINKAGE_H
#define H_THIRD_PARTY_NUMERICS_LINKAGE_H

#include <geogram/api/defs.h>

#ifdef geogram_num_3rdparty_EXPORTS
#define NUMERICS_API GEO_EXPORT
#else
#define NUMERICS_API GEO_IMPORT
#endif

#endif
