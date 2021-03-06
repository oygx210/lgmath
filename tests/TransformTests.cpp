//////////////////////////////////////////////////////////////////////////////////////////////
/// \file TransformTests.cpp
/// \brief Unit tests for the implementation of the transformation matrix class.
/// \details Unit tests for the various Lie Group functions will test both special cases,
///          and randomly generated cases.
///
/// \author Sean Anderson
//////////////////////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <iostream>
#include <iomanip>
#include <ios>

#include <Eigen/Dense>
#include <lgmath/CommonMath.hpp>

#include <lgmath/so3/Operations.hpp>
#include <lgmath/se3/Operations.hpp>
#include <lgmath/se3/Transformation.hpp>

#include "catch.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// UNIT TESTS OF TRANSFORMATION MATRIX
///
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief General test of transformation constructors
/////////////////////////////////////////////////////////////////////////////////////////////
TEST_CASE("Transformation Constructors.", "[lgmath]" ) {

  // Generate random transform from most basic constructor
  Eigen::Matrix<double,3,3> C_ba = lgmath::so3::vec2rot(Eigen::Matrix<double,3,1>::Random());
  Eigen::Matrix<double,3,1> r_ba_ina = Eigen::Matrix<double,3,1>::Random();
  lgmath::se3::Transformation rand(C_ba, r_ba_ina);

  // Transformation();
  SECTION("default" ) {
    lgmath::se3::Transformation tmatrix;
    Eigen::Matrix4d test = Eigen::Matrix4d::Identity();
    INFO("tmat: " << tmatrix.matrix());
    INFO("test: " << test);
    CHECK(lgmath::common::nearEqual(tmatrix.matrix(), test, 1e-6));
  }

  // Transformation(const Transformation& T);
  SECTION("copy constructor" ) {
    lgmath::se3::Transformation test(rand);
    INFO("tmat: " << rand.matrix());
    INFO("test: " << test.matrix());
    CHECK(lgmath::common::nearEqual(rand.matrix(), test.matrix(), 1e-6));
  }

  // Transformation(const Eigen::Matrix4d& T);
  SECTION("matrix constructor" ) {
    lgmath::se3::Transformation test(rand.matrix());
    INFO("tmat: " << rand.matrix());
    INFO("test: " << test.matrix());
    CHECK(lgmath::common::nearEqual(rand.matrix(), test.matrix(), 1e-6));

    // Test forced reprojection (ones to identity)
    Eigen::Matrix4d proj_test = Eigen::Matrix4d::Identity();
    proj_test.topRightCorner<3,1>() = -r_ba_ina;
    Eigen::Matrix3d notRotation = Eigen::Matrix3d::Ones();
    Eigen::Matrix4d notTransform = Eigen::Matrix4d::Identity();
    notTransform.topLeftCorner<3,3>() = notRotation;
    notTransform.topRightCorner<3,1>() = -r_ba_ina;
    lgmath::se3::Transformation test_bad(notTransform); // force reproj
    INFO("cmat: " << proj_test.matrix());
    INFO("test: " << test_bad.matrix());
    CHECK(lgmath::common::nearEqual(proj_test.matrix(), test_bad.matrix(), 1e-6));
  }

  // Transformation& operator=(Transformation T);
  SECTION("assignment operator" ) {
    lgmath::se3::Transformation test = rand;
    INFO("tmat: " << rand.matrix());
    INFO("test: " << test.matrix());
    CHECK(lgmath::common::nearEqual(rand.matrix(), test.matrix(), 1e-6));
  }

  // Transformation(const Eigen::Matrix<double,6,1>& vec, unsigned int numTerms = 0);
  SECTION("exponential map" ) {
    Eigen::Matrix<double,6,1> vec = Eigen::Matrix<double,6,1>::Random();
    Eigen::Matrix4d tmat = lgmath::se3::vec2tran(vec);
    lgmath::se3::Transformation testAnalytical(vec);
    lgmath::se3::Transformation testNumerical(vec,15);
    INFO("tmat: " << tmat);
    INFO("testAnalytical: " << testAnalytical.matrix());
    INFO("testNumerical: " << testNumerical.matrix());
    CHECK(lgmath::common::nearEqual(tmat, testAnalytical.matrix(), 1e-6));
    CHECK(lgmath::common::nearEqual(tmat, testNumerical.matrix(), 1e-6));
  }

  // Transformation(const Eigen::VectorXd& vec);
  SECTION("exponential map with VectorXd" ) {
    Eigen::VectorXd vec = Eigen::Matrix<double,6,1>::Random();
    Eigen::Matrix4d tmat = lgmath::se3::vec2tran(vec);
    lgmath::se3::Transformation test(vec);
    INFO("tmat: " << tmat);
    INFO("test: " << test.matrix());
    CHECK(lgmath::common::nearEqual(tmat, test.matrix(), 1e-6));
  }

  // Transformation(const Eigen::VectorXd& vec);
  SECTION("exponential map with bad VectorXd" ) {
    Eigen::VectorXd vec = Eigen::Matrix<double,6,1>::Random();
    lgmath::se3::Transformation test(vec);

    // Wrong size vector
    Eigen::VectorXd badvec = Eigen::Matrix<double,3,1>::Random();
    lgmath::se3::Transformation testFailure;
    try {
      testFailure = lgmath::se3::Transformation(badvec);
    } catch (const std::invalid_argument& e) {
      testFailure = test;
    }
    INFO("tmat: " << testFailure.matrix());
    INFO("test: " << test.matrix());
    CHECK(lgmath::common::nearEqual(testFailure.matrix(), test.matrix(), 1e-6));
  }

  //Transformation(const Eigen::Matrix3d& C_ba,
  //               const Eigen::Vector3d& r_ba_ina);
  SECTION("test C/r constructor" ) {
    lgmath::se3::Transformation tmat(C_ba, r_ba_ina);
    Eigen::Matrix4d test = Eigen::Matrix4d::Identity();
    test.topLeftCorner<3,3>() = C_ba;
    test.topRightCorner<3,1>() = -C_ba*r_ba_ina;
    INFO("tmat: " << tmat.matrix());
    INFO("test: " << test);
    CHECK(lgmath::common::nearEqual(tmat.matrix(), test, 1e-6));

    // Test forced reprojection (ones to identity)
    Eigen::Matrix4d proj_test = Eigen::Matrix4d::Identity();
    proj_test.topRightCorner<3,1>() = -Eigen::Matrix3d::Identity()*r_ba_ina;
    Eigen::Matrix3d notRotation = Eigen::Matrix3d::Ones();
    lgmath::se3::Transformation test_bad(notRotation, r_ba_ina); // forces reprojection
    INFO("cmat: " << proj_test.matrix());
    INFO("test: " << test_bad.matrix());
    CHECK(lgmath::common::nearEqual(proj_test.matrix(), test_bad.matrix(), 1e-6));
  }

  // Transformation(Transformation&&);
  SECTION("move constructor" ) {
    lgmath::se3::Transformation test(std::move(rand));

    INFO("tmat: " << test.matrix());
    INFO("test: " << rand.matrix());
    CHECK(lgmath::common::nearEqual(test.matrix(),rand.matrix(), 1e-6));
  }

  // Transformation = Transformation&&;
  SECTION("move assignment" ) {
    lgmath::se3::Transformation test;
    test = std::move(rand);

    INFO("tmat: " << test.matrix());
    INFO("test: " << rand.matrix());
    CHECK(lgmath::common::nearEqual(test.matrix(),rand.matrix(), 1e-6));
  }

} // TEST_CASE

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Test some get methods
/////////////////////////////////////////////////////////////////////////////////////////////
TEST_CASE("Transformation get methods.", "[lgmath]" ) {

  // Generate random transform from most basic constructor
  Eigen::Matrix<double,3,3> C_ba = lgmath::so3::vec2rot(Eigen::Matrix<double,3,1>::Random());
  Eigen::Matrix<double,3,1> r_ba_ina = Eigen::Matrix<double,3,1>::Random();
  lgmath::se3::Transformation T_ba(C_ba, r_ba_ina);

  // Construct simple eigen matrix from random rotation and translation
  Eigen::Matrix4d test = Eigen::Matrix4d::Identity();
  Eigen::Matrix<double,3,1> r_ab_inb = -C_ba*r_ba_ina;
  test.topLeftCorner<3,3>() = C_ba;
  test.topRightCorner<3,1>() = r_ab_inb;

  // Test matrix()
  INFO("T_ba: " << T_ba.matrix());
  INFO("test: " << test);
  CHECK(lgmath::common::nearEqual(T_ba.matrix(), test, 1e-6));

  // Test C_ba()
  INFO("T_ba: " << T_ba.C_ba());
  INFO("C_ba: " << C_ba);
  CHECK(lgmath::common::nearEqual(T_ba.C_ba(), C_ba, 1e-6));

  // Test r_ba_ina()
  INFO("T_ba: " << T_ba.r_ba_ina());
  INFO("r_ba_ina: " << r_ba_ina);
  CHECK(lgmath::common::nearEqual(T_ba.r_ba_ina(), r_ba_ina, 1e-6));

  // Test r_ab_inb()
  INFO("T_ba: " << T_ba.r_ab_inb());
  INFO("r_ab_inb: " << r_ab_inb);
  CHECK(lgmath::common::nearEqual(T_ba.r_ab_inb(), r_ab_inb, 1e-6));

} // TEST_CASE

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Test exponential map construction and logarithmic vec() method
/////////////////////////////////////////////////////////////////////////////////////////////
TEST_CASE("Test transformation to/from SE(3) algebra.", "[lgmath]" ) {

  // Add vectors to be tested
  std::vector<Eigen::Matrix<double,6,1> > trueVecs;
  Eigen::Matrix<double,6,1> temp;
  temp << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0; trueVecs.push_back(temp);
  temp << 1.0, 0.0, 0.0, 0.0, 0.0, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 1.0, 0.0, 0.0, 0.0, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 1.0, 0.0, 0.0, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, lgmath::constants::PI, 0.0, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, 0.0, lgmath::constants::PI, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, 0.0, 0.0, lgmath::constants::PI; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, -lgmath::constants::PI, 0.0, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, 0.0, -lgmath::constants::PI, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, 0.0, 0.0, -lgmath::constants::PI; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, 0.5*lgmath::constants::PI, 0.0, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, 0.0, 0.5*lgmath::constants::PI, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, 0.0, 0.0, 0.5*lgmath::constants::PI; trueVecs.push_back(temp);
  const unsigned numRand = 20;
  for (unsigned i = 0; i < numRand; i++) {
    trueVecs.push_back(Eigen::Matrix<double,6,1>::Random());
  }

  // Get number of tests
  const unsigned numTests = trueVecs.size();

  // Calc transformation matrices
  std::vector<Eigen::Matrix4d > transMatrices;
  for (unsigned i = 0; i < numTests; i++) {
    transMatrices.push_back(lgmath::se3::vec2tran(trueVecs.at(i)));
  }

  // Calc transformations
  std::vector<lgmath::se3::Transformation > transformations;
  for (unsigned i = 0; i < numTests; i++) {
    transformations.push_back(lgmath::se3::Transformation(trueVecs.at(i)));
  }

  // Compare matrices
  SECTION("vec2tran") {
    for (unsigned i = 0; i < numTests; i++) {
      INFO("matr: " << transMatrices.at(i));
      INFO("tran: " << transformations.at(i).matrix());
      CHECK(lgmath::common::nearEqual(transMatrices.at(i), transformations.at(i).matrix(), 1e-6));
    }
  }

  // Test logarithmic map
  SECTION("tran2vec") {
    for (unsigned i = 0; i < numTests; i++) {
      Eigen::Matrix<double,6,1> testVec = transformations.at(i).vec();
      INFO("true: " << trueVecs.at(i));
      INFO("func: " << testVec);
      CHECK(lgmath::common::nearEqualLieAlg(trueVecs.at(i), testVec, 1e-6));
    }
  }

} // TEST_CASE

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Test inverse, adjoint and operatations
/////////////////////////////////////////////////////////////////////////////////////////////
TEST_CASE("Test transformation inverse.", "[lgmath]" ) {

  // Add vectors to be tested
  std::vector<Eigen::Matrix<double,6,1> > trueVecs;
  Eigen::Matrix<double,6,1> temp;
  temp << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0; trueVecs.push_back(temp);
  temp << 1.0, 0.0, 0.0, 0.0, 0.0, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 1.0, 0.0, 0.0, 0.0, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 1.0, 0.0, 0.0, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, lgmath::constants::PI, 0.0, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, 0.0, lgmath::constants::PI, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, 0.0, 0.0, lgmath::constants::PI; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, -lgmath::constants::PI, 0.0, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, 0.0, -lgmath::constants::PI, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, 0.0, 0.0, -lgmath::constants::PI; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, 0.5*lgmath::constants::PI, 0.0, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, 0.0, 0.5*lgmath::constants::PI, 0.0; trueVecs.push_back(temp);
  temp << 0.0, 0.0, 0.0, 0.0, 0.0, 0.5*lgmath::constants::PI; trueVecs.push_back(temp);
  const unsigned numRand = 20;
  for (unsigned i = 0; i < numRand; i++) {
    trueVecs.push_back(Eigen::Matrix<double,6,1>::Random());
  }

  // Get number of tests
  const unsigned numTests = trueVecs.size();

  // Add vectors to be tested - random
  std::vector<Eigen::Matrix<double,4,1> > landmarks;
  for (unsigned i = 0; i < numTests; i++) {
    landmarks.push_back(Eigen::Matrix<double,4,1>::Random());
  }

  // Calc transformation matrices
  std::vector<Eigen::Matrix4d > transMatrices;
  for (unsigned i = 0; i < numTests; i++) {
    transMatrices.push_back(lgmath::se3::vec2tran(trueVecs.at(i)));
  }

  // Calc transformations
  std::vector<lgmath::se3::Transformation > transformations;
  for (unsigned i = 0; i < numTests; i++) {
    transformations.push_back(lgmath::se3::Transformation(trueVecs.at(i)));
  }

  // Compare inverse to basic matrix inverse
  SECTION("compare inverse") {
    for (unsigned i = 0; i < numTests; i++) {
      INFO("matr: " << transMatrices.at(i).inverse());
      INFO("tran: " << transformations.at(i).inverse().matrix());
      CHECK(lgmath::common::nearEqual(transMatrices.at(i).inverse(), transformations.at(i).inverse().matrix(), 1e-6));
    }
  }

  // Test that product of inverse and self make identity
  SECTION("test product of inverse") {
    for (unsigned i = 0; i < numTests; i++) {
      INFO("T*Tinv: " << transformations.at(i).matrix()*transformations.at(i).inverse().matrix());
      CHECK(lgmath::common::nearEqual(transformations.at(i).matrix()*transformations.at(i).inverse().matrix(), Eigen::Matrix4d::Identity(), 1e-6));
    }
  }

  // Test adjoint
  SECTION("test adjoint") {
    for (unsigned i = 0; i < numTests; i++) {
      INFO("matr: " << lgmath::se3::tranAd(transMatrices.at(i)));
      INFO("tran: " << transformations.at(i).adjoint());
      CHECK(lgmath::common::nearEqual(lgmath::se3::tranAd(transMatrices.at(i)), transformations.at(i).adjoint(), 1e-6));
    }
  }

  // Test self-product
  SECTION("test self product") {
    for (unsigned i = 0; i < numTests-1; i++) {
      lgmath::se3::Transformation test = transformations.at(i);
      test *= transformations.at(i+1);
      Eigen::Matrix4d matrix = transMatrices.at(i)*transMatrices.at(i+1);
      INFO("matr: " << matrix);
      INFO("tran: " << test.matrix());
      CHECK(lgmath::common::nearEqual(matrix, test.matrix(), 1e-6));
    }
  }

  // Test product
  SECTION("test product") {
    for (unsigned i = 0; i < numTests-1; i++) {
      lgmath::se3::Transformation test = transformations.at(i)*transformations.at(i+1);
      Eigen::Matrix4d matrix = transMatrices.at(i)*transMatrices.at(i+1);
      INFO("matr: " << matrix);
      INFO("tran: " << test.matrix());
      CHECK(lgmath::common::nearEqual(matrix, test.matrix(), 1e-6));
    }
  }

  // Test self product with inverse
  SECTION("test self product with inverse") {
    for (unsigned i = 0; i < numTests-1; i++) {
      lgmath::se3::Transformation test = transformations.at(i);
      test /= transformations.at(i+1);
      Eigen::Matrix4d matrix = transMatrices.at(i) * transMatrices.at(i+1).inverse();
      INFO("matr: " << matrix);
      INFO("tran: " << test.matrix());
      CHECK(lgmath::common::nearEqual(matrix, test.matrix(), 1e-6));
    }
  }

  // Test product with inverse
  SECTION("test product with inverse") {
    for (unsigned i = 0; i < numTests-1; i++) {
      lgmath::se3::Transformation test = transformations.at(i) / transformations.at(i+1);
      Eigen::Matrix4d matrix = transMatrices.at(i) * transMatrices.at(i+1).inverse();
      INFO("matr: " << matrix);
      INFO("tran: " << test.matrix());
      CHECK(lgmath::common::nearEqual(matrix, test.matrix(), 1e-6));
    }
  }

  // Test product with landmark
  SECTION("test product with landmark") {
    for (unsigned i = 0; i < numTests; i++) {
      Eigen::Matrix<double,4,1> mat = transMatrices.at(i)*landmarks.at(i);
      Eigen::Matrix<double,4,1> test = transformations.at(i)*landmarks.at(i);

      INFO("matr: " << mat);
      INFO("test: " << test);
      CHECK(lgmath::common::nearEqual(mat, test, 1e-6));
    }
  }

} // TEST_CASE

