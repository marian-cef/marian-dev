#pragma once

#include "expression_graph.h"

namespace marian {

/**
 * @brief Namespace for code related to managing models in Marian
 */
namespace models {

/**
 * @brief Constructs an expression graph representing a feed-forward classifier.
 *
 * @param dims number of nodes in each layer of the feed-forward classifier
 *
 * @return a shared pointer to the newly constructed expression graph
 */
ExpressionGraphPtr FeedforwardClassifier(const std::vector<int>& dims) {
  using namespace keywords;
  std::cerr << "Building Multi-layer Feedforward network" << std::endl;
  std::cerr << "\tLayer dimensions:";
  for(auto d : dims)
    std::cerr << " " << d;
  std::cerr << std::endl;
  boost::timer::cpu_timer timer;

  // Construct a shared pointer to an empty expression graph
  ExpressionGraphPtr g(new ExpressionGraph());
  auto x = named(g->input(shape={whatevs, dims.front()}), "x");
  auto y = named(g->input(shape={whatevs, dims.back()}), "y");

  std::vector<Expr> layers, weights, biases;
  for(int i = 0; i < dims.size()-1; ++i) {
    int in = dims[i];
    int out = dims[i+1];

    if(i == 0)
      layers.emplace_back(dropout(x, value=0.2));
    else
      layers.emplace_back(dropout(relu(dot(layers.back(), weights.back()) + biases.back()),
                                  value=0.5));

    weights.emplace_back(
      named(g->param(shape={in, out}, init=uniform()), "W" + std::to_string(i)));
    biases.emplace_back(
      named(g->param(shape={1, out}, init=zeros), "b" + std::to_string(i)));
  }

  auto linear = dot(layers.back(), weights.back()) + biases.back();
  auto scores = named(inference(softmax(linear)), "scores");

  // @TODO: throw exception if more than one final training node
  // and keep track of training nodes, as we need to initialize
  // adjoints correctly.
  auto cost = named(mean(training(cross_entropy(linear, y)), axis=0), "cost");

  std::cerr << "\tTotal time: " << timer.format(5, "%ws") << std::endl;
  return g;
};

}
}
