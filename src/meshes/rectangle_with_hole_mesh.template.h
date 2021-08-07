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
#ifndef OOMPH_RECTANGLE_WITH_HOLE_MESH_HEADER
#define OOMPH_RECTANGLE_WITH_HOLE_MESH_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

// OOMPH-LIB headers
#include "../generic/mesh.h"
#include "../generic/quadtree.h"
#include "../generic/quad_mesh.h"
#include "../generic/refineable_quad_mesh.h"
#include "rectangle_with_hole_domain.h"

namespace oomph
{
  //=============================================================
  /// Domain-based mesh for rectangular mesh with circular hole
  //=============================================================
  template<class ELEMENT>
  class RectangleWithHoleMesh : public virtual Mesh
  {
  public:
    /// \short Constructor: Pass pointer to geometric object that
    /// represents the cylinder, the length and height of the domain.
    /// The GeomObject must be parametrised such that
    /// \f$\zeta \in [0,2\pi]\f$ sweeps around the circumference
    /// in anticlockwise direction. Timestepper defaults to Steady
    /// default timestepper.
    RectangleWithHoleMesh(
      GeomObject* cylinder_pt,
      const double& length,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper)
    {
      // Create the domain
      Domain_pt = new RectangleWithHoleDomain(cylinder_pt, length);

      // Initialise the node counter
      unsigned long node_count = 0;

      // Vectors used to get data from domains
      Vector<double> s(2), r(2);

      // Setup temporary storage for the Node
      Vector<Node*> Tmp_node_pt;

      // Now blindly loop over the macro elements and associate and finite
      // element with each
      unsigned nmacro_element = Domain_pt->nmacro_element();
      for (unsigned e = 0; e < nmacro_element; e++)
      {
        // Create the FiniteElement and add to the Element_pt Vector
        Element_pt.push_back(new ELEMENT);

        // Read out the number of linear points in the element
        unsigned np = dynamic_cast<ELEMENT*>(finite_element_pt(e))->nnode_1d();

        // Loop over nodes in the column
        for (unsigned l1 = 0; l1 < np; l1++)
        {
          // Loop over the nodes in the row
          for (unsigned l2 = 0; l2 < np; l2++)
          {
            // Allocate the memory for the node
            Tmp_node_pt.push_back(finite_element_pt(e)->construct_node(
              l1 * np + l2, time_stepper_pt));

            // Read out the position of the node from the macro element
            s[0] = -1.0 + 2.0 * (double)l2 / (double)(np - 1);
            s[1] = -1.0 + 2.0 * (double)l1 / (double)(np - 1);
            Domain_pt->macro_element_pt(e)->macro_map(s, r);

            // Set the position of the node
            Tmp_node_pt[node_count]->x(0) = r[0];
            Tmp_node_pt[node_count]->x(1) = r[1];

            // Increment the node number
            node_count++;
          }
        }
      } // End of loop over macro elements


      // Now the elements have been created, but there will be nodes in
      // common, need to loop over the common edges and sort it, by reassigning
      // pointers and the deleting excess nodes

      // Read out the number of linear points in the element
      unsigned np = dynamic_cast<ELEMENT*>(finite_element_pt(0))->nnode_1d();

      // Edge between Elements 0 and 1
      for (unsigned n = 0; n < np; n++)
      {
        // Set the nodes in element 1 to be the same as in element 0
        finite_element_pt(1)->node_pt(n * np) =
          finite_element_pt(0)->node_pt((np - 1) * np + np - 1 - n);

        // Remove the nodes in element 1 from the temporary node list
        delete Tmp_node_pt[np * np + n * np];
        Tmp_node_pt[np * np + n * np] = 0;
      }

      // Edge between Elements 0 and 3
      for (unsigned n = 0; n < np; n++)
      {
        // Set the nodes in element 3 to be the same as in element 0
        finite_element_pt(3)->node_pt(n * np) =
          finite_element_pt(0)->node_pt(n);

        // Remove the nodes in element 3 from the temporary node list
        delete Tmp_node_pt[3 * np * np + n * np];
        Tmp_node_pt[3 * np * np + n * np] = 0;
      }

      // Edge between Element 1 and 2
      for (unsigned n = 0; n < np; n++)
      {
        // Set the nodes in element 2 to be the same as in element 1
        finite_element_pt(2)->node_pt(np * (np - 1) + n) =
          finite_element_pt(1)->node_pt(np * n + np - 1);

        // Remove the nodes in element 2 from the temporary node list
        delete Tmp_node_pt[2 * np * np + np * (np - 1) + n];
        Tmp_node_pt[2 * np * np + np * (np - 1) + n] = 0;
      }


      // Edge between Element 3 and 2
      for (unsigned n = 0; n < np; n++)
      {
        // Set the nodes in element 2 to be the same as in element 3
        finite_element_pt(2)->node_pt(n) =
          finite_element_pt(3)->node_pt(np * (np - n - 1) + np - 1);

        // Remove the nodes in element 2 from the temporary node list
        delete Tmp_node_pt[2 * np * np + n];
        Tmp_node_pt[2 * np * np + n] = 0;
      }


      // Now set the actual true nodes
      for (unsigned long n = 0; n < node_count; n++)
      {
        if (Tmp_node_pt[n] != 0)
        {
          Node_pt.push_back(Tmp_node_pt[n]);
        }
      }

      // Finally set the nodes on the boundaries
      set_nboundary(5);

      for (unsigned n = 0; n < np; n++)
      {
        // Left hand side
        Node* nod_pt = finite_element_pt(0)->node_pt(n * np);
        convert_to_boundary_node(nod_pt);
        add_boundary_node(3, nod_pt);

        // Right hand side
        nod_pt = finite_element_pt(2)->node_pt(n * np + np - 1);
        convert_to_boundary_node(nod_pt);
        add_boundary_node(1, nod_pt);

        // First part of lower boundary
        nod_pt = finite_element_pt(3)->node_pt(n);
        convert_to_boundary_node(nod_pt);
        add_boundary_node(0, nod_pt);

        // First part of upper boundary
        nod_pt = finite_element_pt(1)->node_pt(np * (np - 1) + n);
        convert_to_boundary_node(nod_pt);
        add_boundary_node(2, nod_pt);

        // First part of hole boundary
        nod_pt = finite_element_pt(3)->node_pt(np * (np - 1) + n);
        convert_to_boundary_node(nod_pt);
        add_boundary_node(4, nod_pt);
      }

      for (unsigned n = 1; n < np; n++)
      {
        // Next part of hole
        Node* nod_pt = finite_element_pt(2)->node_pt(n * np);
        convert_to_boundary_node(nod_pt);
        add_boundary_node(4, nod_pt);
      }

      for (unsigned n = 1; n < np; n++)
      {
        // Next part of hole
        Node* nod_pt = finite_element_pt(1)->node_pt(np - n - 1);
        convert_to_boundary_node(nod_pt);
        add_boundary_node(4, nod_pt);
      }

      for (unsigned n = 1; n < np - 1; n++)
      {
        // Final part of hole
        Node* nod_pt =
          finite_element_pt(0)->node_pt(np * (np - n - 1) + np - 1);
        convert_to_boundary_node(nod_pt);
        add_boundary_node(4, nod_pt);
      }
    }

