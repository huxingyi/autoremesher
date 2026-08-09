// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2017 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <vector>

#include "main.h"

using Eigen::placeholders::all;
using Eigen::placeholders::last;
using Eigen::placeholders::lastN;
using Eigen::placeholders::lastp1;
#include <array>

namespace test {
typedef std::pair<Index, Index> IndexPair;
}

int encode(Index i, Index j) { return int(i * 100 + j); }

test::IndexPair decode(Index ij) { return test::IndexPair(ij / 100, ij % 100); }

template <typename T>
bool match(const T& xpr, std::string ref, std::string str_xpr = "") {
  EIGEN_UNUSED_VARIABLE(str_xpr);
  std::stringstream str;
  str << xpr;
  if (!(str.str() == ref)) std::cout << str_xpr << "\n" << xpr << "\n\n";
  return str.str() == ref;
}

#define MATCH(X, R) match(X, R, #X)

template <typename T1, typename T2>
std::enable_if_t<internal::is_same<T1, T2>::value, bool> is_same_eq(const T1& a, const T2& b) {
  return (a == b).all();
}

template <typename T1, typename T2>
bool is_same_seq(const T1& a, const T2& b) {
  bool ok = a.first() == b.first() && a.size() == b.size() && Index(a.incrObject()) == Index(b.incrObject());
  ;
  if (!ok) {
    std::cerr << "seqN(" << a.first() << ", " << a.size() << ", " << Index(a.incrObject()) << ") != ";
    std::cerr << "seqN(" << b.first() << ", " << b.size() << ", " << Index(b.incrObject()) << ")\n";
  }
  return ok;
}

template <typename T1, typename T2>
std::enable_if_t<internal::is_same<T1, T2>::value, bool> is_same_seq_type(const T1& a, const T2& b) {
  return is_same_seq(a, b);
}

#define VERIFY_EQ_INT(A, B) VERIFY_IS_APPROX(int(A), int(B))

// C++03 does not allow local or unnamed enums as index
enum DummyEnum { XX = 0, YY = 1 };

