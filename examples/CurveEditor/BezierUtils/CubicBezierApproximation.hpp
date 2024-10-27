#pragma once

#include "LeastSquareSystem.hpp"
#include <Core/Geometry/Curve2D.hpp>
#include <Core/Utils/Log.hpp>
#include <set>

#include <fstream>

using namespace Ra::Core;
using namespace Ra::Core::Geometry;

class CubicBezierApproximation
{
  public:
    CubicBezierApproximation() {}

    void init( const VectorArray<Curve2D::Vector>& data,
               int nb_min_bz          = 1,
               float epsilon          = 0.1,
               std::ofstream* logfile = nullptr ) {
        if ( logfile != nullptr ) {
            ( *logfile ) << "# Initialising optimization with epsilon " << epsilon << std::endl;
            ( *logfile ) << "eps= " << epsilon << ";" << std::endl;
        }

        m_data          = data;
        m_distThreshold = epsilon;
        m_logfile       = logfile;

        recomputeJunctions( nb_min_bz + 1 );
        if ( !data.empty() ) { m_dim = data[0].rows(); }

        updateParameters();

        m_step = 0;

        m_isInitialized = true;
        m_hasComputed   = false;
    }

    bool compute() {
        using namespace Ra::Core::Utils;

        if ( !m_isInitialized ) {
            LOG( logERROR ) << "CubicBezierApproximation is not initialized";
            return false;
        }

        if ( m_hasComputed ) {
            LOG( logERROR ) << "CubicBezierApproximation has already a solution";
            return false;
        }

        if ( m_data.size() < 4 ) {
            auto okflag = computeDegeneratedSolution();
            if ( !okflag ) { return false; }
            m_hasComputed = true;
            return true;
        }

        ++m_step;
        printPolygonMatlab( m_data, "P_" + std::to_string( m_step ) );

        auto okflag = computeLeastSquareSolution();
        if ( !okflag ) { return false; }

        printPolygonMatlab( m_curSol.getCtrlPoints(), "B_" + std::to_string( m_step ) );

        float err = evaluateSolution();

        if ( err > m_distThreshold ) {
            bool stopFlag { recomputeJunctions( m_bzjunctions.size() + 1 ) };
            if ( !stopFlag ) { return false; }
            return compute();
        }

        m_hasComputed = true;

        return true;
    }

    bool compute( int nbz ) {
        using namespace Ra::Core::Utils;

        if ( !m_isInitialized ) {
            LOG( logERROR ) << "CubicBezierApproximation is not initialized";
            return false;
        }

        if ( m_hasComputed ) {
            LOG( logERROR ) << "CubicBezierApproximation has already a solution";
            return false;
        }

        if ( m_data.size() < 4 ) {
            auto okflag = computeDegeneratedSolution();
            if ( !okflag ) { return false; }
            m_hasComputed = true;
            return true;
        }

        ++m_step;
        printPolygonMatlab( m_data, "P_" + std::to_string( m_step ) );

        auto okflag = computeLeastSquareSolution();
        if ( !okflag ) { return false; }

        printPolygonMatlab( m_curSol.getCtrlPoints(), "B_" + std::to_string( m_step ) );

        if ( m_curSol.getNbBezier() < nbz ) {
            bool stopFlag { recomputeJunctions( m_bzjunctions.size() + 1 ) };

            if ( !stopFlag ) { return false; }

            return compute( nbz );
        }

        m_hasComputed = true;

        return true;
    }

    PiecewiseCubicBezier getSolution() const { return m_curSol; }

    int getNstep() const { return m_step; }

  private:
    bool m_isInitialized { false };
    bool m_hasComputed { false };
    int m_dim { 0 };
    int m_step { 0 };
    float m_distThreshold { 0.f };
    VectorArray<Curve2D::Vector> m_data;
    std::vector<float> m_params;
    std::set<int> m_bzjunctions;
    PiecewiseCubicBezier m_curSol;

    std::ofstream* m_logfile { nullptr };

