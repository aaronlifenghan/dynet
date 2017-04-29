#include "dynet/nodes.h"
#include "dynet/dynet.h"
#include "dynet/training.h"
#include "dynet/gpu-ops.h"
#include "dynet/expr.h"
#include "dynet/io.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace dynet;

int main(int argc, char** argv) {
  dynet::initialize(argc, argv);

  // parameters
  const unsigned HIDDEN_SIZE = 8;
  const unsigned ITERATIONS = 200;
  ParameterCollection m;
  SimpleSGDTrainer sgd(m);

  ComputationGraph cg;
  Parameter p_W, p_b, p_V, p_a;

  if (argc == 2) {
    // Load the model and parameters from
    // file if given.
    Pack packer(argv[1]);
    packer.populate(m, "model");
    p_W = packer.load_param(m, "p_W");
    p_b = packer.load_param(m, "p_b");
    p_V = packer.load_param(m, "p_V");
    p_a = packer.load_param(m, "p_a");
  }
  else {
    // Otherwise, just create a new model.
    p_W = m.add_parameters({HIDDEN_SIZE, 2});
    p_b = m.add_parameters({HIDDEN_SIZE});
    p_V = m.add_parameters({1, HIDDEN_SIZE});
    p_a = m.add_parameters({1});
  }

  Expression W = parameter(cg, p_W);
  Expression b = parameter(cg, p_b);
  Expression V = parameter(cg, p_V);
  Expression a = parameter(cg, p_a);

  // set x_values to change the inputs to the network
  Dim x_dim({2}, 4), y_dim({1}, 4);
  cerr << "x_dim=" << x_dim << ", y_dim=" << y_dim << endl;
  vector<dynet::real> x_values = {1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0, -1.0};
  Expression x = input(cg, x_dim, &x_values);
  // set y_values expressing the output
  vector<dynet::real> y_values = {-1.0, 1.0, 1.0, -1.0};
  Expression y = input(cg, y_dim, &y_values);

  Expression h = tanh(W*x + b);
  //Expression h = tanh(affine_transform({b, W, x}));
  //Expression h = softsign(W*x + b);
  Expression y_pred = V*h + a;
  Expression loss = squared_distance(y_pred, y);
  Expression sum_loss = sum_batches(loss);

  // train the parameters
  for (unsigned iter = 0; iter < ITERATIONS; ++iter) {
    float my_loss = as_scalar(cg.forward(sum_loss)) / 4;
    cg.backward(sum_loss);
    sgd.update(0.25);
    sgd.update_epoch();
    cerr << "E = " << my_loss << endl;
  }

  // Output the model and parameter objects
  // to a cout.
  std::remove("xor-batch.model.meta"); std::remove("xor-batch.model");
  Pack packer("xor-batch.model");
  packer.save(m, "model");
  packer.save(p_W, "p_W");
  packer.save(p_b, "p_b");
  packer.save(p_V, "p_V");
  packer.save(p_a, "p_a");
}