void check_indexed_view() {
  Index n = 10;

  ArrayXd a = ArrayXd::LinSpaced(n, 0, n - 1);
  Array<double, 1, Dynamic> b = a.transpose();

  ArrayXXi A = ArrayXXi::NullaryExpr(n, n, std::ref(encode));

  for (Index i = 0; i < n; ++i)
    for (Index j = 0; j < n; ++j) VERIFY(decode(A(i, j)) == test::IndexPair(i, j));

  Array4i eii(4);
  eii << 3, 1, 6, 5;
  std::vector<int> veci(4);
  Map<ArrayXi>(veci.data(), 4) = eii;

  VERIFY(MATCH(A(3, seq(9, 3, -1)), "309  308  307  306  305  304  303"));

  VERIFY(MATCH(A(seqN(2, 5), seq(9, 3, -1)),
               "209  208  207  206  205  204  203\n"
               "309  308  307  306  305  304  303\n"
               "409  408  407  406  405  404  403\n"
               "509  508  507  506  505  504  503\n"
               "609  608  607  606  605  604  603"));

  VERIFY(MATCH(A(seqN(2, 5), 5),
               "205\n"
               "305\n"
               "405\n"
               "505\n"
               "605"));

  VERIFY(MATCH(A(seqN(last, 5, -1), seq(2, last)),
               "902  903  904  905  906  907  908  909\n"
               "802  803  804  805  806  807  808  809\n"
               "702  703  704  705  706  707  708  709\n"
               "602  603  604  605  606  607  608  609\n"
               "502  503  504  505  506  507  508  509"));

  VERIFY(MATCH(A(eii, veci),
               "303  301  306  305\n"
               "103  101  106  105\n"
               "603  601  606  605\n"
               "503  501  506  505"));

  VERIFY(MATCH(A(eii, all),
               "300  301  302  303  304  305  306  307  308  309\n"
               "100  101  102  103  104  105  106  107  108  109\n"
               "600  601  602  603  604  605  606  607  608  609\n"
               "500  501  502  503  504  505  506  507  508  509"));

  // take row number 3, and repeat it 5 times
  VERIFY(MATCH(A(seqN(3, 5, 0), all),
               "300  301  302  303  304  305  306  307  308  309\n"
               "300  301  302  303  304  305  306  307  308  309\n"
               "300  301  302  303  304  305  306  307  308  309\n"
               "300  301  302  303  304  305  306  307  308  309\n"
               "300  301  302  303  304  305  306  307  308  309"));

  VERIFY(MATCH(a(seqN(3, 3), 0), "3\n4\n5"));
  VERIFY(MATCH(a(seq(3, 5)), "3\n4\n5"));
  VERIFY(MATCH(a(seqN(3, 3, 1)), "3\n4\n5"));
  VERIFY(MATCH(a(seqN(5, 3, -1)), "5\n4\n3"));

  VERIFY(MATCH(b(0, seqN(3, 3)), "3  4  5"));
  VERIFY(MATCH(b(seq(3, 5)), "3  4  5"));
  VERIFY(MATCH(b(seqN(3, 3, 1)), "3  4  5"));
  VERIFY(MATCH(b(seqN(5, 3, -1)), "5  4  3"));

  VERIFY(MATCH(b(all), "0  1  2  3  4  5  6  7  8  9"));
  VERIFY(MATCH(b(eii), "3  1  6  5"));

  Array44i B;
  B.setRandom();
  VERIFY((A(seqN(2, 5), 5)).ColsAtCompileTime == 1);
  VERIFY((A(seqN(2, 5), 5)).RowsAtCompileTime == Dynamic);
  VERIFY_EQ_INT((A(seqN(2, 5), 5)).InnerStrideAtCompileTime, A.InnerStrideAtCompileTime);
  VERIFY_EQ_INT((A(seqN(2, 5), 5)).OuterStrideAtCompileTime, A.col(5).OuterStrideAtCompileTime);

  VERIFY_EQ_INT((A(5, seqN(2, 5))).InnerStrideAtCompileTime, A.row(5).InnerStrideAtCompileTime);
  VERIFY_EQ_INT((A(5, seqN(2, 5))).OuterStrideAtCompileTime, A.row(5).OuterStrideAtCompileTime);
  VERIFY_EQ_INT((B(1, seqN(1, 2))).InnerStrideAtCompileTime, B.row(1).InnerStrideAtCompileTime);
  VERIFY_EQ_INT((B(1, seqN(1, 2))).OuterStrideAtCompileTime, B.row(1).OuterStrideAtCompileTime);

  VERIFY_EQ_INT((A(seqN(2, 5), seq(1, 3))).InnerStrideAtCompileTime, A.InnerStrideAtCompileTime);
  VERIFY_EQ_INT((A(seqN(2, 5), seq(1, 3))).OuterStrideAtCompileTime, A.OuterStrideAtCompileTime);
  VERIFY_EQ_INT((B(seqN(1, 2), seq(1, 3))).InnerStrideAtCompileTime, B.InnerStrideAtCompileTime);
  VERIFY_EQ_INT((B(seqN(1, 2), seq(1, 3))).OuterStrideAtCompileTime, B.OuterStrideAtCompileTime);
  VERIFY_EQ_INT((A(seqN(2, 5, 2), seq(1, 3, 2))).InnerStrideAtCompileTime, Dynamic);
  VERIFY_EQ_INT((A(seqN(2, 5, 2), seq(1, 3, 2))).OuterStrideAtCompileTime, Dynamic);
  VERIFY_EQ_INT((A(seqN(2, 5, fix<2>), seq(1, 3, fix<3>))).InnerStrideAtCompileTime, 2);
  VERIFY_EQ_INT((A(seqN(2, 5, fix<2>), seq(1, 3, fix<3>))).OuterStrideAtCompileTime, Dynamic);
  VERIFY_EQ_INT((B(seqN(1, 2, fix<2>), seq(1, 3, fix<3>))).InnerStrideAtCompileTime, 2);
  VERIFY_EQ_INT((B(seqN(1, 2, fix<2>), seq(1, 3, fix<3>))).OuterStrideAtCompileTime, 3 * 4);

  VERIFY_EQ_INT((A(seqN(2, fix<5>), seqN(1, fix<3>))).RowsAtCompileTime, 5);
  VERIFY_EQ_INT((A(seqN(2, fix<5>), seqN(1, fix<3>))).ColsAtCompileTime, 3);
  VERIFY_EQ_INT((A(seqN(2, fix<5>(5)), seqN(1, fix<3>(3)))).RowsAtCompileTime, 5);
  VERIFY_EQ_INT((A(seqN(2, fix<5>(5)), seqN(1, fix<3>(3)))).ColsAtCompileTime, 3);
  VERIFY_EQ_INT((A(seqN(2, fix<Dynamic>(5)), seqN(1, fix<Dynamic>(3)))).RowsAtCompileTime, Dynamic);
  VERIFY_EQ_INT((A(seqN(2, fix<Dynamic>(5)), seqN(1, fix<Dynamic>(3)))).ColsAtCompileTime, Dynamic);
  VERIFY_EQ_INT((A(seqN(2, fix<Dynamic>(5)), seqN(1, fix<Dynamic>(3)))).rows(), 5);
  VERIFY_EQ_INT((A(seqN(2, fix<Dynamic>(5)), seqN(1, fix<Dynamic>(3)))).cols(), 3);

  VERIFY(is_same_seq_type(seqN(2, 5, fix<-1>), seqN(2, 5, fix<-1>(-1))));
  VERIFY(is_same_seq_type(seqN(2, 5), seqN(2, 5, fix<1>(1))));
  VERIFY(is_same_seq_type(seqN(2, 5, 3), seqN(2, 5, fix<DynamicIndex>(3))));
  VERIFY(is_same_seq_type(seq(2, 7, fix<3>), seqN(2, 2, fix<3>)));
  VERIFY(is_same_seq_type(seqN(2, fix<Dynamic>(5), 3), seqN(2, 5, fix<DynamicIndex>(3))));
  VERIFY(is_same_seq_type(seqN(2, fix<5>(5), fix<-2>), seqN(2, fix<5>, fix<-2>())));

  VERIFY(is_same_seq_type(seq(2, fix<5>), seqN(2, 4)));
  VERIFY(is_same_seq_type(seq(fix<2>, fix<5>), seqN(fix<2>, fix<4>)));
  VERIFY(is_same_seq(seqN(2, std::integral_constant<int, 5>(), std::integral_constant<int, -2>()),
                     seqN(2, fix<5>, fix<-2>())));
  VERIFY(is_same_seq(
      seq(std::integral_constant<int, 1>(), std::integral_constant<int, 5>(), std::integral_constant<int, 2>()),
      seq(fix<1>, fix<5>, fix<2>())));
  VERIFY(is_same_seq_type(seqN(2, std::integral_constant<int, 5>(), std::integral_constant<int, -2>()),
                          seqN(2, fix<5>, fix<-2>())));
  VERIFY(is_same_seq_type(
      seq(std::integral_constant<int, 1>(), std::integral_constant<int, 5>(), std::integral_constant<int, 2>()),
      seq(fix<1>, fix<5>, fix<2>())));

  VERIFY(is_same_seq_type(seqN(2, std::integral_constant<int, 5>()), seqN(2, fix<5>)));
  VERIFY(
      is_same_seq_type(seq(std::integral_constant<int, 1>(), std::integral_constant<int, 5>()), seq(fix<1>, fix<5>)));

  VERIFY((A(seqN(2, fix<5>), 5)).RowsAtCompileTime == 5);
  VERIFY((A(4, all)).ColsAtCompileTime == Dynamic);
  VERIFY((A(4, all)).RowsAtCompileTime == 1);
  VERIFY((B(1, all)).ColsAtCompileTime == 4);
  VERIFY((B(1, all)).RowsAtCompileTime == 1);
  VERIFY((B(all, 1)).ColsAtCompileTime == 1);
  VERIFY((B(all, 1)).RowsAtCompileTime == 4);

  VERIFY(int((A(all, eii)).ColsAtCompileTime) == int(eii.SizeAtCompileTime));
  VERIFY_EQ_INT((A(eii, eii)).Flags & DirectAccessBit, (unsigned int)(0));
  VERIFY_EQ_INT((A(eii, eii)).InnerStrideAtCompileTime, 0);
  VERIFY_EQ_INT((A(eii, eii)).OuterStrideAtCompileTime, 0);

  VERIFY_IS_APPROX(A(seq(n - 1, 2, -2), seqN(n - 1 - 6, 3, -1)), A(seq(last, 2, fix<-2>), seqN(last - 6, 3, fix<-1>)));

  VERIFY_IS_APPROX(A(seq(n - 1, 2, -2), seqN(n - 1 - 6, 4)), A(seq(last, 2, -2), seqN(last - 6, 4)));
  VERIFY_IS_APPROX(A(seq(n - 1 - 6, n - 1 - 2), seqN(n - 1 - 6, 4)),
                   A(seq(last - 6, last - 2), seqN(6 + last - 6 - 6, 4)));
  VERIFY_IS_APPROX(A(seq((n - 1) / 2, (n) / 2 + 3), seqN(2, 4)),
                   A(seq(last / 2, (last + 1) / 2 + 3), seqN(last + 2 - last, 4)));
  VERIFY_IS_APPROX(A(seq(n - 2, 2, -2), seqN(n - 8, 4)), A(seq(lastp1 - 2, 2, -2), seqN(lastp1 - 8, 4)));

  // Check all combinations of seq:
  VERIFY_IS_APPROX(A(seq(1, n - 1 - 2, 2), seq(1, n - 1 - 2, 2)), A(seq(1, last - 2, 2), seq(1, last - 2, fix<2>)));
  VERIFY_IS_APPROX(A(seq(n - 1 - 5, n - 1 - 2, 2), seq(n - 1 - 5, n - 1 - 2, 2)),
                   A(seq(last - 5, last - 2, 2), seq(last - 5, last - 2, fix<2>)));
  VERIFY_IS_APPROX(A(seq(n - 1 - 5, 7, 2), seq(n - 1 - 5, 7, 2)), A(seq(last - 5, 7, 2), seq(last - 5, 7, fix<2>)));
  VERIFY_IS_APPROX(A(seq(1, n - 1 - 2), seq(n - 1 - 5, 7)), A(seq(1, last - 2), seq(last - 5, 7)));
  VERIFY_IS_APPROX(A(seq(n - 1 - 5, n - 1 - 2), seq(n - 1 - 5, n - 1 - 2)),
                   A(seq(last - 5, last - 2), seq(last - 5, last - 2)));

  VERIFY_IS_APPROX(A.col(A.cols() - 1), A(all, last));
  VERIFY_IS_APPROX(A(A.rows() - 2, A.cols() / 2), A(last - 1, lastp1 / 2));
  VERIFY_IS_APPROX(a(a.size() - 2), a(last - 1));
  VERIFY_IS_APPROX(a(a.size() / 2), a((last + 1) / 2));

  // Check fall-back to Block
  {
    VERIFY(is_same_eq(A.col(0), A(all, 0)));
    VERIFY(is_same_eq(A.row(0), A(0, all)));
    VERIFY(is_same_eq(A.block(0, 0, 2, 2), A(seqN(0, 2), seq(0, 1))));
    VERIFY(is_same_eq(A.middleRows(2, 4), A(seqN(2, 4), all)));
    VERIFY(is_same_eq(A.middleCols(2, 4), A(all, seqN(2, 4))));

    VERIFY(is_same_eq(A.col(A.cols() - 1), A(all, last)));

    const ArrayXXi& cA(A);
    VERIFY(is_same_eq(cA.col(0), cA(all, 0)));
    VERIFY(is_same_eq(cA.row(0), cA(0, all)));
    VERIFY(is_same_eq(cA.block(0, 0, 2, 2), cA(seqN(0, 2), seq(0, 1))));
    VERIFY(is_same_eq(cA.middleRows(2, 4), cA(seqN(2, 4), all)));
    VERIFY(is_same_eq(cA.middleCols(2, 4), cA(all, seqN(2, 4))));

    VERIFY(is_same_eq(a.head(4), a(seq(0, 3))));
    VERIFY(is_same_eq(a.tail(4), a(seqN(last - 3, 4))));
    VERIFY(is_same_eq(a.tail(4), a(seq(lastp1 - 4, last))));
    VERIFY(is_same_eq(a.segment<4>(3), a(seqN(3, fix<4>))));
  }

  ArrayXXi A1 = A, A2 = ArrayXXi::Random(4, 4);
  ArrayXi range25(4);
  range25 << 3, 2, 4, 5;
  A1(seqN(3, 4), seq(2, 5)) = A2;
  VERIFY_IS_APPROX(A1.block(3, 2, 4, 4), A2);
  A1 = A;
  A2.setOnes();
  A1(seq(6, 3, -1), range25) = A2;
  VERIFY_IS_APPROX(A1.block(3, 2, 4, 4), A2);

  // check reverse
  {
    VERIFY(is_same_seq_type(seq(3, 7).reverse(), seqN(7, 5, fix<-1>)));
    VERIFY(is_same_seq_type(seq(7, 3, fix<-2>).reverse(), seqN(3, 3, fix<2>)));
    VERIFY_IS_APPROX(a(seqN(2, last / 2).reverse()), a(seqN(2 + (last / 2 - 1) * 1, last / 2, fix<-1>)));
    VERIFY_IS_APPROX(a(seqN(last / 2, fix<4>).reverse()), a(seqN(last / 2, fix<4>)).reverse());
    VERIFY_IS_APPROX(A(seq(last - 5, last - 1, 2).reverse(), seqN(last - 3, 3, fix<-2>).reverse()),
                     A(seq(last - 5, last - 1, 2), seqN(last - 3, 3, fix<-2>)).reverse());
  }

  // check lastN
  VERIFY_IS_APPROX(a(lastN(3)), a.tail(3));
  VERIFY(MATCH(a(lastN(3)), "7\n8\n9"));
  VERIFY_IS_APPROX(a(lastN(fix<3>())), a.tail<3>());
  VERIFY(MATCH(a(lastN(3, 2)), "5\n7\n9"));
  VERIFY(MATCH(a(lastN(3, fix<2>())), "5\n7\n9"));
  VERIFY(a(lastN(fix<3>())).SizeAtCompileTime == 3);

  VERIFY((A(all, std::array<int, 4>{{1, 3, 2, 4}})).ColsAtCompileTime == 4);

  VERIFY_IS_APPROX((A(std::array<int, 3>{{1, 3, 5}}, std::array<int, 4>{{9, 6, 3, 0}})),
                   A(seqN(1, 3, 2), seqN(9, 4, -3)));
  VERIFY_IS_EQUAL(A(std::array<int, 3>{1, 3, 5}, std::array<int, 4>{3, 1, 6, 5}).RowsAtCompileTime, 3);
  VERIFY_IS_EQUAL(A(std::array<int, 3>{1, 3, 5}, std::array<int, 4>{3, 1, 6, 5}).ColsAtCompileTime, 4);

  VERIFY_IS_EQUAL(a(std::array<int, 3>{1, 3, 5}).SizeAtCompileTime, 3);
  VERIFY_IS_EQUAL(b(std::array<int, 3>{1, 3, 5}).SizeAtCompileTime, 3);

  // check different index types (C-style array, STL container, Eigen type)
  {
    Index size = 10;
    ArrayXd r = ArrayXd::Random(size);
    ArrayXi idx = ArrayXi::EqualSpaced(size, 0, 1);
    std::shuffle(idx.begin(), idx.end(), std::random_device());

    int c_array[3] = {idx[0], idx[1], idx[2]};
    std::vector<int> std_vector{idx[0], idx[1], idx[2]};
    Matrix<int, 3, 1> eigen_matrix{idx[0], idx[1], idx[2]};

    // non-const access
    VERIFY_IS_CWISE_EQUAL(r({idx[0], idx[1], idx[2]}), r(c_array));
    VERIFY_IS_CWISE_EQUAL(r({idx[0], idx[1], idx[2]}), r(std_vector));
    VERIFY_IS_CWISE_EQUAL(r({idx[0], idx[1], idx[2]}), r(eigen_matrix));
    VERIFY_IS_CWISE_EQUAL(r(std_vector), r(c_array));
    VERIFY_IS_CWISE_EQUAL(r(std_vector), r(eigen_matrix));
    VERIFY_IS_CWISE_EQUAL(r(eigen_matrix), r(c_array));

    const ArrayXd& r_ref = r;
    // const access
    VERIFY_IS_CWISE_EQUAL(r_ref({idx[0], idx[1], idx[2]}), r_ref(c_array));
    VERIFY_IS_CWISE_EQUAL(r_ref({idx[0], idx[1], idx[2]}), r_ref(std_vector));
    VERIFY_IS_CWISE_EQUAL(r_ref({idx[0], idx[1], idx[2]}), r_ref(eigen_matrix));
    VERIFY_IS_CWISE_EQUAL(r_ref(std_vector), r_ref(c_array));
    VERIFY_IS_CWISE_EQUAL(r_ref(std_vector), r_ref(eigen_matrix));
    VERIFY_IS_CWISE_EQUAL(r_ref(eigen_matrix), r_ref(c_array));
  }

  {
    Index rows = 8;
    Index cols = 11;
    ArrayXXd R = ArrayXXd::Random(rows, cols);
    ArrayXi r_idx = ArrayXi::EqualSpaced(rows, 0, 1);
    ArrayXi c_idx = ArrayXi::EqualSpaced(cols, 0, 1);
    std::shuffle(r_idx.begin(), r_idx.end(), std::random_device());
    std::shuffle(c_idx.begin(), c_idx.end(), std::random_device());

    int c_array_rows[3] = {r_idx[0], r_idx[1], r_idx[2]};
    int c_array_cols[4] = {c_idx[0], c_idx[1], c_idx[2], c_idx[3]};
    std::vector<int> std_vector_rows{r_idx[0], r_idx[1], r_idx[2]};
    std::vector<int> std_vector_cols{c_idx[0], c_idx[1], c_idx[2], c_idx[3]};
    Matrix<int, 3, 1> eigen_matrix_rows{r_idx[0], r_idx[1], r_idx[2]};
    Matrix<int, 4, 1> eigen_matrix_cols{c_idx[0], c_idx[1], c_idx[2], c_idx[3]};

    // non-const access
    VERIFY_IS_CWISE_EQUAL(R({r_idx[0], r_idx[1], r_idx[2]}, {c_idx[0], c_idx[1], c_idx[2], c_idx[3]}),
                          R(c_array_rows, c_array_cols));
    VERIFY_IS_CWISE_EQUAL(R({r_idx[0], r_idx[1], r_idx[2]}, {c_idx[0], c_idx[1], c_idx[2], c_idx[3]}),
                          R(std_vector_rows, std_vector_cols));
    VERIFY_IS_CWISE_EQUAL(R({r_idx[0], r_idx[1], r_idx[2]}, {c_idx[0], c_idx[1], c_idx[2], c_idx[3]}),
                          R(eigen_matrix_rows, eigen_matrix_cols));
    VERIFY_IS_CWISE_EQUAL(R(std_vector_rows, std_vector_cols), R(c_array_rows, c_array_cols));
    VERIFY_IS_CWISE_EQUAL(R(std_vector_rows, std_vector_cols), R(eigen_matrix_rows, eigen_matrix_cols));
    VERIFY_IS_CWISE_EQUAL(R(eigen_matrix_rows, eigen_matrix_cols), R(c_array_rows, c_array_cols));

    const ArrayXXd& R_ref = R;
    // const access
    VERIFY_IS_CWISE_EQUAL(R_ref({r_idx[0], r_idx[1], r_idx[2]}, {c_idx[0], c_idx[1], c_idx[2], c_idx[3]}),
                          R_ref(c_array_rows, c_array_cols));
    VERIFY_IS_CWISE_EQUAL(R_ref({r_idx[0], r_idx[1], r_idx[2]}, {c_idx[0], c_idx[1], c_idx[2], c_idx[3]}),
                          R_ref(std_vector_rows, std_vector_cols));
    VERIFY_IS_CWISE_EQUAL(R_ref({r_idx[0], r_idx[1], r_idx[2]}, {c_idx[0], c_idx[1], c_idx[2], c_idx[3]}),
                          R_ref(eigen_matrix_rows, eigen_matrix_cols));
    VERIFY_IS_CWISE_EQUAL(R_ref(std_vector_rows, std_vector_cols), R_ref(c_array_rows, c_array_cols));
    VERIFY_IS_CWISE_EQUAL(R_ref(std_vector_rows, std_vector_cols), R_ref(eigen_matrix_rows, eigen_matrix_cols));
    VERIFY_IS_CWISE_EQUAL(R_ref(eigen_matrix_rows, eigen_matrix_cols), R_ref(c_array_rows, c_array_cols));
  }

  // check mat(i,j) with weird types for i and j
  {
    VERIFY_IS_APPROX(A(B.RowsAtCompileTime - 1, 1), A(3, 1));
    VERIFY_IS_APPROX(A(B.RowsAtCompileTime, 1), A(4, 1));
    VERIFY_IS_APPROX(A(B.RowsAtCompileTime - 1, B.ColsAtCompileTime - 1), A(3, 3));
    VERIFY_IS_APPROX(A(B.RowsAtCompileTime, B.ColsAtCompileTime), A(4, 4));
    const Index I_ = 3, J_ = 4;
    VERIFY_IS_APPROX(A(I_, J_), A(3, 4));
  }

  // check extended block API
  {
    VERIFY(is_same_eq(A.block<3, 4>(1, 1), A.block(1, 1, fix<3>, fix<4>)));
    VERIFY(is_same_eq(A.block<3, 4>(1, 1, 3, 4), A.block(1, 1, fix<3>(), fix<4>(4))));
    VERIFY(is_same_eq(A.block<3, Dynamic>(1, 1, 3, 4), A.block(1, 1, fix<3>, 4)));
    VERIFY(is_same_eq(A.block<Dynamic, 4>(1, 1, 3, 4), A.block(1, 1, fix<Dynamic>(3), fix<4>)));
    VERIFY(is_same_eq(A.block(1, 1, 3, 4), A.block(1, 1, fix<Dynamic>(3), fix<Dynamic>(4))));

    VERIFY(is_same_eq(A.topLeftCorner<3, 4>(), A.topLeftCorner(fix<3>, fix<4>)));
    VERIFY(is_same_eq(A.bottomLeftCorner<3, 4>(), A.bottomLeftCorner(fix<3>, fix<4>)));
    VERIFY(is_same_eq(A.bottomRightCorner<3, 4>(), A.bottomRightCorner(fix<3>, fix<4>)));
    VERIFY(is_same_eq(A.topRightCorner<3, 4>(), A.topRightCorner(fix<3>, fix<4>)));

    VERIFY(is_same_eq(A.leftCols<3>(), A.leftCols(fix<3>)));
    VERIFY(is_same_eq(A.rightCols<3>(), A.rightCols(fix<3>)));
    VERIFY(is_same_eq(A.middleCols<3>(1), A.middleCols(1, fix<3>)));

    VERIFY(is_same_eq(A.topRows<3>(), A.topRows(fix<3>)));
    VERIFY(is_same_eq(A.bottomRows<3>(), A.bottomRows(fix<3>)));
    VERIFY(is_same_eq(A.middleRows<3>(1), A.middleRows(1, fix<3>)));

    VERIFY(is_same_eq(a.segment<3>(1), a.segment(1, fix<3>)));
    VERIFY(is_same_eq(a.head<3>(), a.head(fix<3>)));
    VERIFY(is_same_eq(a.tail<3>(), a.tail(fix<3>)));

    const ArrayXXi& cA(A);
    VERIFY(is_same_eq(cA.block<Dynamic, 4>(1, 1, 3, 4), cA.block(1, 1, fix<Dynamic>(3), fix<4>)));

    VERIFY(is_same_eq(cA.topLeftCorner<3, 4>(), cA.topLeftCorner(fix<3>, fix<4>)));
    VERIFY(is_same_eq(cA.bottomLeftCorner<3, 4>(), cA.bottomLeftCorner(fix<3>, fix<4>)));
    VERIFY(is_same_eq(cA.bottomRightCorner<3, 4>(), cA.bottomRightCorner(fix<3>, fix<4>)));
    VERIFY(is_same_eq(cA.topRightCorner<3, 4>(), cA.topRightCorner(fix<3>, fix<4>)));

    VERIFY(is_same_eq(cA.leftCols<3>(), cA.leftCols(fix<3>)));
    VERIFY(is_same_eq(cA.rightCols<3>(), cA.rightCols(fix<3>)));
    VERIFY(is_same_eq(cA.middleCols<3>(1), cA.middleCols(1, fix<3>)));

    VERIFY(is_same_eq(cA.topRows<3>(), cA.topRows(fix<3>)));
    VERIFY(is_same_eq(cA.bottomRows<3>(), cA.bottomRows(fix<3>)));
    VERIFY(is_same_eq(cA.middleRows<3>(1), cA.middleRows(1, fix<3>)));
  }

  // Check compilation of enums as index type:
  a(XX) = 1;
  A(XX, YY) = 1;
  // Anonymous enums only work with C++11
  enum { X = 0, Y = 1 };
  a(X) = 1;
  A(X, Y) = 1;
  A(XX, Y) = 1;
  A(X, YY) = 1;
  // check symbolic indices
  a(last) = 1.0;
  A(last, last) = 1;
  // check weird non-const, non-lvalue scenarios
  {
    // in these scenarios, the objects are not declared 'const', and the compiler will atttempt to use the non-const
    // overloads without intervention

    // non-const map to a const object
    Map<const ArrayXd> a_map(a.data(), a.size());
    Map<const ArrayXXi> A_map(A.data(), A.rows(), A.cols());

    VERIFY_IS_EQUAL(a_map(last), a.coeff(a.size() - 1));
    VERIFY_IS_EQUAL(A_map(last, last), A.coeff(A.rows() - 1, A.cols() - 1));

    // non-const expressions that have no modifiable data
    using Op = internal::scalar_constant_op<double>;
    using VectorXpr = CwiseNullaryOp<Op, VectorXd>;
    using MatrixXpr = CwiseNullaryOp<Op, MatrixXd>;
    double constant_val = internal::random<double>();
    Op op(constant_val);
    VectorXpr vectorXpr(10, 1, op);
    MatrixXpr matrixXpr(8, 11, op);

    VERIFY_IS_EQUAL(vectorXpr.coeff(vectorXpr.size() - 1), vectorXpr(last));
    VERIFY_IS_EQUAL(matrixXpr.coeff(matrixXpr.rows() - 1, matrixXpr.cols() - 1), matrixXpr(last, last));
  }

  // Check compilation of varying integer types as index types:
  Index i = n / 2;
  short i_short = static_cast<short>(i);
  std::size_t i_sizet(i);
  VERIFY_IS_EQUAL(a(i), a.coeff(i_short));
  VERIFY_IS_EQUAL(a(i), a.coeff(i_sizet));

  VERIFY_IS_EQUAL(A(i, i), A.coeff(i_short, i_short));
  VERIFY_IS_EQUAL(A(i, i), A.coeff(i_short, i));
  VERIFY_IS_EQUAL(A(i, i), A.coeff(i, i_short));
  VERIFY_IS_EQUAL(A(i, i), A.coeff(i, i_sizet));
  VERIFY_IS_EQUAL(A(i, i), A.coeff(i_sizet, i));
  VERIFY_IS_EQUAL(A(i, i), A.coeff(i_sizet, i_short));
  VERIFY_IS_EQUAL(A(i, i), A.coeff(5, i_sizet));

  // Regression test for Max{Rows,Cols}AtCompileTime
  {
    Matrix3i A3 = Matrix3i::Random();
    ArrayXi ind(5);
    ind << 1, 1, 1, 1, 1;
    VERIFY_IS_EQUAL(A3(ind, ind).eval(), MatrixXi::Constant(5, 5, A3(1, 1)));
  }

  // Regression for bug 1736
  {
    VERIFY_IS_APPROX(A(all, eii).col(0).eval(), A.col(eii(0)));
    A(all, eii).col(0) = A.col(eii(0));
  }

  // bug 1815: IndexedView should allow linear access
  {
    VERIFY(MATCH(b(eii)(0), "3"));
    VERIFY(MATCH(a(eii)(0), "3"));
    VERIFY(MATCH(A(1, eii)(0), "103"));
    VERIFY(MATCH(A(eii, 1)(0), "301"));
    VERIFY(MATCH(A(1, all)(1), "101"));
    VERIFY(MATCH(A(all, 1)(1), "101"));
  }

  // bug #2375: indexing over matrices of dim >128 should compile on gcc
  {
    Matrix<double, 513, 3> large_mat = Matrix<double, 513, 3>::Random();
    std::array<int, 2> test_indices = {0, 1};
    Matrix<double, 513, 2> thin_slice = large_mat(all, test_indices);
    for (int col = 0; col < int(test_indices.size()); ++col)
      for (int row = 0; row < large_mat.rows(); ++row) VERIFY_IS_EQUAL(thin_slice(row, col), large_mat(row, col));
  }

  // Bug IndexView with a single static row should be RowMajor:
  {
    // A(1, seq(0,2,1)).cwiseAbs().colwise().replicate(2).eval();
    STATIC_CHECK(((internal::evaluator<decltype(A(1, seq(0, 2, 1)))>::Flags & RowMajorBit) == RowMajorBit));
  }

  // Direct access.
  {
    int rows = 3;
    int row_start = internal::random<int>(0, rows - 1);
    int row_inc = internal::random<int>(1, rows - row_start);
    int row_size = internal::random<int>(1, (rows - row_start) / row_inc);
    auto row_seq = seqN(row_start, row_size, row_inc);

    int cols = 3;
    int col_start = internal::random<int>(0, cols - 1);
    int col_inc = internal::random<int>(1, cols - col_start);
    int col_size = internal::random<int>(1, (cols - col_start) / col_inc);
    auto col_seq = seqN(col_start, col_size, col_inc);

    MatrixXd m1 = MatrixXd::Random(rows, cols);
    MatrixXd m2 = MatrixXd::Random(cols, rows);
    VERIFY_IS_APPROX(m1(row_seq, indexing::all) * m2, m1(row_seq, indexing::all).eval() * m2);
    VERIFY_IS_APPROX(m1 * m2(indexing::all, col_seq), m1 * m2(indexing::all, col_seq).eval());
    VERIFY_IS_APPROX(m1(row_seq, col_seq) * m2(col_seq, row_seq),
                     m1(row_seq, col_seq).eval() * m2(col_seq, row_seq).eval());

    VectorXd v1 = VectorXd::Random(cols);
    VERIFY_IS_APPROX(m1(row_seq, col_seq) * v1(col_seq), m1(row_seq, col_seq).eval() * v1(col_seq).eval());
    VERIFY_IS_APPROX(v1(col_seq).transpose() * m2(col_seq, row_seq),
                     v1(col_seq).transpose().eval() * m2(col_seq, row_seq).eval());
  }
}

