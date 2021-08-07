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
#ifndef OOMPH_MACROELEMENT_HEADER
#define OOMPH_MACROELEMENT_HEADER


// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

#ifdef OOMPH_HAS_MPI
#include "mpi.h"
#endif

// oomph-lib headers
#include "Vector.h"
#include "oomph_utilities.h"
#include "quadtree.h"
#include "octree.h"

namespace oomph
{
  class Domain;

  //================================================================
  /// Base class for MacroElement s that are used during mesh refinement
  /// in domains with curvlinear and/or time-dependent
  /// boundaries; see the description of the Domain class for more
  /// details.
  ///
  /// A macro element provides a parametrisation of a sub-domain
  /// by providing a mapping between a set of local coordinates
  /// \f$ {\bf S} \f$ and global coordinates \f$ {\bf r} ({\bf S}) \f$ . This
  /// must be implemented in the function
  ///
  /// \code MacroElement::macro_map(...) \endcode
  ///
  /// The time-dependent version of the macro map returns the mapping
  /// from local to global coordinates: \f$ {\bf r} (t,{\bf S}) \f$
  /// where t is the discrete timelevel: t=0: current time; t>0:
  /// previous timestep.
  ///
  /// The MacroElement s establish the current (and previous) domain
  /// shape via member function pointers to the Domain 's
  /// \code Domain::macro_element_boundary(...) \endcode
  /// member function.
  //=================================================================
  class MacroElement
  {
  public:
    /// \short Constructor: Pass pointer to Domain and the number of the
    ///  MacroElement within that Domain.
    MacroElement(Domain* domain_pt, const unsigned& macro_element_number)
      : Domain_pt(domain_pt), Macro_element_number(macro_element_number)
    {
#ifdef LEAK_CHECK
      LeakCheckNames::MacroElement_build += 1;
#endif
    }

    /// Default constructor (empty and broken)
    MacroElement()
    {
      throw OomphLibError("Don't call empty constructor for MacroElement!",
                          OOMPH_CURRENT_FUNCTION,
                          OOMPH_EXCEPTION_LOCATION);
    }


    /// Broken copy constructor
    MacroElement(const MacroElement& dummy)
    {
      BrokenCopy::broken_copy("MacroElement");
    }


    /// Broken assignment operator
    void operator=(const MacroElement&)
    {
      BrokenCopy::broken_assign("MacroElement");
    }


    /// Empty destructor
    virtual ~MacroElement()
    {
#ifdef LEAK_CHECK
      LeakCheckNames::MacroElement_build -= 1;
#endif
    }


    /// \short Plot: x,y (or x,y,z) at current time in tecplot
    /// format
    void output(std::ostream& outfile, const int& nplot)
    {
      unsigned t = 0;
      output(t, outfile, nplot);
    }


    /// \short Plot: x,y (or x,y,z) in tecplot format at time level t
    ///  (t=0: current; t>0: previous)
    virtual void output(const unsigned& t,
                        std::ostream& outfile,
                        const unsigned& nplot) = 0;


    /// The mapping from local to global coordinates at the current time : r(s)
    void macro_map(const Vector<double>& s, Vector<double>& r)
    {
      // Evaluate at current timestep
      unsigned t = 0;
      macro_map(t, s, r);
    }


    ///\short The time-dependent mapping from local to global coordinates:
    ///r(t,s).
    /// t is the discrete timelevel: t=0: current time; t>0: previous timestep.
    virtual void macro_map(const unsigned& t,
                           const Vector<double>& s,
                           Vector<double>& r) = 0;


    /// \short Get global position r(s) at continuous time value, t.
    virtual void macro_map(const double& t,
                           const Vector<double>& s,
                           Vector<double>& r)
    {
      // Create an output stream
      std::ostringstream error_message_stream;

      // Create an error message
      error_message_stream << "The function macro_map(...) is broken virtual\n"
                           << "If you need it, please implement it!"
                           << std::endl;

      // Throw an error
      throw OomphLibError(error_message_stream.str(),
                          OOMPH_CURRENT_FUNCTION,
                          OOMPH_EXCEPTION_LOCATION);
    } // End of macro_map


    /// \short Output all macro element boundaries as tecplot zones
    virtual void output_macro_element_boundaries(std::ostream& outfile,
                                                 const unsigned& nplot) = 0;


