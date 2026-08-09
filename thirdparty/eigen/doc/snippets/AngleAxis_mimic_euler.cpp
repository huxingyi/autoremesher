Matrix3f m;
m = AngleAxisf(0.25 * EIGEN_PI, Vector3f::UnitX()) * AngleAxisf(0.5 * EIGEN_PI, Vector3f::UnitY()) *
    AngleAxisf(0.33 * EIGEN_PI, Vector3f::UnitZ());
cout << m << endl << "is unitary: " << m.isUnitary() << endl;