    void updateParameters() {
        if ( m_bzjunctions.size() == 0 ) return;

        m_params.resize( m_data.size() );

        auto b { m_bzjunctions.cbegin() };
        int d1 { *b };
        int numseg { 0 };
        while ( ( ++b ) != m_bzjunctions.cend() ) {
            int d0 { d1 };
            d1 = *b;

            float totalDist { 0. };
            m_params[d0] = 0;
            for ( int i = d0 + 1; i <= d1; ++i ) {
                float dist { ( m_data[i] - m_data[i - 1] ).norm() };
                m_params[i] = m_params[i - 1] + dist;
                totalDist += dist;
            }

            for ( int i = d0; i <= d1; ++i ) {
                m_params[i] = ( m_params[i] / totalDist ) + numseg;
            }
            ++numseg;
        }
    }

    float errorAt( int i ) { return ( m_data[i] - m_curSol.f( m_params[i] ) ).squaredNorm(); }

    bool recomputeJunctions( int nb_junctions ) {
        m_bzjunctions.clear();

        int a { 0 };
        int b { (int)( m_data.size() - 1 ) };
        float h = ( b - a ) / (float)( nb_junctions - 1 );
        std::vector<int> xs( nb_junctions );
        typename std::vector<int>::iterator it;
        float val;
        for ( it = xs.begin(), val = a; it != xs.end(); ++it, val += h )
            *it = (int)val;

        for ( int x : xs ) {
            m_bzjunctions.insert( x );
        }

        updateParameters();

        return true;
    }

    float evaluateSolution() {
        float errMax { -1 };
        using namespace Ra::Core::Utils;
        //        LOG(logDEBUG) << "Evaluation";
        for ( int i = 0; i < (int)m_data.size(); ++i ) {
            //            LOG(logDEBUG) << i << " : " << errorAt(i);
            errMax = std::max( errMax, errorAt( i ) );
        }
        return errMax;
    }

    void computeConstraints( LeastSquareSystem& lss ) {
        int nbz { (int)( m_bzjunctions.size() ) - 1 };

        auto computePointDistanceConstraint = [nbz]( float u ) {
            std::map<int, float> A_cstr;
            auto locpar = PiecewiseCubicBezier::getLocalParameter( u, nbz );
            auto bcoefs = CubicBezier::bernsteinCoefsAt( locpar.second );
            int bi      = locpar.first;

            for ( int i = 0; i < 4; ++i ) {
                A_cstr[3 * bi + i] = bcoefs[i];
            }

            return A_cstr;
        };

        std::set<int>::const_iterator bzit = m_bzjunctions.cbegin();
        for ( int b = 0; b < nbz; ++b ) {
            int s0 { *bzit };
            int s1 { *( ++bzit ) };
            int nb_pts_in_bz { s1 - s0 + 1 };

            if ( nb_pts_in_bz >= 4 ) {
                // Bezier segment is constrained by at least 4 points => well-posed system
                for ( int s = s0; s <= s1; ++s ) {
                    if ( ( s == s0 ) && ( s0 != 0 ) ) { continue; }
                    // The problem is over (or perfectly) constrained : for each sample in between
                    // the Bezier junction we minimize the distance with the optimized curve at the
                    // associated parameter
                    lss.addConstraint( computePointDistanceConstraint( m_params[s] ), m_data[s] );
                }
            }
            else {
                // Bezier segment is constrained by less than 4 points => degenerated case
                VectorArray<Curve2D::Vector> data( m_data.cbegin() + s0, m_data.cbegin() + s1 + 1 );
                VectorArray<Curve2D::Vector> cpts;

                // The problem is underconstrained : we compute one solution that perfectly fits the
                // data And we constrain the control points of the solution to match this solution
                computeDegeneratedSolution( data, cpts );

                for ( int k = 0; k < 4; ++k ) {
                    if ( ( k == 0 ) && ( s0 != 0 ) ) { continue; }
                    lss.addConstraint( { { 3 * b + k, 1 } }, cpts[k] );
                }
            }
        }
    }