    /// Access function to the domain
    RectangleWithHoleDomain* domain_pt()
    {
      return Domain_pt;
    }

  protected:
    /// Pointer to the domain
    RectangleWithHoleDomain* Domain_pt;
  };


  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////


  //===================================================================
  /// Refineable version of RectangleWithHoleMesh. For some reason
  /// this needs on uniform refinement to work...
  //===================================================================
  template<class ELEMENT>
  class RefineableRectangleWithHoleMesh : public RectangleWithHoleMesh<ELEMENT>,
                                          public RefineableQuadMesh<ELEMENT>
  {
  public:
    /// \short Constructor. Pass pointer to geometric object that
    /// represents the cylinder, the length and height of the domain.
    /// The GeomObject must be parametrised such that
    /// \f$\zeta \in [0,2\pi]\f$ sweeps around the circumference
    /// in anticlockwise direction. Timestepper defaults to Steady
    /// default timestepper.
    RefineableRectangleWithHoleMesh(
      GeomObject* cylinder_pt,
      const double& length,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper)
      : RectangleWithHoleMesh<ELEMENT>(cylinder_pt, length, time_stepper_pt)
    {
      // Nodal positions etc. were created in constructor for
      // Cylinder...<...>. Need to setup adaptive information.

      // Loop over all elements and set macro element pointer
      for (unsigned e = 0; e < 4; e++)
      {
        dynamic_cast<ELEMENT*>(this->element_pt(e))
          ->set_macro_elem_pt(this->Domain_pt->macro_element_pt(e));
      }

      // Setup boundary element lookup schemes
      this->setup_boundary_element_info();

      // Nodal positions etc. were created in constructor for
      // RectangularMesh<...>. Only need to setup quadtree forest
      this->setup_quadtree_forest();
    }

    /// \short Destructor: Empty
    virtual ~RefineableRectangleWithHoleMesh() {}
  };


} // namespace oomph


#endif