    ///\short the jacobian of the mapping from the macro coordinates to the
    ///global
    /// coordinates
    virtual void assemble_macro_to_eulerian_jacobian(
      const unsigned& t, const Vector<double>& s, DenseMatrix<double>& jacobian)
    {
      // error message stream
      std::ostringstream error_message;
      error_message << "assemble_macro_to_eulerian_jacobian(...) not \n"
                    << "implemented for this element\n"
                    << std::endl;
      // throw error
      throw OomphLibError(
        error_message.str(), OOMPH_CURRENT_FUNCTION, OOMPH_EXCEPTION_LOCATION);
    }


    ///\short Assembles the second derivative jacobian of the mapping from the
    /// macro coordinates to the global coordinates
    virtual void assemble_macro_to_eulerian_jacobian2(
      const unsigned& t,
      const Vector<double>& s,
      DenseMatrix<double>& jacobian2)
    {
      // error message stream
      std::ostringstream error_message;
      error_message << "assemble_macro_to_eulerian_jacobian2(...) not \n"
                    << "implemented for this element\n"
                    << std::endl;
      // throw error
      throw OomphLibError(
        error_message.str(), OOMPH_CURRENT_FUNCTION, OOMPH_EXCEPTION_LOCATION);
    }


    ///\short Assembles the jacobian of the mapping from the macro coordinates
    ///to
    /// the global coordinates
    void assemble_macro_to_eulerian_jacobian(const Vector<double>& s,
                                             DenseMatrix<double>& jacobian)
    {
      unsigned t = 0;
      assemble_macro_to_eulerian_jacobian(t, s, jacobian);
    }


    ///\short Assembles the second derivative jacobian of the mapping from the
    /// macro coordinates to the global coordinates
    void assemble_macro_to_eulerian_jacobian2(const Vector<double>& s,
                                              DenseMatrix<double>& jacobian2)
    {
      unsigned t = 0;
      assemble_macro_to_eulerian_jacobian2(t, s, jacobian2);
    }

    ///\short Access function to the Macro_element_number
    unsigned& macro_element_number()
    {
      return Macro_element_number;
    }

    ///\short Access function to the Domain_pt
    Domain*& domain_pt()
    {
      return Domain_pt;
    }

  protected:
    /// Pointer to domain
    Domain* Domain_pt;

    /// What is the number of the current macro element within its domain
    unsigned Macro_element_number;
  };


  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////


  //================================================================
  /// QMacroElement
  ///
  /// QMacroElements are MacroElement s with linear/quadrilateral/hexahedral
  /// shape. This class is empty and simply establishes the dimension
  /// as the template parameter.
  //=================================================================
  template<int DIM>
  class QMacroElement : public MacroElement
  {
  };


  //================================================================
  /// QMacroElement specialised to 2 spatial dimensions.
  ///
  /// The macro element mapping is based on the member function pointer
  /// to  the associated Domain 's
  /// \code Domain::macro_element_boundary(...) \endcode
  /// function which provides a parametrisation of the macro element's
  /// boundaries in the form:
  ///       \f[  {\bf f}_{i} (t,{\bf S}) \f]
  /// where
  /// - \f$ i \f$  labels the boundary (N/S/W/E)
  /// - \f$ {\bf S} \in [-1,1]^1  \f$  is the (1D) Vector of local coordinate(s)
  ///   along the boundary.
  /// - \f$ {\bf f} \f$  is the position Vector to the boundary.
  /// - \f$ t \f$ is the time level (t=0: current; t>0 previous timestep)
  //=================================================================
  template<>
  class QMacroElement<2> : public MacroElement
  {
  public:
    /// \short Constructor: Pass the pointer to the domain and the macro
    /// element's number within this domain
    QMacroElement(Domain* domain_pt, const unsigned& macro_element_number)
      : MacroElement(domain_pt, macro_element_number){};


    /// Default constructor (empty and broken)
    QMacroElement()
    {
      throw OomphLibError("Don't call empty constructor for QMacroElement!",
                          OOMPH_CURRENT_FUNCTION,
                          OOMPH_EXCEPTION_LOCATION);
    }

    /// Broken copy constructor
    QMacroElement(const QMacroElement& dummy)
    {
      BrokenCopy::broken_copy("QMacroElement");
    }

    /// Broken assignment operator
    void operator=(const QMacroElement&)
    {
      BrokenCopy::broken_assign("QMacroElement");
    }

    /// Empty destructor
    virtual ~QMacroElement(){};


