// LIC// ====================================================================
// LIC// This file forms part of oomph-lib, the object-oriented,
// LIC// multi-physics finite-element library, available
// LIC// at http://www.oomph-lib.org.
// LIC//
// LIC// Copyright (C) 2006-2021 Matthias Heil and Andrew Hazel
// LIC//
// LIC// This library is free software; you can redistribute it and/or
// LIC// modify it under the terms of the GNU Lesser General Public
// LIC// License as published by the Free Software Foundation; either
// LIC// version 2.1 of the License, or (at your option) any later version.
// LIC//
// LIC// This library is distributed in the hope that it will be useful,
// LIC// but WITHOUT ANY WARRANTY; without even the implied warranty of
// LIC// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// LIC// Lesser General Public License for more details.
// LIC//
// LIC// You should have received a copy of the GNU Lesser General Public
// LIC// License along with this library; if not, write to the Free Software
// LIC// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// LIC// 02110-1301  USA.
// LIC//
// LIC// The authors may be contacted at oomph-lib@maths.man.ac.uk.
// LIC//
// LIC//====================================================================
// Header file for Tri/Tet linear elasticity elements
#ifndef OOMPH_TLINEAR_ELASTICITY_ELEMENTS_HEADER
#define OOMPH_TLINEAR_ELASTICITY_ELEMENTS_HEADER


// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif


// OOMPH-LIB headers
#include "../generic/nodes.h"
#include "../generic/oomph_utilities.h"
#include "../generic/Telements.h"
#include "linear_elasticity_elements.h"
#include "../generic/error_estimator.h"


namespace oomph
{
  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  // TLinearElasticityElement
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////


  //======================================================================
  /// TLinearElasticityElement<DIM,NNODE_1D> elements are
  /// isoparametric triangular
  /// DIM-dimensional LinearElasticity elements with
  /// NNODE_1D nodal points along each
  /// element edge. Inherits from TElement and LinearElasticityEquations
  //======================================================================
  template<unsigned DIM, unsigned NNODE_1D>
  class TLinearElasticityElement
    : public virtual TElement<DIM, NNODE_1D>,
      public virtual LinearElasticityEquations<DIM>,
      public virtual ElementWithZ2ErrorEstimator
  {
  public:
    ///\short  Constructor: Call constructors for TElement and
    /// LinearElasticity equations
    TLinearElasticityElement()
      : TElement<DIM, NNODE_1D>(), LinearElasticityEquations<DIM>()
    {
    }


    /// Broken copy constructor
    TLinearElasticityElement(
      const TLinearElasticityElement<DIM, NNODE_1D>& dummy)
    {
      BrokenCopy::broken_copy("TLinearElasticityElement");
    }

    /// Broken assignment operator
    // Commented out broken assignment operator because this can lead to a
    // conflict warning when used in the virtual inheritence hierarchy.
    // Essentially the compiler doesn't realise that two separate implementations
    // of the broken function are the same and so, quite rightly, it shouts.
    /*void operator=(const TLinearElasticityElement<DIM,NNODE_1D>&)
     {
      BrokenCopy::broken_assign("TLinearElasticityElement");
      }*/

    /// \short Output function:
    void output(std::ostream& outfile)
    {
      LinearElasticityEquations<DIM>::output(outfile);
    }

    ///  \short Output function:
    void output(std::ostream& outfile, const unsigned& nplot)
    {
      LinearElasticityEquations<DIM>::output(outfile, nplot);
    }


    /// \short C-style output function:
    void output(FILE* file_pt)
    {
      LinearElasticityEquations<DIM>::output(file_pt);
    }

    ///  \short C-style output function:
    void output(FILE* file_pt, const unsigned& n_plot)
    {
      LinearElasticityEquations<DIM>::output(file_pt, n_plot);
    }

    /// \short Number of vertex nodes in the element
    unsigned nvertex_node() const
    {
      return TElement<DIM, NNODE_1D>::nvertex_node();
    }

    /// \short Pointer to the j-th vertex node in the element
    Node* vertex_node_pt(const unsigned& j) const
    {
      return TElement<DIM, NNODE_1D>::vertex_node_pt(j);
    }

    /// \short Order of recovery shape functions for Z2 error estimation:
    /// Same order as shape functions.
    unsigned nrecovery_order()
    {
      return NNODE_1D - 1;
    }

    /// Number of 'flux' terms for Z2 error estimation
    unsigned num_Z2_flux_terms()
    {
      // DIM Diagonal strain rates and DIM*(DIM-1)/2 off diagonal terms
      return (DIM + DIM * (DIM - 1) / 2);
    }

    /// \short Get 'flux' for Z2 error recovery:   Upper triangular entries
    /// in strain tensor.
    void get_Z2_flux(const Vector<double>& s, Vector<double>& flux)
    {
#ifdef PARANOID
      unsigned num_entries = (DIM + ((DIM * DIM) - DIM) / 2);
      if (flux.size() != num_entries)
      {
        std::ostringstream error_message;
        error_message << "The flux vector has the wrong number of entries, "
                      << flux.size() << ", whereas it should be " << num_entries
                      << std::endl;
        throw OomphLibError(error_message.str(),
                            OOMPH_CURRENT_FUNCTION,
                            OOMPH_EXCEPTION_LOCATION);
      }
#endif

      // Get strain matrix
      DenseMatrix<double> strain(DIM);
      this->get_strain(s, strain);

      // Pack into flux Vector
      unsigned icount = 0;

      // Start with diagonal terms
      for (unsigned i = 0; i < DIM; i++)
      {
        flux[icount] = strain(i, i);
        icount++;
      }

      // Off diagonals row by row
      for (unsigned i = 0; i < DIM; i++)
      {
        for (unsigned j = i + 1; j < DIM; j++)
        {
          flux[icount] = strain(i, j);
          icount++;
        }
      }
    }
  };

  //=======================================================================
  /// Face geometry for the TLinearElasticityElement elements: The spatial
  /// dimension of the face elements is one lower than that of the
  /// bulk element but they have the same number of points
  /// along their 1D edges.
  //=======================================================================
  template<unsigned DIM, unsigned NNODE_1D>
  class FaceGeometry<TLinearElasticityElement<DIM, NNODE_1D>>
    : public virtual TElement<DIM - 1, NNODE_1D>
  {
  public:
    /// \short Constructor: Call the constructor for the
    /// appropriate lower-dimensional QElement
    FaceGeometry() : TElement<DIM - 1, NNODE_1D>() {}
  };

  //=======================================================================
  /// Face geometry for the 1D TLinearElasticityElement elements: Point elements
  //=======================================================================
  template<unsigned NNODE_1D>
  class FaceGeometry<TLinearElasticityElement<1, NNODE_1D>>
    : public virtual PointElement
  {
  public:
    /// \short Constructor: Call the constructor for the
    /// appropriate lower-dimensional TElement
    FaceGeometry() : PointElement() {}
  };


} // namespace oomph

#endif
