#include <CGALmini/Static_filter_error.h>

#include <FPG/Group_algebra.h>
#include <FPG/Symbol.h>
#include <cassert>
#include <iostream>
#include <iomanip>

void
test_group_algebra() {
    using namespace Group_algebra;
    Variable *var_a = new Variable( "a", type_float );
    Variable *var_b = new Variable( "b", type_float );
    Variable *var_c = new Variable( "c", type_float );
    Variable *var_d = new Variable( "d", type_float );
    Variable *var_e = new Variable( "e", type_float );
    Leaf_item *l1 = new Leaf_item( 1, 1 );
    Leaf_item *l2 = new Leaf_item( 1, 2 );
    Leaf_item *l3 = new Leaf_item( 1, 3 );
    Leaf_item *l4_deg2 = new Leaf_item( 2, 4 );
    Leaf_item *l5_deg3 = new Leaf_item( 3, 5 );
    l1->add_to_group( var_a );
    l2->add_to_group( var_b );
    l3->add_to_group( var_c );
    l4_deg2->add_to_group( var_d );
    l5_deg3->add_to_group( var_e );
    l1->dump();
    assert( l1     ->degree() == 1 );
    assert( l4_deg2->degree() == 2 );
    assert( !l1->is_equal(l2) );
    Group_item *prod1 = l1->mul(l2)->mul(l3);
    Group_item *prod2 = l1->mul(l2)->mul(l3);
    assert( prod1->degree() == 3 );
    assert( prod2->degree() == 3 );
    assert( !l1->is_equal(prod1) );
    assert( prod1->is_equal( prod2 ) );
    assert( !prod1->mul(prod1)->is_equal(prod1) );
    //prod1->dump();
    assert( l1->is_equal( l1->add(l1) ) );
    //prod1->add(prod1)->dump();

    prod1 = l1->mul(l2);
    std::cout << "a=1*2:" << std::endl;
    prod1->dump();
    std::cout << "b=4@2:" << std::endl;
    l4_deg2->dump();
    std::cout << "a+b:" << std::endl;
    Group_item *sum1 = prod1->add(l4_deg2);
    sum1->dump();
    assert( sum1->degree() == 2 );

    Group_item *prod3 = l1->mul(l4_deg2);
    assert( prod3->degree() == 3 );
    Group_item *sum2 = prod3->add( l5_deg3 );
    sum2->dump();
    assert( sum1->degree() != sum2->degree() );

    Group_item *sum3 = l4_deg2->add( l1->mul(l2) );
    assert( sum3->degree() == sum1->degree() );
    sum3->dump();
    sum1->dump();
    std::cout << "adding the two above sums: " << std::endl;
    Group_item *sum4 = sum1->add(sum3);
    std::cout << "sum4:" << std::endl;
    sum4->dump();
    assert( sum4->degree() == 2 );
    assert( sum4->is_equal( sum3->add(sum1 ) ) );
    assert( sum4->is_equal( sum4->add(sum4 ) ) );
    assert( sum4->is_equal( sum4->add(sum4 )->add(sum4 ) ) );

    prod1=l1->mul(l1);
    prod2=l2->mul(l2);
    prod3=l3->mul(l3);
    sum1 = prod1->add(prod2)->add(prod3);
    sum2 = sum1->add(l4_deg2);
    assert( !sum1->is_equal(sum2) );
    prod1=sum2->mul(l1)->mul(l2);
    prod1->dump();
    assert( prod1->is_equal(prod1->add(prod1) ) );
    std::cout << "adding" << std::endl;
    sum2->dump();
    prod2->dump();
    sum3 = prod2->add( sum2 );
    sum3->dump();
}

int
main() {
    //test_group_algebra();
#if 1
    std::cout << std::setprecision(15) << std::scientific;
    double eps = 1e-40;
    CGAL::FPU_CW_t backup = CGAL::FPU_get_and_set_cw(CGAL_FE_UPWARD);
    for( unsigned int i = 0; i < 100; ++i ) {
        typedef CGAL::Static_filter_error F;
        F e0(1,0);
        F e1(1,eps);
        //e0 = e0-e0;
        //e1 = e1-e1;
        //e1 = e1*e1*e1*e1*e1*e1*e1*e1 + e1*e1*e1*e1*e1*e1*e1*e1 + e1*e1*e1*e1*e1*e1*e1*e1 + e1*e1*e1*e1*e1*e1*e1*e1;
        //e1 = e1*e1*e1*e1*e1*e1 + e1*e1*e1*e1*e1*e1 + e1*e1*e1*e1*e1*e1 + e1*e1*e1*e1*e1*e1;
        e0 = e0*e0;// + e0*e0*e0 + e0*e0*e0;
        e1 = e1*e1;// + e1*e1*e1 + e1*e1*e1;
        double error0 = e0.error();
        double error1 = e1.error();
        double bound0 = e0.bound();
        double bound1 = e1.bound();
        std::cout << F::ulp()/2.0 << " " << F::ulp( 1.0 + F::ulp()/2.0 )/2.0 << std::endl;
        //double error1_from_0 = eps*(3.0+F::ulp()/2.0+eps*(3.0+eps)) + F::ulp( 1.0 + F::ulp()/2.0 )/2.0;
        double error1_derived = eps*(2.0 + eps) + F::ulp()/2.0;
        std::cout << std::setw(2) << i << " eps: " << std::setw(10) << eps
                                       << "   error0: " << std::setw(20) << error0
                                       << "   error1: " << std::setw(20) << error1
                                       << "   error1_derived: " << std::setw(20) << error1_derived
                                       << "   err: " << std::setw(20) << error1/error1_derived << std::endl;
        /*
        std::cout << std::setw(2) << i << " eps: " << std::setw(10) << eps
                                       << "   abs error: " << std::setw(20) << error
                                       << "   rel error: " << std::setw(20) << error/bound
                                       << "   eps/abs error: " << std::setw(20) << eps/error << std::endl;
        */
        eps *= 10.0;
        /*if( error1/bound1 > 0.2 ) {
            std::cout << "relative error > 0.2" << std::endl;
            break;
        }*/
    }
    return 0;
#endif
}