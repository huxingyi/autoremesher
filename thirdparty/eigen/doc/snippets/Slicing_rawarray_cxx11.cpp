MatrixXi A = MatrixXi::Random(4, 6);
cout << "Initial matrix A:\n" << A << "\n\n";
cout << "A(all,{4,2,5,5,3}):\n" << A(Eigen::placeholders::all, {4, 2, 5, 5, 3}) << "\n\n";