    bool computeLeastSquareSolution() {
        int nbz { (int)( m_bzjunctions.size() ) - 1 };
        int nvar { 3 * nbz + 1 };

        if ( nbz < 1 ) {
            using namespace Ra::Core::Utils;
            LOG( logERROR ) << "Error while approximating stroke : not enough points";
            return false;
        }

        LeastSquareSystem lss( nvar, m_dim );
        computeConstraints( lss );

        Eigen::MatrixXf sol;
        if ( !lss.solve( sol ) ) { return false; }

        VectorArray<Curve2D::Vector> cpts( nvar );
        for ( int i = 0; i < nvar; ++i ) {
            cpts[i] = Curve2D::Vector( sol.row( i ) );
        }

        m_curSol.setCtrlPoints( cpts );

        return true;
    }

    bool computeDegeneratedSolution( const VectorArray<Curve2D::Vector>& data,
                                     VectorArray<Curve2D::Vector>& cpts ) {

        int npts { (int)data.size() };
        if ( ( npts == 0 ) || ( npts >= 4 ) ) { return false; }

        cpts.resize( 4 );
        if ( npts == 1 ) {
            // One point in the stroke : the Bezier curve is degenerated
            cpts[0] = data[0];
            cpts[1] = data[0];
            cpts[2] = data[0];
            cpts[3] = data[0];
        }
        if ( npts == 2 ) {
            // Two points in the stroke : straight line segment
            cpts[0] = data[0];
            cpts[1] = ( 2. / 3. ) * data[0] + ( 1. / 3. ) * data[1];
            cpts[2] = ( 1. / 3. ) * data[0] + ( 2. / 3. ) * data[1];
            cpts[3] = data[1];
        }
        if ( npts == 3 ) {
            // Three points in the stroke : we find the quadratic bezier best fitting solution
            // and raise the degree to 3
            float l0 { ( data[1] - data[0] ).norm() };
            float l1 { ( data[2] - data[1] ).norm() };
            float tau { l0 / ( l0 + l1 ) }; // parameter of the middle point in the curve

            /** === Details of the computation
             * p0,p1,p2 the data to approximate
             * t : parameter for p1 in the fitted bezier
             *
             * best fitting quadratic bezier has control points :
             * { p0, (1/(2t(1-t))*(p1 - (t^2)p2 - ((1-t)^2)p0), p2 }
             *
             * we can raise degree of the bezier by applying :
             * { b0, b0 + (2/3)*(b1-b0), b2 + (2/3)*(b1-b2), b2 }
             * where {b0, b1, b2} are the control points of the quadratic
             *
             * hence the control points of the best fitting cubic bezier
             * { p0, p0 + (2/3)*( (1/(2t(1-t))*(p1 - (t^2)p2 - ((1-t)^2)p0) - p0)
             *     , p2 + (2/3)*( (1/(2t(1-t))*(p1 - (t^2)p2 - ((1-t)^2)p0) - p0), p2}
             * with some simplifications, we get
             * { p0, (1/(3t(1-t))*(p1 - (t^2)p2 + (-1+2t)*(1-t)p0)
             *     , (1/(3t(1-t))*(p1 + t(1-2t)p2 - (1-t)^2)p0), p2}
             *
             **/

            float omt { 1 - tau };
            float fact { 1.f / ( 3 * tau * omt ) };
            cpts[0] = data[0];
            cpts[1] = fact * ( ( -1 + 2 * tau ) * omt * data[0] + data[1] - tau * tau * data[2] );
            cpts[2] = fact * ( -omt * omt * data[0] + data[1] + ( 1 - 2 * tau ) * tau * data[2] );
            cpts[3] = data[2];
        }

        return true;
    }

    bool computeDegeneratedSolution() {
        VectorArray<Curve2D::Vector> cpts;
        if ( !computeDegeneratedSolution( m_data, cpts ) ) { return false; }
        m_curSol.setCtrlPoints( cpts );
        return true;
    }

    void printPolygonMatlab( const VectorArray<Curve2D::Vector>& poly,
                             const std::string& varname ) {
        if ( m_logfile == nullptr ) { return; }
        ( *m_logfile ) << varname << "= [";
        for ( unsigned int i = 0; i < poly.size(); i++ ) {
            ( *m_logfile ) << "[" << poly[i].x() << ";" << poly[i].y() << "] ";
        }
        ( *m_logfile ) << "];" << std::endl;
    }
};
