#include <Eigen/Core>          // for matrix
#include <Eigen/Dense>         // for matrix
#include <tuple>               // for multiple values return

#include <functional>
#include <vector>
#include <algorithm>

using namespace std;

//Eigen::Array3f (*f_3s_ptr) (Eigen::Array3f, std::vector<Eigen::ArrayXf>); //def f_3s(z, u, vhMdl, trMdl, F_ext, dt):

Eigen::MatrixXf numerical_jac(
    std::function<Eigen::MatrixXf(Eigen::MatrixXf, std::vector<Eigen::MatrixXf>, double)> f,
    Eigen::MatrixXf x,
    std::vector<Eigen::MatrixXf> args,
    double dt)
{
  Eigen::MatrixXf y = f(x, args, dt);  // return [1,3] matrix
  Eigen::MatrixXf xp(x);
  double eps = 1e-5;

  // create zero matrix
  Eigen::MatrixXf jac(y.cols(),x.cols());
  jac.setZero(y.cols(), x.cols());

  Eigen::MatrixXf yhi, ylo;

  for(int i =0; i < x.cols(); i++){
    xp(0,i) = x(0,i) + eps/2.0;
    yhi = f(xp, args, dt);
    xp(0,i) = x(0,i) - eps/2.0;
    ylo = f(xp, args, dt);
    xp(0,i) = x(0,i);
    jac.col(i) = (yhi - ylo) / eps;
  }

  return jac;
}

Eigen::MatrixXf numerical_jac2(
    std::function<Eigen::MatrixXf(Eigen::MatrixXf)> h,
    Eigen::MatrixXf x)
{
  Eigen::MatrixXf y = h(x);  // return [1, 2] matrix
  Eigen::MatrixXf xp(x);
  double eps = 1e-5;

  // create zero matrix
  Eigen::MatrixXf jac(y.cols(), x.cols());  // [2x3]
  jac.setZero(y.cols(), x.cols());

  Eigen::MatrixXf yhi, ylo;

  for(int i =0; i < x.cols(); i++){
    xp(0,i) = x(0,i) + eps/2.0;
    yhi = h(xp);
    xp(0,i) = x(0,i) - eps/2.0;
    ylo = h(xp);
    xp(0,i) = x(0,i);
    jac.col(i) = (yhi - ylo) / eps;
  }

  return jac;  // [2x3]
}



std::tuple<Eigen::MatrixXf, Eigen::MatrixXf> ekf (
    std::function<Eigen::MatrixXf(Eigen::MatrixXf, std::vector<Eigen::MatrixXf>, double)> f,
    Eigen::MatrixXf mx_k,  // z_EKF
    Eigen::MatrixXf P_k,
    std::function<Eigen::MatrixXf(Eigen::MatrixXf)> h,
    Eigen::MatrixXf y_kp1,
    Eigen::MatrixXf Q,
    Eigen::MatrixXf R,
    std::vector<Eigen::MatrixXf> args,
    double dt)
{
  double xDim = mx_k.cols();
  Eigen::MatrixXf mx_kp1 = f(mx_k, args, dt);              // [] matrix
  Eigen::MatrixXf A = numerical_jac(f, mx_k, args, dt);    // [] 
  Eigen::MatrixXf P_kp1 = (A * P_k) * A.transpose() + Q;   // [] 
  Eigen::MatrixXf my_kp1 = h(mx_kp1);                      // [] 
  Eigen::MatrixXf H = numerical_jac2(h, mx_kp1);           // [] 
  Eigen::MatrixXf P12 = P_kp1 * H.transpose();             // [] 
  Eigen::MatrixXf K = P12 * (((H*P12) + R).inverse());     // [] 
  mx_kp1 = mx_kp1 + K * (y_kp1 - my_kp1);                  // [] 

  // create diagonal matrix 6x6
  Eigen::MatrixXf eye6(6,6);
  eye6 << 1,0,0,0,0,0,
          0,1,0,0,0,0,
          0,0,1,0,0,0,
          0,0,0,1,0,0,
          0,0,0,0,1,0,
          0,0,0,0,0,1;

  P_kp1 = (K*R)*K.transpose() + ((eye6 - (K*H)) * P_kp1) * (eye6 - (K*H).transpose());  // []

  return std::make_tuple(mx_kp1, P_kp1);  // [] , []
}


