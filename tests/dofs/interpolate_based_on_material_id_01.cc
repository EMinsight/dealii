// ------------------------------------------------------------------------
//
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2006 - 2021 by the deal.II authors
//
// This file is part of the deal.II library.
//
// Part of the source code is dual licensed under Apache-2.0 WITH
// LLVM-exception OR LGPL-2.1-or-later. Detailed license information
// governing the source code and code contributions can be found in
// LICENSE.md and CONTRIBUTING.md at the top level directory of deal.II.
//
// ------------------------------------------------------------------------



// check VectorTools::interpolate_based_on_material_id

#include <deal.II/base/function.h>
#include <deal.II/base/quadrature_lib.h>

#include <deal.II/dofs/dof_accessor.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_dgq.h>

#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_refinement.h>
#include <deal.II/grid/manifold_lib.h>
#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>

#include <deal.II/lac/vector.h>

#include <deal.II/numerics/vector_tools.h>

#include <vector>

#include "../tests.h"


template <int dim>
class F : public Function<dim>
{
public:
  F(const unsigned int q)
    : q(q)
  {}

  virtual double
  value(const Point<dim> &p, const unsigned int) const
  {
    double v = 0;
    for (unsigned int d = 0; d < dim; ++d)
      for (unsigned int i = 0; i <= q; ++i)
        v += (d + 1) * (i + 1) * std::pow(p[d], 1. * i);
    return v;
  }

private:
  const unsigned int q;
};



template <int dim>
void
test()
{
  Triangulation<dim> triangulation;
  GridGenerator::hyper_cube(triangulation);
  triangulation.refine_global(3);
  std::map<types::material_id, const Function<dim> *> functions;
  for (typename Triangulation<dim>::active_cell_iterator cell =
         triangulation.begin_active();
       cell != triangulation.end();
       ++cell)
    {
      cell->set_material_id(cell->index() % 128);
      if (functions.find(cell->index() % 128) == functions.end())
        functions[cell->index() % 128] =
          new Functions::ConstantFunction<dim>(cell->index() % 128);
    }

  for (unsigned int p = 1; p < 7 - dim; ++p)
    {
      FE_DGQ<dim>     fe(p);
      DoFHandler<dim> dof_handler(triangulation);
      dof_handler.distribute_dofs(fe);

      Vector<double> interpolant(dof_handler.n_dofs());
      VectorTools::interpolate_based_on_material_id(MappingQ<dim>(1),
                                                    dof_handler,
                                                    functions,
                                                    interpolant);
      for (typename DoFHandler<dim>::active_cell_iterator cell =
             dof_handler.begin_active();
           cell != dof_handler.end();
           ++cell)
        {
          Vector<double> values(fe.dofs_per_cell);
          cell->get_dof_values(interpolant, values);
          for (unsigned int i = 0; i < fe.dofs_per_cell; ++i)
            AssertThrow(values[i] == cell->index() % 128, ExcInternalError());
        }
    }

  for (typename std::map<types::material_id, const Function<dim> *>::iterator
         p = functions.begin();
       p != functions.end();
       ++p)
    delete p->second;

  deallog << "OK" << std::endl;
}



int
main()
{
  initlog();
  deallog << std::setprecision(3);

  test<1>();
  test<2>();
  test<3>();
}
