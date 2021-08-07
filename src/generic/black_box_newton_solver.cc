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


// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

#include "black_box_newton_solver.h"


namespace oomph
{
  //======================================================================
  /// Namespace for black-box FD Newton solver.
  //======================================================================
  namespace BlackBoxFDNewtonSolver
  {
    /// Max. # of Newton iterations
    unsigned Max_iter = 20;

    /// Number of Newton iterations taken in most recent invocation
    unsigned N_iter_taken = 0;

    /// \short Flag to indicate if progress of Newton iteration is to be
    /// documented (defaults to false)
    bool Doc_Progress = false;

    /// FD step
    double FD_step = 1.0e-8;

    /// Tolerance
    double Tol = 1.0e-8;

    /// Use steplength control do make globally convergent (default false)
    bool Use_step_length_control = false;

    /// \short Black-box FD Newton solver:
    /// Calling sequence for residual function is
    /// \code residual_fct(parameters,unknowns,residuals) \endcode
    /// where all arguments are double Vectors.
    /// unknowns.size() = residuals.size()
    void black_box_fd_newton_solve(ResidualFctPt residual_fct,
                                   const Vector<double>& params,
                                   Vector<double>& unknowns,
                                   JacobianFctPt jacobian_fct)
    {
      // Jacobian, current and advanced residual Vectors
      unsigned ndof = unknowns.size();
      DenseDoubleMatrix jacobian(ndof);
      Vector<double> residuals(ndof);
      Vector<double> residuals_pls(ndof);
      Vector<double> dx(ndof);
      Vector<double> gradient(ndof);
      Vector<double> newton_direction(ndof);

      double half_residual_squared = 0.0;
      double max_step = 0.0;

      /// Reset number of Newton iterations taken in most recent invocation
      N_iter_taken = 0;

      // Newton iterations
      for (unsigned iloop = 0; iloop < Max_iter; iloop++)
      {
        // Evaluate current residuals
        residual_fct(params, unknowns, residuals);

        // Get half of squared residual and find maximum step length
        // for step length control
        if (Use_step_length_control)
        {
          half_residual_squared = 0.0;
          double sum = 0.0;
          for (unsigned i = 0; i < ndof; i++)
          {
            sum += unknowns[i] * unknowns[i];
            half_residual_squared += residuals[i] * residuals[i];
          }
          half_residual_squared *= 0.5;
          max_step = 100.0 * std::max(sqrt(sum), double(ndof));
        }


        // Check max. residuals
        double max_res = std::fabs(*std::max_element(
          residuals.begin(), residuals.end(), AbsCmp<double>()));


        // Doc progress?
        if (Doc_Progress)
        {
          oomph_info << "\nNewton iteration iter=" << iloop
                     << "\ni residual[i] unknown[i] " << std::endl;
          for (unsigned i = 0; i < ndof; i++)
          {
            oomph_info << i << " " << residuals[i] << " " << unknowns[i]
                       << std::endl;
          }
        }


        // Converged?
        if (max_res < Tol)
        {
          return;
        }

        // Next iteration...
        N_iter_taken++;

        // ...and how would Sir like his Jacobian?
        if (jacobian_fct == 0)
        {
          // FD loop for Jacobian
          for (unsigned i = 0; i < ndof; i++)
          {
            double backup = unknowns[i];
            unknowns[i] += FD_step;

            // Evaluate advanced residuals
            residual_fct(params, unknowns, residuals_pls);

            // Do FD
            for (unsigned j = 0; j < ndof; j++)
            {
              jacobian(j, i) = (residuals_pls[j] - residuals[j]) / FD_step;
            }

            // Reset fd step
            unknowns[i] = backup;
          }
        }
        // Analytical Jacobian
        else
        {
          jacobian_fct(params, unknowns, jacobian);
        }


        if (Doc_Progress)
        {
          oomph_info << "\n\nJacobian: " << std::endl;
          jacobian.sparse_indexed_output(*(oomph_info.stream_pt()));
          oomph_info << std::endl;
        }

        // Get gradient
        if (Use_step_length_control)
        {
          for (unsigned i = 0; i < ndof; i++)
          {
            double sum = 0.0;
            for (unsigned j = 0; j < ndof; j++)
            {
              sum += jacobian(j, i) * residuals[j];
            }
            gradient[i] = sum;
          }
        }

        // Solve
        jacobian.solve(residuals, newton_direction);

        // Update
        if (Use_step_length_control)
        {
          for (unsigned i = 0; i < ndof; i++)
          {
            newton_direction[i] *= -1.0;
          }
          // Update with steplength control
          Vector<double> unknowns_old(unknowns);
          double half_residual_squared_old = half_residual_squared;
          line_search(unknowns_old,
                      half_residual_squared_old,
                      gradient,
                      residual_fct,
                      params,
                      newton_direction,
                      unknowns,
                      half_residual_squared,
                      max_step);
        }
        else
        {
          // Direct Newton update:
          for (unsigned i = 0; i < ndof; i++)
          {
            unknowns[i] -= newton_direction[i];
          }
        }
      }


      // Failed to converge
      std::ostringstream error_stream;
      error_stream << "Newton solver did not converge in " << Max_iter
                   << " steps " << std::endl;

      throw OomphLibError(
        error_stream.str(), OOMPH_CURRENT_FUNCTION, OOMPH_EXCEPTION_LOCATION);
    }