    /// \short Plot: x,y in tecplot format at time level t (t=0: current;
    /// t>0: previous)
    void output(const unsigned& t, std::ostream& outfile, const unsigned& nplot)
    {
      Vector<double> x(2), f(2);
      outfile << "ZONE I=" << nplot << ", J=" << nplot << std::endl;
      for (unsigned i = 0; i < nplot; i++)
      {
        x[1] = -1.0 + 2.0 * double(i) / double(nplot - 1);
        for (unsigned j = 0; j < nplot; j++)
        {
          x[0] = -1.0 + 2.0 * double(j) / double(nplot - 1);
          macro_map(t, x, f);
          outfile << f[0] << " " << f[1] << std::endl;
        }
      }
    }


    /// \short Output all macro element boundaries as tecplot zones
    void output_macro_element_boundaries(std::ostream& outfile,
                                         const unsigned& nplot);

    /// \short Get global position r(S) at discrete time level t.
    /// t=0: Present time; t>0: previous timestep.
    void macro_map(const unsigned& t,
                   const Vector<double>& S,
                   Vector<double>& r);


    /// \short Get global position r(s) at continuous time value, t.
    void macro_map(const double& t, const Vector<double>& s, Vector<double>& r);


    ///\short assemble the jacobian of the mapping from the macro coordinates to
    /// the global coordinates
    virtual void assemble_macro_to_eulerian_jacobian(
      const unsigned& t,
      const Vector<double>& s,
      DenseMatrix<double>& jacobian);


    /// \short Assembles the second derivative jacobian of the mapping from the
    /// macro coordinates to global coordinates x
    virtual void assemble_macro_to_eulerian_jacobian2(
      const unsigned& t,
      const Vector<double>& s,
      DenseMatrix<double>& jacobian2);
  };


  //================================================================
  /// QMacroElement specialised to 3 spatial dimensions.
  ///
  /// The macro element mapping is based on the member function pointer
  /// to  the associated Domain 's
  /// \code Domain::macro_element_boundary(...) \endcode
  /// function which provides a parametrisation of the macro element's
  /// boundaries in the form:
  ///       \f[  {\bf f}_{i} (t,{\bf S}) \f]
  /// where
  /// - \f$ i \f$  labels the boundary (L/R/D/U/B/F)
  /// - \f$ {\bf S} \in [-1,1]^2  \f$  is the (2D) Vector of local coordinate(s)
  ///   along the boundary.
  /// - \f$ {\bf f} \f$  is the position Vector to the boundary.
  /// - \f$ t \f$ is the time level (t=0: current; t>0 previous timestep)
  //=================================================================
  template<>
  class QMacroElement<3> : public MacroElement
  {
  public:
    /// \short Constructor: Pass the pointer to the domain and the macro
    /// element's number within this domain
    QMacroElement(Domain* domain_pt, const unsigned& macro_element_number)
      : MacroElement(domain_pt, macro_element_number){};

    /// Default constructor (empty and broken)
    QMacroElement()
    {
      throw OomphLibError("Don't call empty constructor for QMacroElement!",
                          OOMPH_CURRENT_FUNCTION,
                          OOMPH_EXCEPTION_LOCATION);
    }

    /// Broken copy constructor
    QMacroElement(const QMacroElement& dummy)
    {
      BrokenCopy::broken_copy("QMacroElement");
    }

    /// Broken assignment operator
    void operator=(const QMacroElement&)
    {
      BrokenCopy::broken_assign("QMacroElement");
    }


    /// Empty destructor
    virtual ~QMacroElement(){};


    /// \short Plot: x,y in tecplot format at time level t (t=0: current;
    /// t>0: previous)
    void output(const unsigned& t, std::ostream& outfile, const unsigned& nplot)
    {
      Vector<double> x(3), f(3);

      outfile << "ZONE I=" << nplot << ", J=" << nplot << ", k=" << nplot
              << std::endl;
      for (unsigned i = 0; i < nplot; i++)
      {
        x[2] = -1.0 + 2.0 * double(i) / double(nplot - 1);

        for (unsigned j = 0; j < nplot; j++)
        {
          x[1] = -1.0 + 2.0 * double(j) / double(nplot - 1);

          for (unsigned k = 0; k < nplot; k++)
          {
            x[0] = -1.0 + 2.0 * double(k) / double(nplot - 1);

            macro_map(t, x, f);

            outfile << f[0] << " " << f[1] << " " << f[2] << std::endl;
          }
        }
      }
    }


    /// \short Output all macro element boundaries as tecplot zones
    void output_macro_element_boundaries(std::ostream& outfile,
                                         const unsigned& nplot);

    /// \short Get global position r(S) at discrete time level t.
    /// t=0: Present time; t>0: previous timestep.
    void macro_map(const unsigned& t,
                   const Vector<double>& S,
                   Vector<double>& r);
  };

} // namespace oomph

#endif