void check_tutorial_examples() {
  constexpr int kRows = 11;
  constexpr int kCols = 21;
  Matrix<double, kRows, kCols> A = Matrix<double, kRows, kCols>::Random();
  Vector<double, kRows> v = Vector<double, kRows>::Random();

  {
    auto slice = A(seqN(fix<0>, fix<5>, fix<2>), seqN(fix<2>, fix<7>, fix<1>));
    EIGEN_UNUSED_VARIABLE(slice);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), 5);
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), 7);
  }
  {
    auto slice = A(seqN(fix<0>, fix<5>, fix<2>), indexing::all);
    EIGEN_UNUSED_VARIABLE(slice);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), 5);
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), kCols);
  }

  // Examples from slicing tutorial.
  // Bottom-left corner.
  {
    Index i = 3;
    Index n = 5;
    auto slice = A(seq(i, indexing::last), seqN(0, n));
    auto block = A.bottomLeftCorner(A.rows() - i, n);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), Dynamic);
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), Dynamic);
    VERIFY_IS_EQUAL(slice, block);
  }
  {
    auto i = fix<3>;
    auto n = fix<5>;
    auto slice = A(seq(i, indexing::last), seqN(fix<0>, n));
    auto block = A.bottomLeftCorner(fix<kRows> - i, n);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), A.RowsAtCompileTime - i);
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), n);
    VERIFY_IS_EQUAL(slice, block);
  }

  // Block starting at i,j of size m,n.
  {
    Index i = 4;
    Index j = 2;
    Index m = 3;
    Index n = 5;
    auto slice = A(seqN(i, m), seqN(j, n));
    auto block = A.block(i, j, m, n);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }
  {
    auto i = fix<4>;
    auto j = fix<2>;
    auto m = fix<3>;
    auto n = fix<5>;
    auto slice = A(seqN(i, m), seqN(j, n));
    auto block = A.block(i, j, m, n);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }

  // Block starting at i0,j0 and ending at i1,j1.
  {
    Index i0 = 4;
    Index i1 = 7;
    Index j0 = 3;
    Index j1 = 5;
    auto slice = A(seq(i0, i1), seq(j0, j1));
    auto block = A.block(i0, j0, i1 - i0 + 1, j1 - j0 + 1);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }
  {
    auto i0 = fix<4>;
    auto i1 = fix<7>;
    auto j0 = fix<3>;
    auto j1 = fix<5>;
    auto slice = A(seq(i0, i1), seq(j0, j1));
    auto block = A.block(i0, j0, i1 - i0 + fix<1>, j1 - j0 + fix<1>);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }

  // Even columns of A.
  {
    auto slice = A(all, seq(0, last, 2));
    auto block =
        Eigen::Map<Eigen::Matrix<double, kRows, Dynamic>, 0, OuterStride<2 * kRows>>(A.data(), kRows, (kCols + 1) / 2);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }
  {
    auto slice = A(all, seq(fix<0>, last, fix<2>));
    auto block = Eigen::Map<Eigen::Matrix<double, kRows, (kCols + 1) / 2>, 0, OuterStride<2 * kRows>>(A.data());
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }

  // First n odd rows of A.
  {
    Index n = 3;
    auto slice = A(seqN(1, n, 2), all);
    auto block = Eigen::Map<Eigen::Matrix<double, Dynamic, kCols>, 0, Stride<kRows, 2>>(A.data() + 1, n, kCols);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }
  {
    auto n = fix<3>;
    auto slice = A(seqN(fix<1>, n, fix<2>), all);
    auto block = Eigen::Map<Eigen::Matrix<double, 3, kCols>, 0, Stride<kRows, 2>>(A.data() + 1);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }

  // The second-last column.
  {
    auto slice = A(all, last - 1);
    auto block = A.col(A.cols() - 2);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }
  {
    auto slice = A(all, last - fix<1>);
    auto block = A.col(fix<kCols> - fix<2>);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }

  // The middle row.
  {
    auto slice = A(last / 2, all);
    auto block = A.row((A.rows() - 1) / 2);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }
  {
    auto slice = A(last / fix<2>, all);
    auto block = A.row(fix<(kRows - 1) / 2>);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }

  // Last elements of v starting at i.
  {
    Index i = 7;
    auto slice = v(seq(i, last));
    auto block = v.tail(v.size() - i);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }
  {
    auto i = fix<7>;
    auto slice = v(seq(i, last));
    auto block = v.tail(fix<kRows> - i);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }

  // Last n elements of v.
  {
    Index n = 6;
    auto slice = v(seq(last + 1 - n, last));
    auto block = v.tail(n);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }
  {
    auto n = fix<6>;
    auto slice = v(seq(last + fix<1> - n, last));
    auto block = v.tail(n);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }

  // Last n elements of v.
  {
    Index n = 6;
    auto slice = v(lastN(n));
    auto block = v.tail(n);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }
  {
    auto n = fix<6>;
    auto slice = v(lastN(n));
    auto block = v.tail(n);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }

  // Bottom-right corner of A of size m times n.
  {
    Index m = 3;
    Index n = 6;
    auto slice = A(lastN(m), lastN(n));
    auto block = A.bottomRightCorner(m, n);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }
  {
    auto m = fix<3>;
    auto n = fix<6>;
    auto slice = A(lastN(m), lastN(n));
    auto block = A.bottomRightCorner(m, n);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }

  // Last n columns with a stride of 3.
  {
    Index n = 4;
    constexpr Index stride = 3;
    auto slice = A(all, lastN(n, stride));
    auto block = Eigen::Map<Eigen::Matrix<double, kRows, Dynamic>, 0, OuterStride<stride * kRows>>(
        A.data() + (kCols - 1 - (n - 1) * stride) * kRows, A.rows(), n);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }
  {
    constexpr auto n = fix<4>;
    constexpr auto stride = fix<3>;
    auto slice = A(all, lastN(n, stride));
    auto block = Eigen::Map<Eigen::Matrix<double, kRows, n>, 0, OuterStride<stride * kRows>>(
        A.data() + (kCols - 1 - (n - 1) * stride) * kRows, A.rows(), n);
    VERIFY_IS_EQUAL(int(slice.RowsAtCompileTime), int(block.RowsAtCompileTime));
    VERIFY_IS_EQUAL(int(slice.ColsAtCompileTime), int(block.ColsAtCompileTime));
    VERIFY_IS_EQUAL(slice, block);
  }

  // Compile time size and increment.
  {
    auto slice1 = v(seq(last - fix<7>, last - fix<2>));
    auto slice2 = v(seqN(last - 7, fix<6>));
    VERIFY_IS_EQUAL(slice1, slice2);
    VERIFY_IS_EQUAL(int(slice1.SizeAtCompileTime), 6);
    VERIFY_IS_EQUAL(int(slice2.SizeAtCompileTime), 6);
    auto slice3 = A(all, seq(fix<0>, last, fix<2>));
    TEST_SET_BUT_UNUSED_VARIABLE(slice3)
    VERIFY_IS_EQUAL(int(slice3.RowsAtCompileTime), kRows);
    VERIFY_IS_EQUAL(int(slice3.ColsAtCompileTime), (kCols + 1) / 2);
  }

  // Reverse order.
  {
    auto slice = A(all, seq(20, 10, fix<-2>));
    auto block = Eigen::Map<Eigen::Matrix<double, kRows, Dynamic>, 0, OuterStride<-2 * kRows>>(
        A.data() + 20 * kRows, A.rows(), (20 - 10 + 2) / 2);
    VERIFY_IS_EQUAL(slice, block);
  }
  {
    Index n = 10;
    auto slice1 = A(seqN(last, n, fix<-1>), all);
    auto slice2 = A(lastN(n).reverse(), all);
    VERIFY_IS_EQUAL(slice1, slice2);
  }

  // Array of indices.
  {
    std::vector<int> ind{4, 2, 5, 5, 3};
    auto slice1 = A(all, ind);
    for (size_t i = 0; i < ind.size(); ++i) {
      VERIFY_IS_EQUAL(slice1.col(i), A.col(ind[i]));
    }

    auto slice2 = A(all, {4, 2, 5, 5, 3});
    VERIFY_IS_EQUAL(slice1, slice2);

    Eigen::ArrayXi indarray(5);
    indarray << 4, 2, 5, 5, 3;
    auto slice3 = A(all, indarray);
    VERIFY_IS_EQUAL(slice1, slice3);
  }

  // Custom index list.
  {
    struct pad {
      Index size() const { return out_size; }
      Index operator[](Index i) const { return std::max<Index>(0, i - (out_size - in_size)); }
      Index in_size, out_size;
    };

    auto slice = A(pad{3, 5}, pad{3, 5});
    Eigen::MatrixXd B = slice;
    VERIFY_IS_EQUAL(B.block(2, 2, 3, 3), A.block(0, 0, 3, 3));
  }
}

