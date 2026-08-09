#include "main.h"

#ifdef EIGEN_EXCEPTIONS
#include <exception>  // std::exception
#endif

#include <Eigen/src/Core/util/MaxSizeVector.h>

struct Foo {
  static Index object_count;
  static Index object_limit;
  EIGEN_ALIGN_TO_BOUNDARY(128) int dummy;

  Foo(int x = 0) : dummy(x) {
#ifdef EIGEN_EXCEPTIONS
    // TODO: Is this the correct way to handle this?
    if (Foo::object_count > Foo::object_limit) {
      std::cout << "\nThrow!\n";
      throw Foo::Fail();
    }
#endif
    std::cout << '+';
    ++Foo::object_count;
    eigen_assert((std::uintptr_t(this) & (127)) == 0);
  }
  Foo(const Foo&) {
    std::cout << 'c';
    ++Foo::object_count;
    eigen_assert((std::uintptr_t(this) & (127)) == 0);
  }

  ~Foo() {
    std::cout << '~';
    --Foo::object_count;
  }
#ifdef EIGEN_EXCEPTIONS
  class Fail : public std::exception {};
#endif
};

Index Foo::object_count = 0;
Index Foo::object_limit = 0;

EIGEN_DECLARE_TEST(maxsizevector) {
  typedef MaxSizeVector<Foo> VectorX;
  Foo::object_count = 0;
  for (int r = 0; r < g_repeat; r++) {
    Index rows = internal::random<Index>(3, 30);
    Foo::object_limit = internal::random<Index>(0, rows - 2);
    std::cout << "object_limit = " << Foo::object_limit << std::endl;
#ifdef EIGEN_EXCEPTIONS
    bool exception_raised = false;
    try {
      std::cout << "\nVectorX m(" << rows << ");\n";
      VectorX vect(rows);
      for (int i = 0; i < rows; ++i) vect.push_back(Foo());
      VERIFY(false);  // not reached if exceptions are enabled
    } catch (const Foo::Fail&) {
      exception_raised = true;
    }
    VERIFY(exception_raised);
#endif
    VERIFY_IS_EQUAL(Index(0), Foo::object_count);

    {
      Foo::object_limit = rows + 1;
      VectorX vect2(rows, Foo());
      VERIFY_IS_EQUAL(Foo::object_count, rows);
    }
    VERIFY_IS_EQUAL(Index(0), Foo::object_count);
    std::cout << '\n';
  }
}
