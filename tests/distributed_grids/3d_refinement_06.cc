// ------------------------------------------------------------------------
//
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2010 - 2020 by the deal.II authors
//
// This file is part of the deal.II library.
//
// Part of the source code is dual licensed under Apache-2.0 WITH
// LLVM-exception OR LGPL-2.1-or-later. Detailed license information
// governing the source code and code contributions can be found in
// LICENSE.md and CONTRIBUTING.md at the top level directory of deal.II.
//
// ------------------------------------------------------------------------



// check if p4est does limit_level_difference_at_vertices in one 3d tree
// and in different trees
// test1 divides the lower-right cell of a square three times
// test2 does the same with a subdivided_hyper_cube
//
// this test is exactly analogous to 2d_refinement_06


#include <deal.II/base/tensor.h>

#include <deal.II/distributed/tria.h>

#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_out.h>
#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>

#include "../tests.h"

#include "coarse_grid_common.h"



template <int dim>
void
test(std::ostream & /*out*/)
{
  {
    parallel::distributed::Triangulation<dim> tr(MPI_COMM_WORLD);

    GridGenerator::hyper_cube(tr);
    tr.begin_active()->set_refine_flag();
    tr.execute_coarsening_and_refinement();
    tr.begin_active()->set_refine_flag();
    tr.execute_coarsening_and_refinement();

    // for better visibility, refine
    // all children of the sole
    // level-1 cell once more. this
    // will introduce more level-1
    // cells as well.
    for (unsigned int c = 0; c < 8; ++c)
      tr.begin(1)->child(c)->set_refine_flag();
    tr.execute_coarsening_and_refinement();

    //    write_vtk(tr, "1");
    deallog << "cells test1: " << tr.n_active_cells() << std::endl;

    Assert(tr.n_active_cells() == 120, ExcInternalError());
  }
  {
    parallel::distributed::Triangulation<dim> tr(MPI_COMM_WORLD);

    GridGenerator::subdivided_hyper_cube(tr, 2);
    tr.begin_active()->set_refine_flag();
    tr.execute_coarsening_and_refinement();
    for (unsigned int c = 0; c < 8; ++c)
      tr.begin(0)->child(c)->set_refine_flag();
    tr.execute_coarsening_and_refinement();

    //    write_vtk(tr, "2");
    deallog << "cells test2: " << tr.n_active_cells() << std::endl;

    Assert(tr.n_active_cells() == 120, ExcInternalError());
  }
}


int
main(int argc, char *argv[])
{
  initlog();
#ifdef DEAL_II_WITH_MPI
  Utilities::MPI::MPI_InitFinalize mpi_initialization(argc, argv, 1);
#else
  (void)argc;
  (void)argv;
#endif

  deallog.push("3d");
  test<3>(deallog.get_file_stream());
  deallog.pop();
}
