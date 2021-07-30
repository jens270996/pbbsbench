// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2011 Guy Blelloch and the PBBS team
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include<iostream>
#include<cstdlib>
#include<math.h>
#include<limits>
#include<cfloat>
#include <algorithm>
#include <type_traits> 
#include "parlay/parallel.h"
#include "parlay/primitives.h"
#include "common/geometry.h"
#include "common/get_time.h" 
// #include "../../octTree/oct_tree.h"

bool report_stats = true;
#include "../include/sfcnn_knng.hpp"
#include "../include/dpoint.hpp"
#include "../KNN/helper.h"

bool check_correctness = false;
int algorithm_version = 0;
int num_threads = parlay::num_workers();

template<int maxK, class vtx, int Dim>
void ANN_(parlay::sequence<vtx*> &v, int k) {
  timer t("ANN", report_stats);
  if (report_stats)
    cout << "num threads = " << num_threads << endl;

  uint n = v.size();
  typedef reviver::dpoint<uint, Dim> Point;       
  Point *P;
  P = (Point*) parlay::p_malloc(n*sizeof(Point)*1);                  
  NN_helper<Dim, vtx> N;

  N.convert(v, n, P);
  t.next("convert to Kumar's types");

  sfcnn_knng<Point, Dim, uint> NN(P, n, k, num_threads); 
  t.next("create nearest neighbor graph");

  if (check_correctness){
    parlay::parallel_for(0, n, [&] (uint i) {
	 if (do_check_correct()){ 
	   N.check_correct(i, NN[i][0], P, n);}});
    t.next("check correctness");
  }
  parlay::p_free(P);
  t.next("delete data");
}

template<int maxK, class vtx>
void ANN(parlay::sequence<vtx*> &v, int k){
  int d = (v[0]->pt.dimension());
  if (d==2) ANN_<maxK, vtx, 2>(v, k);
  else ANN_<maxK, vtx, 3>(v, k);
}