void check_aliasing() {
  Eigen::Vector<float, 5> z = {0.0f, 1.1f, 2.2f, 3.3f, 4.4f};
  std::vector<int> left_indices = {0, 1, 3, 4};
  std::vector<int> right_indices = {1, 3, 4, 0};
  z(left_indices) = z(right_indices);
  Eigen::Vector<float, 5> expected = {1.1f, 3.3f, 2.2f, 4.4f, 0.0f};
  VERIFY_IS_EQUAL(z, expected);
}

EIGEN_DECLARE_TEST(indexed_view) {
  for (int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST_1(check_indexed_view());
  }
  CALL_SUBTEST_1(check_tutorial_examples());
  CALL_SUBTEST_1(check_aliasing());

  // static checks of some internals:
  STATIC_CHECK((internal::is_valid_index_type<int>::value));
  STATIC_CHECK((internal::is_valid_index_type<unsigned int>::value));
  STATIC_CHECK((internal::is_valid_index_type<short>::value));
  STATIC_CHECK((internal::is_valid_index_type<std::ptrdiff_t>::value));
  STATIC_CHECK((internal::is_valid_index_type<std::size_t>::value));
  STATIC_CHECK((!internal::valid_indexed_view_overload<int, int>::value));
  STATIC_CHECK((!internal::valid_indexed_view_overload<int, std::ptrdiff_t>::value));
  STATIC_CHECK((!internal::valid_indexed_view_overload<std::ptrdiff_t, int>::value));
  STATIC_CHECK((!internal::valid_indexed_view_overload<std::size_t, int>::value));
}