    //=======================================================================
    /// Line search helper for globally convergent Newton method
    //=======================================================================
    void line_search(const Vector<double>& x_old,
                     const double half_residual_squared_old,
                     const Vector<double>& gradient,
                     ResidualFctPt residual_fct,
                     const Vector<double>& params,
                     Vector<double>& newton_dir,
                     Vector<double>& x,
                     double& half_residual_squared,
                     const double& stpmax)
    {
      const double min_fct_decrease = 1.0e-4;
      double convergence_tol_on_x = 1.0e-16;
      double f_aux = 0.0;
      double lambda_aux = 0.0;
      double proposed_lambda = 0.0;
      unsigned n = x_old.size();
      double sum = 0.0;
      for (unsigned i = 0; i < n; i++)
      {
        sum += newton_dir[i] * newton_dir[i];
      }
      sum = sqrt(sum);
      if (sum > stpmax)
      {
        for (unsigned i = 0; i < n; i++)
        {
          newton_dir[i] *= stpmax / sum;
        }
      }
      double slope = 0.0;
      for (unsigned i = 0; i < n; i++)
      {
        slope += gradient[i] * newton_dir[i];
      }
      if (slope >= 0.0)
      {
        std::ostringstream error_stream;
        error_stream << "Roundoff problem in lnsrch: slope=" << slope << "\n";
        throw OomphLibError(
          error_stream.str(), OOMPH_CURRENT_FUNCTION, OOMPH_EXCEPTION_LOCATION);
      }
      double test = 0.0;
      for (unsigned i = 0; i < n; i++)
      {
        double temp =
          std::fabs(newton_dir[i]) / std::max(std::fabs(x_old[i]), 1.0);
        if (temp > test) test = temp;
      }
      double lambda_min = convergence_tol_on_x / test;
      double lambda = 1.0;
      while (true)
      {
        for (unsigned i = 0; i < n; i++)
        {
          x[i] = x_old[i] + lambda * newton_dir[i];
        }

        // Evaluate current residuals
        Vector<double> residuals(n);
        residual_fct(params, x, residuals);
        half_residual_squared = 0.0;
        for (unsigned i = 0; i < n; i++)
        {
          half_residual_squared += residuals[i] * residuals[i];
        }
        half_residual_squared *= 0.5;

        if (lambda < lambda_min)
        {
          for (unsigned i = 0; i < n; i++) x[i] = x_old[i];

          // Create an Oomph Lib warning
          OomphLibWarning("Warning: Line search converged on x only!",
                          "BlackBoxFDNewtonSolver::line_search()",
                          OOMPH_EXCEPTION_LOCATION);
          return;
        }
        else if (half_residual_squared <=
                 half_residual_squared_old + min_fct_decrease * lambda * slope)
        {
          return;
        }
        else
        {
          if (lambda == 1.0)
          {
            proposed_lambda =
              -slope / (2.0 * (half_residual_squared -
                               half_residual_squared_old - slope));
          }
          else
          {
            double r1 = half_residual_squared - half_residual_squared_old -
                        lambda * slope;
            double r2 = f_aux - half_residual_squared_old - lambda_aux * slope;
            double a_poly =
              (r1 / (lambda * lambda) - r2 / (lambda_aux * lambda_aux)) /
              (lambda - lambda_aux);
            double b_poly = (-lambda_aux * r1 / (lambda * lambda) +
                             lambda * r2 / (lambda_aux * lambda_aux)) /
                            (lambda - lambda_aux);
            if (a_poly == 0.0)
            {
              proposed_lambda = -slope / (2.0 * b_poly);
            }
            else
            {
              double discriminant = b_poly * b_poly - 3.0 * a_poly * slope;
              if (discriminant < 0.0)
              {
                proposed_lambda = 0.5 * lambda;
              }
              else if (b_poly <= 0.0)
              {
                proposed_lambda =
                  (-b_poly + sqrt(discriminant)) / (3.0 * a_poly);
              }
              else
              {
                proposed_lambda = -slope / (b_poly + sqrt(discriminant));
              }
            }
            if (proposed_lambda > 0.5 * lambda)
            {
              proposed_lambda = 0.5 * lambda;
            }
          }
        }
        lambda_aux = lambda;
        f_aux = half_residual_squared;
        lambda = std::max(proposed_lambda, 0.1 * lambda);
      }
    }
  } // namespace BlackBoxFDNewtonSolver


} // namespace oomph
