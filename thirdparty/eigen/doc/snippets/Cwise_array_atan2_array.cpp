Array<double, 1, 3> x(8, -25, 3), y(1. / 3., 0.5, -2.);
cout << "atan2([" << x << "], [" << y << "]) = " << x.atan2(y) << endl;  // using ArrayBase::pow
cout << "atan2([" << x << "], [" << y << "] = " << atan2(x, y) << endl;  // using Eigen::pow
