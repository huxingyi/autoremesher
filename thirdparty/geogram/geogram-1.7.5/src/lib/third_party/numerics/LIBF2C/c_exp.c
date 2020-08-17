#include "f2c.h"

#ifdef KR_headers
extern double exp(), cos(), sin();

 VOID c_exp(r, z) complex *r, *z;
#else
#undef abs
#include "math.h"
#ifdef complex /* [BL] MSVC defines a complex macro that collides with f2c's complex */
#undef complex
#endif

#ifdef __cplusplus
extern "C" {
#endif

void c_exp(complex *r, complex *z)
#endif
{
	double expx, zi = z->i;

	expx = exp(z->r);
	r->r = expx * cos(zi);
	r->i = expx * sin(zi);
	}
#ifdef __cplusplus
}
#endif
